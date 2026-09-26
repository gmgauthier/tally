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
  try {
    if (kf.has_key("files", "last_folder"))
      last_folder = kf.get_string("files", "last_folder");
  } catch (const Glib::Error&) {
  }
  try {
    if (kf.has_key("capture", "format"))
      format = kf.get_string("capture", "format");
  } catch (const Glib::Error&) {
  }
  try {
    if (kf.has_key("capture", "fps"))
      fps = kf.get_integer("capture", "fps");
  } catch (const Glib::Error&) {
  }
  try {
    if (kf.has_key("capture", "hide_window"))
      hide_window = kf.get_boolean("capture", "hide_window");
  } catch (const Glib::Error&) {
  }
  if (fps != 5 && fps != 10 && fps != 15 && fps != 30)
    fps = 10;
  if (format != "avi" && format != "mp4" && format != "mkv")
    format = "webm";
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
  kf.set_string("files", "last_folder", last_folder);
  kf.set_string("capture", "format", format);
  kf.set_integer("capture", "fps", fps);
  kf.set_boolean("capture", "hide_window", hide_window);
  try {
    kf.save_to_file(config_path());
  } catch (const Glib::Error&) {
  }
}

}  // namespace tally
