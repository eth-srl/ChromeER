// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "athena/system/public/system_ui.h"

#include "athena/common/container_priorities.h"
#include "athena/common/fill_layout_manager.h"
#include "athena/screen/public/screen_manager.h"
#include "athena/system/background_controller.h"
#include "athena/system/device_socket_listener.h"
#include "athena/system/orientation_controller.h"
#include "athena/system/power_button_controller.h"
#include "athena/system/status_icon_container_view.h"
#include "athena/system/time_view.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "ui/aura/window.h"

namespace athena {
namespace {

SystemUI* instance = NULL;

class SystemUIImpl : public SystemUI {
 public:
  SystemUIImpl(scoped_refptr<base::TaskRunner> file_task_runner)
      : orientation_controller_(new OrientationController()),
        power_button_controller_(new PowerButtonController),
        background_container_(NULL),
        system_modal_container_(NULL) {
    orientation_controller_->InitWith(file_task_runner);
  }

  virtual ~SystemUIImpl() {
    // Stops file watching now if exists. Waiting until message loop shutdon
    // leads to FilePathWatcher crash.
    orientation_controller_->Shutdown();
  }

  void Init() {
    background_container_ = ScreenManager::Get()->CreateContainer(
        ScreenManager::ContainerParams("AthenaBackground", CP_BACKGROUND));
    background_container_->SetLayoutManager(
        new FillLayoutManager(background_container_));
    ScreenManager::ContainerParams system_modal_params(
        "AthenaSystemModalContainer", CP_SYSTEM_MODAL);
    system_modal_params.can_activate_children = true;
    system_modal_container_ =
        ScreenManager::Get()->CreateContainer(system_modal_params);

    background_controller_.reset(
        new BackgroundController(background_container_));
  }

  virtual void SetBackgroundImage(const gfx::ImageSkia& image) OVERRIDE {
    background_controller_->SetImage(image);
  }

  virtual views::View* CreateTimeView() OVERRIDE {
    return new TimeView;
  }

  virtual views::View* CreateStatusIconView() OVERRIDE {
    return new StatusIconContainerView(system_modal_container_);
  }

 private:
  scoped_refptr<OrientationController> orientation_controller_;
  scoped_ptr<PowerButtonController> power_button_controller_;
  scoped_ptr<BackgroundController> background_controller_;

  // The parent container for the background.
  aura::Window* background_container_;

  // The parent container used by the "select network" dialog.
  aura::Window* system_modal_container_;

  DISALLOW_COPY_AND_ASSIGN(SystemUIImpl);
};

}  // namespace

// static
SystemUI* SystemUI::Create(scoped_refptr<base::TaskRunner> file_task_runner) {
  DeviceSocketListener::CreateSocketManager(file_task_runner);
  SystemUIImpl* system_ui = new SystemUIImpl(file_task_runner);
  instance = system_ui;
  system_ui->Init();
  return instance;
}

// static
SystemUI* SystemUI::Get() {
  DCHECK(instance);
  return instance;
}

// static
void SystemUI::Shutdown() {
  CHECK(instance);
  delete instance;
  instance = NULL;
  DeviceSocketListener::ShutdownSocketManager();
}

}  // namespace athena
