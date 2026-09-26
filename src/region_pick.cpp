/* SPDX-License-Identifier: Unlicense */

#include "region_pick.hpp"

#include <gdkmm/cursor.h>

#include <algorithm>

namespace tally {
namespace {

int iabs(int v)
{
  return v < 0 ? -v : v;
}

bool contains(const Rect& r, int x, int y)
{
  return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
}

}  // namespace

RegionPick::RegionPick()
{
  set_title("Select");
  set_decorated(false);
  set_app_paintable(true);
  set_keep_above(true);
  set_skip_taskbar_hint(true);
  set_accept_focus(true);
  fullscreen();
  add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK |
             Gdk::KEY_PRESS_MASK);
  set_can_focus(true);
}

void RegionPick::begin(const Glib::RefPtr<Gdk::Pixbuf>& desktop, Mode mode,
                       const std::vector<ClientWin>& windows)
{
  desktop_ = desktop;
  mode_ = mode;
  windows_ = windows;
  dragging_ = false;
  hover_ = -1;
  x0_ = y0_ = x1_ = y1_ = 0;
  if (auto gdk = get_window()) {
    auto cur = Gdk::Cursor::create(gdk->get_display(), Gdk::CROSSHAIR);
    gdk->set_cursor(cur);
  }
  queue_draw();
}

const ClientWin* RegionPick::hit_window(int x, int y) const
{
  for (int i = static_cast<int>(windows_.size()) - 1; i >= 0; --i) {
    if (contains(windows_[static_cast<size_t>(i)].r, x, y))
      return &windows_[static_cast<size_t>(i)];
  }
  return nullptr;
}

void RegionPick::finish_ok(Rect r)
{
  hide();
  if (r.w < 2 || r.h < 2)
    signal_cancelled_.emit();
  else
    signal_picked_.emit(r);
}

void RegionPick::finish_cancel()
{
  hide();
  signal_cancelled_.emit();
}

bool RegionPick::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  const int w = get_allocated_width();
  const int h = get_allocated_height();
  cr->set_source_rgb(0.1, 0.1, 0.1);
  cr->rectangle(0, 0, w, h);
  cr->fill();

  int ox = 0;
  int oy = 0;
  if (auto win = get_window())
    win->get_origin(ox, oy);

  if (desktop_) {
    Gdk::Cairo::set_source_pixbuf(cr, desktop_, -ox, -oy);
    cr->paint();
  }

  int hx = 0;
  int hy = 0;
  int hw = 0;
  int hh = 0;
  Glib::ustring caption;
  if (mode_ == Mode::region && dragging_) {
    hx = x0_ < x1_ ? x0_ : x1_;
    hy = y0_ < y1_ ? y0_ : y1_;
    hw = iabs(x1_ - x0_);
    hh = iabs(y1_ - y0_);
    caption = Glib::ustring::compose("%1 × %2", hw, hh);
  } else if (mode_ == Mode::window && hover_ >= 0 &&
             static_cast<size_t>(hover_) < windows_.size()) {
    const auto& cw = windows_[static_cast<size_t>(hover_)];
    hx = cw.r.x;
    hy = cw.r.y;
    hw = cw.r.w;
    hh = cw.r.h;
    caption = cw.title;
  }

  cr->set_source_rgba(0, 0, 0, 0.28);
  cr->rectangle(0, 0, w, h);
  cr->fill();

  if (hw > 0 && hh > 0 && desktop_) {
    const int dx = hx - ox;
    const int dy = hy - oy;
    cr->save();
    cr->rectangle(dx, dy, hw, hh);
    cr->clip();
    Gdk::Cairo::set_source_pixbuf(cr, desktop_, -ox, -oy);
    cr->paint();
    cr->restore();
    cr->set_source_rgb(1.0, 0.85, 0.2);
    cr->set_line_width(2);
    cr->rectangle(dx + 0.5, dy + 0.5, hw, hh);
    cr->stroke();
    if (!caption.empty()) {
      const int capy = dy - 18 < 0 ? dy : dy - 18;
      cr->set_source_rgba(0, 0, 0, 0.65);
      cr->rectangle(dx, capy, std::min(hw, 420), 18);
      cr->fill();
      cr->set_source_rgb(1, 1, 1);
      cr->move_to(dx + 6, capy + 14);
      cr->show_text(caption.raw());
    }
  }
  return true;
}

bool RegionPick::on_button_press_event(GdkEventButton* event)
{
  if (!event || event->button != 1)
    return false;
  const int x = static_cast<int>(event->x_root);
  const int y = static_cast<int>(event->y_root);
  if (mode_ == Mode::window) {
    if (const ClientWin* cw = hit_window(x, y))
      finish_ok(cw->r);
    else
      finish_cancel();
    return true;
  }
  dragging_ = true;
  x0_ = x1_ = x;
  y0_ = y1_ = y;
  queue_draw();
  return true;
}

bool RegionPick::on_button_release_event(GdkEventButton* event)
{
  if (mode_ != Mode::region || !event || event->button != 1 || !dragging_)
    return false;
  dragging_ = false;
  x1_ = static_cast<int>(event->x_root);
  y1_ = static_cast<int>(event->y_root);
  Rect r;
  r.x = x0_ < x1_ ? x0_ : x1_;
  r.y = y0_ < y1_ ? y0_ : y1_;
  r.w = iabs(x1_ - x0_);
  r.h = iabs(y1_ - y0_);
  finish_ok(r);
  return true;
}

bool RegionPick::on_motion_notify_event(GdkEventMotion* event)
{
  if (!event)
    return false;
  const int x = static_cast<int>(event->x_root);
  const int y = static_cast<int>(event->y_root);
  if (mode_ == Mode::region && dragging_) {
    x1_ = x;
    y1_ = y;
    queue_draw();
    return true;
  }
  if (mode_ == Mode::window) {
    int next = -1;
    for (int i = static_cast<int>(windows_.size()) - 1; i >= 0; --i) {
      if (contains(windows_[static_cast<size_t>(i)].r, x, y)) {
        next = i;
        break;
      }
    }
    if (next != hover_) {
      hover_ = next;
      queue_draw();
    }
    return true;
  }
  return false;
}

bool RegionPick::on_key_press_event(GdkEventKey* event)
{
  if (event && event->keyval == GDK_KEY_Escape) {
    finish_cancel();
    return true;
  }
  return Gtk::Window::on_key_press_event(event);
}

}  // namespace tally
