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

glsl_shaders = files(
	'gear.frag',
	'gear.vert',
)

_gen = generator(
  prog_glslang,
  output : '@PLAINNAME@.spv.h',
  arguments : [ '@INPUT@', '-V', '-x', '-o', '@OUTPUT@' ]
)

spirv_shaders = _gen.process(glsl_shaders)

sources = files('vkgears.c')

if dep_wayland.found()
  sources += files('wayland.c')
  sources += [
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

executable(
  'vkgears', files('vkgears.c'), sources,
  spirv_shaders,
  dependencies: [dep_vulkan, dep_wayland, idep_util]
)
