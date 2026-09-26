# Tally backlog

Current release: **unreleased (M0–M1 in tree)**. Last updated: 2026-09-26.

HyperCam-shaped screen recorder. Binary `tally`. Suite catalog: `lcos-projects/PRODUCT-BACKLOG.md`. Plan: [DEVELOPMENT.md](DEVELOPMENT.md). How to land work: [DEVELOPMENT.md](DEVELOPMENT.md#process) — `feature/` / `fix/` branches, PRs to `master`, lint gate, semver on shipped PRs.

## High Priority

- Hide Tally while recording (default on)
- Remember last folder / format / mic in `~/.config/tally/tally.ini`

## Low Priority

- FPS combo (5 / 10 / 15 / 30)
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
