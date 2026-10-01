/* SPDX-License-Identifier: Unlicense */

#include "capture.hpp"
#include "check.hpp"

#include <string>

int main()
{
  CHECK(tally::known_format_id("webm"));
  CHECK(tally::known_format_id("avi"));
  CHECK(tally::known_format_id("mp4"));
  CHECK(tally::known_format_id("mkv"));
  CHECK(!tally::known_format_id("gif"));
  CHECK(!tally::known_format_id(""));
  CHECK(!tally::known_format_id("MP4"));

  CHECK(tally::format_from_id("avi") == tally::Format::avi);
  CHECK(tally::format_from_id("mp4") == tally::Format::mp4);
  CHECK(tally::format_from_id("mkv") == tally::Format::mkv);
  CHECK(tally::format_from_id("webm") == tally::Format::webm);
  CHECK(tally::format_from_id("nope") == tally::Format::webm);

  CHECK(std::string(tally::format_id(tally::Format::mp4)) == "mp4");
  CHECK(std::string(tally::format_id(tally::Format::mkv)) == "mkv");
  CHECK(std::string(tally::format_id(tally::Format::avi)) == "avi");
  CHECK(std::string(tally::format_id(tally::Format::webm)) == "webm");
  CHECK(std::string(tally::format_ext(tally::Format::mkv)) == "mkv");
  CHECK(std::string(tally::format_id(tally::format_from_id("mp4"))) == "mp4");
  CHECK(std::string(tally::format_id(tally::format_from_id("bogus"))) == "webm");

  return suite_test::done("format");
}
