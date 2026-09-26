/* SPDX-License-Identifier: Unlicense */

#include "region_pick.hpp"

namespace tally {
namespace {

int iabs(int v)
{
  return v < 0 ? -v : v;
}

}  // namespace

RegionPick::RegionPick()
{
  set_title("Select region");
  set_decorated(false);
  set_app_paintable(true);
  set_keep_above(true);
  fullscreen();
  add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK |
             Gdk::KEY_PRESS_MASK);
  set_can_focus(true);

  auto screen = Gdk::Screen::get_default();
  if (screen) {
    if (auto visual = screen->get_rgba_visual())
      gtk_widget_set_visual(GTK_WIDGET(gobj()), visual->gobj());
  }
}

bool RegionPick::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  const int w = get_allocated_width();
  const int h = get_allocated_height();
  cr->set_source_rgba(0, 0, 0, 0.35);
  cr->rectangle(0, 0, w, h);
  cr->fill();
  if (!dragging_)
    return true;
  int x = x0_ < x1_ ? x0_ : x1_;
  int y = y0_ < y1_ ? y0_ : y1_;
  int rw = iabs(x1_ - x0_);
  int rh = iabs(y1_ - y0_);
  cr->set_source_rgba(1, 1, 1, 0.15);
  cr->rectangle(x, y, rw, rh);
  cr->fill();
  cr->set_source_rgb(1, 1, 1);
  cr->set_line_width(2);
  cr->rectangle(x + 0.5, y + 0.5, rw, rh);
  cr->stroke();
  return true;
}

bool RegionPick::on_button_press_event(GdkEventButton* event)
{
  if (!event || event->button != 1)
    return false;
  dragging_ = true;
  x0_ = x1_ = static_cast<int>(event->x);
  y0_ = y1_ = static_cast<int>(event->y);
  queue_draw();
  return true;
}

bool RegionPick::on_button_release_event(GdkEventButton* event)
{
  if (!event || event->button != 1 || !dragging_)
    return false;
  dragging_ = false;
  x1_ = static_cast<int>(event->x);
  y1_ = static_cast<int>(event->y);
  int ox = 0;
  int oy = 0;
  if (auto win = get_window())
    win->get_origin(ox, oy);
  Rect r;
  r.x = (x0_ < x1_ ? x0_ : x1_) + ox;
  r.y = (y0_ < y1_ ? y0_ : y1_) + oy;
  r.w = iabs(x1_ - x0_);
  r.h = iabs(y1_ - y0_);
  hide();
  if (r.w < 2 || r.h < 2)
    signal_cancelled_.emit();
  else
    signal_picked_.emit(r);
  return true;
}

bool RegionPick::on_motion_notify_event(GdkEventMotion* event)
{
  if (!dragging_ || !event)
    return false;
  x1_ = static_cast<int>(event->x);
  y1_ = static_cast<int>(event->y);
  queue_draw();
  return true;
}

bool RegionPick::on_key_press_event(GdkEventKey* event)
{
  if (event && event->keyval == GDK_KEY_Escape) {
    hide();
    signal_cancelled_.emit();
    return true;
  }
  return Gtk::Window::on_key_press_event(event);
}

}  // namespace tally
