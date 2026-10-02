/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <gdk/gdk.h>

#include <array>

namespace tally {

/* Ctrl+. is Stop while another application is focused. Caps Lock and Num Lock
 * stay out of the comparison. Shift, Alt, and Super do not count as the chord. */
inline bool stop_chord(guint keyval, guint state)
{
  if (keyval != GDK_KEY_period)
    return false;
  const guint ignored = GDK_LOCK_MASK | GDK_MOD2_MASK;
  const guint relevant = GDK_MODIFIER_MASK & ~ignored;
  return (state & relevant) == GDK_CONTROL_MASK;
}

/* Grab masks so the chord still arrives with Caps Lock or Num Lock down.
 * The values match the X11 modifier masks GDK uses. */
inline std::array<unsigned, 4> stop_grab_masks()
{
  return {
      GDK_CONTROL_MASK,
      GDK_CONTROL_MASK | GDK_LOCK_MASK,
      GDK_CONTROL_MASK | GDK_MOD2_MASK,
      GDK_CONTROL_MASK | GDK_LOCK_MASK | GDK_MOD2_MASK,
  };
}

}  // namespace tally
