inc_glut = include_directories('.')

_libeglut_x11 = static_library(
  'eglut_x11',
  files('eglut.c', 'eglut_x11.c'),
  dependencies: [dep_x11, dep_egl])

idep_eglut_x11 = declare_dependency(
  link_with: _libeglut_x11,
  include_directories: inc_glut)

_wayland_sources = files('eglut.c', 'eglut_wayland.c')
if dep_wayland.found()
  _wayland_sources += [
    custom_target(
      'wayland-xdg-shell-protocol.c',
      input : wayland_xdg_shell_xml,
      output : 'wayland-xdg-shell-protocol.c',
      command : [prog_wl_scanner, wl_scanner_arg, '@INPUT@', '@OUTPUT@'],
    ),
    custom_target(
      'wayland-xdg-shell-client-protocol.h',
      input : wayland_xdg_shell_xml,
      output : 'wayland-xdg-shell-client-protocol.h',
      command : [prog_wl_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
    )
  ]
endif

_libeglut_wayland = static_library(
  'eglut_wayland', _wayland_sources,
  dependencies: [dep_wayland, dep_egl]
)

idep_eglut_wayland = declare_dependency(
  link_with: _libeglut_wayland,
  include_directories: inc_glut
)
