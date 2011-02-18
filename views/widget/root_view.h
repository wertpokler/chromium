// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIEWS_WIDGET_ROOT_VIEW_H_
#define VIEWS_WIDGET_ROOT_VIEW_H_
#pragma once

#include <string>

#include "base/ref_counted.h"
#include "views/focus/focus_manager.h"
#include "views/focus/focus_search.h"
#include "views/view.h"

#if defined(OS_LINUX)
typedef struct _GdkEventExpose GdkEventExpose;
#endif

namespace views {

class PaintTask;
class Widget;
#if defined(TOUCH_UI)
class GestureManager;
#endif

////////////////////////////////////////////////////////////////////////////////
// RootView class
//
//  The RootView is the root of a View hierarchy. A RootView is attached to a
//  Widget. The Widget is responsible for receiving events from the host
//  environment, converting them to views-compatible events and then forwarding
//  them to the RootView for propagation into the View hierarchy.
//
//  A RootView can have only one child, called its "Contents View" which is
//  sized to fill the bounds of the RootView (and hence the client area of the
//  Widget). Call SetContentsView() after the associated Widget has been
//  initialized to attach the contents view to the RootView.
//  TODO(beng): Enforce no other callers to AddChildView/tree functions by
//              overriding those methods as private here.
//  TODO(beng): Get rid of the scheduled paint rect tracking that this class
//              does - it is superfluous to the underlying environment's invalid
//              rect tracking.
//  TODO(beng): Move to internal namespace and remove accessors from
//              View/Widget.
//  TODO(beng): Clean up API further, make WidgetImpl a friend.
//
class RootView : public View,
                 public FocusTraversable {
 public:
  static const char kViewClassName[];

  // Creation and lifetime -----------------------------------------------------
  explicit RootView(Widget* widget);
  virtual ~RootView();

  // Tree operations -----------------------------------------------------------

  // Sets the "contents view" of the RootView. This is the single child view
  // that is responsible for laying out the contents of the widget.
  void SetContentsView(View* contents_view);

  // Called when parent of the host changed.
  void NotifyNativeViewHierarchyChanged(bool attached,
                                        gfx::NativeView native_view);

  // Painting ------------------------------------------------------------------

  // Whether or not this View needs repainting. If |urgent| is true, this method
  // returns whether this root view needs to paint as soon as possible.
  bool NeedsPainting(bool urgent);

  // Invoked by the Widget to discover what rectangle should be painted.
  const gfx::Rect& GetScheduledPaintRect();

  // Returns the region scheduled to paint clipped to the RootViews bounds.
  gfx::Rect GetScheduledPaintRectConstrainedToSize();

  // Clears the region that is schedule to be painted. You nearly never need
  // to invoke this. This is primarily intended for Widgets.
  void ClearPaintRect();

  // TODO(beng): These should be handled at the NativeWidget level. NativeWidget
  //             should crack and create a gfx::Canvas which is passed to a
  //             paint processing routine here.
#if defined(OS_WIN)
  // Invoked from the Widget to service a WM_PAINT call.
  void OnPaint(HWND hwnd);
#elif defined(OS_LINUX)
  void OnPaint(GdkEventExpose* event);
#endif

  // Enables debug painting. See |debug_paint_enabled_| for details.
  static void EnableDebugPaint();

  // Input ---------------------------------------------------------------------

  // Invoked By the Widget if the mouse drag is interrupted by
  // the system. Invokes OnMouseReleased with a value of true for canceled.
  void ProcessMouseDragCanceled();

  // Invoked by the Widget instance when the mouse moves outside of the Widget
  // bounds.
  virtual void ProcessOnMouseExited();

  // Process a key event. Send the event to the focused view and up the focus
  // path, and finally to the default keyboard handler, until someone consumes
  // it.  Returns whether anyone consumed the event.
  bool ProcessKeyEvent(const KeyEvent& event);

  // Set the default keyboard handler. The default keyboard handler is
  // a view that will get an opportunity to process key events when all
  // views in the focus path did not process an event.
  //
  // Note: this is a single view at this point. We may want to make
  // this a list if needed.
  void SetDefaultKeyboardHandler(View* v);

  // Process a mousewheel event. Return true if the event was processed
  // and false otherwise.
  // MouseWheel events are sent on the focus path.
  virtual bool ProcessMouseWheelEvent(const MouseWheelEvent& e);

#if defined(TOUCH_UI) && defined(UNIT_TEST)
  // For unit testing purposes, we use this method to set a mock
  // GestureManager
  void SetGestureManager(GestureManager* g) { gesture_manager_ = g; }
#endif

  // Focus ---------------------------------------------------------------------

  // Set whether this root view should focus the corresponding hwnd
  // when an unprocessed mouse event occurs.
  void SetFocusOnMousePressed(bool f);

  // Used to set the FocusTraversable parent after the view has been created
  // (typically when the hierarchy changes and this RootView is added/removed).
  virtual void SetFocusTraversableParent(FocusTraversable* focus_traversable);

  // Used to set the View parent after the view has been created.
  virtual void SetFocusTraversableParentView(View* view);

  // System events -------------------------------------------------------------

  // Public API for broadcasting theme change notifications to this View
  // hierarchy.
  void NotifyThemeChanged();

  // Public API for broadcasting locale change notifications to this View
  // hierarchy.
  void NotifyLocaleChanged();

  // Overridden from FocusTraversable:
  virtual FocusSearch* GetFocusSearch();
  virtual FocusTraversable* GetFocusTraversableParent();
  virtual View* GetFocusTraversableParentView();

  // Overridden from View:
  virtual void SchedulePaintInRect(const gfx::Rect& r, bool urgent);
  virtual void Paint(gfx::Canvas* canvas);
  virtual void PaintNow();
  virtual const Widget* GetWidget() const;
  virtual Widget* GetWidget();
  virtual bool OnMousePressed(const MouseEvent& e);
  virtual bool OnMouseDragged(const MouseEvent& e);
  virtual void OnMouseReleased(const MouseEvent& e, bool canceled);
  virtual void OnMouseMoved(const MouseEvent& e);
  virtual void SetMouseHandler(View* new_mouse_handler);
#if defined(TOUCH_UI)
  virtual TouchStatus OnTouchEvent(const TouchEvent& e);
#endif
  virtual bool IsVisibleInRootView() const;
  virtual std::string GetClassName() const;
  virtual AccessibilityTypes::Role GetAccessibleRole();

 protected:
  // Overridden from View:
  virtual void ViewHierarchyChanged(bool is_add, View *parent, View *child);
#ifndef NDEBUG
  virtual bool IsProcessingPaint() const { return is_processing_paint_; }
#endif

 private:
  friend class View;

#if defined(TOUCH_UI)
  // Required so the GestureManager can call the Process* entry points
  // with synthetic events as necessary.
  friend class GestureManager;
#endif

  // Coordinate conversion -----------------------------------------------------

  // Convert a point to our current mouse handler. Returns false if the
  // mouse handler is not connected to a Widget. In that case, the
  // conversion cannot take place and *p is unchanged
  bool ConvertPointToMouseHandler(const gfx::Point& l, gfx::Point *p);

  // Input ---------------------------------------------------------------------

  // Update the cursor given a mouse event. This is called by non mouse_move
  // event handlers to honor the cursor desired by views located under the
  // cursor during drag operations.
  void UpdateCursor(const MouseEvent& e);

  // Sets the current cursor, or resets it to the last one if NULL is provided.
  void SetActiveCursor(gfx::NativeCursor cursor);

  // Updates the last_mouse_* fields from e.
  void SetMouseLocationAndFlags(const MouseEvent& e);

  //////////////////////////////////////////////////////////////////////////////

  // Tree operations -----------------------------------------------------------

  // The host Widget
  Widget* widget_;

  // Painting ------------------------------------------------------------------

  // The rectangle that should be painted
  gfx::Rect invalid_rect_;

  // Whether the current invalid rect should be painted urgently.
  bool invalid_rect_urgent_;

  // The task that we are using to trigger some non urgent painting or NULL
  // if no painting has been scheduled yet.
  PaintTask* pending_paint_task_;

  // Indicate if, when the pending_paint_task_ is run, actual painting is still
  // required.
  bool paint_task_needed_;

#ifndef NDEBUG
  // True if we're currently processing paint.
  bool is_processing_paint_;
#endif

  // True to enable debug painting. Enabling causes the damaged
  // region to be painted to flash in red.
  static bool debug_paint_enabled_;

  // Input ---------------------------------------------------------------------

  // The view currently handing down - drag - up
  View* mouse_pressed_handler_;

  // The view currently handling enter / exit
  View* mouse_move_handler_;

  // The last view to handle a mouse click, so that we can determine if
  // a double-click lands on the same view as its single-click part.
  View* last_click_handler_;

  // true if mouse_handler_ has been explicitly set
  bool explicit_mouse_handler_;

  // Previous cursor
  gfx::NativeCursor previous_cursor_;

  // Default keyboard handler
  View* default_keyboard_handler_;

  // Last position/flag of a mouse press/drag. Used if capture stops and we need
  // to synthesize a release.
  int last_mouse_event_flags_;
  int last_mouse_event_x_;
  int last_mouse_event_y_;

#if defined(TOUCH_UI)
  // The gesture_manager_ for this.
  GestureManager* gesture_manager_;

  // The view currently handling touch events.
  View* touch_pressed_handler_;
#endif

  // Focus ---------------------------------------------------------------------

  // The focus search algorithm.
  FocusSearch focus_search_;

  // Whether this root view should make our hwnd focused
  // when an unprocessed mouse press event occurs
  bool focus_on_mouse_pressed_;

  // Flag used to ignore focus events when we focus the native window associated
  // with a view.
  bool ignore_set_focus_calls_;

  // Whether this root view belongs to the current active window.
  // bool activated_;

  // The parent FocusTraversable, used for focus traversal.
  FocusTraversable* focus_traversable_parent_;

  // The View that contains this RootView. This is used when we have RootView
  // wrapped inside native components, and is used for the focus traversal.
  View* focus_traversable_parent_view_;

  // Drag and drop -------------------------------------------------------------

  // Tracks drag state for a view.
  View::DragInfo drag_info;

  DISALLOW_IMPLICIT_CONSTRUCTORS(RootView);
};
}  // namespace views

#endif  // VIEWS_WIDGET_ROOT_VIEW_H_
