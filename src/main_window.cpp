/* SPDX-License-Identifier: Unlicense */

#include "main_window.hpp"
#include "about_dialog.hpp"
#include "paths.hpp"
#include "x11_windows.hpp"

#include <gdk/gdk.h>

#include <algorithm>
#include <cstdlib>
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
  format_.append("mp4", "MP4 (H.264)");
  format_.append("mkv", "MKV (H.264)");
  format_.set_active_id("webm");
  fps_.append("5", "5");
  fps_.append("10", "10");
  fps_.append("15", "15");
  fps_.append("30", "30");
  fps_.set_active_id("10");
  fps_row_.pack_start(fps_lab_, Gtk::PACK_SHRINK);
  fps_row_.pack_start(fps_, Gtk::PACK_EXPAND_WIDGET);
  dest_.set_hexpand(true);
  dest_.set_tooltip_text("Default folder for new recordings");
  dest_row_.pack_start(dest_lab_, Gtk::PACK_SHRINK);
  dest_row_.pack_start(dest_, Gtk::PACK_EXPAND_WIDGET);
  hide_win_.set_active(true);
  hide_win_.set_tooltip_text(
      "Minimize Tally while recording. Stop from the floating Stop button, the taskbar, or Ctrl+.");
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
  right_.pack_start(fps_row_, Gtk::PACK_SHRINK);
  right_.pack_start(dest_row_, Gtk::PACK_SHRINK);
  right_.pack_start(hide_win_, Gtk::PACK_SHRINK);
  right_.pack_start(elapsed_, Gtk::PACK_SHRINK);

  well_.pack_start(left_, Gtk::PACK_EXPAND_WIDGET);
  well_.pack_start(right_, Gtk::PACK_SHRINK);

  btn_rec_.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_record));
  btn_stop_.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_stop));
  cap_.signal_stopped().connect(sigc::mem_fun(*this, &MainWindow::on_stopped));
  cap_.signal_error().connect(sigc::mem_fun(*this, &MainWindow::on_error));
  root_.pack_start(menubar_, Gtk::PACK_SHRINK);
  root_.pack_start(well_, Gtk::PACK_SHRINK);
  add(root_);
  settings_.load();
  mic_.set_active(settings_.mic);
  hide_win_.set_active(settings_.hide_window);
  if (known_format_id(settings_.format))
    format_.set_active_id(settings_.format);
  fps_.set_active_id(Glib::ustring::compose("%1", settings_.fps));
  fill_devices();
  sync_dest();
  persist_ok_ = true;
  /* Persist after widgets match the ini. Connecting earlier overwrote
   * device/format/fps with combo defaults on set_active. */
  mic_.signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::persist_audio));
  device_.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::persist_audio));
  format_.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::persist_capture));
  fps_.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::persist_capture));
  hide_win_.signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::persist_capture));
  dest_.signal_file_set().connect(sigc::mem_fun(*this, &MainWindow::on_dest_set));
  refresh_preview();
  show_all();
}

MainWindow::~MainWindow()
{
  picking_ = false;
  hidden_for_record_ = false;
  if (tick_.connected())
    tick_.disconnect();
  drop_picker();
  drop_chip();
  if (held_) {
    held_ = false;
    if (auto app = get_application())
      app->release();
  }
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
  add_item(*file, "Default _folder…", sigc::mem_fun(*this, &MainWindow::on_default_folder));
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
    if (pix && pix->get_width() > 400) {
      const int nh = std::max(1, pix->get_height() * 400 / pix->get_width());
      pix = pix->scale_simple(400, nh, Gdk::INTERP_BILINEAR);
    }
    preview_.set_pixbuf(pix);
  } catch (const Glib::Error&) {
    preview_.clear();
  }
}

std::string MainWindow::ext() const
{
  return format_ext(format_from_id(format_.get_active_id().raw()));
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
  if (!device_.get_active_id().empty())
    settings_.audio_device = selected_device().id;
  if (persist_ok_)
    settings_.save();
  device_.set_sensitive(mic_.get_active() && !cap_.running());
}

void MainWindow::persist_capture()
{
  const std::string fmt = format_.get_active_id().raw();
  if (known_format_id(fmt))
    settings_.format = fmt;
  settings_.fps = selected_fps();
  settings_.hide_window = hide_win_.get_active();
  if (!save_path_.empty()) {
    const std::string dir = Glib::path_get_dirname(save_path_);
    std::string base = Glib::path_get_basename(save_path_);
    const auto dot = base.rfind('.');
    if (dot != std::string::npos)
      base = base.substr(0, dot);
    save_path_ = Glib::build_filename(dir, base + "." + ext());
  }
  if (persist_ok_)
    settings_.save();
}

void MainWindow::persist_folder()
{
  if (!persist_ok_)
    return;
  settings_.save();
}

void MainWindow::sync_dest()
{
  dest_.set_current_folder(default_output_dir(settings_.last_folder));
}

void MainWindow::on_dest_set()
{
  const std::string dir = dest_.get_filename();
  if (dir.empty() || !Glib::file_test(dir, Glib::FILE_TEST_IS_DIR))
    return;
  settings_.last_folder = dir;
  save_path_.clear();
  persist_folder();
}

void MainWindow::on_default_folder()
{
  Gtk::FileChooserDialog dlg(*this, "Default folder for recordings",
                             Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  dlg.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dlg.add_button("_Select", Gtk::RESPONSE_ACCEPT);
  dlg.set_current_folder(default_output_dir(settings_.last_folder));
  if (dlg.run() != Gtk::RESPONSE_ACCEPT)
    return;
  const std::string dir = dlg.get_filename();
  if (dir.empty() || !Glib::file_test(dir, Glib::FILE_TEST_IS_DIR))
    return;
  settings_.last_folder = dir;
  save_path_.clear();
  persist_folder();
  sync_dest();
}

int MainWindow::selected_fps() const
{
  const int n = std::atoi(fps_.get_active_id().c_str());
  if (n == 5 || n == 10 || n == 15 || n == 30)
    return n;
  return 10;
}

CaptureOpts MainWindow::current_opts() const
{
  CaptureOpts o;
  o.rect = last_rect_.w >= 2 ? last_rect_ : full_screen();
  o.mic = mic_.get_active();
  o.audio = selected_device();
  o.format = format_from_id(format_.get_active_id().raw());
  o.fps = selected_fps();
  o.path = save_path_.empty() ? default_output_path(ext(), settings_.last_folder) : save_path_;
  return o;
}

void MainWindow::withdraw_main()
{
  if (!held_) {
    if (auto app = get_application()) {
      app->hold();
      held_ = true;
    }
  }
  hide();
}

void MainWindow::restore_main()
{
  show();
  present();
  if (held_) {
    if (auto app = get_application())
      app->release();
    held_ = false;
  }
}

void MainWindow::drop_picker()
{
  if (!picker_)
    return;
  picker_->hide();
  picker_->release_snapshot();
  if (auto app = get_application())
    app->remove_window(*picker_);
  delete picker_;
  picker_ = nullptr;
}

void MainWindow::drop_chip()
{
  if (!chip_)
    return;
  if (chip_mapped_.connected())
    chip_mapped_.disconnect();
  chip_->hide();
  delete chip_;
  chip_ = nullptr;
}

void MainWindow::start_pick(RegionPick::Mode mode)
{
  picking_ = true;
  withdraw_main();
  if (auto dpy = Gdk::Display::get_default())
    dpy->sync();
  Glib::signal_timeout().connect(
      [this, mode]() {
        if (auto dpy = Gdk::Display::get_default())
          dpy->sync();
        auto pix = snapshot_desktop();
        if (!pix) {
          picking_ = false;
          restore_main();
          status_.set_text("Could not snapshot the desktop");
          return false;
        }
        if (!picker_) {
          picker_ = new RegionPick();
          picker_->signal_picked().connect(sigc::mem_fun(*this, &MainWindow::on_region));
          picker_->signal_cancelled().connect(sigc::mem_fun(*this, &MainWindow::on_region_cancel));
        }
        std::vector<ClientWin> wins;
        if (mode == RegionPick::Mode::window) {
          try {
            wins = list_client_windows(window_xid(*this));
          } catch (const std::exception&) {
            wins.clear();
          }
        }
        picker_->begin(pix, mode, wins);
        picker_->show();
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
  begin_capture();
}

void MainWindow::on_region(Rect r)
{
  picking_ = false;
  drop_picker();
  last_rect_ = r;
  begin_capture();
  if (!hidden_for_record_)
    restore_main();
}

void MainWindow::on_region_cancel()
{
  picking_ = false;
  drop_picker();
  restore_main();
  status_.set_text("Cancelled");
}

void MainWindow::on_stop()
{
  cap_.stop();
  status_.set_text("Stopping…");
}

void MainWindow::begin_capture()
{
  persist_capture();
  auto opts = current_opts();
  save_path_ = opts.path;
  settings_.last_folder = Glib::path_get_dirname(save_path_);
  persist_folder();
  sync_dest();
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
  conceal_for_record();
}

void MainWindow::ensure_stop_chip()
{
  if (chip_)
    return;
  chip_ = new StopChip();
  chip_->signal_stop().connect(
      [this]() { Glib::signal_idle().connect_once([this]() { on_stop(); }); });
  chip_->signal_realize().connect([this]() {
    if (!chip_)
      return;
    if (auto gdk = chip_->get_window())
      gdk_window_set_group(gdk->gobj(), gdk->gobj());
  });
}

void MainWindow::conceal_for_record()
{
  if (!hide_win_.get_active())
    return;
  hidden_for_record_ = true;
  ensure_stop_chip();
  chip_->set_elapsed(elapsed_.get_text());
  if (!chip_mapped_.connected()) {
    chip_mapped_ = chip_->signal_map().connect([this]() {
      if (hidden_for_record_ && get_visible())
        withdraw_main();
    });
  }
  chip_->show_all();
  Glib::signal_idle().connect_once([this]() {
    if (chip_ && chip_->get_visible())
      chip_->place_corner();
    if (hidden_for_record_ && get_visible())
      withdraw_main();
  });
}

void MainWindow::reveal_after_record()
{
  hidden_for_record_ = false;
  if (chip_)
    chip_->hide();
  restore_main();
}

void MainWindow::on_stopped()
{
  if (tick_.connected())
    tick_.disconnect();
  set_lamp(false);
  sync_buttons();
  status_.set_text("Saved " + Glib::path_get_basename(cap_.path()));
  reveal_after_record();
}

void MainWindow::on_error(const Glib::ustring& msg)
{
  status_.set_text(msg);
  reveal_after_record();
}

bool MainWindow::on_tick()
{
  ++seconds_;
  const Glib::ustring t = mmss(seconds_);
  elapsed_.set_text(t);
  if (chip_ && chip_->get_visible())
    chip_->set_elapsed(t);
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
  fps_.set_sensitive(!run);
  dest_.set_sensitive(!run);
  hide_win_.set_sensitive(!run);
}

void MainWindow::on_save_as()
{
  Gtk::FileChooserDialog dlg(*this, "Save recording as", Gtk::FILE_CHOOSER_ACTION_SAVE);
  dlg.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dlg.add_button("_Save", Gtk::RESPONSE_ACCEPT);
  dlg.set_do_overwrite_confirmation(true);
  if (!settings_.last_folder.empty() &&
      Glib::file_test(settings_.last_folder, Glib::FILE_TEST_IS_DIR))
    dlg.set_current_folder(settings_.last_folder);
  dlg.set_current_name(Glib::path_get_basename(default_output_path(ext(), settings_.last_folder)));
  if (dlg.run() != Gtk::RESPONSE_ACCEPT)
    return;
  save_path_ = dlg.get_filename();
  settings_.last_folder = Glib::path_get_dirname(save_path_);
  persist_capture();
  sync_dest();
  status_.set_text("Next capture: " + Glib::path_get_basename(save_path_));
}

void MainWindow::on_quit()
{
  picking_ = false;
  hidden_for_record_ = false;
  if (tick_.connected())
    tick_.disconnect();
  drop_chip();
  drop_picker();
  if (cap_.running())
    cap_.kill_now();
  if (held_) {
    if (auto app = get_application())
      app->release();
    held_ = false;
  }
  /* GtkApplication::quit() does not return from run() while a window still
   * exists; hide-to-delete fights Hide-while-recording. End the process
   * after ffmpeg is reaped. */
  std::exit(0);
}

bool MainWindow::on_delete_event(GdkEventAny*)
{
  on_quit();
  return true;
}

void MainWindow::on_about()
{
  AboutDialog dlg(*this);
  dlg.run();
}

}  // namespace tally
