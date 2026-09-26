/* SPDX-License-Identifier: Unlicense */

#pragma once

#include "audio_devices.hpp"
#include "capture.hpp"
#include "preview.hpp"
#include "region_pick.hpp"
#include "settings.hpp"

#include <gtkmm.h>

#include <vector>

namespace tally {

class MainWindow : public Gtk::Window {
 public:
  MainWindow();
  ~MainWindow() override;

 private:
  void load_css();
  void build_menu();
  void on_quit();
  void on_about();
  void on_record();
  void on_stop();
  void on_save_as();
  void on_stopped();
  void on_error(const Glib::ustring& msg);
  void on_region(Rect r);
  void on_region_cancel();
  bool on_tick();
  void set_lamp(bool on);
  void sync_buttons();
  void refresh_preview();
  Rect full_screen() const;
  void start_pick(RegionPick::Mode mode);
  CaptureOpts current_opts() const;
  std::string ext() const;
  void fill_devices();
  void persist_audio();
  AudioDevice selected_device() const;

  Gtk::MenuItem* add_item(Gtk::Menu& menu, const Glib::ustring& label,
                          const sigc::slot<void()>& slot, guint key = 0,
                          Gdk::ModifierType mods = Gdk::ModifierType(0));

  Gtk::Box root_{Gtk::ORIENTATION_VERTICAL, 0};
  Gtk::MenuBar menubar_;
  Gtk::Box well_{Gtk::ORIENTATION_HORIZONTAL, 10};
  Gtk::Box left_{Gtk::ORIENTATION_VERTICAL, 6};
  Gtk::Box right_{Gtk::ORIENTATION_VERTICAL, 6};
  Preview preview_;
  Gtk::EventBox lamp_;
  Gtk::Button btn_rec_{"Record"};
  Gtk::Button btn_stop_{"Stop"};
  Gtk::RadioButton src_full_{"Full screen"};
  Gtk::RadioButton src_region_{"Region"};
  Gtk::RadioButton src_window_{"Window"};
  Gtk::CheckButton mic_{"Microphone"};
  Gtk::ComboBoxText device_;
  Gtk::ComboBoxText format_;
  Gtk::Label elapsed_{"0:00"};
  Gtk::Label status_{"Ready"};
  Glib::RefPtr<Gtk::AccelGroup> accel_;
  Capture cap_;
  Settings settings_;
  std::vector<AudioDevice> devices_;
  RegionPick* picker_ = nullptr;
  Rect last_rect_{};
  std::string save_path_;
  sigc::connection tick_;
  int seconds_ = 0;
};

}  // namespace tally
