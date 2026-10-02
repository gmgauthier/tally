/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <string>

namespace tally {

/* The file the next take writes. A Save As name is used for one take only. */
class NextTake {
 public:
  void set(const std::string& path)
  {
    pinned_ = path;
  }
  void clear()
  {
    pinned_.clear();
  }
  const std::string& pinned() const
  {
    return pinned_;
  }

  /* Keeps a pinned Save As name in step with the format combo. */
  void change_ext(const std::string& ext);

  /* Path for the take about to start, in FOLDER with EXT unless Save As pinned one. */
  std::string take(const std::string& ext, const std::string& folder);

 private:
  std::string pinned_;
};

}  // namespace tally
