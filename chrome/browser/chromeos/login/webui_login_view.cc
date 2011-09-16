// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/webui_login_view.h"

#include "base/i18n/rtl.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/browser/chromeos/accessibility_util.h"
#include "chrome/browser/chromeos/login/proxy_settings_dialog.h"
#include "chrome/browser/chromeos/login/webui_login_display.h"
#include "chrome/browser/chromeos/status/clock_menu_button.h"
#include "chrome/browser/chromeos/status/input_method_menu_button.h"
#include "chrome/browser/chromeos/status/network_menu_button.h"
#include "chrome/browser/chromeos/status/status_area_view.h"
#include "chrome/browser/chromeos/wm_ipc.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/views/dom_view.h"
#include "chrome/common/render_messages.h"
#include "content/browser/renderer_host/render_view_host_observer.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"
#include "views/widget/native_widget_gtk.h"
#include "views/widget/widget.h"

#if defined(TOUCH_UI)
#include "chrome/browser/ui/touch/keyboard/keyboard_manager.h"
#endif

namespace {

const char kViewClassName[] = "browser/chromeos/login/WebUILoginView";

// These strings must be kept in sync with handleAccelerator() in oobe.js.
const char kAccelNameAccessibility[] = "accessibility";
const char kAccelNameEnrollment[] = "enrollment";

// Observes IPC messages from the FrameSniffer and notifies JS if error
// appears.
class SnifferObserver : public RenderViewHostObserver {
 public:
  SnifferObserver(RenderViewHost* host, WebUI* webui)
      : RenderViewHostObserver(host), webui_(webui) {
    DCHECK(webui_);
    Send(new ChromeViewMsg_StartFrameSniffer(routing_id(),
                                             UTF8ToUTF16("gaia-frame")));
  }

  virtual ~SnifferObserver() {}

  // IPC::Channel::Listener implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) {
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(SnifferObserver, message)
      IPC_MESSAGE_HANDLER(ChromeViewHostMsg_FrameLoadingError, OnError)
      IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled;
  }

 private:
  void OnError(int error) {
    base::FundamentalValue error_value(error);
    webui_->CallJavascriptFunction("login.OfflineMessageScreen.onFrameError",
                                   error_value);
  }

  WebUI* webui_;
};

// A View class which places its first child at the right most position.
class RightAlignedView : public views::View {
 public:
  virtual void Layout() OVERRIDE;
  virtual void ChildPreferredSizeChanged(View* child) OVERRIDE;
};

void RightAlignedView::Layout() {
  if (has_children()) {
    views::View* child = child_at(0);
    gfx::Size preferred_size = child->GetPreferredSize();
    child->SetBounds(width() - preferred_size.width(),
                     0, preferred_size.width(), preferred_size.height());
  }
}

void RightAlignedView::ChildPreferredSizeChanged(View* child) {
  Layout();
}

}  // namespace

namespace chromeos {

// static
const int WebUILoginView::kStatusAreaCornerPadding = 5;

// WebUILoginView public: ------------------------------------------------------

WebUILoginView::WebUILoginView()
    : status_area_(NULL),
      webui_login_(NULL),
      status_window_(NULL),
      host_window_frozen_(false),
      status_area_visibility_on_init_(true) {
#if defined(TOUCH_UI)
  // Make sure the singleton KeyboardManager object is created.
  KeyboardManager::GetInstance();
#endif
  accel_map_[views::Accelerator(ui::VKEY_Z, false, true, true)] =
      kAccelNameAccessibility;
  accel_map_[views::Accelerator(ui::VKEY_E, false, true, true)] =
      kAccelNameEnrollment;

  for (AccelMap::iterator i(accel_map_.begin()); i != accel_map_.end(); ++i)
    AddAccelerator(i->first);
}

WebUILoginView::~WebUILoginView() {
  if (status_window_)
    status_window_->CloseNow();
  status_window_ = NULL;
}

void WebUILoginView::Init() {

  webui_login_ = new DOMView();
  AddChildView(webui_login_);
  webui_login_->Init(ProfileManager::GetDefaultProfile(), NULL);
  webui_login_->SetVisible(true);
  webui_login_->tab_contents()->set_delegate(this);

  tab_watcher_.reset(new TabFirstRenderWatcher(webui_login_->tab_contents(),
                                               this));
}

std::string WebUILoginView::GetClassName() const {
  return kViewClassName;
}

bool WebUILoginView::AcceleratorPressed(
    const views::Accelerator& accelerator) {
  AccelMap::const_iterator entry = accel_map_.find(accelerator);
  if (entry == accel_map_.end())
    return false;

  if (!webui_login_)
    return true;

  WebUI* web_ui = webui_login_->tab_contents()->web_ui();
  if (web_ui) {
    base::StringValue accel_name(entry->second);
    web_ui->CallJavascriptFunction("cr.ui.Oobe.handleAccelerator",
                                   accel_name);
  }

  return true;
}

gfx::NativeWindow WebUILoginView::GetNativeWindow() const {
  return GetWidget()->GetNativeWindow();
}

void WebUILoginView::OnWindowCreated() {
  // Freezes host window update until the tab is rendered.
  host_window_frozen_ = static_cast<views::NativeWidgetGtk*>(
      GetWidget()->native_widget())->SuppressFreezeUpdates();
}

void WebUILoginView::UpdateWindowType() {
  std::vector<int> params;
  WmIpc::instance()->SetWindowType(
      GTK_WIDGET(GetNativeWindow()),
      WM_IPC_WINDOW_LOGIN_WEBUI,
      &params);
}

void WebUILoginView::LoadURL(const GURL & url) {
  webui_login_->LoadURL(url);
  webui_login_->RequestFocus();
}

WebUI* WebUILoginView::GetWebUI() {
  return webui_login_->tab_contents()->web_ui();
}

void WebUILoginView::SetStatusAreaEnabled(bool enable) {
  if (status_area_)
    status_area_->MakeButtonsActive(enable);
}

void WebUILoginView::SetStatusAreaVisible(bool visible) {
  if (status_area_)
    status_area_->SetVisible(visible);
  else
    status_area_visibility_on_init_ = visible;
}

// WebUILoginView protected: ---------------------------------------------------

void WebUILoginView::Layout() {
  DCHECK(webui_login_);
  webui_login_->SetBoundsRect(bounds());
}

void WebUILoginView::ChildPreferredSizeChanged(View* child) {
  Layout();
  SchedulePaint();
}

Profile* WebUILoginView::GetProfile() const {
  return NULL;
}

void WebUILoginView::ExecuteBrowserCommand(int id) const {
}

bool WebUILoginView::ShouldOpenButtonOptions(
    const views::View* button_view) const {
  if (button_view == status_area_->network_view())
    return true;

  if (button_view == status_area_->clock_view() ||
      button_view == status_area_->input_method_view())
    return false;

  return true;
}

void WebUILoginView::OpenButtonOptions(const views::View* button_view) {
  if (button_view == status_area_->network_view()) {
    if (proxy_settings_dialog_.get() == NULL) {
      proxy_settings_dialog_.reset(new ProxySettingsDialog(
          this, GetNativeWindow()));
    }
    proxy_settings_dialog_->Show();
  }
}

StatusAreaHost::ScreenMode WebUILoginView::GetScreenMode() const {
  return kWebUILoginMode;
}

StatusAreaHost::TextStyle WebUILoginView::GetTextStyle() const {
  return kGrayPlain;
}

void WebUILoginView::ButtonVisibilityChanged(views::View* button_view) {
  status_area_->ButtonVisibilityChanged(button_view);
}

void WebUILoginView::OnDialogClosed() {
}

void WebUILoginView::OnLocaleChanged() {
  // Proxy settings dialog contains localized strings.
  proxy_settings_dialog_.reset();
  SchedulePaint();
}

void WebUILoginView::OnRenderHostCreated(RenderViewHost* host) {
  new SnifferObserver(host, GetWebUI());
}

void WebUILoginView::OnTabMainFrameLoaded() {
}

void WebUILoginView::OnTabMainFrameFirstRender() {
  InitStatusArea();

  if (host_window_frozen_) {
    host_window_frozen_ = false;

    // Unfreezes the host window since tab is rendereed now.
    views::NativeWidgetGtk::UpdateFreezeUpdatesProperty(
        GetNativeWindow(), false);
  }
}

void WebUILoginView::InitStatusArea() {
  DCHECK(status_area_ == NULL);
  DCHECK(status_window_ == NULL);
  status_area_ = new StatusAreaView(this);
  status_area_->Init();
  status_area_->SetVisible(status_area_visibility_on_init_);

  views::Widget* login_window = WebUILoginDisplay::GetLoginWindow();
  // Width of |status_window| is meant to be large enough.
  // The current value of status_area_->GetPreferredSize().width()
  // will be too small when button status is changed.
  // (e.g. when CapsLock indicator appears)
  gfx::Size widget_size(width()/2,
                        status_area_->GetPreferredSize().height());
  const int widget_x = base::i18n::IsRTL() ?
      kStatusAreaCornerPadding :
      width() - widget_size.width() - kStatusAreaCornerPadding;
  gfx::Rect widget_bounds(widget_x, kStatusAreaCornerPadding,
                          widget_size.width(), widget_size.height());
  // TODO(nkostylev): Make status area in the same window as |webui_login_|
  // once RenderWidgetHostViewViews is ready.
#if defined(TOUCH_UI)
  // TODO(oshima): Window manager doesn't know about touch event, hence can't
  // activate the window. Use POPUP for now. This will be non issue
  // once we move to pure views + in chrome WM.
  views::Widget::InitParams widget_params(
      views::Widget::InitParams::TYPE_POPUP);
#else
  views::Widget::InitParams widget_params(
      views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
#endif
  widget_params.bounds = widget_bounds;
  widget_params.transparent = true;
  widget_params.parent = login_window->GetNativeView();
  status_window_ = new views::Widget;
  status_window_->Init(widget_params);
  chromeos::WmIpc::instance()->SetWindowType(
      status_window_->GetNativeView(),
      chromeos::WM_IPC_WINDOW_CHROME_INFO_BUBBLE,
      NULL);
  views::View* contents_view = new RightAlignedView;
  contents_view->AddChildView(status_area_);
  status_window_->SetContentsView(contents_view);
  status_window_->Show();
}

// WebUILoginView private: -----------------------------------------------------

bool WebUILoginView::HandleContextMenu(const ContextMenuParams& params) {
  // Do not show the context menu.
#ifndef NDEBUG
  return false;
#else
  return true;
#endif
}

bool WebUILoginView::IsPopupOrPanel(const TabContents* source) const {
  return true;
}

bool WebUILoginView::TakeFocus(bool reverse) {
  // Forward the focus back to web contents.
  webui_login_->tab_contents()->FocusThroughTabTraversal(reverse);
  return true;
}

void WebUILoginView::HandleKeyboardEvent(const NativeWebKeyboardEvent& event) {
  unhandled_keyboard_event_handler_.HandleKeyboardEvent(event,
                                                        GetFocusManager());

  // Make sure error bubble is cleared on keyboard event. This is needed
  // when the focus is inside an iframe.
  WebUI* web_ui = GetWebUI();
  if (web_ui)
    web_ui->CallJavascriptFunction("cr.ui.Oobe.clearErrors");
}

}  // namespace chromeos
