/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <gtkmm.h>

namespace tally {

struct Rect {
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
};

class RegionPick : public Gtk::Window {
 public:
  RegionPick();
  sigc::signal<void, Rect>& signal_picked()
  {
    return signal_picked_;
  }
  sigc::signal<void>& signal_cancelled()
  {
    return signal_cancelled_;
  }

 protected:
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
  bool on_button_press_event(GdkEventButton* event) override;
  bool on_button_release_event(GdkEventButton* event) override;
  bool on_motion_notify_event(GdkEventMotion* event) override;
  bool on_key_press_event(GdkEventKey* event) override;

 private:
  bool dragging_ = false;
  int x0_ = 0;
  int y0_ = 0;
  int x1_ = 0;
  int y1_ = 0;
  sigc::signal<void, Rect> signal_picked_;
  sigc::signal<void> signal_cancelled_;
};

}  // namespace tally
