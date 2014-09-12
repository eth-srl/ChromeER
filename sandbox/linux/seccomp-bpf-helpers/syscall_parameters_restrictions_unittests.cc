// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sandbox/linux/seccomp-bpf-helpers/syscall_parameters_restrictions.h"

#include <time.h>

#include "base/sys_info.h"
#include "base/time/time.h"
#include "build/build_config.h"
#include "sandbox/linux/bpf_dsl/bpf_dsl.h"
#include "sandbox/linux/seccomp-bpf-helpers/sigsys_handlers.h"
#include "sandbox/linux/seccomp-bpf/bpf_tests.h"
#include "sandbox/linux/seccomp-bpf/sandbox_bpf.h"
#include "sandbox/linux/seccomp-bpf/syscall.h"
#include "sandbox/linux/services/linux_syscalls.h"
#include "sandbox/linux/tests/unit_tests.h"

#if !defined(OS_ANDROID)
#include "third_party/lss/linux_syscall_support.h"  // for MAKE_PROCESS_CPUCLOCK
#endif

namespace sandbox {

namespace {

// NOTE: most of the parameter restrictions are tested in
// baseline_policy_unittest.cc as a more end-to-end test.

using sandbox::bpf_dsl::Allow;
using sandbox::bpf_dsl::ResultExpr;
using sandbox::bpf_dsl::SandboxBPFDSLPolicy;

class RestrictClockIdPolicy : public SandboxBPFDSLPolicy {
 public:
  RestrictClockIdPolicy() {}
  virtual ~RestrictClockIdPolicy() {}

  virtual ResultExpr EvaluateSyscall(int sysno) const OVERRIDE {
    switch (sysno) {
      case __NR_clock_gettime:
      case __NR_clock_getres:
        return RestrictClockID();
      default:
        return Allow();
    }
  }
};

void CheckClock(clockid_t clockid) {
  struct timespec ts;
  ts.tv_sec = ts.tv_nsec = -1;
  BPF_ASSERT_EQ(0, clock_gettime(clockid, &ts));
  BPF_ASSERT_LE(0, ts.tv_sec);
  BPF_ASSERT_LE(0, ts.tv_nsec);
}

BPF_TEST_C(ParameterRestrictions,
           clock_gettime_allowed,
           RestrictClockIdPolicy) {
  CheckClock(CLOCK_MONOTONIC);
  CheckClock(CLOCK_PROCESS_CPUTIME_ID);
  CheckClock(CLOCK_REALTIME);
  CheckClock(CLOCK_THREAD_CPUTIME_ID);
}

BPF_DEATH_TEST_C(ParameterRestrictions,
                 clock_gettime_crash_monotonic_raw,
                 DEATH_SEGV_MESSAGE(sandbox::GetErrorMessageContentForTests()),
                 RestrictClockIdPolicy) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
}

#if defined(OS_CHROMEOS)

// A custom BPF tester delegate to run IsRunningOnChromeOS() before
// the sandbox is enabled because we cannot run it with non-SFI BPF
// sandbox enabled.
class ClockSystemTesterDelegate : public sandbox::BPFTesterDelegate {
 public:
  ClockSystemTesterDelegate()
      : is_running_on_chromeos_(base::SysInfo::IsRunningOnChromeOS()) {}
  virtual ~ClockSystemTesterDelegate() {}

  virtual scoped_ptr<sandbox::SandboxBPFPolicy> GetSandboxBPFPolicy() OVERRIDE {
    return scoped_ptr<sandbox::SandboxBPFPolicy>(
        new RestrictClockIdPolicy());
  }
  virtual void RunTestFunction() OVERRIDE {
    if (is_running_on_chromeos_) {
      CheckClock(base::TimeTicks::kClockSystemTrace);
    } else {
      struct timespec ts;
      // kClockSystemTrace is 11, which is CLOCK_THREAD_CPUTIME_ID of
      // the init process (pid=1). If kernel supports this feature,
      // this may succeed even if this is not running on Chrome OS. We
      // just check this clock_gettime call does not crash.
      clock_gettime(base::TimeTicks::kClockSystemTrace, &ts);
    }
  }

 private:
  const bool is_running_on_chromeos_;
  DISALLOW_COPY_AND_ASSIGN(ClockSystemTesterDelegate);
};

BPF_TEST_D(BPFTest, BPFTestWithDelegateClass, ClockSystemTesterDelegate);

#elif defined(OS_LINUX)

BPF_DEATH_TEST_C(ParameterRestrictions,
                 clock_gettime_crash_system_trace,
                 DEATH_SEGV_MESSAGE(sandbox::GetErrorMessageContentForTests()),
                 RestrictClockIdPolicy) {
  struct timespec ts;
  clock_gettime(base::TimeTicks::kClockSystemTrace, &ts);
}

#endif  // defined(OS_CHROMEOS)

#if !defined(OS_ANDROID)
BPF_DEATH_TEST_C(ParameterRestrictions,
                 clock_gettime_crash_cpu_clock,
                 DEATH_SEGV_MESSAGE(sandbox::GetErrorMessageContentForTests()),
                 RestrictClockIdPolicy) {
  // We can't use clock_getcpuclockid() because it's not implemented in newlib,
  // and it might not work inside the sandbox anyway.
  const pid_t kInitPID = 1;
  const clockid_t kInitCPUClockID =
      MAKE_PROCESS_CPUCLOCK(kInitPID, CPUCLOCK_SCHED);

  struct timespec ts;
  clock_gettime(kInitCPUClockID, &ts);
}
#endif  // !defined(OS_ANDROID)

}  // namespace

}  // namespace sandbox
