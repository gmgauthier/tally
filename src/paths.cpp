/* SPDX-License-Identifier: Unlicense */

#include "paths.hpp"
#include "config.hpp"

#include <glib.h>
#include <glibmm.h>

#include <ctime>
#include <string>
#include <vector>

namespace tally {
namespace {

bool exists_regular(const std::string& path)
{
  return Glib::file_test(path, Glib::FILE_TEST_IS_REGULAR);
}

}  // namespace

std::string find_data_file(const std::string& relative)
{
  std::vector<std::string> roots;

  if (const char* env = g_getenv("TALLY_DATA"))
    roots.emplace_back(env);

  if (const char* appdir = g_getenv("APPDIR"))
    roots.emplace_back(Glib::build_filename(appdir, "usr/share/tally"));

  roots.emplace_back(SOURCE_ROOT);
  roots.emplace_back(std::string(SOURCE_ROOT) + "/data");
  roots.emplace_back(DATADIR);

  for (const auto& root : roots) {
    const std::string candidate = Glib::build_filename(root, relative);
    if (exists_regular(candidate))
      return candidate;
  }
  return {};
}

std::string default_output_dir(const std::string& dir_in)
{
  std::string dir = dir_in;
  if (dir.empty() || !Glib::file_test(dir, Glib::FILE_TEST_IS_DIR))
    dir = Glib::build_filename(Glib::get_home_dir(), "Videos");
  if (!Glib::file_test(dir, Glib::FILE_TEST_IS_DIR))
    dir = Glib::get_home_dir();
  return dir;
}

std::string default_output_path(const std::string& ext, const std::string& dir_in)
{
  std::time_t now = std::time(nullptr);
  std::tm tm{};
  localtime_r(&now, &tm);
  char stamp[32];
  std::strftime(stamp, sizeof(stamp), "%Y%m%d-%H%M%S", &tm);
  const std::string dir = default_output_dir(dir_in);
  const std::string stem = std::string("tally-") + stamp;
  std::string path = Glib::build_filename(dir, stem + "." + ext);
  /* ffmpeg runs with -y: never hand it a take that is already on disk. */
  for (int n = 2; Glib::file_test(path, Glib::FILE_TEST_EXISTS); ++n)
    path = Glib::build_filename(dir, stem + "-" + std::to_string(n) + "." + ext);
  return path;
}

}  // namespace tally
