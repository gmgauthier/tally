/* SPDX-License-Identifier: Unlicense */

#include "check.hpp"
#include "stop_chip.hpp"
#include "stop_key.hpp"

#include <gtkmm.h>

int main(int argc, char** argv)
{
  CHECK(tally::stop_chord(GDK_KEY_period, GDK_CONTROL_MASK));
  CHECK(!tally::stop_chord(GDK_KEY_period, 0));
  CHECK(!tally::stop_chord(GDK_KEY_a, GDK_CONTROL_MASK));
  CHECK(!tally::stop_chord(GDK_KEY_period, GDK_CONTROL_MASK | GDK_SHIFT_MASK));
  CHECK(!tally::stop_chord(GDK_KEY_period, GDK_CONTROL_MASK | GDK_MOD1_MASK));
  CHECK(tally::stop_chord(GDK_KEY_period, GDK_CONTROL_MASK | GDK_LOCK_MASK));
  CHECK(tally::stop_chord(GDK_KEY_period, GDK_CONTROL_MASK | GDK_MOD2_MASK));
  CHECK(tally::stop_chord(GDK_KEY_period, GDK_CONTROL_MASK | GDK_LOCK_MASK | GDK_MOD2_MASK));

  const auto masks = tally::stop_grab_masks();
  CHECK(masks.size() == 4);
  for (const unsigned mask : masks)
    CHECK(tally::stop_chord(GDK_KEY_period, mask));

  Gtk::Main kit(argc, argv);
  tally::StopChip chip;
  /* Withdrawn main window: this chip is the taskbar entry. */
  CHECK(!chip.get_skip_taskbar_hint());

  int hits = 0;
  chip.signal_stop().connect([&hits]() { ++hits; });
  CHECK(chip.handle_stop_key(GDK_KEY_period, GDK_CONTROL_MASK));
  CHECK(hits == 1);
  CHECK(!chip.handle_stop_key(GDK_KEY_period, 0));
  CHECK(hits == 1);
  CHECK(chip.handle_stop_key(GDK_KEY_period, GDK_CONTROL_MASK | GDK_LOCK_MASK | GDK_MOD2_MASK));
  CHECK(hits == 2);

  /* Map and unmap so the root grab is installed and then released. */
  chip.show();
  while (Gtk::Main::events_pending())
    Gtk::Main::iteration();
  chip.hide();
  while (Gtk::Main::events_pending())
    Gtk::Main::iteration();

  return suite_test::done("stop-key");
}
