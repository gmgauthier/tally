# Bug backlog

Reviewed 2026-10-01 against the 0.3.0 sources.

`meson test` runs `tests/test_format.cpp` (`format`), `tests/test_next_take.cpp` (`next-take`), and `tests/test_capture.cpp` (`capture`). `capture` runs a take against a stand-in `ffmpeg` on `PATH` and checks that a non-zero exit is an error and not saved, and that a clean exit on Stop is saved. `format` checks the shipped format ids (`webm`, `avi`, `mp4`, `mkv`) and the unknown-id fallback. `next-take` checks that a second take never gets the first take's file, and that a Save As name is used for one take. Capture, geometry, and the stop chip need a display, so they are not in that binary. ffmpeg is spawned with an argv vector, not a shell.

## Open

### Off-screen window geometry is shifted or rejected

- Severity: incorrect
- Confidence: high
- Where: `src/capture.cpp:55`, `src/x11_windows.cpp:117`, `src/main_window.cpp:227`
- Trigger: Window pick of a window that hangs off the left or top, or whose `_NET_FRAME_EXTENTS` push the rectangle off that edge. The same for a rectangle that extends past the right or bottom of the X root.
- Outcome: `build_argv` clamps a negative origin to 0 and does not shrink the width or height by the clamped amount, so the grab slides toward the bottom-right. The preview clips a local copy (`refresh_preview`), so the thumbnail can look fitted while the ffmpeg rectangle is still too big. x11grab then errors, and the previous defect reports Saved. Region drag itself stays on-screen. Negative width and height are rejected in `Capture::start` before ffmpeg.

### Full-screen recording starts before the window is hidden

- Severity: incorrect
- Confidence: high
- Where: `src/main_window.cpp:508`, `src/main_window.cpp:536`
- Trigger: Full screen (the default source) with "Hide this window" on, which is the default. Region and window mode withdraw the main window inside the picker, before `begin_capture`.
- Outcome: `cap_.start` runs, then `conceal_for_record`. The hide waits for the Stop chip to map and for an idle callback. The first frames of a full-screen take include the Tally window.

### ALSA fallback still records with Pulse

- Severity: incorrect
- Confidence: high
- Where: `src/audio_devices.cpp:136`, `src/capture.cpp:80`
- Trigger: `pactl list sources` fails, so the device list is filled from `arecord -l`, and the user leaves the Default row selected with audio enabled.
- Outcome: Default is `AudioBackend::system_default`. `build_argv` passes `-f pulse` unless the backend is ALSA and the id is a real device. There is no Pulse server on this fallback path, so ffmpeg exits and the Saved defect reports success. The ALSA rows are used only if the user picks one.

### Stop shortcut does not work while the window is hidden

- Severity: incorrect
- Confidence: high
- Where: `src/main_window.cpp:192`, `src/stop_chip.cpp:12`, `src/stop_chip.cpp:26`
- Trigger: Record with hide-window on. Focus is in another application. Press Ctrl+. or look for Tally on the taskbar.
- Outcome: Both accelerators are window `AccelGroup`s. The main window is withdrawn, so it gets no keys and is not on the taskbar. The chip sets `skip_taskbar_hint` and does not take focus, so its Ctrl+. does not fire either. The control that works is clicking the chip. The tooltip says Stop is available from the taskbar or Ctrl+.

## Closed

### A failed ffmpeg is reported as Saved

- Severity: incorrect
- Confidence: high
- Where: `src/capture.cpp` `on_child`, `src/main_window.cpp` `on_stopped`
- Trigger: ffmpeg exits non-zero after spawn. An x11grab rectangle outside the root, or a bad Pulse device, does this and does not leave a usable file.
- Outcome: `on_child` emits the error and then `signal_stopped_`. `on_stopped` is connected first and always sets the status to `Saved <name>`, so the error line is replaced. The UI says the take was saved.
- Fixed in v0.3.3: `signal_stopped` says whether ffmpeg exited cleanly. The status shows `Saved <name>` only then; after a non-zero exit, or a signal, the error line stays.

### Later takes overwrite the first file

- Severity: data-loss
- Confidence: high
- Where: `src/main_window.cpp` `begin_capture`, `src/next_take.cpp`, `src/paths.cpp` `default_output_path`
- Trigger: Record once (default timestamp name, or Save As). Record again without changing the folder.
- Outcome: `begin_capture` stores the path in `save_path_` and never clears it after a take. `save_path_` is cleared only when the folder changes (`on_dest_set`, `on_default_folder`). The next take reuses that path, and ffmpeg is passed `-y`, so the previous recording is truncated as soon as the new input opens. A failed `start()` still leaves the path pinned.
- Fixed in v0.3.2: Each take asks for its own file. A Save As name is used for the next take only, and is spent even when `start()` fails. A default name that already exists gets `-2`, `-3`, and so on, so two takes in the same second do not share a file.
