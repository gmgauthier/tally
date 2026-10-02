/* SPDX-License-Identifier: Unlicense */

#include "next_take.hpp"
#include "paths.hpp"

#include <glibmm.h>

namespace tally {

void NextTake::change_ext(const std::string& ext)
{
  if (pinned_.empty())
    return;
  const std::string dir = Glib::path_get_dirname(pinned_);
  std::string base = Glib::path_get_basename(pinned_);
  const auto dot = base.rfind('.');
  if (dot != std::string::npos)
    base = base.substr(0, dot);
  pinned_ = Glib::build_filename(dir, base + "." + ext);
}

std::string NextTake::take(const std::string& ext, const std::string& folder)
{
  if (pinned_.empty())
    return default_output_path(ext, folder);
  std::string path;
  path.swap(pinned_);
  return path;
}

}  // namespace tally
