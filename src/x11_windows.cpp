/* SPDX-License-Identifier: Unlicense */

#include "x11_windows.hpp"

#include <gdk/gdkx.h>
#include <glib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <algorithm>
#include <cstring>
#include <exception>
#include <string>

namespace tally {
namespace {

Glib::ustring utf8_from_bytes(const char* p, unsigned long nbytes)
{
  if (!p || nbytes == 0)
    return {};
  if (nbytes > 2048)
    nbytes = 2048;
  std::string raw(p, static_cast<std::size_t>(nbytes));
  while (!raw.empty() && raw.back() == '\0')
    raw.pop_back();
  if (raw.empty())
    return {};
  if (!g_utf8_validate(raw.data(), static_cast<gssize>(raw.size()), nullptr)) {
    gchar* valid = g_utf8_make_valid(raw.data(), static_cast<gssize>(raw.size()));
    Glib::ustring t(valid ? valid : "");
    g_free(valid);
    return t;
  }
  return Glib::ustring(raw.c_str());
}

Glib::ustring window_title(Display* dpy, Window w)
{
  Atom net = XInternAtom(dpy, "_NET_WM_NAME", True);
  Atom utf8 = XInternAtom(dpy, "UTF8_STRING", True);
  if (net != None && utf8 != None) {
    Atom actual = None;
    int fmt = 0;
    unsigned long n = 0;
    unsigned long bytes = 0;
    unsigned char* data = nullptr;
    if (XGetWindowProperty(dpy, w, net, 0, 256, False, utf8, &actual, &fmt, &n, &bytes, &data) ==
            Success &&
        data && n > 0 && fmt == 8) {
      Glib::ustring t = utf8_from_bytes(reinterpret_cast<char*>(data), n);
      XFree(data);
      if (!t.empty())
        return t;
    } else if (data) {
      XFree(data);
    }
  }
  char* name = nullptr;
  if (XFetchName(dpy, w, &name) && name) {
    Glib::ustring t = utf8_from_bytes(name, std::strlen(name));
    XFree(name);
    return t;
  }
  return {};
}

bool skip_type(Display* dpy, Window w)
{
  Atom type_atom = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", True);
  if (type_atom == None)
    return false;
  Atom actual = None;
  int fmt = 0;
  unsigned long n = 0;
  unsigned long bytes = 0;
  unsigned char* data = nullptr;
  if (XGetWindowProperty(dpy, w, type_atom, 0, 16, False, XA_ATOM, &actual, &fmt, &n, &bytes,
                         &data) != Success ||
      !data) {
    if (data)
      XFree(data);
    return false;
  }
  const Atom dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", True);
  const Atom desktop = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", True);
  const Atom notify = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NOTIFICATION", True);
  const Atom splash = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_SPLASH", True);
  const auto* atoms = reinterpret_cast<Atom*>(data);
  bool skip = false;
  for (unsigned long i = 0; i < n; ++i) {
    if (atoms[i] == dock || atoms[i] == desktop || atoms[i] == notify || atoms[i] == splash)
      skip = true;
  }
  XFree(data);
  return skip;
}

void add_frame_extents(Display* dpy, Window w, Rect& r)
{
  Atom atom = XInternAtom(dpy, "_NET_FRAME_EXTENTS", True);
  if (atom == None)
    return;
  Atom actual = None;
  int fmt = 0;
  unsigned long n = 0;
  unsigned long bytes = 0;
  unsigned char* data = nullptr;
  if (XGetWindowProperty(dpy, w, atom, 0, 4, False, XA_CARDINAL, &actual, &fmt, &n, &bytes,
                         &data) != Success ||
      !data || n < 4) {
    if (data)
      XFree(data);
    return;
  }
  const auto* e = reinterpret_cast<long*>(data);
  r.x -= static_cast<int>(e[0]);
  r.y -= static_cast<int>(e[2]);
  r.w += static_cast<int>(e[0] + e[1]);
  r.h += static_cast<int>(e[2] + e[3]);
  XFree(data);
}

}  // namespace

unsigned long window_xid(const Gtk::Window& w)
{
  auto gdk = const_cast<Gtk::Window&>(w).get_window();
  if (!gdk)
    return 0;
  return gdk_x11_window_get_xid(gdk->gobj());
}

Glib::RefPtr<Gdk::Pixbuf> snapshot_desktop()
{
  auto screen = Gdk::Screen::get_default();
  if (!screen)
    return {};
  auto root = screen->get_root_window();
  if (!root)
    return {};
  const int w = screen->get_width();
  const int h = screen->get_height();
  if (w < 2 || h < 2)
    return {};
  try {
    return Gdk::Pixbuf::create(root, 0, 0, w, h);
  } catch (const Glib::Error&) {
    return {};
  }
}

std::vector<ClientWin> list_client_windows(unsigned long skip_xid)
{
  std::vector<ClientWin> out;
  GdkDisplay* gd = gdk_display_get_default();
  GdkScreen* gs = gdk_screen_get_default();
  if (!gd || !gs)
    return out;
  Display* dpy = gdk_x11_display_get_xdisplay(gd);
  Window root = gdk_x11_window_get_xid(gdk_screen_get_root_window(gs));
  Atom atom = XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", True);
  if (atom == None)
    atom = XInternAtom(dpy, "_NET_CLIENT_LIST", True);
  if (atom == None)
    return out;

  Atom actual = None;
  int fmt = 0;
  unsigned long n = 0;
  unsigned long bytes = 0;
  unsigned char* data = nullptr;
  if (XGetWindowProperty(dpy, root, atom, 0, 4096, False, XA_WINDOW, &actual, &fmt, &n, &bytes,
                         &data) != Success ||
      !data) {
    if (data)
      XFree(data);
    return out;
  }
  const unsigned long count = std::min(n, 512UL);
  const auto* wins = reinterpret_cast<Window*>(data);
  for (unsigned long i = 0; i < count; ++i) {
    try {
      if (skip_xid && wins[i] == skip_xid)
        continue;
      if (skip_type(dpy, wins[i]))
        continue;
      XWindowAttributes attr;
      std::memset(&attr, 0, sizeof(attr));
      if (!XGetWindowAttributes(dpy, wins[i], &attr) || attr.map_state != IsViewable)
        continue;
      Window child = None;
      int x = 0;
      int y = 0;
      XTranslateCoordinates(dpy, wins[i], root, 0, 0, &x, &y, &child);
      ClientWin cw;
      cw.xid = wins[i];
      cw.r.x = x;
      cw.r.y = y;
      cw.r.w = attr.width;
      cw.r.h = attr.height;
      add_frame_extents(dpy, wins[i], cw.r);
      if (cw.r.w < 32 || cw.r.h < 32)
        continue;
      cw.title = window_title(dpy, wins[i]);
      if (cw.title.empty())
        cw.title = Glib::ustring::compose("%1x%2", cw.r.w, cw.r.h);
      out.push_back(cw);
    } catch (const std::exception&) {
      continue;
    }
  }
  XFree(data);
  return out;
}

}  // namespace tally
