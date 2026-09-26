/* SPDX-License-Identifier: Unlicense */

#pragma once

#include "audio_devices.hpp"
#include "capture.hpp"
#include "preview.hpp"
#include "region_pick.hpp"
#include "settings.hpp"
#include "stop_chip.hpp"

#include <gtkmm.h>

#include <vector>

namespace tally {

class MainWindow : public Gtk::Window {
 public:
  MainWindow();
  ~MainWindow() override;
  bool keep_alive() const
  {
    return picking_ || hidden_for_record_;
  }

 protected:
  bool on_delete_event(GdkEventAny* event) override;

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
  void withdraw_main();
  void restore_main();
  void drop_picker();
  void drop_chip();
  CaptureOpts current_opts() const;
  std::string ext() const;
  void fill_devices();
  void persist_audio();
  void persist_capture();
  void begin_capture();
  void conceal_for_record();
  void reveal_after_record();
  void ensure_stop_chip();
  AudioDevice selected_device() const;
  int selected_fps() const;

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
  Gtk::Box fps_row_{Gtk::ORIENTATION_HORIZONTAL, 6};
  Gtk::Label fps_lab_{"FPS"};
  Gtk::ComboBoxText fps_;
  Gtk::CheckButton hide_win_{"Hide this window"};
  Gtk::Label elapsed_{"0:00"};
  Gtk::Label status_{"Ready"};
  Glib::RefPtr<Gtk::AccelGroup> accel_;
  Capture cap_;
  Settings settings_;
  std::vector<AudioDevice> devices_;
  RegionPick* picker_ = nullptr;
  StopChip* chip_ = nullptr;
  Rect last_rect_{};
  std::string save_path_;
  sigc::connection tick_;
  int seconds_ = 0;
  bool picking_ = false;
  bool hidden_for_record_ = false;
};

}  // namespace tally
