/* SPDX-License-Identifier: Unlicense */

#pragma once

#include "region_pick.hpp"

#include <gtkmm.h>

#include <string>
#include <vector>

namespace tally {

std::vector<ClientWin> list_client_windows(unsigned long skip_xid);
unsigned long window_xid(const Gtk::Window& w);
Glib::RefPtr<Gdk::Pixbuf> snapshot_desktop();

}  // namespace tally
