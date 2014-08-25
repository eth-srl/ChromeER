// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/extension_action/extension_action_api.h"
#include "chrome/browser/extensions/extension_action.h"
#include "chrome/browser/extensions/extension_action_icon_factory.h"
#include "chrome/browser/extensions/extension_action_manager.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_tab_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/sessions/session_tab_helper.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/location_bar/location_bar.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test_utils.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/extension.h"

using content::WebContents;

namespace extensions {
namespace {

class PageActionApiTest : public ExtensionApiTest {
 protected:
  ExtensionAction* GetPageAction(const Extension& extension) {
    return ExtensionActionManager::Get(browser()->profile())->
        GetPageAction(extension);
  }
};

IN_PROC_BROWSER_TEST_F(PageActionApiTest, Basic) {
  ASSERT_TRUE(test_server()->Start());
  ASSERT_TRUE(RunExtensionTest("page_action/basics")) << message_;
  const Extension* extension = GetSingleLoadedExtension();
  ASSERT_TRUE(extension) << message_;
  {
    // Tell the extension to update the page action state.
    ResultCatcher catcher;
    ui_test_utils::NavigateToURL(browser(),
        GURL(extension->GetResourceURL("update.html")));
    ASSERT_TRUE(catcher.GetNextResult());
  }

  // Test that we received the changes.
  int tab_id = SessionTabHelper::FromWebContents(
      browser()->tab_strip_model()->GetActiveWebContents())->session_id().id();
  ExtensionAction* action = GetPageAction(*extension);
  ASSERT_TRUE(action);
  EXPECT_EQ("Modified", action->GetTitle(tab_id));

  {
    // Simulate the page action being clicked.
    ResultCatcher catcher;
    ExtensionActionAPI::Get(browser()->profile())->ExecuteExtensionAction(
        extension, browser(), true);
    EXPECT_TRUE(catcher.GetNextResult());
  }

  {
    // Tell the extension to update the page action state again.
    ResultCatcher catcher;
    ui_test_utils::NavigateToURL(browser(),
        GURL(extension->GetResourceURL("update2.html")));
    ASSERT_TRUE(catcher.GetNextResult());
  }

  // We should not be creating icons asynchronously, so we don't need an
  // observer.
  ExtensionActionIconFactory icon_factory(profile(), extension, action, NULL);

  // Test that we received the changes.
  tab_id = SessionTabHelper::FromWebContents(
      browser()->tab_strip_model()->GetActiveWebContents())->session_id().id();
  EXPECT_FALSE(icon_factory.GetIcon(tab_id).IsEmpty());
}

// Test that calling chrome.pageAction.setPopup() can enable a popup.
IN_PROC_BROWSER_TEST_F(PageActionApiTest, AddPopup) {
  // Load the extension, which has no default popup.
  ASSERT_TRUE(RunExtensionTest("page_action/add_popup")) << message_;
  const Extension* extension = GetSingleLoadedExtension();
  ASSERT_TRUE(extension) << message_;

  int tab_id = ExtensionTabUtil::GetTabId(
      browser()->tab_strip_model()->GetActiveWebContents());

  ExtensionAction* page_action = GetPageAction(*extension);
  ASSERT_TRUE(page_action)
      << "Page action test extension should have a page action.";

  ASSERT_FALSE(page_action->HasPopup(tab_id));

  // Simulate the page action being clicked.  The resulting event should
  // install a page action popup.
  {
    ResultCatcher catcher;
    ExtensionActionAPI::Get(browser()->profile())->ExecuteExtensionAction(
        extension, browser(), true);
    ASSERT_TRUE(catcher.GetNextResult());
  }

  ASSERT_TRUE(page_action->HasPopup(tab_id))
      << "Clicking on the page action should have caused a popup to be added.";

  ASSERT_STREQ("/a_popup.html",
               page_action->GetPopupUrl(tab_id).path().c_str());

  // Now change the popup from a_popup.html to a_second_popup.html .
  // Load a page which removes the popup using chrome.pageAction.setPopup().
  {
    ResultCatcher catcher;
    ui_test_utils::NavigateToURL(
        browser(),
        GURL(extension->GetResourceURL("change_popup.html")));
    ASSERT_TRUE(catcher.GetNextResult());
  }

  ASSERT_TRUE(page_action->HasPopup(tab_id));
  ASSERT_STREQ("/another_popup.html",
               page_action->GetPopupUrl(tab_id).path().c_str());
}

// Test that calling chrome.pageAction.setPopup() can remove a popup.
IN_PROC_BROWSER_TEST_F(PageActionApiTest, RemovePopup) {
  // Load the extension, which has a page action with a default popup.
  ASSERT_TRUE(RunExtensionTest("page_action/remove_popup")) << message_;
  const Extension* extension = GetSingleLoadedExtension();
  ASSERT_TRUE(extension) << message_;

  int tab_id = ExtensionTabUtil::GetTabId(
      browser()->tab_strip_model()->GetActiveWebContents());

  ExtensionAction* page_action = GetPageAction(*extension);
  ASSERT_TRUE(page_action)
      << "Page action test extension should have a page action.";

  ASSERT_TRUE(page_action->HasPopup(tab_id))
      << "Expect a page action popup before the test removes it.";

  // Load a page which removes the popup using chrome.pageAction.setPopup().
  {
    ResultCatcher catcher;
    ui_test_utils::NavigateToURL(
        browser(),
        GURL(extension->GetResourceURL("remove_popup.html")));
    ASSERT_TRUE(catcher.GetNextResult());
  }

  ASSERT_FALSE(page_action->HasPopup(tab_id))
      << "Page action popup should have been removed.";
}

// Tests popups in page actions.
// Flaky on the trybots. See http://crbug.com/96725.
IN_PROC_BROWSER_TEST_F(PageActionApiTest, DISABLED_ShowPageActionPopup) {
  ASSERT_TRUE(RunExtensionTest("page_action/popup")) << message_;
  const Extension* extension = GetSingleLoadedExtension();
  ASSERT_TRUE(extension) << message_;

  ASSERT_TRUE(WaitForPageActionVisibilityChangeTo(1));

  {
    ResultCatcher catcher;
    LocationBarTesting* location_bar =
        browser()->window()->GetLocationBar()->GetLocationBarForTesting();
    location_bar->TestPageActionPressed(0);
    ASSERT_TRUE(catcher.GetNextResult());
  }
}

// Test http://crbug.com/57333: that two page action extensions using the same
// icon for the page action icon and the extension icon do not crash.
IN_PROC_BROWSER_TEST_F(PageActionApiTest, TestCrash57333) {
  // Load extension A.
  ASSERT_TRUE(LoadExtension(test_data_dir_.AppendASCII("page_action")
                                          .AppendASCII("crash_57333")
                                          .AppendASCII("Extension1")));
  // Load extension B.
  ASSERT_TRUE(LoadExtension(test_data_dir_.AppendASCII("page_action")
                                          .AppendASCII("crash_57333")
                                          .AppendASCII("Extension2")));
}

IN_PROC_BROWSER_TEST_F(PageActionApiTest, Getters) {
  ASSERT_TRUE(RunExtensionTest("page_action/getters")) << message_;
  const Extension* extension = GetSingleLoadedExtension();
  ASSERT_TRUE(extension) << message_;

  ResultCatcher catcher;
  ui_test_utils::NavigateToURL(browser(),
      GURL(extension->GetResourceURL("update.html")));
  ASSERT_TRUE(catcher.GetNextResult());
}

// Verify triggering page action.
IN_PROC_BROWSER_TEST_F(PageActionApiTest, TestTriggerPageAction) {
  ASSERT_TRUE(test_server()->Start());

  ASSERT_TRUE(RunExtensionTest("trigger_actions/page_action")) << message_;
  const Extension* extension = GetSingleLoadedExtension();
  ASSERT_TRUE(extension) << message_;

  // Page action icon is displayed when a tab is created.
  ui_test_utils::NavigateToURL(browser(),
                               test_server()->GetURL("files/simple.html"));
  chrome::NewTab(browser());
  browser()->tab_strip_model()->ActivateTabAt(0, true);

  // Give the extension time to show the page action on the tab.
  WaitForPageActionVisibilityChangeTo(1);

  ExtensionAction* page_action = GetPageAction(*extension);
  ASSERT_TRUE(page_action);

  WebContents* tab =
      browser()->tab_strip_model()->GetActiveWebContents();
  ASSERT_TRUE(tab);

  EXPECT_TRUE(page_action->GetIsVisible(ExtensionTabUtil::GetTabId(tab)));

  {
    // Simulate the page action being clicked.
    ResultCatcher catcher;
    ExtensionActionAPI::Get(browser()->profile())->ExecuteExtensionAction(
        extension, browser(), true);
    EXPECT_TRUE(catcher.GetNextResult());
  }

  // Verify that the browser action turned the background color red.
  const std::string script =
      "window.domAutomationController.send(document.body.style."
      "backgroundColor);";
  std::string result;
  EXPECT_TRUE(content::ExecuteScriptAndExtractString(tab, script, &result));
  EXPECT_EQ(result, "red");
}

}  // namespace
}  // namespace extensions
