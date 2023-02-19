# Copyright © 2022 Collabora Ltd

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

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
      dep_egl, dep_m, idep_glad, idep_util, idep_eglut_x11
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

if host_machine.system() == 'linux'
executable(
  'eglfbdev', files('eglfbdev.c'),
  dependencies: [
     dep_gles1, dep_egl
  ])
endif
