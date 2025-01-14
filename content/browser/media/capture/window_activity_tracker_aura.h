// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_MEDIA_CAPTURE_WINDOW_ACTIVITY_TRACKER_AURA_H_
#define CONTENT_BROWSER_MEDIA_CAPTURE_WINDOW_ACTIVITY_TRACKER_AURA_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "content/browser/media/capture/window_activity_tracker.h"
#include "content/common/content_export.h"
#include "ui/aura/window.h"
#include "ui/events/event_handler.h"

namespace content {

// Tracks UI events and makes a decision on whether the user has been
// actively interacting with a specified window.
class CONTENT_EXPORT WindowActivityTrackerAura : public WindowActivityTracker,
                                                 public ui::EventHandler,
                                                 public aura::WindowObserver {
 public:
  explicit WindowActivityTrackerAura(aura::Window* window);
  ~WindowActivityTrackerAura() final;

  // WindowActivityTracker overrides.
  bool IsUiInteractionActive() const final;
  void RegisterMouseInteractionObserver(const base::Closure& observer) final;
  void Reset() final;
  base::WeakPtr<WindowActivityTracker> GetWeakPtr() final;

 private:
  // ui::EventHandler overrides.
  void OnEvent(ui::Event* event) final;

  // aura::WindowObserver overrides.
  void OnWindowDestroying(aura::Window* window) final;

  aura::Window* window_;

  // The last time a UI event was detected.
  base::TimeTicks last_time_ui_event_detected_;

  // Runs on mouse movement or mouse cursor changes.
  base::Closure mouse_interaction_observer_;

  // The number of UI events detected so far. In case of continuous events
  // such as mouse movement, a single continuous movement is treated
  // as one event.
  int ui_events_count_;

  base::WeakPtrFactory<WindowActivityTrackerAura> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WindowActivityTrackerAura);
};

}  // namespace content

#endif  // CONTENT_BROWSER_MEDIA_CAPTURE_WINDOW_ACTIVITY_TRACKER_AURA_H_
