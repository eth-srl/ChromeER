// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GOOGLE_APIS_GCM_ENGINE_HEARTBEAT_MANAGER_H_
#define GOOGLE_APIS_GCM_ENGINE_HEARTBEAT_MANAGER_H_

#include "base/callback.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "google_apis/gcm/base/gcm_export.h"

namespace base {
class Timer;
}

namespace mcs_proto {
class HeartbeatConfig;
}

namespace gcm {

// A heartbeat management class, capable of sending and handling heartbeat
// receipt/failures and triggering reconnection as necessary.
class GCM_EXPORT HeartbeatManager {
 public:
  HeartbeatManager();
  ~HeartbeatManager();

  // Start the heartbeat logic.
  // |send_heartbeat_callback_| is the callback the HeartbeatManager uses to
  // send new heartbeats. Only one heartbeat can be outstanding at a time.
  void Start(const base::Closure& send_heartbeat_callback,
             const base::Closure& trigger_reconnect_callback);

  // Stop the timer. Start(..) must be called again to begin sending heartbeats
  // afterwards.
  void Stop();

  // Reset the heartbeat timer. It is valid to call this even if no heartbeat
  // is associated with the ack (for example if another signal is used to
  // determine that the connection is alive).
  void OnHeartbeatAcked();

  // Updates the current heartbeat interval.
  void UpdateHeartbeatConfig(const mcs_proto::HeartbeatConfig& config);

  // Returns the next scheduled heartbeat time. A null time means
  // no heartbeat is pending. If non-null and less than the
  // current time (in ticks), the heartbeat has been triggered and an ack is
  // pending.
  base::TimeTicks GetNextHeartbeatTime() const;

  // Updates the timer used for scheduling heartbeats.
  void UpdateHeartbeatTimer(scoped_ptr<base::Timer> timer);

 protected:
  // Helper method to send heartbeat on timer trigger.
  void OnHeartbeatTriggered();

  // Periodic check to see if the heartbeat has been missed due to some system
  // issue (e.g. the machine was suspended and the timer did not account for
  // that).
  void CheckForMissedHeartbeat();

 private:
  // Restarts the heartbeat timer.
  void RestartTimer();

  // The base::Time at which the heartbeat timer is expected to fire. Used to
  // check if a heartbeat was somehow lost/delayed.
  base::Time heartbeat_expected_time_;

  // Whether the last heartbeat ping sent has been acknowledged or not.
  bool waiting_for_ack_;

  // The current heartbeat interval.
  int heartbeat_interval_ms_;
  // The most recent server-provided heartbeat interval (0 if none has been
  // provided).
  int server_interval_ms_;

  // Timer for triggering heartbeats.
  scoped_ptr<base::Timer> heartbeat_timer_;

  // Callbacks for interacting with the the connection.
  base::Closure send_heartbeat_callback_;
  base::Closure trigger_reconnect_callback_;

  base::WeakPtrFactory<HeartbeatManager> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(HeartbeatManager);
};

}  // namespace gcm

#endif  // GOOGLE_APIS_GCM_ENGINE_HEARTBEAT_MANAGER_H_
