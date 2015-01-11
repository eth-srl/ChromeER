// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/net/spdyproxy/data_reduction_proxy_chrome_configurator.h"

#include <string>

#include "base/memory/scoped_ptr.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/scoped_user_pref_update.h"
#include "base/prefs/testing_pref_service.h"
#include "base/test/test_simple_task_runner.h"
#include "base/values.h"
#include "chrome/common/pref_names.h"
#include "components/data_reduction_proxy/core/common/data_reduction_proxy_event_store.h"
#include "components/data_reduction_proxy/core/common/data_reduction_proxy_params.h"
#include "net/base/capturing_net_log.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

class DataReductionProxyConfigTest : public testing::Test {
 public:
  void SetUp() override {
    PrefRegistrySimple* registry = pref_service_.registry();
    registry->RegisterDictionaryPref(prefs::kProxy);
    net_log_.reset(new net::CapturingNetLog());
    data_reduction_proxy_event_store_.reset(
        new data_reduction_proxy::DataReductionProxyEventStore(
            new base::TestSimpleTaskRunner()));
    config_.reset(new DataReductionProxyChromeConfigurator(
        &pref_service_,
        new base::TestSimpleTaskRunner(),
        net_log_.get(),
        data_reduction_proxy_event_store_.get()));
  }

  void CheckProxyConfig(
      const std::string& expected_mode,
      const std::string& expected_server,
      const std::string& expected_bypass_list) {

    const base::DictionaryValue* dict =
        pref_service_.GetDictionary(prefs::kProxy);
    std::string mode;
    std::string server;
    std::string bypass_list;
    dict->GetString("mode", &mode);
    ASSERT_EQ(expected_mode, mode);
    dict->GetString("server", &server);
    ASSERT_EQ(expected_server, server);
    dict->GetString("bypass_list", &bypass_list);
    ASSERT_EQ(expected_bypass_list, bypass_list);
  }

  scoped_ptr<DataReductionProxyChromeConfigurator> config_;
  TestingPrefServiceSimple pref_service_;
  scoped_ptr<net::NetLog> net_log_;
  scoped_ptr<data_reduction_proxy::DataReductionProxyEventStore>
      data_reduction_proxy_event_store_;
};

TEST_F(DataReductionProxyConfigTest, TestUnrestricted) {
  config_->Enable(false,
                  false,
                  "https://www.foo.com:443/",
                  "http://www.bar.com:80/",
                  "");
  CheckProxyConfig(
      "fixed_servers",
      "http=https://www.foo.com:443,http://www.bar.com:80,direct://;",
      "");
}

TEST_F(DataReductionProxyConfigTest, TestUnrestrictedSSL) {
  config_->Enable(false,
                  false,
                  "https://www.foo.com:443/",
                  "http://www.bar.com:80/",
                  "http://www.ssl.com:80/");
  CheckProxyConfig(
      "fixed_servers",
      "http=https://www.foo.com:443,http://www.bar.com:80,direct://;"
      "https=http://www.ssl.com:80,direct://;",
      "");
}

TEST_F(DataReductionProxyConfigTest, TestUnrestrictedWithBypassRule) {
  config_->AddHostPatternToBypass("<local>");
  config_->AddHostPatternToBypass("*.goo.com");
  config_->Enable(false,
                  false,
                  "https://www.foo.com:443/",
                  "http://www.bar.com:80/",
                  "");
  CheckProxyConfig(
      "fixed_servers",
      "http=https://www.foo.com:443,http://www.bar.com:80,direct://;",
      "<local>, *.goo.com");
}

TEST_F(DataReductionProxyConfigTest, TestUnrestrictedWithoutFallback) {
  config_->Enable(false, false, "https://www.foo.com:443/", "", "");
  CheckProxyConfig("fixed_servers",
                   "http=https://www.foo.com:443,direct://;",
                   "");
}

TEST_F(DataReductionProxyConfigTest, TestRestricted) {
  config_->Enable(true,
                  false,
                  "https://www.foo.com:443/",
                  "http://www.bar.com:80/",
                  "");
  CheckProxyConfig("fixed_servers",
                   "http=http://www.bar.com:80,direct://;",
                   "");
}

TEST_F(DataReductionProxyConfigTest, TestFallbackRestricted) {
  config_->Enable(false,
                  true,
                  "https://www.foo.com:443/",
                  "http://www.bar.com:80/",
                  "");
  CheckProxyConfig("fixed_servers",
                   "http=https://www.foo.com:443,direct://;",
                   "");
}

TEST_F(DataReductionProxyConfigTest, TestBothRestricted) {
  DictionaryPrefUpdate update(&pref_service_, prefs::kProxy);
  base::DictionaryValue* dict = update.Get();
  dict->SetString("mode", "system");

  config_->Enable(true,
                  true,
                  "https://www.foo.com:443/",
                  "http://www.bar.com:80/",
                  "");
  CheckProxyConfig("system", "", "");
}

TEST_F(DataReductionProxyConfigTest, TestDisable) {
  data_reduction_proxy::DataReductionProxyParams params(
      data_reduction_proxy::DataReductionProxyParams::
          kAllowAllProxyConfigurations);
  config_->Enable(false,
                  false,
                  params.origin().spec(),
                  params.fallback_origin().spec(),
                  "");
  config_->Disable();
  CheckProxyConfig("system", "", "");
}

TEST_F(DataReductionProxyConfigTest, TestDisableWithUserOverride) {
  data_reduction_proxy::DataReductionProxyParams params(
      data_reduction_proxy::DataReductionProxyParams::
          kAllowAllProxyConfigurations);
  config_->Enable(false,
                  false,
                  params.origin().spec(),
                  params.fallback_origin().spec(),
                  "");

  // Override the data reduction proxy.
  DictionaryPrefUpdate update(&pref_service_, prefs::kProxy);
  base::DictionaryValue* dict = update.Get();
  dict->SetString("server", "https://www.baz.com:22/");

  // This should have no effect since proxy server was overridden.
  config_->Disable();

  CheckProxyConfig("fixed_servers", "https://www.baz.com:22/", "");
}

TEST_F(DataReductionProxyConfigTest, TestBypassList) {
  config_->AddHostPatternToBypass("http://www.google.com");
  config_->AddHostPatternToBypass("fefe:13::abc/33");
  config_->AddURLPatternToBypass("foo.org/images/*");
  config_->AddURLPatternToBypass("http://foo.com/*");
  config_->AddURLPatternToBypass("http://baz.com:22/bar/*");
  config_->AddURLPatternToBypass("http://*bat.com/bar/*");

  std::string expected[] = {
    "http://www.google.com",
    "fefe:13::abc/33",
    "foo.org",
    "http://foo.com",
    "http://baz.com:22",
    "http://*bat.com"
  };

  ASSERT_EQ(config_->bypass_rules_.size(), 6u);
  int i = 0;
  for (std::vector<std::string>::iterator it = config_->bypass_rules_.begin();
       it != config_->bypass_rules_.end(); ++it) {
    EXPECT_EQ(expected[i++], *it);
  }
}

