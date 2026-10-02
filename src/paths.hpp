/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <string>

namespace tally {

std::string find_data_file(const std::string& relative);
std::string default_output_dir(const std::string& dir = {});
/* tally-YYYYMMDD-HHMMSS.EXT in DIR, with -2, -3… added when that file exists. */
std::string default_output_path(const std::string& ext, const std::string& dir = {});

}  // namespace tally
