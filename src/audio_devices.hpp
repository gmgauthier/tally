/* SPDX-License-Identifier: Unlicense */

#pragma once

#include <glibmm/ustring.h>

#include <string>
#include <vector>

namespace tally {

enum class AudioBackend { pulse, alsa, system_default };

struct AudioDevice {
  std::string id;
  Glib::ustring label;
  AudioBackend backend = AudioBackend::system_default;
};

/* Pulse sources first (PipeWire Pulse too), then arecord -l. Always includes
 * a Default row. Empty hardware list still yields Default so the combo is
 * never blank. */
std::vector<AudioDevice> list_audio_inputs();
/* The combo rows for PULSE sources, or the ALSA capture devices when there are none. */
std::vector<AudioDevice> assemble_audio_inputs(const std::vector<AudioDevice>& pulse,
                                               const std::vector<AudioDevice>& alsa);
std::string pulse_default_source();
bool looks_internal(const AudioDevice& d);
std::string pick_audio_device(const std::vector<AudioDevice>& devices, const std::string& saved);

}  // namespace tally
