/* SPDX-License-Identifier: Unlicense */

#include "capture.hpp"
#include "check.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace {

/* The value after FLAG in ARGV, or empty. */
std::string arg_after(const std::vector<std::string>& argv, const std::string& flag)
{
  auto it = std::find(argv.begin(), argv.end(), flag);
  if (it == argv.end() || it + 1 == argv.end())
    return {};
  return *(it + 1);
}

std::vector<std::string> argv_for(tally::Rect r)
{
  tally::CaptureOpts o;
  o.rect = r;
  o.screen_w = 1024;
  o.screen_h = 768;
  o.path = "/tmp/x.webm";
  return tally::ffmpeg_argv(o, ":9");
}

}  // namespace

int main()
{
  /* Inside the root: unchanged. */
  auto argv = argv_for({100, 50, 400, 300});
  CHECK(arg_after(argv, "-video_size") == "400x300");
  CHECK(arg_after(argv, "-i") == ":9+100,50");

  /* Hanging off the left and top: the origin moves to 0 and the size shrinks with it. */
  argv = argv_for({-100, -50, 400, 300});
  CHECK(arg_after(argv, "-video_size") == "300x250");
  CHECK(arg_after(argv, "-i") == ":9+0,0");

  /* Past the right and bottom: the size stops at the root edge. */
  argv = argv_for({900, 700, 300, 200});
  CHECK(arg_after(argv, "-video_size") == "124x68");
  CHECK(arg_after(argv, "-i") == ":9+900,700");

  /* Larger than the root on every side. */
  argv = argv_for({-10, -10, 2000, 2000});
  CHECK(arg_after(argv, "-video_size") == "1024x768");
  CHECK(arg_after(argv, "-i") == ":9+0,0");

  /* clip_to_screen itself, including a rectangle wholly off-screen. */
  tally::Rect c = tally::clip_to_screen({-30, 10, 100, 100}, 1024, 768);
  CHECK(c.x == 0 && c.y == 10 && c.w == 70 && c.h == 100);
  c = tally::clip_to_screen({2000, 10, 100, 100}, 1024, 768);
  CHECK(c.w <= 0);
  c = tally::clip_to_screen({-30, -30, 100, 100}, 0, 0);
  CHECK(c.x == 0 && c.y == 0 && c.w == 70 && c.h == 70);

  return suite_test::done("geometry");
}
