/* SPDX-License-Identifier: Unlicense */

#include "next_take.hpp"
#include "paths.hpp"

#include <glibmm.h>

namespace tally {

void NextTake::change_ext(const std::string& ext)
{
  if (pinned_.empty())
    return;
  const std::string source = confirmed_.empty() ? pinned_ : confirmed_;
  const std::string dir = Glib::path_get_dirname(source);
  std::string base = Glib::path_get_basename(source);
  const auto dot = base.rfind('.');
  if (dot != std::string::npos)
    base = base.substr(0, dot);
  std::string path = Glib::build_filename(dir, base + "." + ext);
  /* ffmpeg runs with -y. The chooser confirmed one path. A rewritten extension
     that names a different existing file gets a free sibling. */
  if (path != confirmed_) {
    for (int n = 2; Glib::file_test(path, Glib::FILE_TEST_EXISTS); ++n)
      path = Glib::build_filename(dir, base + "-" + std::to_string(n) + "." + ext);
  }
  pinned_ = path;
}

std::string NextTake::take(const std::string& ext, const std::string& folder)
{
  if (pinned_.empty())
    return default_output_path(ext, folder);
  std::string path;
  path.swap(pinned_);
  confirmed_.clear();
  return path;
}

}  // namespace tally
