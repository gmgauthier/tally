/* SPDX-License-Identifier: Unlicense */

#include "record_plan.hpp"
#include "check.hpp"

int main()
{
  using tally::StartOrder;
  using tally::start_order;

  /* Full screen with Hide this window on: the window goes before ffmpeg starts. */
  CHECK(start_order(true, true) == StartOrder::conceal_then_start);
  /* Region and window picks have already withdrawn the window. */
  CHECK(start_order(true, false) == StartOrder::start_then_conceal);
  /* Hide off: just start. */
  CHECK(start_order(false, true) == StartOrder::start_only);
  CHECK(start_order(false, false) == StartOrder::start_only);

  return suite_test::done("record-plan");
}
