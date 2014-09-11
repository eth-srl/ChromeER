// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_WEB_VIEW_CHROME_WEB_VIEW_INTERNAL_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_WEB_VIEW_CHROME_WEB_VIEW_INTERNAL_API_H_

#include "extensions/browser/api/web_view/web_view_internal_api.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/guest_view/web_view/web_view_guest.h"

// WARNING: *WebViewInternal could be loaded in an unblessed context, thus any
// new APIs must extend WebViewInternalExtensionFunction or
// WebViewInternalExecuteCodeFunction which do a process ID check to prevent
// abuse by normal renderer processes.
namespace extensions {

class ChromeWebViewInternalContextMenusCreateFunction
    : public AsyncExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("chromeWebViewInternal.contextMenusCreate",
                             WEBVIEWINTERNAL_CONTEXTMENUSCREATE);
  ChromeWebViewInternalContextMenusCreateFunction() {}

 protected:
  virtual ~ChromeWebViewInternalContextMenusCreateFunction() {}

  // ExtensionFunction implementation.
  virtual bool RunAsync() OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(ChromeWebViewInternalContextMenusCreateFunction);
};

class ChromeWebViewInternalContextMenusUpdateFunction
    : public AsyncExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("chromeWebViewInternal.contextMenusUpdate",
                             WEBVIEWINTERNAL_CONTEXTMENUSUPDATE);
  ChromeWebViewInternalContextMenusUpdateFunction() {}

 protected:
  virtual ~ChromeWebViewInternalContextMenusUpdateFunction() {}

  // ExtensionFunction implementation.
  virtual bool RunAsync() OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(ChromeWebViewInternalContextMenusUpdateFunction);
};

class ChromeWebViewInternalContextMenusRemoveFunction
    : public AsyncExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("chromeWebViewInternal.contextMenusRemove",
                             WEBVIEWINTERNAL_CONTEXTMENUSREMOVE);
  ChromeWebViewInternalContextMenusRemoveFunction() {}

 protected:
  virtual ~ChromeWebViewInternalContextMenusRemoveFunction() {}

  // ExtensionFunction implementation.
  virtual bool RunAsync() OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(ChromeWebViewInternalContextMenusRemoveFunction);
};

class ChromeWebViewInternalContextMenusRemoveAllFunction
    : public AsyncExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("chromeWebViewInternal.contextMenusRemoveAll",
                             WEBVIEWINTERNAL_CONTEXTMENUSREMOVEALL);
  ChromeWebViewInternalContextMenusRemoveAllFunction() {}

 protected:
  virtual ~ChromeWebViewInternalContextMenusRemoveAllFunction() {}

  // ExtensionFunction implementation.
  virtual bool RunAsync() OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(ChromeWebViewInternalContextMenusRemoveAllFunction);
};

class ChromeWebViewInternalClearDataFunction
    : public WebViewInternalExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("chromeWebViewInternal.clearData",
                             WEBVIEWINTERNAL_CLEARDATA);

  ChromeWebViewInternalClearDataFunction();

 protected:
  virtual ~ChromeWebViewInternalClearDataFunction();

 private:
  // WebViewInternalExtensionFunction implementation.
  virtual bool RunAsyncSafe(WebViewGuest* guest) OVERRIDE;

  uint32 GetRemovalMask();
  void ClearDataDone();

  // Removal start time.
  base::Time remove_since_;
  // Removal mask, corresponds to StoragePartition::RemoveDataMask enum.
  uint32 remove_mask_;
  // Tracks any data related or parse errors.
  bool bad_message_;

  DISALLOW_COPY_AND_ASSIGN(ChromeWebViewInternalClearDataFunction);
};

class ChromeWebViewInternalShowContextMenuFunction
    : public WebViewInternalExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("chromeWebViewInternal.showContextMenu",
                             WEBVIEWINTERNAL_SHOWCONTEXTMENU);

  ChromeWebViewInternalShowContextMenuFunction();

 protected:
  virtual ~ChromeWebViewInternalShowContextMenuFunction();

 private:
  // WebViewInternalExtensionFunction implementation.
  virtual bool RunAsyncSafe(WebViewGuest* guest) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(ChromeWebViewInternalShowContextMenuFunction);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_WEB_VIEW_CHROME_WEB_VIEW_INTERNAL_API_H_
