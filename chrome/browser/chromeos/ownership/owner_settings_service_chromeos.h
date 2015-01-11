// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_OWNERSHIP_OWNER_SETTINGS_SERVICE_CHROMEOS_H_
#define CHROME_BROWSER_CHROMEOS_OWNERSHIP_OWNER_SETTINGS_SERVICE_CHROMEOS_H_

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/containers/scoped_ptr_hash_map.h"
#include "base/macros.h"
#include "base/values.h"
#include "chrome/browser/chromeos/policy/proto/chrome_device_policy.pb.h"
#include "chrome/browser/chromeos/settings/device_settings_service.h"
#include "chromeos/dbus/session_manager_client.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/ownership/owner_key_util.h"
#include "components/ownership/owner_settings_service.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

class Profile;

namespace ownership {
class OwnerKeyUtil;
}

namespace chromeos {

// The class is a profile-keyed service which holds public/private
// keypair corresponds to a profile. The keypair is reloaded automatically when
// profile is created and TPM token is ready. Note that the private part of a
// key can be loaded only for the owner.
//
// TODO (ygorshenin@): move write path for device settings here
// (crbug.com/230018).
class OwnerSettingsServiceChromeOS : public ownership::OwnerSettingsService,
                                     public content::NotificationObserver,
                                     public SessionManagerClient::Observer,
                                     public DeviceSettingsService::Observer {
 public:
  virtual ~OwnerSettingsServiceChromeOS();

  void OnTPMTokenReady(bool tpm_token_enabled);

  // ownership::OwnerSettingsService implementation:
  virtual bool HandlesSetting(const std::string& setting) override;
  virtual bool Set(const std::string& setting,
                   const base::Value& value) override;
  virtual bool CommitTentativeDeviceSettings(
      scoped_ptr<enterprise_management::PolicyData> policy) override;

  // NotificationObserver implementation:
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) override;

  // SessionManagerClient::Observer:
  virtual void OwnerKeySet(bool success) override;

  // DeviceSettingsService::Observer:
  virtual void OwnershipStatusChanged() override;
  virtual void DeviceSettingsUpdated() override;
  virtual void OnDeviceSettingsServiceShutdown() override;

  // Checks if the user is the device owner, without the user profile having to
  // been initialized. Should be used only if login state is in safe mode.
  static void IsOwnerForSafeModeAsync(
      const std::string& user_hash,
      const scoped_refptr<ownership::OwnerKeyUtil>& owner_key_util,
      const IsOwnerCallback& callback);

  // Assembles PolicyData based on |settings|, |policy_data| and
  // |user_id|.
  static scoped_ptr<enterprise_management::PolicyData> AssemblePolicy(
      const std::string& user_id,
      const enterprise_management::PolicyData* policy_data,
      const enterprise_management::ChromeDeviceSettingsProto* settings);

  // Updates device |settings|.
  static void UpdateDeviceSettings(
      const std::string& path,
      const base::Value& value,
      enterprise_management::ChromeDeviceSettingsProto& settings);

  bool has_pending_changes() const {
    return !pending_changes_.empty() || tentative_settings_.get();
  }

 private:
  friend class OwnerSettingsServiceChromeOSFactory;

  OwnerSettingsServiceChromeOS(
      DeviceSettingsService* device_settings_service,
      Profile* profile,
      const scoped_refptr<ownership::OwnerKeyUtil>& owner_key_util);

  // OwnerSettingsService protected interface overrides:

  // Reloads private key from profile's NSS slots, responds via |callback|.
  virtual void ReloadKeypairImpl(const base::Callback<
      void(const scoped_refptr<ownership::PublicKey>& public_key,
           const scoped_refptr<ownership::PrivateKey>& private_key)>& callback)
      override;

  // Possibly notifies DeviceSettingsService that owner's keypair is loaded.
  virtual void OnPostKeypairLoadedActions() override;

  // Tries to apply recent changes to device settings proto, sign it and store.
  void StorePendingChanges();

  // Called when current device settings are successfully signed.
  // Sends signed settings for storage.
  void OnPolicyAssembledAndSigned(
      scoped_ptr<enterprise_management::PolicyFetchResponse> policy_response);

  // Called by DeviceSettingsService when modified and signed device
  // settings are stored.
  void OnSignedPolicyStored(bool success);

  // Report status to observers and tries to continue storing pending chages to
  // device settings.
  void ReportStatusAndContinueStoring(bool success);

  DeviceSettingsService* device_settings_service_;

  // Profile this service instance belongs to.
  Profile* profile_;

  // User ID this service instance belongs to.
  std::string user_id_;

  // Whether profile still needs to be initialized.
  bool waiting_for_profile_creation_;

  // Whether TPM token still needs to be initialized.
  bool waiting_for_tpm_token_;

  // A set of pending changes to device settings.
  base::ScopedPtrHashMap<std::string, base::Value> pending_changes_;

  // A protobuf containing pending changes to device settings.
  scoped_ptr<enterprise_management::ChromeDeviceSettingsProto>
      tentative_settings_;

  content::NotificationRegistrar registrar_;

  base::WeakPtrFactory<OwnerSettingsServiceChromeOS> weak_factory_;

  base::WeakPtrFactory<OwnerSettingsServiceChromeOS> store_settings_factory_;

  DISALLOW_COPY_AND_ASSIGN(OwnerSettingsServiceChromeOS);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_OWNERSHIP_OWNER_SETTINGS_SERVICE_CHROMEOS_H_
