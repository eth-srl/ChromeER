// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_DEVTOOLS_DEVICE_WEBRTC_WEBRTC_DEVICE_PROVIDER_H_
#define CHROME_BROWSER_DEVTOOLS_DEVICE_WEBRTC_WEBRTC_DEVICE_PROVIDER_H_

#include "chrome/browser/devtools/device/android_device_manager.h"
#include "content/public/browser/web_ui_controller.h"

namespace content {
class BrowserContext;
class WebUI;
}

// Provides access to remote DevTools targets over WebRTC data channel and GCD.
class WebRTCDeviceProvider final : public AndroidDeviceManager::DeviceProvider {
 public:
  /**
   * Provides resources for provider's background worker. Background worker
   * is a windowless page that implements most of functionality of the
   * provider. It sandboxes WebRTC connections with remote devices and other
   * provider implementation details.
   */
  class WebUI : public content::WebUIController {
   public:
    explicit WebUI(content::WebUI* web_ui);
    ~WebUI() override;
  };

  explicit WebRTCDeviceProvider(content::BrowserContext* context);

  // AndroidDeviceManager::DeviceProvider implementation.
  void QueryDevices(const SerialsCallback& callback) override;

  void QueryDeviceInfo(const std::string& serial,
                       const DeviceInfoCallback& callback) override;

  void OpenSocket(const std::string& serial,
                  const std::string& socket_name,
                  const SocketCallback& callback) override;

 private:
  ~WebRTCDeviceProvider() override;

  scoped_ptr<content::WebContents> background_worker_;

  DISALLOW_COPY_AND_ASSIGN(WebRTCDeviceProvider);
};

#endif  // CHROME_BROWSER_DEVTOOLS_DEVICE_WEBRTC_WEBRTC_DEVICE_PROVIDER_H_
