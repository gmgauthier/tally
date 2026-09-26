/* SPDX-License-Identifier: Unlicense */

#include "main_window.hpp"
#include "about_dialog.hpp"
#include "paths.hpp"
#include "x11_windows.hpp"

#include <gdk/gdk.h>

#include <cstdio>
#include <iostream>

namespace tally {
namespace {

Glib::ustring mmss(int seconds)
{
  if (seconds < 0)
    seconds = 0;
  char buf[16];
  std::snprintf(buf, sizeof(buf), "%d:%02d", seconds / 60, seconds % 60);
  return buf;
}

}  // namespace

MainWindow::MainWindow()
{
  set_title("Tally");
  set_resizable(false);
  set_border_width(0);
  get_style_context()->add_class("tally-window");

  accel_ = Gtk::AccelGroup::create();
  add_accel_group(accel_);

  load_css();
  build_menu();

  last_rect_ = full_screen();

  lamp_.set_size_request(22, 22);
  lamp_.set_visible_window(true);
  lamp_.get_style_context()->add_class("tally-lamp-off");
  btn_rec_.get_style_context()->add_class("tally-record");
  btn_rec_.set_size_request(110, 32);
  btn_stop_.set_size_request(110, 32);
  btn_stop_.set_sensitive(false);

  src_region_.join_group(src_full_);
  src_window_.join_group(src_full_);
  src_full_.set_active(true);

  format_.append("webm", "WebM (VP8)");
  format_.append("avi", "AVI (MJPEG)");
  format_.set_active_id("webm");
  device_.set_hexpand(true);

  left_.set_border_width(8);
  left_.pack_start(preview_, Gtk::PACK_SHRINK);
  left_.pack_start(status_, Gtk::PACK_SHRINK);

  auto* src = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 2));
  src->pack_start(src_full_, Gtk::PACK_SHRINK);
  src->pack_start(src_region_, Gtk::PACK_SHRINK);
  src->pack_start(src_window_, Gtk::PACK_SHRINK);

  right_.set_border_width(8);
  right_.pack_start(lamp_, Gtk::PACK_SHRINK);
  right_.pack_start(btn_rec_, Gtk::PACK_SHRINK);
  right_.pack_start(btn_stop_, Gtk::PACK_SHRINK);
  right_.pack_start(*src, Gtk::PACK_SHRINK);
  right_.pack_start(mic_, Gtk::PACK_SHRINK);
  right_.pack_start(device_, Gtk::PACK_SHRINK);
  right_.pack_start(format_, Gtk::PACK_SHRINK);
  right_.pack_start(elapsed_, Gtk::PACK_SHRINK);

  well_.pack_start(left_, Gtk::PACK_EXPAND_WIDGET);
  well_.pack_start(right_, Gtk::PACK_SHRINK);

  btn_rec_.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_record));
  btn_stop_.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_stop));
  cap_.signal_stopped().connect(sigc::mem_fun(*this, &MainWindow::on_stopped));
  cap_.signal_error().connect(sigc::mem_fun(*this, &MainWindow::on_error));
  mic_.signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::persist_audio));
  device_.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::persist_audio));

  root_.pack_start(menubar_, Gtk::PACK_SHRINK);
  root_.pack_start(well_, Gtk::PACK_SHRINK);
  add(root_);
  settings_.load();
  mic_.set_active(settings_.mic);
  fill_devices();
  refresh_preview();
  show_all();
}

MainWindow::~MainWindow()
{
  if (tick_.connected())
    tick_.disconnect();
  delete picker_;
}

void MainWindow::load_css()
{
  const std::string css_path = find_data_file("skin/lcos/lcos.css");
  if (css_path.empty()) {
    std::cerr << "tally: lcos.css not found\n";
    return;
  }
  try {
    auto css = Gtk::CssProvider::create();
    css->load_from_path(css_path);
    Gtk::StyleContext::add_provider_for_screen(Gdk::Screen::get_default(), css,
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  } catch (const Glib::Error& e) {
    std::cerr << "tally: CSS: " << e.what() << "\n";
  }
}

Gtk::MenuItem* MainWindow::add_item(Gtk::Menu& menu, const Glib::ustring& label,
                                    const sigc::slot<void()>& slot, guint key,
                                    Gdk::ModifierType mods)
{
  auto* item = Gtk::manage(new Gtk::MenuItem(label, true));
  item->signal_activate().connect(slot);
  if (key != 0)
    item->add_accelerator("activate", accel_, key, mods, Gtk::ACCEL_VISIBLE);
  menu.append(*item);
  return item;
}

void MainWindow::build_menu()
{
  auto add_menu = [this](const Glib::ustring& label, Gtk::Menu& menu) {
    auto* top = Gtk::manage(new Gtk::MenuItem(label, true));
    top->set_submenu(menu);
    menubar_.append(*top);
  };

  auto* file = Gtk::manage(new Gtk::Menu());
  add_item(*file, "Save _As…", sigc::mem_fun(*this, &MainWindow::on_save_as));
  file->append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
  add_item(*file, "E_xit", sigc::mem_fun(*this, &MainWindow::on_quit));
  add_menu("_File", *file);

  auto* cap = Gtk::manage(new Gtk::Menu());
  add_item(*cap, "_Record", sigc::mem_fun(*this, &MainWindow::on_record), GDK_KEY_r,
           Gdk::CONTROL_MASK);
  add_item(*cap, "_Stop", sigc::mem_fun(*this, &MainWindow::on_stop), GDK_KEY_period,
           Gdk::CONTROL_MASK);
  add_menu("_Capture", *cap);

  auto* help = Gtk::manage(new Gtk::Menu());
  add_item(*help, "_About Tally", sigc::mem_fun(*this, &MainWindow::on_about));
  add_menu("_Help", *help);
}

Rect MainWindow::full_screen() const
{
  Rect r;
  auto screen = Gdk::Screen::get_default();
  if (!screen)
    return r;
  r.x = 0;
  r.y = 0;
  r.w = screen->get_width();
  r.h = screen->get_height();
  return r;
}

void MainWindow::refresh_preview()
{
  auto screen = Gdk::Screen::get_default();
  if (!screen)
    return;
  auto root = screen->get_root_window();
  if (!root)
    return;
  Rect r = last_rect_;
  if (r.w < 2 || r.h < 2)
    r = full_screen();
  int sw = root->get_width();
  int sh = root->get_height();
  if (r.x < 0)
    r.x = 0;
  if (r.y < 0)
    r.y = 0;
  if (r.x + r.w > sw)
    r.w = sw - r.x;
  if (r.y + r.h > sh)
    r.h = sh - r.y;
  if (r.w < 2 || r.h < 2)
    return;
  try {
    auto pix = Gdk::Pixbuf::create(root, r.x, r.y, r.w, r.h);
    preview_.set_pixbuf(pix);
  } catch (const Glib::Error&) {
    preview_.clear();
  }
}

std::string MainWindow::ext() const
{
  return format_.get_active_id() == "avi" ? "avi" : "webm";
}

void MainWindow::fill_devices()
{
  devices_ = list_audio_inputs();
  const std::string keep =
      device_.get_active_id().empty() ? settings_.audio_device : device_.get_active_id().raw();
  device_.remove_all();
  for (const auto& d : devices_)
    device_.append(d.id, d.label);
  device_.set_active_id(pick_audio_device(devices_, keep));
  persist_audio();
}

AudioDevice MainWindow::selected_device() const
{
  const std::string id = device_.get_active_id().raw();
  for (const auto& d : devices_) {
    if (d.id == id)
      return d;
  }
  AudioDevice def;
  def.id = "default";
  def.label = "Default";
  def.backend = AudioBackend::system_default;
  return def;
}

void MainWindow::persist_audio()
{
  settings_.mic = mic_.get_active();
  settings_.audio_device = selected_device().id;
  settings_.save();
  device_.set_sensitive(mic_.get_active() && !cap_.running());
}

CaptureOpts MainWindow::current_opts() const
{
  CaptureOpts o;
  o.rect = last_rect_.w >= 2 ? last_rect_ : full_screen();
  o.mic = mic_.get_active();
  o.audio = selected_device();
  o.format = format_.get_active_id() == "avi" ? Format::avi : Format::webm;
  o.path = save_path_.empty() ? default_output_path(ext()) : save_path_;
  return o;
}

void MainWindow::start_pick(RegionPick::Mode mode)
{
  hide();
  if (auto dpy = Gdk::Display::get_default())
    dpy->sync();
  Glib::signal_timeout().connect(
      [this, mode]() {
        if (auto dpy = Gdk::Display::get_default())
          dpy->sync();
        auto pix = snapshot_desktop();
        if (!pix) {
          present();
          status_.set_text("Could not snapshot the desktop");
          return false;
        }
        if (!picker_) {
          picker_ = new RegionPick();
          picker_->signal_picked().connect(sigc::mem_fun(*this, &MainWindow::on_region));
          picker_->signal_cancelled().connect(sigc::mem_fun(*this, &MainWindow::on_region_cancel));
        }
        std::vector<ClientWin> wins;
        if (mode == RegionPick::Mode::window)
          wins = list_client_windows(window_xid(*this));
        picker_->begin(pix, mode, wins);
        picker_->present();
        picker_->grab_focus();
        if (auto gdk = picker_->get_window()) {
          auto cur = Gdk::Cursor::create(gdk->get_display(), Gdk::CROSSHAIR);
          gdk->set_cursor(cur);
        }
        if (mode == RegionPick::Mode::window && wins.empty())
          status_.set_text("No windows to pick");
        return false;
      },
      80);
}

void MainWindow::on_record()
{
  if (cap_.running())
    return;
  fill_devices();
  if (src_full_.get_active()) {
    last_rect_ = full_screen();
  } else if (src_region_.get_active()) {
    start_pick(RegionPick::Mode::region);
    return;
  } else if (src_window_.get_active()) {
    start_pick(RegionPick::Mode::window);
    return;
  }
  auto opts = current_opts();
  save_path_ = opts.path;
  refresh_preview();
  if (!cap_.start(opts))
    return;
  seconds_ = 0;
  elapsed_.set_text("0:00");
  set_lamp(true);
  sync_buttons();
  status_.set_text("Recording " + Glib::path_get_basename(opts.path));
  if (tick_.connected())
    tick_.disconnect();
  tick_ = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::on_tick), 1000);
}

void MainWindow::on_region(Rect r)
{
  present();
  last_rect_ = r;
  auto opts = current_opts();
  save_path_ = opts.path;
  refresh_preview();
  if (!cap_.start(opts))
    return;
  seconds_ = 0;
  elapsed_.set_text("0:00");
  set_lamp(true);
  sync_buttons();
  status_.set_text("Recording " + Glib::path_get_basename(opts.path));
  if (tick_.connected())
    tick_.disconnect();
  tick_ = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::on_tick), 1000);
}

void MainWindow::on_region_cancel()
{
  present();
  status_.set_text("Cancelled");
}

void MainWindow::on_stop()
{
  cap_.stop();
  status_.set_text("Stopping…");
}

void MainWindow::on_stopped()
{
  if (tick_.connected())
    tick_.disconnect();
  set_lamp(false);
  sync_buttons();
  status_.set_text("Saved " + Glib::path_get_basename(cap_.path()));
}

void MainWindow::on_error(const Glib::ustring& msg)
{
  status_.set_text(msg);
}

bool MainWindow::on_tick()
{
  ++seconds_;
  elapsed_.set_text(mmss(seconds_));
  return true;
}

void MainWindow::set_lamp(bool on)
{
  auto ctx = lamp_.get_style_context();
  ctx->remove_class("tally-lamp-on");
  ctx->remove_class("tally-lamp-off");
  ctx->add_class(on ? "tally-lamp-on" : "tally-lamp-off");
}

void MainWindow::sync_buttons()
{
  const bool run = cap_.running();
  btn_rec_.set_sensitive(!run);
  btn_stop_.set_sensitive(run);
  src_full_.set_sensitive(!run);
  src_region_.set_sensitive(!run);
  src_window_.set_sensitive(!run);
  mic_.set_sensitive(!run);
  device_.set_sensitive(!run && mic_.get_active());
  format_.set_sensitive(!run);
}

void MainWindow::on_save_as()
{
  Gtk::FileChooserDialog dlg(*this, "Save recording as", Gtk::FILE_CHOOSER_ACTION_SAVE);
  dlg.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dlg.add_button("_Save", Gtk::RESPONSE_ACCEPT);
  dlg.set_do_overwrite_confirmation(true);
  dlg.set_current_name(Glib::path_get_basename(default_output_path(ext())));
  if (dlg.run() != Gtk::RESPONSE_ACCEPT)
    return;
  save_path_ = dlg.get_filename();
  status_.set_text("Next capture: " + Glib::path_get_basename(save_path_));
}

void MainWindow::on_quit()
{
  if (cap_.running())
    cap_.stop();
  hide();
}

void MainWindow::on_about()
{
  AboutDialog dlg(*this);
  dlg.run();
}

}  // namespace tally
