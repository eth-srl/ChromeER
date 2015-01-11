// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_EXTENSIONS_EXTENSION_ACTION_PLATFORM_DELEGATE_H_
#define CHROME_BROWSER_UI_EXTENSIONS_EXTENSION_ACTION_PLATFORM_DELEGATE_H_

#include "chrome/browser/ui/extensions/extension_action_view_controller.h"

class GURL;

class ExtensionActionPlatformDelegate {
 public:
  virtual ~ExtensionActionPlatformDelegate() {}

  // Returns a created ExtensionActionPlatformDelegate. This is defined in the
  // platform-specific implementation for the class.
  static scoped_ptr<ExtensionActionPlatformDelegate> Create(
      ExtensionActionViewController* controller);

  // The following are forwarded from ToolbarActionViewController. See that
  // class for the definitions.
  virtual gfx::NativeView GetPopupNativeView() = 0;
  virtual bool IsMenuRunning() const = 0;
  virtual void RegisterCommand() = 0;

  // Called once the delegate is set, in order to do any extra initialization.
  virtual void OnDelegateSet() = 0;

  // Returns true if there is currently a popup for this extension action.
  virtual bool IsShowingPopup() const = 0;

  // Closes the active popup (whether it was this action's popup or not).
  virtual void CloseActivePopup() = 0;

  // Closes this action's popup. This will only be called if the popup is
  // showing.
  virtual void CloseOwnPopup() = 0;

  // Shows the popup for the extension action, given the associated |popup_url|.
  // |grant_tab_permissions| is true if active tab permissions should be given
  // to the extension; this is only true if the popup is opened through a user
  // action.
  // Returns true if a popup is successfully shown.
  virtual bool ShowPopupWithUrl(
      ExtensionActionViewController::PopupShowAction show_action,
      const GURL& popup_url,
      bool grant_tab_permissions) = 0;
};

#endif  // CHROME_BROWSER_UI_EXTENSIONS_EXTENSION_ACTION_PLATFORM_DELEGATE_H_
