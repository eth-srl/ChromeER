// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GFX_CANVAS_PAINT_H_
#define GFX_CANVAS_PAINT_H_

#include "gfx/canvas.h"
#include "skia/ext/canvas_paint.h"

// Define a skia::CanvasPaint type that wraps our gfx::Canvas like the
// skia::PlatformCanvasPaint wraps PlatformCanvas.

namespace gfx {

typedef skia::CanvasPaintT<Canvas> CanvasPaint;

}  // namespace gfx

#endif  // GFX_CANVAS_PAINT_H_
