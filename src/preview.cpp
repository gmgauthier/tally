/* SPDX-License-Identifier: Unlicense */

#include "preview.hpp"

namespace tally {

Preview::Preview()
{
  set_size_request(200, 120);
}

void Preview::set_pixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pix)
{
  pix_ = pix;
  queue_draw();
}

void Preview::clear()
{
  pix_.reset();
  queue_draw();
}

bool Preview::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  const int w = get_allocated_width();
  const int h = get_allocated_height();
  cr->set_source_rgb(0.08, 0.08, 0.1);
  cr->rectangle(0, 0, w, h);
  cr->fill();
  if (!pix_)
    return true;
  const double sx = static_cast<double>(w) / pix_->get_width();
  const double sy = static_cast<double>(h) / pix_->get_height();
  const double s = sx < sy ? sx : sy;
  const double dw = pix_->get_width() * s;
  const double dh = pix_->get_height() * s;
  const double ox = (w - dw) * 0.5;
  const double oy = (h - dh) * 0.5;
  Gdk::Cairo::set_source_pixbuf(cr, pix_, 0, 0);
  cr->save();
  cr->translate(ox, oy);
  cr->scale(s, s);
  cr->paint();
  cr->restore();
  return true;
}

}  // namespace tally
