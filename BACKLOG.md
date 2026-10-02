# Tally backlog

Current release: **v0.3.5**. Last updated: 2026-10-02.

HyperCam-shaped screen recorder. Binary `tally`. Suite catalog: `lcos-projects/PRODUCT-BACKLOG.md`. Plan: [DEVELOPMENT.md](DEVELOPMENT.md). How to land work: [DEVELOPMENT.md](DEVELOPMENT.md#process) — `feature/` / `fix/` branches, PRs to `master`, lint gate, semver on shipped PRs.

## High Priority

None.

## Low Priority

- Mouse cursor on/off
- Countdown (3–2–1)
- Follow a moving window

## Out of Scope

- OBS scene stacks, streaming, plugins
- Wayland / PipeWire portal as identity
- Webcam overlay
- Network
- Custom title bar. `GTK_THEME` in the environment still wins; else Clearlooks-Phenix, then Clearlooks, then Adwaita:light (process only)
- Bryan’s seal
- Peek / SimpleScreenRecorder re-theme

## Shipped

**v0.3.5** — A full-screen take hides the Tally window before ffmpeg starts.

**v0.3.4** — A window or region that runs off the screen is clipped to the screen edge before ffmpeg sees it.

**v0.3.3** — A failed ffmpeg leaves its error in the status line instead of Saved.

**v0.3.2** — Each take records to its own file; Save As names the next take only.

**v0.3.1** — Headless meson test suite, and known defects recorded in BUG-BACKLOG.md.

**v0.3.0** — M4: MP4 (H.264 + AAC) and MKV (H.264 + Vorbis).

**v0.2.0** — M2: hide while recording, FPS, default folder, persist audio source / format / FPS.

**v0.1.0** — Window, full-screen / region / window `x11grab`, optional mic with device combo, WebM or AVI, `.deb` / tarball / AppImage.
