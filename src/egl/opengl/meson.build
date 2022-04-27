_deps = [dep_gl, dep_m, idep_util]

executable(
  'eglgears_x11', files('eglgears.c'),
  dependencies: [_deps, idep_eglut_x11]
)
executable(
  'egltri_x11', files('egltri.c'),
  dependencies: [_deps, idep_eglut_x11]
)
executable(
  'xeglgears', files('xeglgears.c'),
  dependencies: [_deps, dep_egl, dep_x11]
)
executable(
  'xeglthreads', files('xeglthreads.c'),
  dependencies: [_deps, dep_x11]
)

executable(
  'eglgears_wayland', files('eglgears.c'),
  dependencies: [_deps, idep_eglut_wayland]
)
executable(
  'egltri_wayland', files('egltri.c'),
  dependencies: [_deps, idep_eglut_wayland]
)

executable(
  'eglkms', 'eglkms.c',
  dependencies: [_deps, dep_drm, dep_gbm, dep_egl]
)

executable(
  'eglinfo', 'eglinfo.c',
  dependencies: [dep_egl],
  install: true
)

executable(
  'peglgears', 'peglgears.c',
  dependencies: [dep_gl, dep_egl, dep_m, idep_util]
)

