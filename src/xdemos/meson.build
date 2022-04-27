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

glx_deps = [dep_gl, dep_x11, dep_m]

progs = [
  'glsync',
  'glxdemo',
  'glxgears',
  'glxgears_pixmap',
  'glxcontexts',
  'glxheads',
  'glxpixmap',
  'glxpbdemo',
  'glxsnoop',
  'glxswapcontrol',
  'manywin',
  'multictx',
  'offset',
  'overlay',
  'shape',
  'sharedtex',
  'texture_from_pixmap',
  'wincopy',
  'xfont',
]
foreach p : progs
  executable(
    p, files(p + '.c'),
    dependencies: glx_deps,
    install: true
  )
endforeach

executable(
  'glxinfo',
  files('glxinfo.c', 'glinfo_common.c'),
  dependencies: glx_deps,
  install: true
)

executable(
  'xrotfontdemo',
  files('xrotfontdemo.c', 'xuserotfont.c'),
  dependencies: glx_deps,
  install: true
)

_libpbutil = static_library('pbutil', files('pbutil.c'))

pbutil_progs = [
  'glxgears_fbconfig',
  'pbinfo',
  'pbdemo',
]
foreach p : pbutil_progs
  executable(
    p, files(p + '.c'),
    dependencies: glx_deps,
    link_with: _libpbutil,
    install: true
  )
endforeach

thread_progs = [
  'glthreads',
  'sharedtex_mt',
]
foreach p : thread_progs
  executable(
    p, files(p + '.c'),
    dependencies: [glx_deps, dep_threads],
    install: true
  )
endforeach
