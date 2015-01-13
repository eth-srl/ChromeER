// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/translate/after_translate_infobar_controller.h"

#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "components/translate/core/browser/translate_infobar_delegate.h"
#include "grit/components_strings.h"
#include "ios/chrome/browser/translate/translate_infobar_tags.h"
#include "ios/public/provider/chrome/browser/chrome_browser_provider.h"
#include "ios/public/provider/chrome/browser/string_provider.h"
#import "ios/public/provider/chrome/browser/ui/infobar_view_delegate.h"
#import "ios/public/provider/chrome/browser/ui/infobar_view_protocol.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/gfx/image/image.h"

@interface AfterTranslateInfoBarController () {
  __weak translate::TranslateInfoBarDelegate* _translateInfoBarDelegate;
}

// Action for any of the user defined buttons.
- (void)infoBarButtonDidPress:(id)sender;

@end

@implementation AfterTranslateInfoBarController

#pragma mark -
#pragma mark InfoBarControllerProtocol

- (void)layoutForDelegate:(infobars::InfoBarDelegate*)delegate
                    frame:(CGRect)frame {
  _translateInfoBarDelegate = delegate->AsTranslateInfoBarDelegate();
  DCHECK(_translateInfoBarDelegate);
  infobars::InfoBarDelegate* infoBarDelegate =
      static_cast<infobars::InfoBarDelegate*>(_translateInfoBarDelegate);
  DCHECK(!infoBarView_);
  infoBarView_.reset([ios::GetChromeBrowserProvider()->CreateInfoBarView()
      initWithFrame:frame
           delegate:delegate_
          isWarning:infoBarDelegate->GetInfoBarType() ==
                    infobars::InfoBarDelegate::WARNING_TYPE]);
  // Icon
  gfx::Image icon = _translateInfoBarDelegate->GetIcon();
  if (!icon.IsEmpty())
    [infoBarView_ addLeftIcon:icon.ToUIImage()];
  // Main text.
  const bool autodeterminedSourceLanguage =
      _translateInfoBarDelegate->original_language_index() ==
      translate::TranslateInfoBarDelegate::kNoIndex;
  bool swappedLanguageButtons;
  std::vector<base::string16> strings;
  translate::TranslateInfoBarDelegate::GetAfterTranslateStrings(
      &strings, &swappedLanguageButtons, autodeterminedSourceLanguage);
  DCHECK_EQ(autodeterminedSourceLanguage ? 2U : 3U, strings.size());
  NSString* label1 = base::SysUTF16ToNSString(strings[0]);
  NSString* label2 = base::SysUTF16ToNSString(strings[1]);
  NSString* label3 = autodeterminedSourceLanguage
                         ? @""
                         : base::SysUTF16ToNSString(strings[2]);
  base::string16 stdOriginal = _translateInfoBarDelegate->language_name_at(
      _translateInfoBarDelegate->original_language_index());
  NSString* original = base::SysUTF16ToNSString(stdOriginal);
  NSString* target =
      base::SysUTF16ToNSString(_translateInfoBarDelegate->language_name_at(
          _translateInfoBarDelegate->target_language_index()));
  base::scoped_nsobject<NSString> label(
      [[NSString alloc] initWithFormat:@"%@ %@ %@%@ %@.", label1, original,
                                       label2, label3, target]);
  [infoBarView_ addLabel:label];
  // Close button.
  [infoBarView_ addCloseButtonWithTag:TranslateInfoBarIOSTag::CLOSE
                               target:self
                               action:@selector(infoBarButtonDidPress:)];
  // Other buttons.
  NSString* buttonRevert = l10n_util::GetNSString(IDS_TRANSLATE_INFOBAR_REVERT);
  NSString* buttonOptions = base::SysUTF16ToNSString(
      ios::GetChromeBrowserProvider()->GetStringProvider()->GetDoneString());
  [infoBarView_ addButton1:buttonOptions
                      tag1:TranslateInfoBarIOSTag::AFTER_DONE
                   button2:buttonRevert
                      tag2:TranslateInfoBarIOSTag::AFTER_REVERT
                    target:self
                    action:@selector(infoBarButtonDidPress:)];
  // Always translate switch.
  if (_translateInfoBarDelegate->ShouldShowAlwaysTranslateShortcut()) {
    base::string16 alwaysTranslate = l10n_util::GetStringFUTF16(
        IDS_TRANSLATE_INFOBAR_ALWAYS_TRANSLATE, stdOriginal);
    const BOOL switchValue = _translateInfoBarDelegate->ShouldAlwaysTranslate();
    [infoBarView_
        addSwitchWithLabel:base::SysUTF16ToNSString(alwaysTranslate)
                      isOn:switchValue
                       tag:TranslateInfoBarIOSTag::ALWAYS_TRANSLATE_SWITCH
                    target:self
                    action:@selector(infoBarSwitchDidPress:)];
  }
}

#pragma mark - Handling of User Events

- (void)infoBarButtonDidPress:(id)sender {
  // This press might have occurred after the user has already pressed a button,
  // in which case the view has been detached from the delegate and this press
  // should be ignored.
  if (!delegate_) {
    return;
  }
  if ([sender isKindOfClass:[UIButton class]]) {
    NSUInteger buttonId = static_cast<UIButton*>(sender).tag;
    if (buttonId == TranslateInfoBarIOSTag::CLOSE) {
      delegate_->InfoBarDidCancel();
    } else {
      DCHECK(buttonId == TranslateInfoBarIOSTag::AFTER_REVERT ||
             buttonId == TranslateInfoBarIOSTag::AFTER_DONE);
      delegate_->InfoBarButtonDidPress(buttonId);
    }
  }
}

- (void)infoBarSwitchDidPress:(id)sender {
  DCHECK_EQ(TranslateInfoBarIOSTag::ALWAYS_TRANSLATE_SWITCH, [sender tag]);
  DCHECK([sender respondsToSelector:@selector(isOn)]);
  if (_translateInfoBarDelegate->ShouldAlwaysTranslate() != [sender isOn])
    _translateInfoBarDelegate->ToggleAlwaysTranslate();
}

@end
