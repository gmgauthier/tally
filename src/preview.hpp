/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <gtkmm.h>

namespace tally {

class Preview : public Gtk::DrawingArea {
 public:
  Preview();
  void set_pixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pix);
  void clear();

 protected:
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

 private:
  Glib::RefPtr<Gdk::Pixbuf> pix_;
};

}  // namespace tally
