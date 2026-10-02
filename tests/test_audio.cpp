/* SPDX-License-Identifier: Unlicense */

#include "audio_devices.hpp"
#include "capture.hpp"
#include "check.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace {

using tally::AudioBackend;
using tally::AudioDevice;

AudioDevice dev(const std::string& id, AudioBackend backend)
{
  AudioDevice d;
  d.id = id;
  d.label = id;
  d.backend = backend;
  return d;
}

/* "-f FMT -i ID" for the audio input in an ffmpeg argv, e.g. "alsa default". */
std::string audio_input(const AudioDevice& d)
{
  tally::CaptureOpts o;
  o.rect = tally::Rect{0, 0, 64, 48};
  o.mic = true;
  o.audio = d;
  o.path = "/tmp/x.webm";
  const auto argv = tally::ffmpeg_argv(o, ":9");
  /* x11grab has options between -f and -i; the audio input is "-f FMT -i ID". */
  for (std::size_t i = 0; i + 3 < argv.size(); ++i) {
    if (argv[i] == "-f" && argv[i + 2] == "-i")
      return argv[i + 1] + " " + argv[i + 3];
  }
  return {};
}

}  // namespace

int main()
{
  /* Pulse is up: Default records from Pulse. */
  auto rows = tally::assemble_audio_inputs({dev("alsa_input.usb", AudioBackend::pulse)}, {});
  CHECK(rows.size() == 2);
  CHECK(rows[0].id == "default");
  CHECK(audio_input(rows[0]) == "pulse default");
  CHECK(audio_input(rows[1]) == "pulse alsa_input.usb");

  /* pactl failed, the list came from arecord -l: Default records from ALSA, not Pulse. */
  rows = tally::assemble_audio_inputs({}, {dev("hw:1,0", AudioBackend::alsa)});
  CHECK(rows.size() == 2);
  CHECK(rows[0].id == "default");
  CHECK(rows[0].backend == AudioBackend::alsa);
  CHECK(audio_input(rows[0]) == "alsa default");
  CHECK(audio_input(rows[1]) == "alsa hw:1,0");

  /* Nothing listed at all: a single Default row, as before. */
  rows = tally::assemble_audio_inputs({}, {});
  CHECK(rows.size() == 1);
  CHECK(rows[0].id == "default");
  CHECK(audio_input(rows[0]) == "pulse default");

  return suite_test::done("audio");
}
