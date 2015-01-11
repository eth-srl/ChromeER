// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/apps/app_info_dialog/app_info_summary_panel.h"

#include <vector>

#include "base/callback_forward.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task_runner_util.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_util.h"
#include "chrome/browser/extensions/launch_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/extensions/extension_constants.h"
#include "chrome/grit/generated_resources.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest.h"
#include "extensions/common/manifest_handlers/shared_module_info.h"
#include "extensions/common/manifest_url_handlers.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/models/combobox_model.h"
#include "ui/base/text/bytes_formatting.h"
#include "ui/views/controls/combobox/combobox.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/link.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/layout_constants.h"
#include "ui/views/view.h"

// A model for a combobox selecting the launch options for a hosted app.
// Displays different options depending on the host OS.
class LaunchOptionsComboboxModel : public ui::ComboboxModel {
 public:
  LaunchOptionsComboboxModel();
  ~LaunchOptionsComboboxModel() override;

  extensions::LaunchType GetLaunchTypeAtIndex(int index) const;
  int GetIndexForLaunchType(extensions::LaunchType launch_type) const;

  // Overridden from ui::ComboboxModel:
  int GetItemCount() const override;
  base::string16 GetItemAt(int index) override;

 private:
  // A list of the launch types available in the combobox, in order.
  std::vector<extensions::LaunchType> launch_types_;

  // A list of the messages to display in the combobox, in order. The indexes in
  // this list correspond to the indexes in launch_types_.
  std::vector<base::string16> launch_type_messages_;
};

LaunchOptionsComboboxModel::LaunchOptionsComboboxModel() {
  if (extensions::util::IsStreamlinedHostedAppsEnabled()) {
    // Streamlined hosted apps can only toggle between LAUNCH_TYPE_WINDOW and
    // LAUNCH_TYPE_REGULAR.
    // TODO(sashab): Use a checkbox for this choice instead of combobox.
    launch_types_.push_back(extensions::LAUNCH_TYPE_REGULAR);
    launch_type_messages_.push_back(
        l10n_util::GetStringUTF16(IDS_APP_CONTEXT_MENU_OPEN_TAB));

    // Although LAUNCH_TYPE_WINDOW doesn't work on Mac, the streamlined hosted
    // apps flag isn't available on Mac, so we must be on a non-Mac OS.
    launch_types_.push_back(extensions::LAUNCH_TYPE_WINDOW);
    launch_type_messages_.push_back(
        l10n_util::GetStringUTF16(IDS_APP_CONTEXT_MENU_OPEN_WINDOW));
  } else {
    launch_types_.push_back(extensions::LAUNCH_TYPE_REGULAR);
    launch_type_messages_.push_back(
        l10n_util::GetStringUTF16(IDS_APP_CONTEXT_MENU_OPEN_REGULAR));

    launch_types_.push_back(extensions::LAUNCH_TYPE_PINNED);
    launch_type_messages_.push_back(
        l10n_util::GetStringUTF16(IDS_APP_CONTEXT_MENU_OPEN_PINNED));

#if defined(OS_MACOSX)
    // Mac does not support standalone web app browser windows or maximize.
    launch_types_.push_back(extensions::LAUNCH_TYPE_FULLSCREEN);
    launch_type_messages_.push_back(
        l10n_util::GetStringUTF16(IDS_APP_CONTEXT_MENU_OPEN_FULLSCREEN));
#else
    launch_types_.push_back(extensions::LAUNCH_TYPE_WINDOW);
    launch_type_messages_.push_back(
        l10n_util::GetStringUTF16(IDS_APP_CONTEXT_MENU_OPEN_WINDOW));

    // Even though the launch type is Full Screen, it is more accurately
    // described as Maximized in non-Mac OSs.
    launch_types_.push_back(extensions::LAUNCH_TYPE_FULLSCREEN);
    launch_type_messages_.push_back(
        l10n_util::GetStringUTF16(IDS_APP_CONTEXT_MENU_OPEN_MAXIMIZED));
#endif
  }
}

LaunchOptionsComboboxModel::~LaunchOptionsComboboxModel() {
}

extensions::LaunchType LaunchOptionsComboboxModel::GetLaunchTypeAtIndex(
    int index) const {
  return launch_types_[index];
}

int LaunchOptionsComboboxModel::GetIndexForLaunchType(
    extensions::LaunchType launch_type) const {
  for (size_t i = 0; i < launch_types_.size(); i++) {
    if (launch_types_[i] == launch_type) {
      return i;
    }
  }
  // If the requested launch type is not available, just select the first one.
  LOG(WARNING) << "Unavailable launch type " << launch_type << " selected.";
  return 0;
}

int LaunchOptionsComboboxModel::GetItemCount() const {
  return launch_types_.size();
}

base::string16 LaunchOptionsComboboxModel::GetItemAt(int index) {
  return launch_type_messages_[index];
}

AppInfoSummaryPanel::AppInfoSummaryPanel(Profile* profile,
                                         const extensions::Extension* app)
    : AppInfoPanel(profile, app),
      size_value_(NULL),
      homepage_link_(NULL),
      licenses_link_(NULL),
      launch_options_combobox_(NULL),
      weak_ptr_factory_(this) {
  SetLayoutManager(new views::BoxLayout(views::BoxLayout::kVertical,
                                        0,
                                        0,
                                        views::kRelatedControlVerticalSpacing));

  AddSubviews();
}

AppInfoSummaryPanel::~AppInfoSummaryPanel() {
  // Destroy view children before their models.
  RemoveAllChildViews(true);
}

void AppInfoSummaryPanel::AddDescriptionAndLinksControl(
    views::View* vertical_stack) {
  views::View* description_and_labels_stack = new views::View();
  description_and_labels_stack->SetLayoutManager(
      new views::BoxLayout(views::BoxLayout::kVertical,
                           0,
                           0,
                           views::kRelatedControlSmallVerticalSpacing));

  if (!app_->description().empty()) {
    // TODO(sashab): Clip the app's description to 4 lines, and use Label's
    // built-in elide behavior to add ellipses at the end: crbug.com/358053
    const size_t max_length = 400;
    base::string16 text = base::UTF8ToUTF16(app_->description());
    if (text.length() > max_length) {
      text = text.substr(0, max_length);
      text += base::ASCIIToUTF16(" ... ");
    }

    views::Label* description_label = new views::Label(text);
    description_label->SetMultiLine(true);
    description_label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    description_and_labels_stack->AddChildView(description_label);
  }

  if (CanShowAppHomePage()) {
    homepage_link_ = new views::Link(
        l10n_util::GetStringUTF16(IDS_APPLICATION_INFO_HOMEPAGE_LINK));
    homepage_link_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    homepage_link_->set_listener(this);
    description_and_labels_stack->AddChildView(homepage_link_);
  }

  if (CanDisplayLicenses()) {
    licenses_link_ = new views::Link(
        l10n_util::GetStringUTF16(IDS_APPLICATION_INFO_LICENSES_BUTTON_TEXT));
    licenses_link_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    licenses_link_->set_listener(this);
    description_and_labels_stack->AddChildView(licenses_link_);
  }

  vertical_stack->AddChildView(description_and_labels_stack);
}

void AppInfoSummaryPanel::AddDetailsControl(views::View* vertical_stack) {
  // Component apps have no details.
  if (app_->location() == extensions::Manifest::COMPONENT)
    return;

  views::View* details_list =
      CreateVerticalStack(views::kRelatedControlSmallVerticalSpacing);

  // Add the size.
  views::Label* size_title = new views::Label(
      l10n_util::GetStringUTF16(IDS_APPLICATION_INFO_SIZE_LABEL));
  size_title->SetHorizontalAlignment(gfx::ALIGN_LEFT);

  size_value_ = new views::Label(
      l10n_util::GetStringUTF16(IDS_APPLICATION_INFO_SIZE_LOADING_LABEL));
  size_value_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  StartCalculatingAppSize();

  details_list->AddChildView(CreateKeyValueField(size_title, size_value_));

  // The version doesn't make sense for bookmark apps.
  if (!app_->from_bookmark()) {
    views::Label* version_title = new views::Label(
        l10n_util::GetStringUTF16(IDS_APPLICATION_INFO_VERSION_LABEL));
    version_title->SetHorizontalAlignment(gfx::ALIGN_LEFT);

    views::Label* version_value =
        new views::Label(base::UTF8ToUTF16(app_->VersionString()));
    version_value->SetHorizontalAlignment(gfx::ALIGN_LEFT);

    details_list->AddChildView(
        CreateKeyValueField(version_title, version_value));
  }

  vertical_stack->AddChildView(details_list);
}

void AppInfoSummaryPanel::AddLaunchOptionControl(views::View* vertical_stack) {
  if (!CanSetLaunchType())
    return;

  launch_options_combobox_model_.reset(new LaunchOptionsComboboxModel());
  launch_options_combobox_ =
      new views::Combobox(launch_options_combobox_model_.get());
  launch_options_combobox_->set_listener(this);
  launch_options_combobox_->SetSelectedIndex(
      launch_options_combobox_model_->GetIndexForLaunchType(GetLaunchType()));

  vertical_stack->AddChildView(launch_options_combobox_);
}

void AppInfoSummaryPanel::AddSubviews() {
  AddChildView(CreateHeading(
      l10n_util::GetStringUTF16(IDS_APPLICATION_INFO_APP_OVERVIEW_TITLE)));

  views::View* vertical_stack =
      CreateVerticalStack(views::kUnrelatedControlVerticalSpacing);
  AddChildView(vertical_stack);

  AddDescriptionAndLinksControl(vertical_stack);
  AddDetailsControl(vertical_stack);
  AddLaunchOptionControl(vertical_stack);
}

void AppInfoSummaryPanel::OnPerformAction(views::Combobox* combobox) {
  if (combobox == launch_options_combobox_) {
    SetLaunchType(launch_options_combobox_model_->GetLaunchTypeAtIndex(
        launch_options_combobox_->selected_index()));
  } else {
    NOTREACHED();
  }
}

void AppInfoSummaryPanel::LinkClicked(views::Link* source, int event_flags) {
  if (source == homepage_link_) {
    ShowAppHomePage();
  } else if (source == licenses_link_) {
    DisplayLicenses();
  } else {
    NOTREACHED();
  }
}

void AppInfoSummaryPanel::StartCalculatingAppSize() {
  base::PostTaskAndReplyWithResult(
      content::BrowserThread::GetBlockingPool(),
      FROM_HERE,
      base::Bind(&base::ComputeDirectorySize, app_->path()),
      base::Bind(&AppInfoSummaryPanel::OnAppSizeCalculated, AsWeakPtr()));
}

void AppInfoSummaryPanel::OnAppSizeCalculated(int64 app_size_in_bytes) {
  const int one_mebibyte_in_bytes = 1024 * 1024;
  if (app_size_in_bytes < one_mebibyte_in_bytes) {
    size_value_->SetText(
        l10n_util::GetStringUTF16(IDS_APPLICATION_INFO_SIZE_SMALL_LABEL));
  } else {
    size_value_->SetText(ui::FormatBytesWithUnits(
        app_size_in_bytes, ui::DATA_UNITS_MEBIBYTE, true));
  }
}

extensions::LaunchType AppInfoSummaryPanel::GetLaunchType() const {
  return extensions::GetLaunchType(extensions::ExtensionPrefs::Get(profile_),
                                   app_);
}

void AppInfoSummaryPanel::SetLaunchType(
    extensions::LaunchType launch_type) const {
  DCHECK(CanSetLaunchType());
  ExtensionService* service =
      extensions::ExtensionSystem::Get(profile_)->extension_service();
  extensions::SetLaunchType(service, app_->id(), launch_type);
}

bool AppInfoSummaryPanel::CanSetLaunchType() const {
  // V2 apps and extensions don't have a launch type, and neither does the
  // Chrome app.
  return !app_->is_platform_app() && !app_->is_extension() &&
         app_->id() != extension_misc::kChromeAppId;
}
void AppInfoSummaryPanel::ShowAppHomePage() {
  DCHECK(CanShowAppHomePage());
  OpenLink(extensions::ManifestURL::GetHomepageURL(app_));
  Close();
}

bool AppInfoSummaryPanel::CanShowAppHomePage() const {
  return extensions::ManifestURL::SpecifiedHomepageURL(app_);
}

void AppInfoSummaryPanel::DisplayLicenses() {
  DCHECK(CanDisplayLicenses());
  for (const auto& license_url : GetLicenseUrls())
    OpenLink(license_url);
  Close();
}

bool AppInfoSummaryPanel::CanDisplayLicenses() const {
  return !GetLicenseUrls().empty();
}

const std::vector<GURL> AppInfoSummaryPanel::GetLicenseUrls() const {
  if (!extensions::SharedModuleInfo::ImportsModules(app_))
    return std::vector<GURL>();

  std::vector<GURL> license_urls;
  ExtensionService* service =
      extensions::ExtensionSystem::Get(profile_)->extension_service();
  DCHECK(service);
  const std::vector<extensions::SharedModuleInfo::ImportInfo>& imports =
      extensions::SharedModuleInfo::GetImports(app_);

  for (const auto& shared_module : imports) {
    const extensions::Extension* imported_module =
        service->GetExtensionById(shared_module.extension_id, true);
    DCHECK(imported_module);

    GURL about_page = extensions::ManifestURL::GetAboutPage(imported_module);
    if (about_page != GURL::EmptyGURL())
      license_urls.push_back(about_page);
  }
  return license_urls;
}
