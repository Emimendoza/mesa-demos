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

inc_glut = include_directories('.')

_libeglut_x11 = static_library(
  'eglut_x11',
  files('eglut.c', 'eglut_x11.c'),
  dependencies: [dep_x11, dep_egl])

idep_eglut_x11 = declare_dependency(
  link_with: _libeglut_x11,
  include_directories: inc_glut)

_wayland_sources = files('eglut.c', 'eglut_wayland.c')

_libeglut_wayland = static_library(
  'eglut_wayland', _wayland_sources,
  dependencies: [dep_wayland, dep_egl, dep_epoll, dep_libdecor]
)

idep_eglut_wayland = declare_dependency(
  link_with: _libeglut_wayland,
  include_directories: inc_glut
)
