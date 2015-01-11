// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_LAYERS_CONTENT_LAYER_CLIENT_H_
#define CC_LAYERS_CONTENT_LAYER_CLIENT_H_

#include "cc/base/cc_export.h"
#include "cc/resources/display_item_list.h"

class SkCanvas;

namespace gfx {
class Rect;
class RectF;
}

namespace cc {

class CC_EXPORT ContentLayerClient {
 public:
  enum GraphicsContextStatus {
    GRAPHICS_CONTEXT_DISABLED,
    GRAPHICS_CONTEXT_ENABLED
  };

  virtual void PaintContents(SkCanvas* canvas,
                             const gfx::Rect& clip,
                             GraphicsContextStatus gc_status) = 0;

  virtual scoped_refptr<DisplayItemList> PaintContentsToDisplayList(
      const gfx::Rect& clip,
      GraphicsContextStatus gc_status) = 0;

  // If true the layer may skip clearing the background before rasterizing,
  // because it will cover any uncleared data with content.
  virtual bool FillsBoundsCompletely() const = 0;

 protected:
  virtual ~ContentLayerClient() {}
};

}  // namespace cc

#endif  // CC_LAYERS_CONTENT_LAYER_CLIENT_H_
