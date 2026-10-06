project(
  'tally',
  'cpp',
  version: '0.3.8',
  license: 'Unlicense',
  meson_version: '>=0.56',
  default_options: [
    'cpp_std=c++17',
    'warning_level=2',
    'prefix=/usr',
  ],
)

gtkmm = dependency('gtkmm-3.0')
x11 = dependency('x11')

conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('APP_ID', 'org.gmgauthier.Tally')
conf.set_quoted('DATADIR', get_option('prefix') / get_option('datadir') / 'tally')
conf.set_quoted('SOURCE_ROOT', meson.project_source_root())

configure_file(
  output: 'config.hpp',
  configuration: conf,
)

inc = include_directories('.', 'src')

executable(
  'tally',
  'src/main.cpp',
  'src/application.cpp',
  'src/paths.cpp',
  'src/next_take.cpp',
  'src/main_window.cpp',
  'src/about_dialog.cpp',
  'src/capture.cpp',
  'src/preview.cpp',
  'src/region_pick.cpp',
  'src/audio_devices.cpp',
  'src/settings.cpp',
  'src/x11_windows.cpp',
  'src/stop_chip.cpp',
  include_directories: inc,
  dependencies: [gtkmm, x11],
  install: true,
  install_dir: get_option('bindir'),
)

install_data(
  'data/tally.desktop',
  install_dir: get_option('datadir') / 'applications',
)

install_data(
  'data/skin/lcos/lcos.css',
  install_dir: get_option('datadir') / 'tally' / 'skin' / 'lcos',
)

install_data(
  'brand/icon-tile.svg',
  install_dir: get_option('datadir') / 'tally' / 'brand',
)

install_data(
  'brand/icon-tile.svg',
  rename: 'tally.svg',
  install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps',
)

tally_tests = executable(
  'tally-tests',
  'tests/test_format.cpp',
  include_directories: inc,
  dependencies: [gtkmm, x11],
)
test('format', tally_tests)

next_take_tests = executable(
  'tally-next-take-tests',
  'tests/test_next_take.cpp',
  'src/next_take.cpp',
  'src/paths.cpp',
  include_directories: inc,
  dependencies: [gtkmm],
)
test('next-take', next_take_tests)

capture_tests = executable(
  'tally-capture-tests',
  'tests/test_capture.cpp',
  'src/capture.cpp',
  include_directories: inc,
  dependencies: [gtkmm],
)
test('capture', capture_tests)

geometry_tests = executable(
  'tally-geometry-tests',
  'tests/test_geometry.cpp',
  'src/capture.cpp',
  include_directories: inc,
  dependencies: [gtkmm],
)
test('geometry', geometry_tests)

region_span_tests = executable(
  'tally-region-span-tests',
  'tests/test_region_span.cpp',
  'src/region_pick.cpp',
  include_directories: inc,
  dependencies: [gtkmm],
)
test('region-span', region_span_tests)

record_plan_tests = executable(
  'tally-record-plan-tests',
  'tests/test_record_plan.cpp',
  include_directories: inc,
)
test('record-plan', record_plan_tests)

audio_tests = executable(
  'tally-audio-tests',
  'tests/test_audio.cpp',
  'src/audio_devices.cpp',
  'src/capture.cpp',
  include_directories: inc,
  dependencies: [gtkmm],
)
test('audio', audio_tests)

stop_key_tests = executable(
  'tally-stop-key-tests',
  'tests/test_stop_key.cpp',
  'src/stop_chip.cpp',
  include_directories: inc,
  dependencies: [gtkmm, x11],
)
xvfb = find_program('xvfb-run', required: false)
if xvfb.found()
  test('stop-key', xvfb, args: ['-a', stop_key_tests], timeout: 60)
endif
