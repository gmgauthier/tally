/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <gtkmm.h>

namespace tally {

/* Tiny always-on-top Stop control while the main window is minimized. */
class StopChip : public Gtk::Window {
 public:
  StopChip();

  ~StopChip() override;
  void set_elapsed(const Glib::ustring& text);
  void place_corner();
  /* True when this event is the Stop chord. Emits signal_stop in that case. */
  bool handle_stop_key(guint keyval, guint state);

  sigc::signal<void>& signal_stop()
  {
    return signal_stop_;
  }

 protected:
  bool on_delete_event(GdkEventAny* event) override;
  void on_map() override;
  void on_unmap() override;

 private:
  void grab_stop_key();
  void ungrab_stop_key();

  Gtk::Box box_{Gtk::ORIENTATION_HORIZONTAL, 8};
  Gtk::EventBox lamp_;
  Gtk::Label time_{"0:00"};
  Gtk::Button stop_{"Stop"};
  sigc::signal<void> signal_stop_;
  bool grabbed_ = false;
  int grab_code_ = 0;
  GdkWindow* grab_root_ = nullptr;
};

}  // namespace tally
