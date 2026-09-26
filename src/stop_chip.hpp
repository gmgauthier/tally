/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <gtkmm.h>

namespace tally {

/* Tiny always-on-top Stop control while the main window is minimized. */
class StopChip : public Gtk::Window {
 public:
  StopChip();

  void set_elapsed(const Glib::ustring& text);
  void place_corner();

  sigc::signal<void>& signal_stop()
  {
    return signal_stop_;
  }

 protected:
  bool on_delete_event(GdkEventAny* event) override;

 private:
  Gtk::Box box_{Gtk::ORIENTATION_HORIZONTAL, 8};
  Gtk::EventBox lamp_;
  Gtk::Label time_{"0:00"};
  Gtk::Button stop_{"Stop"};
  sigc::signal<void> signal_stop_;
};

}  // namespace tally
