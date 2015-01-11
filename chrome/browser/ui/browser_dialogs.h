// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_BROWSER_DIALOGS_H_
#define CHROME_BROWSER_UI_BROWSER_DIALOGS_H_

#include "base/callback.h"
#include "chrome/browser/profiles/profile_window.h"
#include "content/public/common/signed_certificate_timestamp_id_and_status.h"
#include "ipc/ipc_message.h"  // For IPC_MESSAGE_LOG_ENABLED.
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/native_widget_types.h"

class Browser;
class Profile;
class SkBitmap;
class TabModalConfirmDialogDelegate;

namespace content {
class BrowserContext;
class ColorChooser;
class WebContents;
}

namespace extensions {
class Extension;
}

namespace ui {
class ProfileSigninConfirmationDelegate;
class WebDialogDelegate;
}

namespace chrome {

// Creates and shows an HTML dialog with the given delegate and context.
// The window is automatically destroyed when it is closed.
// Returns the created window.
//
// Make sure to use the returned window only when you know it is safe
// to do so, i.e. before OnDialogClosed() is called on the delegate.
gfx::NativeWindow ShowWebDialog(gfx::NativeWindow parent,
                                content::BrowserContext* context,
                                ui::WebDialogDelegate* delegate);

// Shows the collected cookies dialog box.
void ShowCollectedCookiesDialog(content::WebContents* web_contents);

// Creates the ExtensionInstalledBubble and schedules it to be shown once
// the extension has loaded. |extension| is the installed extension. |browser|
// is the browser window which will host the bubble. |icon| is the install
// icon of the extension.
void ShowExtensionInstalledBubble(const extensions::Extension* extension,
                                  Browser* browser,
                                  const SkBitmap& icon);

// Shows or hide the hung renderer dialog for the given WebContents.
// We need to pass the WebContents to the dialog, because multiple tabs can hang
// and it needs to keep track of which tabs are currently hung.
void ShowHungRendererDialog(content::WebContents* contents);
void HideHungRendererDialog(content::WebContents* contents);

// Shows or hides the Task Manager. |browser| can be NULL when called from Ash.
void ShowTaskManager(Browser* browser);
void HideTaskManager();

#if !defined(OS_MACOSX)
// Shows the create web app shortcut dialog box.
void ShowCreateWebAppShortcutsDialog(gfx::NativeWindow parent_window,
                                     content::WebContents* web_contents);
#endif

// Shows the create chrome app shortcut dialog box.
// |close_callback| may be null.
void ShowCreateChromeAppShortcutsDialog(
    gfx::NativeWindow parent_window,
    Profile* profile,
    const extensions::Extension* app,
    const base::Callback<void(bool /* created */)>& close_callback);

// Shows a color chooser that reports to the given WebContents.
content::ColorChooser* ShowColorChooser(content::WebContents* web_contents,
                                        SkColor initial_color);

void ShowProfileSigninConfirmationDialog(
    Browser* browser,
    content::WebContents* web_contents,
    Profile* profile,
    const std::string& username,
    ui::ProfileSigninConfirmationDelegate* delegate);

// Shows the Signed Certificate Timestamps viewer, to view the signed
// certificate timestamps in |sct_ids_list|
void ShowSignedCertificateTimestampsViewer(
    content::WebContents* web_contents,
    const content::SignedCertificateTimestampIDStatusList& sct_ids_list);

// Shows the ManagePasswords bubble for a particular |web_contents|.
void ShowManagePasswordsBubble(content::WebContents* web_contents);

// Closes the bubble if it's shown for |web_contents|.
void CloseManagePasswordsBubble(content::WebContents* web_contents);

}  // namespace chrome

#endif  // CHROME_BROWSER_UI_BROWSER_DIALOGS_H_
