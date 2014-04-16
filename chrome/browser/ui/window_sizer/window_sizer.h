// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WINDOW_SIZER_WINDOW_SIZER_H_
#define CHROME_BROWSER_UI_WINDOW_SIZER_WINDOW_SIZER_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/browser/ui/host_desktop.h"
#include "ui/base/ui_base_types.h"
#include "ui/gfx/rect.h"

class Browser;

namespace gfx {
class Display;
class Screen;
}

///////////////////////////////////////////////////////////////////////////////
// WindowSizer
//
//  A class that determines the best new size and position for a window to be
//  shown at based several factors, including the position and size of the last
//  window of the same type, the last saved bounds of the window from the
//  previous session, and default system metrics if neither of the above two
//  conditions exist. The system has built-in providers for monitor metrics
//  and persistent storage (using preferences) but can be overrided with mocks
//  for testing.
//
class WindowSizer {
 public:
  class StateProvider;
  class TargetDisplayProvider;

  // WindowSizer owns |state_provider| and |target_display_provider|,
  // and will use the platforms's gfx::Screen.
  WindowSizer(scoped_ptr<StateProvider> state_provider,
              scoped_ptr<TargetDisplayProvider> target_display_provider,
              const Browser* browser);

  // WindowSizer owns |state_provider| and |target_display_provider|,
  // and will use the supplied |screen|. Used only for testing.
  WindowSizer(scoped_ptr<StateProvider> state_provider,
              scoped_ptr<TargetDisplayProvider> target_display_provider,
              gfx::Screen* screen,
              const Browser* browser);

  virtual ~WindowSizer();

  // An interface implemented by an object that can retrieve state from either a
  // persistent store or an existing window.
  class StateProvider {
   public:
    virtual ~StateProvider() {}

    // Retrieve the persisted bounds of the window. Returns true if there was
    // persisted data to retrieve state information, false otherwise.
    // The |show_state| variable will only be touched if there was persisted
    // data and the |show_state| variable is SHOW_STATE_DEFAULT.
    virtual bool GetPersistentState(gfx::Rect* bounds,
                                    gfx::Rect* work_area,
                                    ui::WindowShowState* show_state) const = 0;

    // Retrieve the bounds of the most recent window of the matching type.
    // Returns true if there was a last active window to retrieve state
    // information from, false otherwise.
    // The |show_state| variable will only be touched if we have found a
    // suitable window and the |show_state| variable is SHOW_STATE_DEFAULT.
    virtual bool GetLastActiveWindowState(
        gfx::Rect* bounds,
        ui::WindowShowState* show_state) const = 0;
  };

  // An interface implemented by an object to identify on which
  // display a new window should be located.
  class TargetDisplayProvider {
    public:
      virtual ~TargetDisplayProvider() {}
      virtual gfx::Display GetTargetDisplay(const gfx::Screen* screen,
                                            const gfx::Rect& bounds) const = 0;
  };

  // Determines the position and size for a window as it is created as well
  // as the initial state. This function uses several strategies to figure out
  // optimal size and placement, first looking for an existing active window,
  // then falling back to persisted data from a previous session, finally
  // utilizing a default algorithm. If |specified_bounds| are non-empty, this
  // value is returned instead. For use only in testing.
  // |show_state| will be overwritten and return the initial visual state of
  // the window to use.
  void DetermineWindowBoundsAndShowState(
      const gfx::Rect& specified_bounds,
      gfx::Rect* bounds,
      ui::WindowShowState* show_state) const;

  // Determines the size, position and maximized state for the browser window.
  // See documentation for DetermineWindowBounds above. Normally,
  // |window_bounds| is calculated by calling GetLastActiveWindowState(). To
  // explicitly specify a particular window to base the bounds on, pass in a
  // non-NULL value for |browser|.
  static void GetBrowserWindowBoundsAndShowState(
      const std::string& app_name,
      const gfx::Rect& specified_bounds,
      const Browser* browser,
      gfx::Rect* window_bounds,
      ui::WindowShowState* show_state);

  // Returns the default origin for popups of the given size.
  static gfx::Point GetDefaultPopupOrigin(const gfx::Size& size,
                                          chrome::HostDesktopType type);

  // How much horizontal and vertical offset there is between newly
  // opened windows.  This value may be different on each platform.
  static const int kWindowTilePixels;

 private:
  // The edge of the screen to check for out-of-bounds.
  enum Edge { TOP, LEFT, BOTTOM, RIGHT };

  // Gets the size and placement of the last active window. Returns true if this
  // data is valid, false if there is no last window and the application should
  // restore saved state from preferences using RestoreWindowPosition.
  // |show_state| will only be changed if it was set to SHOW_STATE_DEFAULT.
  bool GetLastActiveWindowBounds(gfx::Rect* bounds,
                                 ui::WindowShowState* show_state) const;

  // Gets the size and placement of the last window in the last session, saved
  // in local state preferences. Returns true if local state exists containing
  // this information, false if this information does not exist and a default
  // size should be used.
  // |show_state| will only be changed if it was set to SHOW_STATE_DEFAULT.
  bool GetSavedWindowBounds(gfx::Rect* bounds,
                            ui::WindowShowState* show_state) const;

  // Gets the default window position and size to be shown on
  // |display| if there is no last window and no saved window
  // placement in prefs. This function determines the default size
  // based on monitor size, etc.
  void GetDefaultWindowBounds(const gfx::Display& display,
                              gfx::Rect* default_bounds) const;

  // Adjusts |bounds| to be visible on-screen, biased toward the work area of
  // the |display|.  Despite the name, this doesn't
  // guarantee the bounds are fully contained within this display's work rect;
  // it just tried to ensure the edges are visible on _some_ work rect.
  // If |saved_work_area| is non-empty, it is used to determine whether the
  // monitor configuration has changed. If it has, bounds are repositioned and
  // resized if necessary to make them completely contained in the current work
  // area.
  void AdjustBoundsToBeVisibleOnDisplay(
      const gfx::Display& display,
      const gfx::Rect& saved_work_area,
      gfx::Rect* bounds) const;

  // Determine the target display for a new window based on
  // |bounds|. On ash environment, this returns the display containing
  // ash's the target root window.
  gfx::Display GetTargetDisplay(const gfx::Rect& bounds) const;

#if defined(USE_ASH)
  // Ash specific logic for window placement. Returns true if |bounds| and
  // |show_state| have been fully determined, otherwise returns false (but
  // may still affect |show_state|).
  bool GetBrowserBoundsAsh(gfx::Rect* bounds,
                           ui::WindowShowState* show_state) const;

  // Determines the position and size for a tabbed browser window in
  // ash as it gets created. This will be called before other standard
  // placement logic. |show_state| will only be changed
  // if it was set to SHOW_STATE_DEFAULT.
  void GetTabbedBrowserBoundsAsh(gfx::Rect* bounds,
                                 ui::WindowShowState* show_state) const;
#endif

  // Determine the default show state for the window - not looking at other
  // windows or at persistent information.
  ui::WindowShowState GetWindowDefaultShowState() const;

  // Providers for persistent storage and monitor metrics.
  scoped_ptr<StateProvider> state_provider_;
  scoped_ptr<TargetDisplayProvider> target_display_provider_;
  gfx::Screen* screen_;  // not owned.

  // Note that this browser handle might be NULL.
  const Browser* browser_;

  DISALLOW_COPY_AND_ASSIGN(WindowSizer);
};

#endif  // CHROME_BROWSER_UI_WINDOW_SIZER_WINDOW_SIZER_H_
