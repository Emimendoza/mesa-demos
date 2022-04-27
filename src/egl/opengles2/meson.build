_deps_x11 = [dep_gles2, dep_egl, dep_x11]
executable(
  'es2_info', files('es2_info.c'),
  dependencies: _deps_x11,
  install: true
)
executable(
  'es2gears_x11', files('es2gears.c'),
  dependencies: [dep_gles2, idep_eglut_x11, dep_m],
  install: true
)
executable(
  'es2tri', files('es2tri.c'),
  dependencies: [_deps_x11, dep_m],
  install: true
)
executable(
  'es2gears_wayland', files('es2gears.c'),
  dependencies: [dep_gles2, idep_eglut_wayland, dep_m],
  install: true
)
