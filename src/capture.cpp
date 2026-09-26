/* SPDX-License-Identifier: Unlicense */

#include "capture.hpp"

#include <glib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cstdio>
#include <sstream>
#include <string>

namespace tally {
namespace {

void ffmpeg_child_setup()
{
  /* If Tally dies, ffmpeg must not keep grabbing the desktop into RAM. */
  prctl(PR_SET_PDEATHSIG, SIGTERM);
  setpgid(0, 0);
}

int even(int v)
{
  if (v < 2)
    return 2;
  return v & ~1;
}

std::string display_spec()
{
  const char* d = g_getenv("DISPLAY");
  return d && d[0] ? d : ":0";
}

}  // namespace

Capture::Capture() = default;

Capture::~Capture()
{
  if (child_.connected())
    child_.disconnect();
  stop();
  reap();
}

std::vector<std::string> Capture::build_argv(const CaptureOpts& opts) const
{
  const int w = even(opts.rect.w);
  const int h = even(opts.rect.h);
  const int x = opts.rect.x < 0 ? 0 : opts.rect.x;
  const int y = opts.rect.y < 0 ? 0 : opts.rect.y;
  std::ostringstream size;
  size << w << "x" << h;
  std::ostringstream src;
  src << display_spec() << "+" << x << "," << y;

  std::vector<std::string> argv;
  argv.emplace_back("ffmpeg");
  argv.emplace_back("-hide_banner");
  argv.emplace_back("-loglevel");
  argv.emplace_back("error");
  argv.emplace_back("-y");
  argv.emplace_back("-f");
  argv.emplace_back("x11grab");
  int fps = opts.fps;
  if (fps != 5 && fps != 10 && fps != 15 && fps != 30)
    fps = 10;
  argv.emplace_back("-framerate");
  argv.emplace_back(std::to_string(fps));
  argv.emplace_back("-video_size");
  argv.emplace_back(size.str());
  argv.emplace_back("-i");
  argv.emplace_back(src.str());
  if (opts.mic) {
    if (opts.audio.backend == AudioBackend::alsa && !opts.audio.id.empty() &&
        opts.audio.id != "default") {
      argv.emplace_back("-f");
      argv.emplace_back("alsa");
      argv.emplace_back("-i");
      argv.emplace_back(opts.audio.id);
    } else {
      argv.emplace_back("-f");
      argv.emplace_back("pulse");
      argv.emplace_back("-i");
      argv.emplace_back(opts.audio.id.empty() || opts.audio.id == "default" ? "default"
                                                                            : opts.audio.id);
    }
  }
  if (opts.format == Format::avi) {
    argv.emplace_back("-c:v");
    argv.emplace_back("mjpeg");
    argv.emplace_back("-q:v");
    argv.emplace_back("5");
    if (opts.mic) {
      argv.emplace_back("-c:a");
      argv.emplace_back("pcm_s16le");
    }
    argv.emplace_back("-f");
    argv.emplace_back("avi");
  } else {
    argv.emplace_back("-c:v");
    argv.emplace_back("libvpx");
    argv.emplace_back("-deadline");
    argv.emplace_back("realtime");
    argv.emplace_back("-cpu-used");
    argv.emplace_back("8");
    argv.emplace_back("-b:v");
    argv.emplace_back("1M");
    if (opts.mic) {
      argv.emplace_back("-c:a");
      argv.emplace_back("libvorbis");
    }
    argv.emplace_back("-f");
    argv.emplace_back("webm");
  }
  argv.push_back(opts.path);
  return argv;
}

bool Capture::start(const CaptureOpts& opts)
{
  if (running_)
    return true;
  if (opts.path.empty() || opts.rect.w < 2 || opts.rect.h < 2) {
    signal_error_.emit("Nothing to capture");
    return false;
  }
  if (Glib::find_program_in_path("ffmpeg").empty()) {
    signal_error_.emit("ffmpeg is not on PATH");
    return false;
  }
  const auto argv = build_argv(opts);
  {
    std::string line = "tally: ffmpeg";
    for (const auto& a : argv)
      line += " " + a;
    g_message("%s", line.c_str());
  }
  path_ = opts.path;
  stdin_fd_ = -1;
  pid_ = 0;
  try {
    Glib::spawn_async_with_pipes(
        std::string(), argv, Glib::SPAWN_SEARCH_PATH | Glib::SPAWN_DO_NOT_REAP_CHILD,
        sigc::ptr_fun(&ffmpeg_child_setup), &pid_, &stdin_fd_, nullptr, nullptr);
  } catch (const Glib::Error& e) {
    signal_error_.emit(e.what());
    return false;
  }
  running_ = true;
  child_ = Glib::signal_child_watch().connect(sigc::mem_fun(*this, &Capture::on_child), pid_);
  return true;
}

void Capture::stop()
{
  if (!running_ || pid_ <= 0)
    return;
  if (stdin_fd_ >= 0) {
    const char q[] = "q\n";
    if (write(stdin_fd_, q, 2) < 0)
      kill(-pid_, SIGINT);
    close(stdin_fd_);
    stdin_fd_ = -1;
  } else {
    kill(-pid_, SIGINT);
  }
}

void Capture::kill_now()
{
  reap();
}

void Capture::on_child(GPid pid, int status)
{
  if (pid != pid_)
    return;
  running_ = false;
  if (stdin_fd_ >= 0) {
    close(stdin_fd_);
    stdin_fd_ = -1;
  }
  Glib::spawn_close_pid(pid_);
  pid_ = 0;
  child_.disconnect();
  if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
    signal_error_.emit("ffmpeg exited with an error");
  signal_stopped_.emit();
}

void Capture::reap()
{
  if (child_.connected())
    child_.disconnect();
  if (pid_ > 0) {
    kill(-pid_, SIGTERM);
    waitpid(pid_, nullptr, 0);
    Glib::spawn_close_pid(pid_);
    pid_ = 0;
  }
  if (stdin_fd_ >= 0) {
    close(stdin_fd_);
    stdin_fd_ = -1;
  }
  running_ = false;
}

}  // namespace tally
