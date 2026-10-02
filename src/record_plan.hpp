/* SPDX-License-Identifier: Unlicense */

#pragma once

namespace tally {

/* Order of a take's start. */
enum class StartOrder {
  start_only,         /* Hide this window is off */
  start_then_conceal, /* the window is already withdrawn (region / window pick) */
  conceal_then_start, /* withdraw the visible window first, start once it is gone */
};

inline StartOrder start_order(bool hide_window, bool window_visible)
{
  if (!hide_window)
    return StartOrder::start_only;
  return window_visible ? StartOrder::conceal_then_start : StartOrder::start_then_conceal;
}

}  // namespace tally
