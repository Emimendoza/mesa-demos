inc_glut = include_directories('.')

_libeglut_x11 = static_library(
  'eglut_x11',
  files('eglut.c', 'eglut_x11.c'),
  dependencies: [dep_x11, dep_egl])

idep_eglut_x11 = declare_dependency(
  link_with: _libeglut_x11,
  include_directories: inc_glut)

if dep_wayland.found()
  wayland_xdg_shell_protocol_c = custom_target(
    'wayland-xdg-shell-protocol.c',
    input : wayland_xdg_shell_xml,
    output : 'wayland-xdg-shell-protocol.c',
    command : [prog_wl_scanner, wl_scanner_arg, '@INPUT@', '@OUTPUT@'],
  )

  wayland_xdg_shell_client_protocol_h = custom_target(
    'wayland-xdg-shell-client-protocol.h',
    input : wayland_xdg_shell_xml,
    output : 'wayland-xdg-shell-client-protocol.h',
    command : [prog_wl_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
  )

  _libeglut_wayland = static_library(
    'eglut_wayland', [
      files('eglut.c', 'eglut_wayland.c'),
      wayland_xdg_shell_protocol_c,
      wayland_xdg_shell_client_protocol_h
    ],
    dependencies: [dep_wayland, dep_egl])

  idep_eglut_wayland = declare_dependency(
    link_with: _libeglut_wayland,
    include_directories: inc_glut)
endif
