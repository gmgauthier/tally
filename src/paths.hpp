/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <string>

namespace tally {

std::string find_data_file(const std::string& relative);
std::string default_output_path(const std::string& ext);

}  // namespace tally
