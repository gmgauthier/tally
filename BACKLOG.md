# Tally backlog

Current release: **v0.1.0**. Last updated: 2026-09-26.

HyperCam-shaped screen recorder. Binary `tally`. Suite catalog: `lcos-projects/PRODUCT-BACKLOG.md`. Plan: [DEVELOPMENT.md](DEVELOPMENT.md). How to land work: [DEVELOPMENT.md](DEVELOPMENT.md#process) — `feature/` / `fix/` branches, PRs to `master`, lint gate, semver on shipped PRs.

## High Priority

None. M2 (hide while recording, last folder/format/mic/FPS) is in this branch.

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

**v0.1.0** — Window, full-screen / region / window `x11grab`, optional mic with device combo, WebM or AVI, `.deb` / tarball / AppImage.
