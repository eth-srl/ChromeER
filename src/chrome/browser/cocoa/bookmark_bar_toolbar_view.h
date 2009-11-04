// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The BookmarkBarToolbarView is responsible for drawing the background of the
// BookmarkBar's toolbar in either of its two display modes - permanently
// attached (slimline with a stroke at the bottom edge) or New Tab Page style
// (padded with a round rect border and the New Tab Page theme behind).

#ifndef CHROME_BROWSER_COCOA_BOOKMARK_BAR_TOOLBAR_VIEW_H_
#define CHROME_BROWSER_COCOA_BOOKMARK_BAR_TOOLBAR_VIEW_H_

#import <Cocoa/Cocoa.h>

#import "chrome/browser/cocoa/background_gradient_view.h"

@protocol BookmarkBarFloating;
@class BookmarkBarView;
class TabContents;
class ThemeProvider;

// An interface to allow mocking of a BookmarkBarController by the
// BookmarkBarToolbarView.
@protocol BookmarkBarToolbarViewController
// Displaying the bookmark toolbar background in floating mode requires the
// size of the currently selected tab to properly calculate where the
// background image is joined.
- (int)currentTabContentsHeight;

// Current theme provider, passed to the cross platform NtpBackgroundUtil class.
- (ThemeProvider*)themeProvider;

// Returns true if the bookmark bar should be drawn as if it's a disconnected
// bookmark bar on the New Tag Page.
- (BOOL)drawAsFloatingBar;
@end

@interface BookmarkBarToolbarView : BackgroundGradientView {
 @private
   // The controller which tells us how we should be drawing (as normal or as a
   // floating bar).
   IBOutlet id<BookmarkBarToolbarViewController> controller_;

   // The bookmark bar's contents.
   IBOutlet BookmarkBarView* buttonView_;
}

// Called by our controller to layout our subviews, so that on new tab pages,
// we have a border.
- (void)layoutViews;

@end

#endif  // CHROME_BROWSER_COCOA_BOOKMARK_BAR_TOOLBAR_VIEW_H_
