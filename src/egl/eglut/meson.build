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

eglut_files = files('eglut.c', 'wsi/wsi.c')
wsi_deps = []
wsi_args = []

if dep_wayland.found()
  wsi_args += ['-DWAYLAND_SUPPORT']
  eglut_files += files('wsi/wayland.c')
  wsi_deps += [dep_wayland, dep_epoll, dep_libdecor]
endif

if dep_x11.found()
  wsi_args += ['-DX11_SUPPORT']
  eglut_files += files('wsi/x11.c')
  wsi_deps += dep_x11
endif

if wsi_deps.length() > 0
  _libeglut = static_library(
    'eglut',
    eglut_files,
    dependencies: [dep_egl, wsi_deps],
    c_args: wsi_args
  )

  idep_eglut = declare_dependency(
    link_with: _libeglut,
    include_directories: inc_glut
  )
else
  idep_eglut = disabler()
endif
