/* SPDX-License-Identifier: Unlicense */

#include "capture.hpp"
#include "check.hpp"

#include <glibmm.h>
#include <sys/stat.h>

#include <cstdlib>
#include <fstream>
#include <string>

namespace {

std::string g_bin;

/* Puts a stand-in ffmpeg first on PATH. */
void fake_ffmpeg(const std::string& body)
{
  const std::string path = g_bin + "/ffmpeg";
  std::ofstream(path) << "#!/bin/sh\n" << body << "\n";
  ::chmod(path.c_str(), 0755);
}

tally::CaptureOpts opts(const std::string& out)
{
  tally::CaptureOpts o;
  o.rect = tally::Rect{0, 0, 64, 48};
  o.path = out;
  return o;
}

struct Outcome {
  bool stopped = false;
  bool saved = false;
  int errors = 0;
};

/* Runs one take; STOP_AFTER_MS > 0 presses Stop after that long. */
Outcome run_take(const std::string& out, int stop_after_ms)
{
  Outcome res;
  auto loop = Glib::MainLoop::create();
  tally::Capture cap;
  cap.signal_error().connect([&](const Glib::ustring&) { ++res.errors; });
  cap.signal_stopped().connect([&](bool saved) {
    res.stopped = true;
    res.saved = saved;
    loop->quit();
  });
  if (!cap.start(opts(out)))
    return res;
  if (stop_after_ms > 0)
    Glib::signal_timeout().connect_once([&]() { cap.stop(); }, stop_after_ms);
  Glib::signal_timeout().connect_once([&]() { loop->quit(); }, 5000);
  loop->run();
  return res;
}

}  // namespace

int main()
{
  Glib::init();
  char tmpl[] = "/tmp/tally-capture-XXXXXX";
  const char* dir = ::mkdtemp(tmpl);
  CHECK(dir != nullptr);
  if (!dir)
    return suite_test::done("capture");
  g_bin = std::string(dir) + "/bin";
  ::mkdir(g_bin.c_str(), 0755);
  const char* old_path = g_getenv("PATH");
  g_setenv("PATH", (g_bin + ":" + (old_path ? old_path : "/usr/bin:/bin")).c_str(), TRUE);

  /* ffmpeg fails after spawn: an error, and the take is not reported saved. */
  fake_ffmpeg("echo 'x11grab: bad geometry' >&2\nexit 1");
  const Outcome bad = run_take(std::string(dir) + "/bad.webm", 0);
  CHECK(bad.stopped);
  CHECK(bad.errors == 1);
  CHECK(!bad.saved);

  /* ffmpeg finishes on q: saved, no error. */
  fake_ffmpeg("read line\nexit 0");
  const Outcome good = run_take(std::string(dir) + "/good.webm", 200);
  CHECK(good.stopped);
  CHECK(good.errors == 0);
  CHECK(good.saved);

  const std::string rm = "rm -rf '" + std::string(dir) + "'";
  if (std::system(rm.c_str()) != 0)
    std::cerr << "could not remove " << dir << "\n";
  return suite_test::done("capture");
}
