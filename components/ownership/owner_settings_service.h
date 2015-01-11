// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OWNERSHIP_OWNER_SETTINGS_SERVICE_H_
#define COMPONENTS_OWNERSHIP_OWNER_SETTINGS_SERVICE_H_

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/threading/thread_checker.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/ownership/ownership_export.h"
#include "policy/proto/device_management_backend.pb.h"

namespace base {
class TaskRunner;
class Value;
}

namespace ownership {
class OwnerKeyUtil;
class PrivateKey;
class PublicKey;

// This class is a common interface for platform-specific classes
// which deal with ownership, keypairs and owner-related settings.
class OWNERSHIP_EXPORT OwnerSettingsService : public KeyedService {
 public:
  class Observer {
   public:
    virtual ~Observer() {}

    // Called when signed policy was stored, or when an error happed during
    // policy storage..
    virtual void OnSignedPolicyStored(bool success) {}

    // Called when tentative changes were made to policy, but the
    // policy is not signed and stored yet.
    //
    // TODO (ygorshenin@, crbug.com/230018): get rid of the method
    // since it creates DeviceSettingsService's dependency on
    // OwnerSettingsService.
    virtual void OnTentativeChangesInPolicy(
        const enterprise_management::PolicyData& policy_data) {}
  };

  typedef base::Callback<void(
      scoped_ptr<enterprise_management::PolicyFetchResponse> policy_response)>
      AssembleAndSignPolicyAsyncCallback;

  typedef base::Callback<void(bool is_owner)> IsOwnerCallback;

  explicit OwnerSettingsService(
      const scoped_refptr<ownership::OwnerKeyUtil>& owner_key_util);
  ~OwnerSettingsService() override;

  base::WeakPtr<OwnerSettingsService> as_weak_ptr() {
    return weak_factory_.GetWeakPtr();
  }

  void AddObserver(Observer* observer);

  void RemoveObserver(Observer* observer);

  // Returns whether current user is owner or not. When this method
  // is called too early, incorrect result can be returned because
  // private key loading may be in progress.
  bool IsOwner();

  // Determines whether current user is owner or not, responds via
  // |callback|.
  void IsOwnerAsync(const IsOwnerCallback& callback);

  // Assembles and signs |policy| on the |task_runner|, responds on
  // the original thread via |callback|.
  bool AssembleAndSignPolicyAsync(
      base::TaskRunner* task_runner,
      scoped_ptr<enterprise_management::PolicyData> policy,
      const AssembleAndSignPolicyAsyncCallback& callback);

  // Checks whether |setting| is handled by OwnerSettingsService.
  virtual bool HandlesSetting(const std::string& setting) = 0;

  // Sets |setting| value to |value|.
  virtual bool Set(const std::string& setting, const base::Value& value) = 0;

  // Sets a bunch of device settings accumulated before ownership gets
  // established.
  //
  // TODO (ygorshenin@, crbug.com/230018): that this is a temporary
  // solution and should be removed.
  virtual bool CommitTentativeDeviceSettings(
      scoped_ptr<enterprise_management::PolicyData> policy) = 0;

  bool SetBoolean(const std::string& setting, bool value);
  bool SetInteger(const std::string& setting, int value);
  bool SetDouble(const std::string& setting, double value);
  bool SetString(const std::string& setting, const std::string& value);

 protected:
  void ReloadKeypair();

  void OnKeypairLoaded(const scoped_refptr<PublicKey>& public_key,
                       const scoped_refptr<PrivateKey>& private_key);

  // Platform-specific keypair loading algorithm.
  virtual void ReloadKeypairImpl(const base::Callback<
      void(const scoped_refptr<PublicKey>& public_key,
           const scoped_refptr<PrivateKey>& private_key)>& callback) = 0;

  // Plafrom-specific actions which should be performed when keypair is loaded.
  virtual void OnPostKeypairLoadedActions() = 0;

  scoped_refptr<ownership::PublicKey> public_key_;

  scoped_refptr<ownership::PrivateKey> private_key_;

  scoped_refptr<ownership::OwnerKeyUtil> owner_key_util_;

  std::vector<IsOwnerCallback> pending_is_owner_callbacks_;

  ObserverList<Observer> observers_;

  base::ThreadChecker thread_checker_;

 private:
  base::WeakPtrFactory<OwnerSettingsService> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(OwnerSettingsService);
};

}  // namespace ownership

#endif  // COMPONENTS_OWNERSHIP_OWNER_SETTINGS_SERVICE_H_
