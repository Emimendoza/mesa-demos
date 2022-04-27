progs = [
  'bindtex',
  'es1_info',
  'msaa',
  'pbuffer',
  'render_tex',
  'texture_from_pixmap',
  'two_win',
]
foreach p : progs
  executable(
    p, files(p + '.c'),
    dependencies: [
      dep_gles1, dep_egl, dep_m, idep_glad, idep_util, dep_x11
    ])
endforeach

executable(
  'clear', files('clear.c'),
  dependencies: [
      # intentionally not link to dep_gles1
      dep_m, idep_glad, idep_util, idep_eglut_x11
  ])

progs_x11 = [
  'drawtex',
  'gears',
  'torus',
  'tri',
]
foreach p : progs_x11
  executable(
    p + '_x11', files(p + '.c'),
    dependencies: [
      dep_gles1, dep_m, idep_glad, idep_util, idep_eglut_x11
    ])
endforeach

executable(
  'eglfbdev', files('eglfbdev.c'),
  dependencies: [
     dep_gles1, dep_egl
  ])
