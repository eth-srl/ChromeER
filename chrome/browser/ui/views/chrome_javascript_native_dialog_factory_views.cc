// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/app_modal/chrome_javascript_native_dialog_factory.h"

#include "components/app_modal/javascript_dialog_manager.h"
#include "components/app_modal/javascript_native_dialog_factory.h"
#include "components/constrained_window/constrained_window_views.h"

#if defined(USE_X11) && !defined(OS_CHROMEOS)
#include "chrome/browser/ui/views/javascript_app_modal_dialog_views_x11.h"
#else
#include "chrome/browser/ui/blocked_content/app_modal_dialog_helper.h"
#include "components/app_modal/javascript_app_modal_dialog.h"
#include "components/app_modal/views/javascript_app_modal_dialog_views.h"
#endif

namespace {

#if !defined(USE_X11) || defined(OS_CHROMEOS)
class ChromeJavaScriptAppModalDialogViews
    : public app_modal::JavaScriptAppModalDialogViews {
 public:
  explicit ChromeJavaScriptAppModalDialogViews(
      app_modal::JavaScriptAppModalDialog* parent)
      : app_modal::JavaScriptAppModalDialogViews(parent),
        helper_(new AppModalDialogHelper(parent->web_contents())) {}
  ~ChromeJavaScriptAppModalDialogViews() override {}

 private:
  scoped_ptr<AppModalDialogHelper> helper_;

  DISALLOW_COPY_AND_ASSIGN(ChromeJavaScriptAppModalDialogViews);
};
#endif

class ChromeJavaScriptNativeDialogViewsFactory
    : public app_modal::JavaScriptNativeDialogFactory {
 public:
  ChromeJavaScriptNativeDialogViewsFactory() {}
  ~ChromeJavaScriptNativeDialogViewsFactory() override {}

 private:
  app_modal::NativeAppModalDialog* CreateNativeJavaScriptDialog(
      app_modal::JavaScriptAppModalDialog* dialog,
      gfx::NativeWindow parent_window) override{
    app_modal::JavaScriptAppModalDialogViews* d = nullptr;
#if defined(USE_X11) && !defined(OS_CHROMEOS)
    d = new JavaScriptAppModalDialogViewsX11(dialog);
#else
    d = new ChromeJavaScriptAppModalDialogViews(dialog);
#endif
    constrained_window::CreateBrowserModalDialogViews(d, parent_window);
    return d;
  }

  DISALLOW_COPY_AND_ASSIGN(ChromeJavaScriptNativeDialogViewsFactory);
};

}  // namespace

void InstallChromeJavaScriptNativeDialogFactory() {
  app_modal::JavaScriptDialogManager::GetInstance()->
      SetNativeDialogFactory(
          make_scoped_ptr(new ChromeJavaScriptNativeDialogViewsFactory));
}
