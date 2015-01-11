// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/signin_screen_controller.h"

#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/chromeos/login/lock/screen_locker.h"
#include "chrome/browser/chromeos/login/screens/chrome_user_selection_screen.h"
#include "chrome/browser/ui/webui/chromeos/login/signin_screen_handler.h"

namespace chromeos {

SignInScreenController* SignInScreenController::instance_ = nullptr;

SignInScreenController::SignInScreenController(
    OobeDisplay* oobe_display,
    LoginDisplay::Delegate* login_display_delegate)
    : oobe_display_(oobe_display),
      login_display_delegate_(login_display_delegate),
      webui_handler_(NULL),
      gaia_screen_(new GaiaScreen()),
      user_selection_screen_(new ChromeUserSelectionScreen()) {
  DCHECK(instance_ == NULL);
  instance_ = this;

  gaia_screen_->SetScreenHandler(oobe_display_->GetGaiaScreenActor());
  user_selection_screen_->SetLoginDisplayDelegate(login_display_delegate);

  registrar_.Add(this, chrome::NOTIFICATION_LOGIN_USER_IMAGE_CHANGED,
                 content::NotificationService::AllSources());
  registrar_.Add(this, chrome::NOTIFICATION_SESSION_STARTED,
                 content::NotificationService::AllSources());
}

SignInScreenController::~SignInScreenController() {
  instance_ = nullptr;
}

void SignInScreenController::Init(const user_manager::UserList& users,
                                  bool show_guest) {
  // TODO(antrim) : This dependency should be inverted, screen should ask about
  // users.
  user_selection_screen_->Init(users, show_guest);
}

void SignInScreenController::OnSigninScreenReady() {
  gaia_screen_->MaybePreloadAuthExtension();
  user_selection_screen_->InitEasyUnlock();
  if (ScreenLocker::default_screen_locker()) {
    ScreenLocker::default_screen_locker()->delegate()->OnLockWebUIReady();
  }
}

void SignInScreenController::RemoveUser(const std::string& user_id) {
  user_manager::UserManager::Get()->RemoveUser(user_id, this);
}

void SignInScreenController::OnBeforeUserRemoved(const std::string& username) {
  user_selection_screen_->OnBeforeUserRemoved(username);
}

void SignInScreenController::OnUserRemoved(const std::string& username) {
  user_selection_screen_->OnUserRemoved(username);
}

void SignInScreenController::SendUserList() {
  user_selection_screen_->HandleGetUsers();
}

const user_manager::UserList& SignInScreenController::GetUsers() {
  return user_selection_screen_->GetUsers();
}

void SignInScreenController::SetWebUIHandler(
    LoginDisplayWebUIHandler* webui_handler) {
  webui_handler_ = webui_handler;
  gaia_screen_->SetLegacyHandler(webui_handler_);
  user_selection_screen_->SetHandler(webui_handler_);
}

void SignInScreenController::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  if (type == chrome::NOTIFICATION_SESSION_STARTED) {
    // Stop listening to any notification once session has started.
    // Sign in screen objects are marked for deletion with DeleteSoon so
    // make sure no object would be used after session has started.
    // http://crbug.com/125276
    registrar_.RemoveAll();
    return;
  } else if (type == chrome::NOTIFICATION_LOGIN_USER_IMAGE_CHANGED) {
    user_selection_screen_->OnUserImageChanged(
        *content::Details<user_manager::User>(details).ptr());
    return;
  }
}

}  // namespace chromeos
