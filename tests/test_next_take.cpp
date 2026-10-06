/* SPDX-License-Identifier: Unlicense */

#include "next_take.hpp"
#include "paths.hpp"
#include "check.hpp"

#include <glibmm.h>

#include <cstdlib>
#include <fstream>
#include <string>

namespace {

void touch(const std::string& path)
{
  std::ofstream(path) << "take";
}

}  // namespace

int main()
{
  char tmpl[] = "/tmp/tally-take-XXXXXX";
  const char* dir_c = ::mkdtemp(tmpl);
  CHECK(dir_c != nullptr);
  if (!dir_c)
    return suite_test::done("next-take");
  const std::string dir = dir_c;

  /* Default names: a second take never reuses the first file. */
  tally::NextTake next;
  const std::string first = next.take("webm", dir);
  CHECK(Glib::path_get_dirname(first) == dir);
  touch(first);
  const std::string second = next.take("webm", dir);
  CHECK(second != first);
  touch(second);
  const std::string third = next.take("webm", dir);
  CHECK(third != first);
  CHECK(third != second);

  /* Save As names one take. The take after it gets a fresh default name. */
  const std::string chosen = dir + "/demo.webm";
  next.set(chosen);
  next.change_ext("mkv");
  CHECK(next.pinned() == dir + "/demo.mkv");
  const std::string as = next.take("mkv", dir);
  CHECK(as == dir + "/demo.mkv");
  touch(as);
  CHECK(next.pinned().empty());
  const std::string after = next.take("mkv", dir);
  CHECK(after != as);
  CHECK(Glib::path_get_dirname(after) == dir);

  /* Changing the format with nothing pinned leaves it unpinned. */
  tally::NextTake idle;
  idle.change_ext("mp4");
  CHECK(idle.pinned().empty());

  /* A rewritten extension must not pin a different file that already exists. */
  const std::string occupied = dir + "/clip.mkv";
  const std::string occupied_2 = dir + "/clip-2.mkv";
  touch(occupied);
  touch(occupied_2);
  tally::NextTake dodge;
  dodge.set(dir + "/clip.webm");
  dodge.change_ext("mkv");
  CHECK(dodge.pinned() == dir + "/clip-3.mkv");
  CHECK(!Glib::file_test(dodge.pinned(), Glib::FILE_TEST_EXISTS));
  /* The stem stays the name the chooser confirmed, not the sibling. */
  dodge.change_ext("mp4");
  CHECK(dodge.pinned() == dir + "/clip.mp4");
  /* The confirmed path itself may still be overwritten. */
  const std::string kept = dir + "/keep.webm";
  touch(kept);
  tally::NextTake same;
  same.set(kept);
  same.change_ext("webm");
  CHECK(same.pinned() == kept);

  const std::string rm = "rm -rf '" + dir + "'";
  if (std::system(rm.c_str()) != 0)
    std::cerr << "could not remove " << dir << "\n";
  return suite_test::done("next-take");
}
