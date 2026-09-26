/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <gtkmm.h>

#include <vector>

namespace tally {

struct Rect {
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
};

struct ClientWin {
  Rect r;
  Glib::ustring title;
  unsigned long xid = 0;
};

class RegionPick : public Gtk::Window {
 public:
  enum class Mode { region, window };

  RegionPick();

  void begin(const Glib::RefPtr<Gdk::Pixbuf>& desktop, Mode mode,
             const std::vector<ClientWin>& windows);
  void release_snapshot()
  {
    desktop_.reset();
    windows_.clear();
  }

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
  const ClientWin* hit_window(int x, int y) const;
  void finish_ok(Rect r);
  void finish_cancel();

  Mode mode_ = Mode::region;
  Glib::RefPtr<Gdk::Pixbuf> desktop_;
  std::vector<ClientWin> windows_;
  bool dragging_ = false;
  int x0_ = 0;
  int y0_ = 0;
  int x1_ = 0;
  int y1_ = 0;
  int hover_ = -1;
  sigc::signal<void, Rect> signal_picked_;
  sigc::signal<void> signal_cancelled_;
};

}  // namespace tally
