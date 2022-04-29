Mesa Demos
==========

Introduction
------------

The Mesa Demos package contains a large number of OpenGL demonstration
and test programs.

These programs were previously distributed as the MesaDemos-x.y.z.tar.gz
package released in conjunction with the MesaLib-x.y.z.tar.gz package.
The demos are separated now since they don't change often and
aren't tied to a particular version of Mesa.

Note that the Mesa demos should be usable with any OpenGL implementation;
not just Mesa.

Latest Release
--------------

The latest version of the Mesa demos can be found on the `mesa3d.org
archives <https://archive.mesa3d.org/demos/>`_ site.

Git Repository
--------------

The Mesa demos git repository can be obtained with:

.. code-block:: sh

  $ git clone https://gitlab.freedesktop.org/mesa/demos.git


Building
--------

The Mesa demos can be built with `Meson <https://mesonbuild.com/>`_.


Prerequisites
^^^^^^^^^^^^^

You'll need GLUT or `FreeGLUT <http://freeglut.sourceforge.net/>`_
(runtime libraries and header files).

If you're using an RPM-based Linux distro you can install these items
with:

.. code-block:: sh

  $ yum install freeglut freeglut-devel

And if you're using a Debian based Linux distro you can install these
items with:

.. code-block:: sh

  $ apt-get install freeglut3-dev

For `Wayland <https://wayland.freedesktop.org/>`_ support,
`wayland-protocols <https://gitlab.freedesktop.org/wayland/wayland-protocols>`_
and
`wayland-scanner <https://gitlab.freedesktop.org/wayland/wayland>`_
must be available.

Meson
^^^^^

Run **meson configure** to see the options available for building
the demos.

Otherwise, just run **meson build-dir; meson compile -C build-dir** to
build the demos.

See the `Meson documentation <https://mesonbuild.com/>`_ for more
information about Meson.


MinGW cross-compilation
"""""""""""""""""""""""

Create a **cross-file** file containing:

.. code-block:: ini

  [binaries]
  c = '/usr/bin/x86_64-w64-mingw32-gcc'
  cpp = '/usr/bin/x86_64-w64-mingw32-g++'
  ar = '/usr/bin/x86_64-w64-mingw32-ar'
  strip = '/usr/bin/x86_64-w64-mingw32-strip'
  pkgconfig = 'x86_64-w64-mingw32-pkg-config'
  exe_wrapper = 'wine'

  [host_machine]
  system = 'windows'
  cpu_family = 'x86_64'
  cpu = 'i686'
  endian = 'little'

Then do:

.. code-block:: sh

  $ meson --prefix=/path/to/install --cross-file /path/to/cross-file build-dir
  $ meson compile -C build-dir
  $ meson install -C build-dir

The Demos
---------

Look in the **src/** directory for these sub-directories, among others:

- **demos**: the original Mesa demos, such as gears, gloss, etc.
- **redbook**: the OpenGL Programming Guide demos
- **samples**: original SGI OpenGL demos
- **glsl**: OpenGL Shading Language demos
- **gs**: Geometry shader demos
- **xdemos**: GLX-based demos
- **egl**: EGL-based demos
- **wgl**: WGL-based demos
- **fp**: fragment program tests
- **vp**: vertex program tests
- **data**: data files used by the demos
- **perf**: performance tests
- **tests**: assorted test programs
- **objviewer**: program to view .obj modes with a skybox


Getting Help
------------

If you have trouble building or using the Mesa demos, you can post
to the `Mesa users <http://lists.freedesktop.org/mailman/listinfo/mesa-users>`_
mailing list.

If your question is especially technical, you can try the
to the `Mesa dev <http://lists.freedesktop.org/mailman/listinfo/mesa-dev>`_
mailing list.


Reporting Bugs
--------------

Bugs in the demos can be reported to the
`Mesa demos issue tracker <https://gitlab.freedesktop.org/mesa/demos/-/issues>`_.

