# Installing Tally

Four ways to get a binary, in the order LCOS cares about:

| Artifact | Who it is for |
|---|---|
| **`.deb`** | LCOS, Devuan Excalibur, Debian Trixie. Preferred. |
| **Source tarball** | Distro packagers and `meson setup && ninja install`. |
| **AppImage** | Fallback for distros that do not install `.deb` files. |
| **Git build** | Developers. See below. |

Version comes from `meson.build` (currently `0.3.6`).

## Runtime needs

- GTK 3 / gtkmm-3.0
- ffmpeg (x11grab + optional Pulse)

```
sudo apt install libgtkmm-3.0-1t64 ffmpeg
```

## 1. Debian package (preferred)

```
sudo apt install ./dist/tally_0.3.6-1_amd64.deb
```

Or, from this tree:

```
./scripts/release.sh deb
sudo apt install ./dist/tally_0.3.6-1_amd64.deb
```

Uninstall: `sudo apt remove tally`.

## 2. Source tarball

```
tar -xf tally-0.3.6.tar.xz
cd tally-0.3.6
sudo apt install build-essential meson ninja-build pkg-config \
  libgtkmm-3.0-dev libx11-dev
meson setup build --prefix=/usr
meson compile -C build
sudo meson install -C build
```

## 3. AppImage (fallback)

```
./scripts/release.sh appimage
```

Requires `linuxdeploy` on `$PATH`. ffmpeg is not bundled; it must be on PATH.

## 4. Git build

```
sudo apt install build-essential meson ninja-build pkg-config g++ \
  libgtkmm-3.0-dev libx11-dev ffmpeg clang-format cppcheck
meson setup build
meson compile -C build
./build/tally
```

## License

[The Unlicense](https://unlicense.org). See [UNLICENSE](UNLICENSE).
