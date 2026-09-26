/* SPDX-License-Identifier: Unlicense */

#include "settings.hpp"

#include <glib.h>
#include <glibmm/fileutils.h>
#include <glibmm/keyfile.h>
#include <glibmm/miscutils.h>

namespace tally {
namespace {

std::string config_path()
{
  const std::string dir = Glib::build_filename(Glib::get_user_config_dir(), "tally");
  g_mkdir_with_parents(dir.c_str(), 0700);
  return Glib::build_filename(dir, "tally.ini");
}

}  // namespace

void Settings::load()
{
  Glib::KeyFile kf;
  try {
    kf.load_from_file(config_path());
  } catch (const Glib::Error&) {
    return;
  }
  try {
    if (kf.has_key("audio", "device"))
      audio_device = kf.get_string("audio", "device");
  } catch (const Glib::Error&) {
  }
  try {
    if (kf.has_key("audio", "mic"))
      mic = kf.get_boolean("audio", "mic");
  } catch (const Glib::Error&) {
  }
}

void Settings::save() const
{
  Glib::KeyFile kf;
  try {
    kf.load_from_file(config_path());
  } catch (const Glib::Error&) {
  }
  kf.set_string("audio", "device", audio_device);
  kf.set_boolean("audio", "mic", mic);
  try {
    kf.save_to_file(config_path());
  } catch (const Glib::Error&) {
  }
}

}  // namespace tally
