// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/bookmark_app_helper.h"

#include <cctype>

#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/extensions/crx_installer.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/favicon_downloader.h"
#include "chrome/browser/extensions/tab_helper.h"
#include "chrome/common/extensions/extension_constants.h"
#include "chrome/common/extensions/manifest_handlers/app_launch_info.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/image_loader.h"
#include "extensions/browser/notification_types.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handlers/icons_handler.h"
#include "extensions/common/url_pattern.h"
#include "grit/platform_locale_settings.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "skia/ext/image_operations.h"
#include "skia/ext/platform_canvas.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/color_analysis.h"
#include "ui/gfx/color_utils.h"
#include "ui/gfx/font.h"
#include "ui/gfx/font_list.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/image/canvas_image_source.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_family.h"

namespace {

// Overlays a shortcut icon over the bottom left corner of a given image.
class GeneratedIconImageSource : public gfx::CanvasImageSource {
 public:
  explicit GeneratedIconImageSource(char letter, SkColor color, int output_size)
      : gfx::CanvasImageSource(gfx::Size(output_size, output_size), false),
        letter_(letter),
        color_(color),
        output_size_(output_size) {}
  ~GeneratedIconImageSource() override {}

 private:
  // gfx::CanvasImageSource overrides:
  void Draw(gfx::Canvas* canvas) override {
    const unsigned char kLuminanceThreshold = 190;
    const int icon_size = output_size_ * 3 / 4;
    const int icon_inset = output_size_ / 8;
    const size_t border_radius = output_size_ / 16;
    const size_t font_size = output_size_ * 7 / 16;

    std::string font_name =
        l10n_util::GetStringUTF8(IDS_SANS_SERIF_FONT_FAMILY);
#if defined(OS_CHROMEOS)
    const std::string kChromeOSFontFamily = "Noto Sans";
    font_name = kChromeOSFontFamily;
#endif

    // Draw a rounded rect of the given |color|.
    SkPaint background_paint;
    background_paint.setFlags(SkPaint::kAntiAlias_Flag);
    background_paint.setColor(color_);

    gfx::Rect icon_rect(icon_inset, icon_inset, icon_size, icon_size);
    canvas->DrawRoundRect(icon_rect, border_radius, background_paint);

    // The text rect's size needs to be odd to center the text correctly.
    gfx::Rect text_rect(icon_inset, icon_inset, icon_size + 1, icon_size + 1);
    // Draw the letter onto the rounded rect. The letter's color depends on the
    // luminance of |color|.
    unsigned char luminance = color_utils::GetLuminanceForColor(color_);
    canvas->DrawStringRectWithFlags(
        base::string16(1, std::toupper(letter_)),
        gfx::FontList(gfx::Font(font_name, font_size)),
        luminance > kLuminanceThreshold ? SK_ColorBLACK : SK_ColorWHITE,
        text_rect,
        gfx::Canvas::TEXT_ALIGN_CENTER);
  }

  char letter_;

  SkColor color_;

  int output_size_;

  DISALLOW_COPY_AND_ASSIGN(GeneratedIconImageSource);
};

void OnIconsLoaded(
    WebApplicationInfo web_app_info,
    const base::Callback<void(const WebApplicationInfo&)> callback,
    const gfx::ImageFamily& image_family) {
  for (gfx::ImageFamily::const_iterator it = image_family.begin();
       it != image_family.end();
       ++it) {
    WebApplicationInfo::IconInfo icon_info;
    icon_info.data = *it->ToSkBitmap();
    icon_info.width = icon_info.data.width();
    icon_info.height = icon_info.data.height();
    web_app_info.icons.push_back(icon_info);
  }
  callback.Run(web_app_info);
}

std::set<int> SizesToGenerate() {
  // Generate container icons from smaller icons.
  const int kIconSizesToGenerate[] = {
      extension_misc::EXTENSION_ICON_SMALL,
      extension_misc::EXTENSION_ICON_MEDIUM,
      extension_misc::EXTENSION_ICON_LARGE,
  };
  return std::set<int>(kIconSizesToGenerate,
                       kIconSizesToGenerate + arraysize(kIconSizesToGenerate));
}

void GenerateIcons(std::set<int> generate_sizes,
                   const GURL& app_url,
                   SkColor generated_icon_color,
                   std::map<int, SkBitmap>* bitmap_map) {
  // The letter that will be painted on the generated icon.
  char icon_letter = ' ';
  std::string domain_and_registry(
      net::registry_controlled_domains::GetDomainAndRegistry(
          app_url,
          net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES));
  if (!domain_and_registry.empty()) {
    icon_letter = domain_and_registry[0];
  } else if (!app_url.host().empty()) {
    icon_letter = app_url.host()[0];
  }

  // If no color has been specified, use a dark gray so it will stand out on the
  // black shelf.
  if (generated_icon_color == SK_ColorTRANSPARENT)
    generated_icon_color = SK_ColorDKGRAY;

  for (std::set<int>::const_iterator it = generate_sizes.begin();
       it != generate_sizes.end(); ++it) {
    extensions::BookmarkAppHelper::GenerateIcon(
        bitmap_map, *it, generated_icon_color, icon_letter);
    // Also generate the 2x resource for this size.
    extensions::BookmarkAppHelper::GenerateIcon(
        bitmap_map, *it * 2, generated_icon_color, icon_letter);
  }
}

void ReplaceWebAppIcons(std::map<int, SkBitmap> bitmap_map,
                        WebApplicationInfo* web_app_info) {
  web_app_info->icons.clear();

  // Populate the icon data into the WebApplicationInfo we are using to
  // install the bookmark app.
  for (std::map<int, SkBitmap>::const_iterator bitmap_map_it =
           bitmap_map.begin();
       bitmap_map_it != bitmap_map.end(); ++bitmap_map_it) {
    WebApplicationInfo::IconInfo icon_info;
    icon_info.data = bitmap_map_it->second;
    icon_info.width = icon_info.data.width();
    icon_info.height = icon_info.data.height();
    web_app_info->icons.push_back(icon_info);
  }
}

}  // namespace

namespace extensions {

// static
void BookmarkAppHelper::UpdateWebAppInfoFromManifest(
    const content::Manifest& manifest,
    WebApplicationInfo* web_app_info) {
  if (!manifest.short_name.is_null())
    web_app_info->title = manifest.short_name.string();

  // Give the full length name priority.
  if (!manifest.name.is_null())
    web_app_info->title = manifest.name.string();

  // Set the url based on the manifest value, if any.
  if (manifest.start_url.is_valid())
    web_app_info->app_url = manifest.start_url;

  // If any icons are specified in the manifest, they take precedence over any
  // we picked up from the web_app stuff.
  if (!manifest.icons.empty()) {
    web_app_info->icons.clear();
    for (const auto& icon : manifest.icons) {
      // TODO(benwells): Take the declared icon density and sizes into account.
      WebApplicationInfo::IconInfo info;
      info.url = icon.src;
      web_app_info->icons.push_back(info);
    }
  }
}

// static
void BookmarkAppHelper::GenerateIcon(std::map<int, SkBitmap>* bitmaps,
                                     int output_size,
                                     SkColor color,
                                     char letter) {
  // Do nothing if there is already an icon of |output_size|.
  if (bitmaps->count(output_size))
    return;

  gfx::ImageSkia icon_image(
      new GeneratedIconImageSource(letter, color, output_size),
      gfx::Size(output_size, output_size));
  icon_image.bitmap()->deepCopyTo(&(*bitmaps)[output_size]);
}

BookmarkAppHelper::BookmarkAppHelper(ExtensionService* service,
                                     WebApplicationInfo web_app_info,
                                     content::WebContents* contents)
    : contents_(contents),
      web_app_info_(web_app_info),
      crx_installer_(extensions::CrxInstaller::CreateSilent(service)) {
  registrar_.Add(this,
                 extensions::NOTIFICATION_CRX_INSTALLER_DONE,
                 content::Source<CrxInstaller>(crx_installer_.get()));

  registrar_.Add(this,
                 extensions::NOTIFICATION_EXTENSION_INSTALL_ERROR,
                 content::Source<CrxInstaller>(crx_installer_.get()));

  crx_installer_->set_error_on_unsupported_requirements(true);
}

BookmarkAppHelper::~BookmarkAppHelper() {}

void BookmarkAppHelper::Create(const CreateBookmarkAppCallback& callback) {
  callback_ = callback;

  if (contents_) {
    contents_->GetManifest(base::Bind(&BookmarkAppHelper::OnDidGetManifest,
                                     base::Unretained(this)));
  } else {
    OnIconsDownloaded(true, std::map<GURL, std::vector<SkBitmap> >());
  }
}

void BookmarkAppHelper::OnDidGetManifest(const content::Manifest& manifest) {
  if (contents_->IsBeingDestroyed())
    return;

  UpdateWebAppInfoFromManifest(manifest, &web_app_info_);

  // Add urls from the WebApplicationInfo.
  std::vector<GURL> web_app_info_icon_urls;
  for (std::vector<WebApplicationInfo::IconInfo>::const_iterator it =
           web_app_info_.icons.begin();
       it != web_app_info_.icons.end();
       ++it) {
    if (it->url.is_valid())
      web_app_info_icon_urls.push_back(it->url);
  }

  favicon_downloader_.reset(
      new FaviconDownloader(contents_,
                            web_app_info_icon_urls,
                            base::Bind(&BookmarkAppHelper::OnIconsDownloaded,
                                       base::Unretained(this))));
  favicon_downloader_->Start();
}

void BookmarkAppHelper::OnIconsDownloaded(
    bool success,
    const std::map<GURL, std::vector<SkBitmap> >& bitmaps) {
  // The tab has navigated away during the icon download. Cancel the bookmark
  // app creation.
  if (!success) {
    favicon_downloader_.reset();
    callback_.Run(NULL, web_app_info_);
    return;
  }

  std::vector<SkBitmap> downloaded_icons;
  for (FaviconDownloader::FaviconMap::const_iterator map_it = bitmaps.begin();
       map_it != bitmaps.end();
       ++map_it) {
    for (std::vector<SkBitmap>::const_iterator bitmap_it =
             map_it->second.begin();
         bitmap_it != map_it->second.end();
         ++bitmap_it) {
      if (bitmap_it->empty() || bitmap_it->width() != bitmap_it->height())
        continue;

      downloaded_icons.push_back(*bitmap_it);
    }
  }

  // Add all existing icons from WebApplicationInfo.
  for (std::vector<WebApplicationInfo::IconInfo>::const_iterator it =
           web_app_info_.icons.begin();
       it != web_app_info_.icons.end();
       ++it) {
    const SkBitmap& icon = it->data;
    if (!icon.drawsNothing() && icon.width() == icon.height())
      downloaded_icons.push_back(icon);
  }

  // Add the downloaded icons. Extensions only allow certain icon sizes. First
  // populate icons that match the allowed sizes exactly and then downscale
  // remaining icons to the closest allowed size that doesn't yet have an icon.
  std::set<int> allowed_sizes(extension_misc::kExtensionIconSizes,
                              extension_misc::kExtensionIconSizes +
                                  extension_misc::kNumExtensionIconSizes);

  web_app_info_.generated_icon_color = SK_ColorTRANSPARENT;
  // Determine the color that will be used for the icon's background. For this
  // the dominant color of the first icon found is used.
  if (downloaded_icons.size()) {
    color_utils::GridSampler sampler;
    web_app_info_.generated_icon_color =
        color_utils::CalculateKMeanColorOfBitmap(downloaded_icons[0]);
  }

  std::set<int> generate_sizes = SizesToGenerate();

  std::map<int, SkBitmap> generated_icons;
  // Icons are always generated, replacing the icons that were downloaded. This
  // is done so that the icons are consistent across machines.
  // TODO(benwells): Use blob sync once it is available to sync the downloaded
  // icons, and then only generate when there are required sizes missing.
  GenerateIcons(generate_sizes, web_app_info_.app_url,
                web_app_info_.generated_icon_color, &generated_icons);

  ReplaceWebAppIcons(generated_icons, &web_app_info_);

  // Install the app.
  crx_installer_->InstallWebApp(web_app_info_);
  favicon_downloader_.reset();
}

void BookmarkAppHelper::Observe(int type,
                                const content::NotificationSource& source,
                                const content::NotificationDetails& details) {
  switch (type) {
    case extensions::NOTIFICATION_CRX_INSTALLER_DONE: {
      const Extension* extension =
          content::Details<const Extension>(details).ptr();
      DCHECK(extension);
      DCHECK_EQ(AppLaunchInfo::GetLaunchWebURL(extension),
                web_app_info_.app_url);
      callback_.Run(extension, web_app_info_);
      break;
    }
    case extensions::NOTIFICATION_EXTENSION_INSTALL_ERROR:
      callback_.Run(NULL, web_app_info_);
      break;
    default:
      NOTREACHED();
      break;
  }
}

void CreateOrUpdateBookmarkApp(ExtensionService* service,
                               WebApplicationInfo* web_app_info) {
  scoped_refptr<extensions::CrxInstaller> installer(
      extensions::CrxInstaller::CreateSilent(service));
  installer->set_error_on_unsupported_requirements(true);
  if (web_app_info->icons.empty()) {
    std::map<int, SkBitmap> bitmap_map;
    GenerateIcons(SizesToGenerate(), web_app_info->app_url,
                  web_app_info->generated_icon_color, &bitmap_map);
    ReplaceWebAppIcons(bitmap_map, web_app_info);
  }

  installer->InstallWebApp(*web_app_info);
}

void GetWebApplicationInfoFromApp(
    content::BrowserContext* browser_context,
    const extensions::Extension* extension,
    const base::Callback<void(const WebApplicationInfo&)> callback) {
  if (!extension->from_bookmark()) {
    callback.Run(WebApplicationInfo());
    return;
  }

  WebApplicationInfo web_app_info;
  web_app_info.app_url = AppLaunchInfo::GetLaunchWebURL(extension);
  web_app_info.title = base::UTF8ToUTF16(extension->non_localized_name());
  web_app_info.description = base::UTF8ToUTF16(extension->description());

  std::vector<extensions::ImageLoader::ImageRepresentation> info_list;
  for (size_t i = 0; i < extension_misc::kNumExtensionIconSizes; ++i) {
    int size = extension_misc::kExtensionIconSizes[i];
    extensions::ExtensionResource resource =
        extensions::IconsInfo::GetIconResource(
            extension, size, ExtensionIconSet::MATCH_EXACTLY);
    if (!resource.empty()) {
      info_list.push_back(extensions::ImageLoader::ImageRepresentation(
          resource,
          extensions::ImageLoader::ImageRepresentation::ALWAYS_RESIZE,
          gfx::Size(size, size),
          ui::SCALE_FACTOR_100P));
    }
  }

  extensions::ImageLoader::Get(browser_context)->LoadImageFamilyAsync(
      extension, info_list, base::Bind(&OnIconsLoaded, web_app_info, callback));
}

bool IsValidBookmarkAppUrl(const GURL& url) {
  URLPattern origin_only_pattern(Extension::kValidWebExtentSchemes);
  origin_only_pattern.SetMatchAllURLs(true);
  return url.is_valid() && origin_only_pattern.MatchesURL(url);
}

}  // namespace extensions
