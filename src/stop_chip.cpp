/* SPDX-License-Identifier: Unlicense */

#include "stop_chip.hpp"

namespace tally {

StopChip::StopChip()
{
  set_title("Tally — recording");
  set_decorated(false);
  set_keep_above(true);
  set_skip_taskbar_hint(true);
  set_skip_pager_hint(true);
  set_resizable(false);
  set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
  set_accept_focus(true);

  box_.set_border_width(8);
  lamp_.set_size_request(16, 16);
  lamp_.set_visible_window(true);
  lamp_.get_style_context()->add_class("tally-lamp-on");
  stop_.get_style_context()->add_class("tally-record");
  stop_.signal_clicked().connect([this]() { signal_stop_.emit(); });
  auto accel = Gtk::AccelGroup::create();
  add_accel_group(accel);
  stop_.add_accelerator("clicked", accel, GDK_KEY_period, Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);

  box_.pack_start(lamp_, Gtk::PACK_SHRINK);
  box_.pack_start(time_, Gtk::PACK_SHRINK);
  box_.pack_start(stop_, Gtk::PACK_SHRINK);
  add(box_);
  show_all_children();
}

void StopChip::set_elapsed(const Glib::ustring& text)
{
  time_.set_text(text);
}

void StopChip::place_corner()
{
  auto screen = Gdk::Screen::get_default();
  if (!screen)
    return;
  int w = get_allocated_width();
  int h = get_allocated_height();
  if (w < 8)
    w = 160;
  if (h < 8)
    h = 40;
  move(screen->get_width() - w - 24, screen->get_height() - h - 56);
}

bool StopChip::on_delete_event(GdkEventAny*)
{
  signal_stop_.emit();
  return true;
}

}  // namespace tally
