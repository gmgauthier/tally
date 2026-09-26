# Tally development plan

A gtkmm-3 **screen recorder** for LCOS. The *window* is HyperCam, not OBS.

Display name: **Tally**  
Binary / repo / package: `tally`  
APP_ID: `org.gmgauthier.Tally`  
License: The Unlicense (`UNLICENSE`)  
Repos: https://gitea.scriptorium/gmgauthier/tally (origin), https://github.com/gmgauthier/tally

Engine: ffmpeg `x11grab` on XLibre. Face: gtkmm. Peek / SimpleScreenRecorder look 2012.

## Status (2026-09-26)

**v0.1.0.** Window plus full-screen / region / window capture to WebM or AVI. Optional mic with a device combo (Pulse then ALSA; Default / Internal microphone if none). Packaged.

## 1. Locked decisions

| Decision | Choice |
|---|---|
| Product | Original. Chrome is HyperCam, not OBS |
| Name | Tally. Binary `tally`. APP_ID `org.gmgauthier.Tally` |
| Rejected | HyperCam (trademark), Screencap, Reel, Camroll |
| Toolkit | C++17, gtkmm-3.0, GTK3 CSS, Meson |
| Engine | ffmpeg on PATH. `x11grab` + optional `pulse`. No daemon |
| File | WebM (VP8) default. AVI (MJPEG) as the 90s option |
| Look | Small decorated window. Big Record / Stop. Red tally lamp. Tiny preview |
| Source | Full screen, drag-region, or click-a-window. Rectangle of the desktop (not a compositor portal) |
| Network | None |
| Never as v1 | Webcam overlay, streaming, scene stacks, hardware encode UI, Wayland portals |
| Init | No systemd |
| Brand | LCOS beige / navy. No Bryan’s seal. Mark is a red tally lamp |
| License | The Unlicense |
| Versioning | Semantic. `meson.build` is the source of truth. See **Process**. |

## 2. Window

```
File  Capture  Help
+---------------------------+
|  [preview]   ( ) Rec lamp |
|              [ Record ]   |
|              [ Stop   ]   |
|  Source: Full / Region / Window
|  [ ] Microphone  [ device combo ]
|  Format: WebM / AVI
|  elapsed
+---------------------------+
```

File: Save as (choose output path), Exit.  
Capture: Record, Stop.  
Help: About Tally.

## 3. Architecture

`Capture` spawns `ffmpeg` with `Glib::spawn_async_with_pipes`. Stop writes `q` to stdin (ffmpeg finalizes the file), SIGINT if that fails.

`RegionPick` snapshots the desktop (so XLibre does not need a compositor), then click-drag a rectangle. The live overlay is a picture of the screen, not a dark pane.

Window pick: same snapshot; hover highlights EWMH client windows (`_NET_CLIENT_LIST_STACKING`); click captures that window’s frame.

Preview: a `Gdk::Pixbuf` grab of that rectangle.

Output default: `~/Videos/tally-YYYYMMDD-HHMMSS.webm` (or `~/` if Videos is missing).

## 4. Milestones

### M0 — Window

Scaffold. Preview well, Record/Stop, source radios, mic, format. Record is a stub.

### M1 — Capture (this slice)

Full screen, region, and window via ffmpeg `x11grab`. Optional mic. WebM or AVI. Elapsed timer. Tally lamp.

### M2 — Chrome polish

FPS control, last folder in `~/.config/tally/tally.ini`, hide-this-window while recording (default on), README screenshot. **Done in this branch.**

### M3 — Package

`debian/` (Depends: ffmpeg), `scripts/release.sh`. Tag `v0.1.0`.

## 5. Parked

FPS combo, cursor on/off, countdown, follow-window, webcam PIP.

## 6. Traps

- OBS / Peek / SimpleScreenRecorder chrome
- PipeWire portal as identity (XLibre is `x11grab`)
- Custom title bar
- Overriding `GTK_THEME` when it is already set. Unset: Clearlooks-Phenix, then Clearlooks, then Adwaita:light
- Bryan’s seal
- Bundling ffmpeg inside the gtkmm binary

## Process

Do not commit to `master`. Every change lands through a pull request.

### Branches

- `feature/<short-name>` — new user-visible work
- `fix/<short-name>` — bugs, packaging nits, regressions

Open a pull request into `master`. Merge only after review.

### Gates

A pull request must pass **lint** before merge. CI runs `./scripts/lint.sh` (no `--fix`). Locally:

- `./scripts/lint.sh --fix` — clang-format rewrites `src/`
- `./scripts/lint.sh` — SPDX headers, no tabs, clang-format `--dry-run --Werror`, cppcheck (`warning`) on `src/`
- `meson compile` with this tree’s `warning_level=2` is clean (no new warnings)

Do not pass `--fix` in CI. Do not merge a red PR.

**Tests** are required when they exist (`meson test -C build`). Until a test suite lands, the gate is lint plus a clean compile plus a manual pass of the change.

### Semantic versioning

Every **shipped** pull request — merged to `master` and tagged as a release — bumps the version. `meson.build` is the source of truth. Keep these in lockstep in the same PR:

- `meson.build` `version:`
- `debian/changelog` (new stanza)
- git tag `vMAJOR.MINOR.PATCH` after merge

Then `./scripts/release.sh` produces `.deb`, tarball, and AppImage.

| Bump | When |
|---|---|
| **PATCH** (`x.y.Z`) | Bug fix or packaging. No new user-facing feature. |
| **MINOR** (`x.Y.0`) | New backward-compatible feature. |
| **MAJOR** (`X.0.0`) | Breaking change: native file format, dropped config keys, removed UI users rely on. |
