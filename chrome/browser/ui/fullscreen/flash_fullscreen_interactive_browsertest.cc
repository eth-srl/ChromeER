// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "base/time/time.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/fullscreen/fullscreen_controller.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/test/base/interactive_test_utils.h"
#include "chrome/test/ppapi/ppapi_test.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/test_utils.h"
#include "third_party/skia/include/core/SkColor.h"

namespace {

#if defined(OS_MACOSX)
const bool kIsMacUI = true;
#else
const bool kIsMacUI = false;
#endif

// Runs the current MessageLoop until |condition| is true or timeout.
bool RunLoopUntil(const base::Callback<bool()>& condition) {
  const base::TimeTicks start_time = base::TimeTicks::Now();
  while (!condition.Run()) {
    const base::TimeTicks current_time = base::TimeTicks::Now();
    if (current_time - start_time > base::TimeDelta::FromSeconds(10)) {
      ADD_FAILURE() << "Condition not met within ten seconds.";
      return false;
    }

    base::MessageLoop::current()->PostDelayedTask(
        FROM_HERE,
        base::MessageLoop::QuitClosure(),
        base::TimeDelta::FromMilliseconds(20));
    content::RunMessageLoop();
  }
  return true;
}

}  // namespace

// A BrowserTest that opens a test page that launches a simulated fullscreen
// Flash plugin.  The plugin responds to mouse clicks and key presses by
// changing color.  Once launched, the browser UI can be tested to confirm the
// desired interactive behaviors.
class FlashFullscreenInteractiveBrowserTest : public OutOfProcessPPAPITest {
 public:
  FlashFullscreenInteractiveBrowserTest() {}
  ~FlashFullscreenInteractiveBrowserTest() override {}

 protected:
  content::WebContents* GetActiveWebContents() const {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }

  // A simple way to convince libcontent and the browser UI that a tab is being
  // screen captured.  During tab capture, Flash fullscreen remains embedded
  // within the tab content area of a non-fullscreened browser window.
  void StartFakingTabCapture() {
    GetActiveWebContents()->IncrementCapturerCount(gfx::Size(360, 240));
  }

  bool LaunchFlashFullscreen() {
    // This navigates to a page that runs the simulated fullscreen Flash
    // plugin.  It will block until the plugin has completed an attempt to enter
    // Flash fullscreen mode.
    OutOfProcessPPAPITest::RunTest("FlashFullscreenForBrowserUI");

    if (::testing::Test::HasFailure()) {
      ADD_FAILURE() << ("Failed to launch simulated fullscreen Flash plugin.  "
                        "Interactive UI testing cannot proceed.");
      return false;
    }

    EXPECT_TRUE(ObserveTabIsInFullscreen(true));

    return !::testing::Test::HasFailure();
  }

  void UseAcceleratorToOpenNewTab() {
    content::WebContents* const old_tab_contents = GetActiveWebContents();
    EXPECT_TRUE(ui_test_utils::SendKeyPressSync(
        browser(), ui::VKEY_T, !kIsMacUI, false, false, kIsMacUI));
    EXPECT_TRUE(RunLoopUntil(base::Bind(
        &FlashFullscreenInteractiveBrowserTest::IsObservingActiveWebContents,
        base::Unretained(this),
        old_tab_contents,
        false)));
  }

  void UseAcceleratorToSwitchToTab(int tab_index) {
    content::WebContents* const old_tab_contents = GetActiveWebContents();
    const ui::KeyboardCode key_code =
        static_cast<ui::KeyboardCode>(ui::VKEY_1 + tab_index);
    EXPECT_TRUE(ui_test_utils::SendKeyPressSync(
        browser(), key_code, !kIsMacUI, false, false, kIsMacUI));
    EXPECT_TRUE(RunLoopUntil(base::Bind(
        &FlashFullscreenInteractiveBrowserTest::IsObservingActiveWebContents,
        base::Unretained(this),
        old_tab_contents,
        false)));
  }

  void PressEscape() {
    EXPECT_TRUE(ui_test_utils::SendKeyPressSync(
        browser(), ui::VKEY_ESCAPE, false, false, false, false));
  }

  void PressSpacebar() {
    EXPECT_TRUE(ui_test_utils::SendKeyPressSync(
        browser(), ui::VKEY_SPACE, false, false, false, false));
  }

  void SpamSpacebar() {
    for (int i = 0; i < 11; ++i)
      PressSpacebar();
  }

  void ClickOnTabContainer() {
    ui_test_utils::ClickOnView(browser(), VIEW_ID_TAB_CONTAINER);
  }

  void ClickOnOmnibox() {
    ui_test_utils::ClickOnView(browser(), VIEW_ID_OMNIBOX);
  }

  bool ObserveTabIsInFullscreen(bool expected_in_fullscreen) const {
    if (!RunLoopUntil(base::Bind(
            &FlashFullscreenInteractiveBrowserTest::IsObservingTabInFullscreen,
            base::Unretained(this),
            GetActiveWebContents(),
            expected_in_fullscreen)))
      return false;

    if (expected_in_fullscreen) {
      if (!GetActiveWebContents()->GetFullscreenRenderWidgetHostView()) {
        ADD_FAILURE()
            << "WebContents should have a fullscreen RenderWidgetHostView.";
        return false;
      }
      EXPECT_EQ(GetActiveWebContents()->GetCapturerCount() > 0,
                !browser()->fullscreen_controller()->
                     IsWindowFullscreenForTabOrPending());
    }

    return true;
  }

  bool ObserveFlashHasFocus(content::WebContents* contents,
                            bool expected_to_have_focus) const {
    if (!RunLoopUntil(base::Bind(
            &FlashFullscreenInteractiveBrowserTest::IsObservingFlashHasFocus,
            base::Unretained(this),
            contents,
            expected_to_have_focus)))
      return false;

    if (expected_to_have_focus) {
      content::RenderWidgetHostView* const web_page_view =
          contents->GetRenderWidgetHostView();
      EXPECT_FALSE(web_page_view && web_page_view->HasFocus())
          << "Both RenderWidgetHostViews cannot have focus at the same time.";

      if (contents == GetActiveWebContents())
        EXPECT_TRUE(ui_test_utils::IsViewFocused(browser(),
                                                 VIEW_ID_TAB_CONTAINER));
    }

    return true;
  }

  bool ObserveFlashFillColor(SkColor expected_color) const {
    return RunLoopUntil(base::Bind(
        &FlashFullscreenInteractiveBrowserTest::IsObservingFlashFillColor,
        base::Unretained(this),
        expected_color));
  }

 private:
  bool IsObservingTabInFullscreen(content::WebContents* contents,
                                  bool expected_in_fullscreen) const {
    return expected_in_fullscreen == browser()->fullscreen_controller()->
                                         IsFullscreenForTabOrPending(contents);
  }

  bool IsObservingFlashHasFocus(content::WebContents* contents,
                                bool expected_to_have_focus) const {
    content::RenderWidgetHostView* const flash_fs_view =
        contents->GetFullscreenRenderWidgetHostView();
    const bool flash_has_focus = flash_fs_view && flash_fs_view->HasFocus();
    return flash_has_focus == expected_to_have_focus;
  }

  bool IsObservingActiveWebContents(content::WebContents* contents,
                                    bool expected_active_contents) const {
    return (contents == GetActiveWebContents()) == expected_active_contents;
  }

  bool IsObservingFlashFillColor(SkColor expected_color) const {
    content::RenderWidgetHostView* const flash_fs_view =
        GetActiveWebContents()->GetFullscreenRenderWidgetHostView();
    content::RenderWidgetHost* const flash_fs_host =
        flash_fs_view ? flash_fs_view->GetRenderWidgetHost() : nullptr;
    if (!flash_fs_host) {
      ADD_FAILURE() << "Flash fullscreen RenderWidgetHost is gone.";
      return false;
    }

    // When a widget is first shown, it can take some time before it is ready
    // for copying from its backing store.  This is a transient condition, and
    // so it is not being treated as a test failure.
    if (!flash_fs_host->CanCopyFromBackingStore())
      return false;

    // Copy and examine the upper-left pixel of the widget and compare it to the
    // |expected_color|.
    bool is_expected_color = false;
    flash_fs_host->CopyFromBackingStore(
        gfx::Rect(0, 0, 1, 1),
        gfx::Size(1, 1),
        base::Bind(
            &FlashFullscreenInteractiveBrowserTest::CheckBitmapForFillColor,
            expected_color,
            &is_expected_color,
            base::MessageLoop::QuitClosure()),
        kN32_SkColorType);
    content::RunMessageLoop();

    return is_expected_color;
  }

  static void CheckBitmapForFillColor(SkColor expected_color,
                                      bool* is_expected_color,
                                      const base::Closure& done_cb,
                                      const SkBitmap& bitmap,
                                      content::ReadbackResponse response) {
    if (response == content::READBACK_SUCCESS) {
      SkAutoLockPixels lock_pixels(bitmap);
      if (bitmap.width() > 0 && bitmap.height() > 0)
        *is_expected_color = (bitmap.getColor(0, 0) == expected_color);
    }
    done_cb.Run();
  }

  DISALLOW_COPY_AND_ASSIGN(FlashFullscreenInteractiveBrowserTest);
};

// Tests that launching and exiting fullscreen-within-tab works.
IN_PROC_BROWSER_TEST_F(FlashFullscreenInteractiveBrowserTest,
                       FullscreenWithinTab_EscapeKeyExitsFullscreen) {
  ASSERT_TRUE(ui_test_utils::BringBrowserWindowToFront(browser()));
  StartFakingTabCapture();
  ASSERT_TRUE(LaunchFlashFullscreen());
  content::WebContents* const first_tab_contents = GetActiveWebContents();
  EXPECT_TRUE(ObserveFlashHasFocus(first_tab_contents, true));
  PressEscape();
  EXPECT_TRUE(ObserveTabIsInFullscreen(false));
}

// This tests that browser UI focus behavior is correct when switching between
// tabs; particularly, that that focus between the omnibox and tab contents is
// stored/restored correctly.  Mouse and keyboard events are used to confirm
// that the widget the UI thinks is focused is the one that responds to these
// input events.
IN_PROC_BROWSER_TEST_F(FlashFullscreenInteractiveBrowserTest,
                       FullscreenWithinTab_FocusWhenSwitchingTabs) {
  ASSERT_TRUE(ui_test_utils::BringBrowserWindowToFront(browser()));
  StartFakingTabCapture();
  ASSERT_TRUE(LaunchFlashFullscreen());

  // Upon entering fullscreen, the Flash widget should have focus and be filled
  // with green.
  content::WebContents* const first_tab_contents = GetActiveWebContents();
  EXPECT_TRUE(ObserveFlashHasFocus(first_tab_contents, true));
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorGREEN));

  // Pressing the spacebar on the keyboard should change the fill color to red
  // to indicate the plugin truly does have the keyboard focus.  Clicking on the
  // view should change the fill color to blue.
  PressSpacebar();
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorRED));
  ClickOnTabContainer();
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorBLUE));

  // Launch a new tab.  The Flash widget should have lost focus.
  UseAcceleratorToOpenNewTab();
  content::WebContents* const second_tab_contents = GetActiveWebContents();
  ASSERT_NE(first_tab_contents, second_tab_contents);
  EXPECT_TRUE(ObserveFlashHasFocus(first_tab_contents, false));
  ClickOnOmnibox();
  EXPECT_TRUE(ObserveFlashHasFocus(first_tab_contents, false));
  SpamSpacebar();

  // Switch back to first tab.  The plugin should not have responded to the key
  // presses above (while the omnibox was focused), and should regain focus only
  // now.  Poke it with key and mouse events to confirm.
  UseAcceleratorToSwitchToTab(0);
  EXPECT_TRUE(ObserveFlashHasFocus(first_tab_contents, true));
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorBLUE));
  PressSpacebar();
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorRED));
  ClickOnTabContainer();
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorBLUE));

  // Click on the omnibox while still in the first tab, and the Flash widget
  // should lose focus.  Key presses should not affect the color of the Flash
  // widget.
  ClickOnOmnibox();
  EXPECT_TRUE(ObserveFlashHasFocus(first_tab_contents, false));
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorBLUE));
  SpamSpacebar();
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorBLUE));

  // Switch to the second tab, click on the web page content, and then go back
  // to the first tab.  Focus should have been restored to the omnibox when
  // going back to the first tab, and so key presses should not change the color
  // of the Flash widget.
  UseAcceleratorToSwitchToTab(1);
  EXPECT_TRUE(ObserveFlashHasFocus(first_tab_contents, false));
  ClickOnTabContainer();
  EXPECT_TRUE(ObserveFlashHasFocus(first_tab_contents, false));
  UseAcceleratorToSwitchToTab(0);
  EXPECT_TRUE(ObserveFlashHasFocus(first_tab_contents, false));
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorBLUE));
  SpamSpacebar();
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorBLUE));

  // Clicking on the Flash widget should give it focus again.
  ClickOnTabContainer();
  EXPECT_TRUE(ObserveFlashHasFocus(first_tab_contents, true));
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorRED));
  PressSpacebar();
  ASSERT_TRUE(ObserveFlashFillColor(SK_ColorBLUE));

  // Test that the Escape key is handled as an exit fullscreen command while the
  // Flash widget has the focus.
  EXPECT_TRUE(ObserveFlashHasFocus(first_tab_contents, true));
  PressEscape();
  EXPECT_TRUE(ObserveTabIsInFullscreen(false));
}
