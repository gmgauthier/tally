/* SPDX-License-Identifier: Unlicense */

#include "stop_chip.hpp"

#include "stop_key.hpp"

#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

namespace tally {
namespace {

GdkFilterReturn stop_key_filter(GdkXEvent* xevent, GdkEvent*, gpointer data)
{
  auto* chip = static_cast<StopChip*>(data);
  auto* ev = static_cast<XEvent*>(xevent);
  if (ev->type != KeyPress || !chip)
    return GDK_FILTER_CONTINUE;
  GdkDisplay* display = gdk_x11_lookup_xdisplay(ev->xkey.display);
  if (!display)
    return GDK_FILTER_CONTINUE;
  guint keyval = 0;
  gdk_keymap_translate_keyboard_state(gdk_keymap_get_for_display(display), ev->xkey.keycode,
                                      static_cast<GdkModifierType>(ev->xkey.state), 0, &keyval,
                                      nullptr, nullptr, nullptr);
  if (!chip->handle_stop_key(keyval, ev->xkey.state))
    return GDK_FILTER_CONTINUE;
  return GDK_FILTER_REMOVE;
}

}  // namespace

StopChip::StopChip()
{
  set_title("Tally — recording");
  set_decorated(false);
  set_keep_above(true);
  set_resizable(false);
  set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
  /* The recording chip is the taskbar entry while the main window is withdrawn. */
  set_skip_taskbar_hint(false);
  set_skip_pager_hint(true);
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
  /* Closing the chip must not destroy it; Stop is the only way out. */
  signal_stop_.emit();
  return true;
}

StopChip::~StopChip()
{
  ungrab_stop_key();
}

bool StopChip::handle_stop_key(guint keyval, guint state)
{
  if (!stop_chord(keyval, state))
    return false;
  signal_stop_.emit();
  return true;
}

void StopChip::on_map()
{
  Gtk::Window::on_map();
  grab_stop_key();
}

void StopChip::on_unmap()
{
  ungrab_stop_key();
  Gtk::Window::on_unmap();
}

void StopChip::grab_stop_key()
{
  if (grabbed_)
    return;
  auto display = Gdk::Display::get_default();
  if (!display || !GDK_IS_X11_DISPLAY(display->gobj()))
    return;
  auto root = display->get_default_screen()->get_root_window();
  if (!root)
    return;
  Display* xdpy = GDK_DISPLAY_XDISPLAY(display->gobj());
  const int code = XKeysymToKeycode(xdpy, XK_period);
  if (code == 0)
    return;
  const ::Window xroot = gdk_x11_window_get_xid(root->gobj());
  for (const unsigned mask : stop_grab_masks())
    XGrabKey(xdpy, code, mask, xroot, False, GrabModeAsync, GrabModeAsync);
  gdk_window_add_filter(root->gobj(), stop_key_filter, this);
  grab_code_ = code;
  grab_root_ = root->gobj();
  grabbed_ = true;
  display->sync();
}

void StopChip::ungrab_stop_key()
{
  if (!grabbed_)
    return;
  auto display = Gdk::Display::get_default();
  if (display && GDK_IS_X11_DISPLAY(display->gobj()) && grab_root_ && grab_code_ != 0) {
    Display* xdpy = GDK_DISPLAY_XDISPLAY(display->gobj());
    const ::Window xroot = gdk_x11_window_get_xid(grab_root_);
    for (const unsigned mask : stop_grab_masks())
      XUngrabKey(xdpy, grab_code_, mask, xroot);
    display->sync();
  }
  if (grab_root_)
    gdk_window_remove_filter(grab_root_, stop_key_filter, this);
  grab_root_ = nullptr;
  grab_code_ = 0;
  grabbed_ = false;
}

}  // namespace tally
