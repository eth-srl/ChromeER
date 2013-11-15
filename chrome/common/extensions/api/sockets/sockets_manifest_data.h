// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_EXTENSIONS_API_SOCKETS_SOCKETS_MANIFEST_DATA_H_
#define CHROME_COMMON_EXTENSIONS_API_SOCKETS_SOCKETS_MANIFEST_DATA_H_

#include <vector>

#include "base/strings/string16.h"
#include "chrome/common/extensions/extension.h"
#include "extensions/common/manifest_handler.h"

namespace content {
struct SocketPermissionRequest;
}

namespace extensions {
class SocketsManifestPermission;
}

namespace extensions {

// The parsed form of the "sockets" manifest entry.
class SocketsManifestData : public Extension::ManifestData {
 public:
  explicit SocketsManifestData(
      scoped_ptr<SocketsManifestPermission> permission);
  virtual ~SocketsManifestData();

  // Gets the SocketsManifestData for |extension|, or NULL if none was
  // specified.
  static SocketsManifestData* Get(const Extension* extension);

  static bool CheckRequest(const Extension* extension,
                           const content::SocketPermissionRequest& request);

  // Tries to construct the info based on |value|, as it would have appeared in
  // the manifest. Sets |error| and returns an empty scoped_ptr on failure.
  static scoped_ptr<SocketsManifestData> FromValue(
      const base::Value& value,
      string16* error);

  const SocketsManifestPermission* permission() const {
    return permission_.get();
  }

 private:
  scoped_ptr<SocketsManifestPermission> permission_;
};

}  // namespace extensions

#endif  // CHROME_COMMON_EXTENSIONS_API_SOCKETS_SOCKETS_MANIFEST_DATA_H_
