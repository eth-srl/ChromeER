// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "athena/content/content_activity_factory.h"

#include "athena/content/shell/shell_app_activity.h"

namespace athena {

Activity* ContentActivityFactory::CreateAppActivity(
    extensions::ShellAppWindow* app_window) {
  return new ShellAppActivity(app_window);
}

Activity* ContentActivityFactory::CreateAppActivity(
    apps::AppWindow* app_window) {
  return NULL;
}

}  // namespace athena
