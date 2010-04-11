// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/gtk/status_icons/status_icon_gtk.h"

#include "base/string16.h"
#include "base/logging.h"
#include "base/utf_string_conversions.h"
#include "gfx/gtk_util.h"
#include "third_party/skia/include/core/SkBitmap.h"

StatusIconGtk::StatusIconGtk() {
  icon_ = gtk_status_icon_new();
  gtk_status_icon_set_visible(icon_, TRUE);

  g_signal_connect(icon_, "activate",
                   G_CALLBACK(OnClick), this);
}

StatusIconGtk::~StatusIconGtk() {
  g_object_unref(icon_);
}

void StatusIconGtk::SetImage(const SkBitmap& image) {
  if (image.isNull())
    return;

  GdkPixbuf* pixbuf = gfx::GdkPixbufFromSkBitmap(&image);
  gtk_status_icon_set_from_pixbuf(icon_, pixbuf);
  g_object_unref(pixbuf);
}

void StatusIconGtk::SetPressedImage(const SkBitmap& image) {
  // Ignore pressed images, since the standard on Linux is to not highlight
  // pressed status icons.
}

void StatusIconGtk::SetToolTip(const string16& tool_tip) {
  gtk_status_icon_set_tooltip(icon_, UTF16ToUTF8(tool_tip).c_str());
}

void StatusIconGtk::OnClick(GtkWidget* widget, StatusIconGtk* status_icon) {
  status_icon->DispatchClickEvent();
}
