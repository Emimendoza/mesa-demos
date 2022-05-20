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

project('mesa-demos', 'c', 'cpp',
  version : '8.5.0',
  meson_version : '>= 0.53')

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

null_dep = dependency('', required : false)

demos_data_dir = '../data/'
if get_option('with-system-data-files')
  demos_data_dir = get_option('datadir') / 'mesa-demos'
endif
add_project_arguments(
  '-DDEMOS_DATA_DIR="@0@"'.format(demos_data_dir),
  language: 'c')

dep_m = cc.find_library('m', required : false)
dep_winmm = cc.find_library('winmm', required : false)

dep_gl = dependency('gl')

dep_gles1 = dependency('glesv1_cm', required : get_option('gles1'))
dep_gles2 = dependency('glesv2', required : get_option('gles2'))
dep_osmesa = dependency('osmesa', required : get_option('osmesa'))
dep_egl = dependency('egl', required : get_option('egl'))

dep_drm = dependency('libdrm',
  required : get_option('libdrm'),
  disabler : true
)
dep_x11 = dependency('x11, xext',
  required : get_option('x11'),
  disabler : true
)
dep_wayland = dependency('wayland-client, wayland-egl',
  required : get_option('wayland'),
  disabler : true
)

if dep_wayland.found()
  dep_wl_scanner = dependency('wayland-scanner', native: true)
  prog_wl_scanner = find_program(dep_wl_scanner.get_variable(pkgconfig : 'wayland_scanner'))
  if dep_wl_scanner.version().version_compare('>= 1.15')
    wl_scanner_arg = 'private-code'
  else
    wl_scanner_arg = 'code'
  endif
  dep_wl_protocols = dependency('wayland-protocols', version : '>= 1.12')
  wayland_xdg_shell_xml = join_paths(
    dep_wl_protocols.get_variable(pkgconfig : 'pkgdatadir'), 'stable',
    'xdg-shell', 'xdg-shell.xml'
  )
endif

dep_threads = dependency('threads')

dep_glu = dependency('glu', required : dep_x11.found())
if not dep_glu.found()
  _glu_name = 'GLU'
  if host_machine.system() == 'windows'
    _glu_name = 'glu32'
  endif
  dep_glu = cc.find_library(_glu_name, has_headers: 'GL/glu.h')
endif

# GBM is needed for EGL on KMS
dep_gbm = dependency('gbm', required : false, disabler : true)

dep_dl = null_dep
if not cc.has_function('dlopen')
  dep_dl = cc.find_library('dl', required : host_machine.system() != 'windows')
endif

glut_libdir = []
glut_incdir = []
with_glut = get_option('with-glut')
if with_glut != ''
  glut_libdir = with_glut / 'lib'
  glut_incdir = include_directories(with_glut / 'include', is_system: true)

  _libglut = cc.find_library(
    'glut',
    dirs: glut_libdir,
    has_headers: 'GL/glut.h',
    header_include_directories: glut_incdir)

  dep_glut = declare_dependency(
    dependencies: _libglut,
    include_directories: glut_incdir
  )
else
  dep_glut = cc.find_library(
    'glut',
    has_headers: 'GL/glut.h',
    required: false
  )
endif

if dep_glut.found() and cc.has_function('glutInitContextProfile',
                                        include_directories: glut_incdir,
                                        prefix : '#include <GL/freeglut.h>')
  add_project_arguments('-DHAVE_FREEGLUT', language : ['c', 'cpp'])
endif

if host_machine.system() == 'darwin'
  add_project_arguments('-DGL_SILENCE_DEPRECATION', language: 'c')
endif

if cc.get_id() == 'msvc'
  c_args = [ '-D_CRT_SECURE_NO_WARNINGS' ]
  c_args += cc.get_supported_arguments([
    '/wd4100', # 'var': unreferenced formal parameter
    '/wd4244', # conversion from 'type1' to 'type2', possible loss of data
    '/wd4305', # trancation from 'type1' to 'type2'
    '/wd4459', # declaration of 'var' hides global declaration
  ])
  add_project_arguments(c_args, language: 'c')
endif

subdir('src')
