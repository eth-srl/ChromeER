// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/widget/root_view_targeter.h"

#include "ui/views/view.h"
#include "ui/views/view_targeter_delegate.h"
#include "ui/views/views_switches.h"
#include "ui/views/widget/root_view.h"
#include "ui/views/widget/widget.h"

namespace views {

RootViewTargeter::RootViewTargeter(ViewTargeterDelegate* delegate,
                                   internal::RootView* root_view)
    : ViewTargeter(delegate), root_view_(root_view) {
}

RootViewTargeter::~RootViewTargeter() {
}

View* RootViewTargeter::FindTargetForGestureEvent(
    View* root,
    const ui::GestureEvent& gesture) {
  CHECK_EQ(root, root_view_);

  // Return the default gesture handler if one is already set.
  if (root_view_->gesture_handler_) {
    CHECK(!root_view_->allow_gesture_event_retargeting_);
    return root_view_->gesture_handler_;
  }

  // If rect-based targeting is enabled, use the gesture's bounding box to
  // determine the target. Otherwise use the center point of the gesture's
  // bounding box to determine the target.
  gfx::Rect rect(gesture.location(), gfx::Size(1, 1));
  if (views::switches::IsRectBasedTargetingEnabled() &&
      !gesture.details().bounding_box().IsEmpty()) {
    // TODO(tdanderson): Pass in the bounding box to GetEventHandlerForRect()
    // once crbug.com/313392 is resolved.
    rect.set_size(gesture.details().bounding_box().size());
    rect.Offset(-rect.width() / 2, -rect.height() / 2);
  }

  return root->GetEffectiveViewTargeter()->TargetForRect(root, rect);
}

ui::EventTarget* RootViewTargeter::FindNextBestTargetForGestureEvent(
    ui::EventTarget* previous_target,
    const ui::GestureEvent& gesture) {
  // GESTURE_SCROLL_BEGIN events are always permitted to be re-targeted, even
  // if |allow_gesture_event_retargeting_| is false.
  if (!root_view_->allow_gesture_event_retargeting_ &&
      gesture.type() != ui::ET_GESTURE_SCROLL_BEGIN) {
    return NULL;
  }

  // If |gesture_handler_| is NULL, it is either because the view was removed
  // from the tree by the previous dispatch of |gesture| or because |gesture| is
  // the GESTURE_END event corresponding to the removal of the last touch
  // point. In either case, no further re-targeting of |gesture| should be
  // permitted.
  if (!root_view_->gesture_handler_)
    return NULL;

  return previous_target->GetParentTarget();
}

}  // namespace views
