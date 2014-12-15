// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_MEDIA_CMA_BACKEND_VIDEO_PLANE_H_
#define CHROMECAST_MEDIA_CMA_BACKEND_VIDEO_PLANE_H_

#include "base/macros.h"

namespace gfx {
class QuadF;
class Size;
}

namespace chromecast {
namespace media {

class VideoPlane {
 public:
  enum CoordinateType {
    // Graphics plane as coordinate type.
    COORDINATE_TYPE_GRAPHICS_PLANE = 0,

    // Output video plane as coordinate type.
    COORDINATE_TYPE_VIDEO_PLANE_RESOLUTION = 1,
  };

  VideoPlane();
  virtual ~VideoPlane();

  // Gets video plane resolution.
  virtual gfx::Size GetVideoPlaneResolution() = 0;

  // Updates the video plane geometry.
  // |quad.p1()| corresponds to the top left of the original video,
  // |quad.p2()| to the top right of the original video,
  // and so on.
  // Depending on the underlying hardware, the exact geometry
  // might not be honored.
  // |coordinate_type| indicates what coordinate type |quad| refers to.
  virtual void SetGeometry(const gfx::QuadF& quad,
                           CoordinateType coordinate_type) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(VideoPlane);
};

// Global accessor to the video plane.
VideoPlane* GetVideoPlane();

}  // namespace media
}  // namespace chromecast

#endif  // CHROMECAST_MEDIA_CMA_BACKEND_VIDEO_PLANE_H_
