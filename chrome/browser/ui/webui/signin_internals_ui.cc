// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/signin_internals_ui.h"

#include "base/hash.h"
#include "base/profiler/scoped_tracker.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/signin/about_signin_internals_factory.h"
#include "chrome/common/url_constants.h"
#include "components/signin/core/browser/about_signin_internals.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "grit/signin_internals_resources.h"

namespace {

content::WebUIDataSource* CreateSignInInternalsHTMLSource() {
  content::WebUIDataSource* source =
      content::WebUIDataSource::Create(chrome::kChromeUISignInInternalsHost);

  source->SetJsonPath("strings.js");
  source->AddResourcePath("signin_internals.js", IDR_SIGNIN_INTERNALS_INDEX_JS);
  source->SetDefaultResource(IDR_SIGNIN_INTERNALS_INDEX_HTML);
  return source;
}

} //  namespace

SignInInternalsUI::SignInInternalsUI(content::WebUI* web_ui)
    : WebUIController(web_ui) {
  Profile* profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource::Add(profile, CreateSignInInternalsHTMLSource());
  if (profile) {
    AboutSigninInternals* about_signin_internals =
        AboutSigninInternalsFactory::GetForProfile(profile);
    if (about_signin_internals)
      about_signin_internals->AddSigninObserver(this);
  }
}

SignInInternalsUI::~SignInInternalsUI() {
  Profile* profile = Profile::FromWebUI(web_ui());
  if (profile) {
    AboutSigninInternals* about_signin_internals =
        AboutSigninInternalsFactory::GetForProfile(profile);
    if (about_signin_internals) {
      about_signin_internals->RemoveSigninObserver(this);
    }
  }
}

bool SignInInternalsUI::OverrideHandleWebUIMessage(
    const GURL& source_url,
    const std::string& name,
    const base::ListValue& content) {
  if (name == "getSigninInfo") {
    Profile* profile = Profile::FromWebUI(web_ui());
    if (!profile)
      return false;

    AboutSigninInternals* about_signin_internals =
        AboutSigninInternalsFactory::GetForProfile(profile);
    // TODO(vishwath): The UI would look better if we passed in a dict with some
    // reasonable defaults, so the about:signin-internals page doesn't look
    // empty in incognito mode. Alternatively, we could force about:signin to
    // open in non-incognito mode always (like about:settings for ex.).
    if (about_signin_internals) {
      web_ui()->CallJavascriptFunction(
          "chrome.signin.getSigninInfo.handleReply",
          *about_signin_internals->GetSigninStatus());
      about_signin_internals->GetCookieAccountsAsync();

      return true;
    }
  }
  return false;
}

void SignInInternalsUI::OnSigninStateChanged(
    const base::DictionaryValue* info) {
  // TODO(vadimt): Remove ScopedTracker below once crbug.com/422460 is fixed.
  tracked_objects::ScopedTracker tracking_profile(
      FROM_HERE_WITH_EXPLICIT_FUNCTION(
          "422460 SignInInternalsUI::OnSigninStateChanged"));

  web_ui()->CallJavascriptFunction(
      "chrome.signin.onSigninInfoChanged.fire", *info);
}

void SignInInternalsUI::OnCookieAccountsFetched(
    const base::DictionaryValue* info) {
  web_ui()->CallJavascriptFunction(
      "chrome.signin.onCookieAccountsFetched.fire", *info);
}
