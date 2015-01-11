// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "cc/output/software_frame_data.h"
#include "content/browser/compositor/software_output_device_ozone.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/skia/include/core/SkDevice.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "ui/compositor/compositor.h"
#include "ui/compositor/test/context_factories_for_test.h"
#include "ui/gfx/size.h"
#include "ui/gfx/skia_util.h"
#include "ui/gfx/vsync_provider.h"
#include "ui/gl/gl_implementation.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/ozone/public/surface_ozone_canvas.h"
#include "ui/platform_window/platform_window.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace {

class TestPlatformWindowDelegate : public ui::PlatformWindowDelegate {
 public:
  TestPlatformWindowDelegate() : widget_(gfx::kNullAcceleratedWidget) {}
  ~TestPlatformWindowDelegate() override {}

  gfx::AcceleratedWidget GetAcceleratedWidget() const { return widget_; }

  // ui::PlatformWindowDelegate:
  void OnBoundsChanged(const gfx::Rect& new_bounds) override {}
  void OnDamageRect(const gfx::Rect& damaged_region) override {}
  void DispatchEvent(ui::Event* event) override {}
  void OnCloseRequest() override {}
  void OnClosed() override {}
  void OnWindowStateChanged(ui::PlatformWindowState new_state) override {}
  void OnLostCapture() override {}
  void OnAcceleratedWidgetAvailable(gfx::AcceleratedWidget widget) override {
    widget_ = widget;
  }
  void OnActivationChanged(bool active) override {}

 private:
  gfx::AcceleratedWidget widget_;

  DISALLOW_COPY_AND_ASSIGN(TestPlatformWindowDelegate);
};

}  // namespace

class SoftwareOutputDeviceOzoneTest : public testing::Test {
 public:
  SoftwareOutputDeviceOzoneTest();
  virtual ~SoftwareOutputDeviceOzoneTest();

  virtual void SetUp() override;
  virtual void TearDown() override;

 protected:
  scoped_ptr<content::SoftwareOutputDeviceOzone> output_device_;
  bool enable_pixel_output_;

 private:
  scoped_ptr<ui::Compositor> compositor_;
  scoped_ptr<base::MessageLoop> message_loop_;
  TestPlatformWindowDelegate window_delegate_;
  scoped_ptr<ui::PlatformWindow> window_;

  DISALLOW_COPY_AND_ASSIGN(SoftwareOutputDeviceOzoneTest);
};

SoftwareOutputDeviceOzoneTest::SoftwareOutputDeviceOzoneTest()
    : enable_pixel_output_(false) {
  message_loop_.reset(new base::MessageLoopForUI);
}

SoftwareOutputDeviceOzoneTest::~SoftwareOutputDeviceOzoneTest() {
}

void SoftwareOutputDeviceOzoneTest::SetUp() {
  ui::ContextFactory* context_factory =
      ui::InitializeContextFactoryForTests(enable_pixel_output_);

  const gfx::Size size(500, 400);
  window_ = ui::OzonePlatform::GetInstance()->CreatePlatformWindow(
      &window_delegate_, gfx::Rect(size));
  compositor_.reset(new ui::Compositor(window_delegate_.GetAcceleratedWidget(),
                                       context_factory,
                                       base::MessageLoopProxy::current()));
  compositor_->SetScaleAndSize(1.0f, size);

  output_device_.reset(new content::SoftwareOutputDeviceOzone(
      compositor_.get()));
  output_device_->Resize(size, 1.f);
}

void SoftwareOutputDeviceOzoneTest::TearDown() {
  output_device_.reset();
  compositor_.reset();
  window_.reset();
  ui::TerminateContextFactoryForTests();
}

class SoftwareOutputDeviceOzonePixelTest
    : public SoftwareOutputDeviceOzoneTest {
 protected:
  virtual void SetUp() override;
};

void SoftwareOutputDeviceOzonePixelTest::SetUp() {
  enable_pixel_output_ = true;
  SoftwareOutputDeviceOzoneTest::SetUp();
}

TEST_F(SoftwareOutputDeviceOzoneTest, CheckCorrectResizeBehavior) {
  gfx::Rect damage(0, 0, 100, 100);
  gfx::Size size(200, 100);
  // Reduce size.
  output_device_->Resize(size, 1.f);

  SkCanvas* canvas = output_device_->BeginPaint(damage);
  gfx::Size canvas_size(canvas->getDeviceSize().width(),
                        canvas->getDeviceSize().height());
  EXPECT_EQ(size.ToString(), canvas_size.ToString());

  size.SetSize(1000, 500);
  // Increase size.
  output_device_->Resize(size, 1.f);

  canvas = output_device_->BeginPaint(damage);
  canvas_size.SetSize(canvas->getDeviceSize().width(),
                      canvas->getDeviceSize().height());
  EXPECT_EQ(size.ToString(), canvas_size.ToString());

}

TEST_F(SoftwareOutputDeviceOzonePixelTest, CheckCopyToBitmap) {
  const int width = 6;
  const int height = 4;
  const gfx::Rect area(width, height);
  output_device_->Resize(area.size(), 1.f);
  SkCanvas* canvas = output_device_->BeginPaint(area);

  // Clear the background to black.
  canvas->drawColor(SK_ColorBLACK);

  cc::SoftwareFrameData frame;
  output_device_->EndPaint(&frame);

  // Draw a white rectangle.
  gfx::Rect damage(area.width() / 2, area.height() / 2);
  canvas = output_device_->BeginPaint(damage);
  canvas->clipRect(gfx::RectToSkRect(damage), SkRegion::kReplace_Op);

  canvas->drawColor(SK_ColorWHITE);

  output_device_->EndPaint(&frame);

  SkPMColor pixels[width * height];
  output_device_->CopyToPixels(area, pixels);

  // Check that the copied bitmap contains the same pixel values as what we
  // painted.
  const SkPMColor white = SkPreMultiplyColor(SK_ColorWHITE);
  const SkPMColor black = SkPreMultiplyColor(SK_ColorBLACK);
  for (int i = 0; i < area.height(); ++i) {
    for (int j = 0; j < area.width(); ++j) {
      if (j < damage.width() && i < damage.height())
        EXPECT_EQ(white, pixels[i * area.width() + j]);
      else
        EXPECT_EQ(black, pixels[i * area.width() + j]);
    }
  }
}
