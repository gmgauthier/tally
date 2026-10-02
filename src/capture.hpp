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

enum class Format { webm, avi, mp4, mkv };

inline Format format_from_id(const std::string& id)
{
  if (id == "avi")
    return Format::avi;
  if (id == "mp4")
    return Format::mp4;
  if (id == "mkv")
    return Format::mkv;
  return Format::webm;
}

inline const char* format_id(Format f)
{
  switch (f) {
    case Format::avi:
      return "avi";
    case Format::mp4:
      return "mp4";
    case Format::mkv:
      return "mkv";
    case Format::webm:
    default:
      return "webm";
  }
}

inline const char* format_ext(Format f)
{
  return format_id(f);
}

inline bool known_format_id(const std::string& id)
{
  return id == "webm" || id == "avi" || id == "mp4" || id == "mkv";
}

struct CaptureOpts {
  Rect rect;
  int screen_w = 0; /* X root size; 0 means unknown */
  int screen_h = 0;
  bool mic = false;
  AudioDevice audio;
  Format format = Format::webm;
  int fps = 10;
  std::string path;
};

/* RECT limited to the X root (SW x SH). A dimension of 0 means unknown and is not limited. */
Rect clip_to_screen(Rect rect, int sw, int sh);

/* ffmpeg argv for OPTS on X display DISPLAY. */
std::vector<std::string> ffmpeg_argv(const CaptureOpts& opts, const std::string& display);

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
  void kill_now();

  /* Emitted when ffmpeg exits. The flag is true only for a clean exit (status 0). */
  sigc::signal<void, bool>& signal_stopped()
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
  sigc::signal<void, bool> signal_stopped_;
  sigc::signal<void, Glib::ustring> signal_error_;
};

}  // namespace tally
