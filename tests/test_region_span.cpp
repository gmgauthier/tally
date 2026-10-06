/* SPDX-License-Identifier: Unlicense */

#include "region_pick.hpp"
#include "check.hpp"

int main()
{
  const tally::Rect left{0, 0, 2560, 1440};
  const tally::Rect right{2560, 0, 2560, 1440};

  /* Two monitors side by side. The snapshot is that whole screen. */
  tally::Rect span = tally::picker_span({left, right}, 5120, 1440);
  CHECK(span.x == 0 && span.y == 0 && span.w == 5120 && span.h == 1440);

  /* One listed monitor must not shrink the overlay below the snapshot. */
  span = tally::picker_span({right}, 5120, 1440);
  CHECK(span.x == 0 && span.y == 0 && span.w == 5120 && span.h == 1440);
  span = tally::picker_span({left}, 5120, 1440);
  CHECK(span.x == 0 && span.y == 0 && span.w == 5120 && span.h == 1440);

  /* One monitor that is the whole screen stays that screen. */
  span = tally::picker_span({left}, 2560, 1440);
  CHECK(span.x == 0 && span.y == 0 && span.w == 2560 && span.h == 1440);

  /* No monitor list still covers the snapshot. */
  span = tally::picker_span({}, 5120, 1440);
  CHECK(span.x == 0 && span.y == 0 && span.w == 5120 && span.h == 1440);

  /* Stacked monitors. */
  const tally::Rect lower{0, 1440, 2560, 1440};
  span = tally::picker_span({left, lower}, 2560, 2880);
  CHECK(span.x == 0 && span.y == 0 && span.w == 2560 && span.h == 2880);

  /* A monitor left of the origin stays inside the span. */
  const tally::Rect west{-1920, 0, 1920, 1080};
  span = tally::picker_span({west, left}, 2560, 1440);
  CHECK(span.x == -1920 && span.y == 0 && span.w == 4480 && span.h == 1440);

  /* A zero-size monitor is ignored. */
  span = tally::picker_span({tally::Rect{0, 0, 0, 0}, right}, 5120, 1440);
  CHECK(span.x == 0 && span.y == 0 && span.w == 5120 && span.h == 1440);

  span = tally::picker_span({}, 0, 0);
  CHECK(span.w == 0 && span.h == 0);

  return suite_test::done("region-span");
}
