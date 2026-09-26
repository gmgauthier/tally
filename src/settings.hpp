/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <string>

namespace tally {

struct Settings {
  std::string audio_device = "default";
  bool mic = false;

  void load();
  void save() const;
};

}  // namespace tally
