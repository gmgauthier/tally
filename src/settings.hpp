/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <string>

namespace tally {

struct Settings {
  std::string audio_device = "default";
  bool mic = false;
  std::string last_folder;
  std::string format = "webm";
  int fps = 10;
  bool hide_window = true;

  void load();
  void save() const;
};

}  // namespace tally
