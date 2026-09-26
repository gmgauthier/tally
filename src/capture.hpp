/* SPDX-License-Identifier: Unlicense */

#pragma once

#include "audio_devices.hpp"
#include "region_pick.hpp"

#include <glibmm/ustring.h>
#include <sigc++/signal.h>
#include <glibmm.h>

#include <string>
#include <vector>

namespace tally {

enum class Format { webm, avi };

struct CaptureOpts {
  Rect rect;
  bool mic = false;
  AudioDevice audio;
  Format format = Format::webm;
  int fps = 10;
  std::string path;
};

class Capture {
 public:
  Capture();
  ~Capture();

  bool running() const
  {
    return running_;
  }
  const std::string& path() const
  {
    return path_;
  }

  bool start(const CaptureOpts& opts);
  void stop();

  sigc::signal<void>& signal_stopped()
  {
    return signal_stopped_;
  }
  sigc::signal<void, Glib::ustring>& signal_error()
  {
    return signal_error_;
  }

 private:
  void reap();
  void on_child(GPid pid, int status);
  std::vector<std::string> build_argv(const CaptureOpts& opts) const;

  bool running_ = false;
  Glib::Pid pid_ = 0;
  int stdin_fd_ = -1;
  std::string path_;
  sigc::connection child_;
  sigc::signal<void> signal_stopped_;
  sigc::signal<void, Glib::ustring> signal_error_;
};

}  // namespace tally
