#!/bin/bash

set -e -x -u

PREFIX="/tmp/freeglut-mingw"

if [ ! -f "$PREFIX/include/GL/glut.h" ]
then
	mkdir -p "$PREFIX/include/GL"
	for header in GL/glut.h GL/freeglut.h GL/freeglut_std.h GL/freeglut_ext.h
	do
		cp -av "/usr/include/$header" "$PREFIX/include/$header"
	done
fi

if [ ! -f "$PREFIX/lib/libglut.a" ]
then
	mkdir -p "$PREFIX/lib"
	i686-w64-mingw32-dlltool --kill-at --def .gitlab-ci/freeglut.def --output-lib "$PREFIX/lib/libglut.a"
fi

if [ ! -f "$PREFIX/share/pkgconfig/glut.pc" ]
then
	mkdir -p "$PREFIX/share/pkgconfig"
	cat <<-EOF > "$PREFIX/share/pkgconfig/glut.pc"
	prefix=$PREFIX
	includedir=\${prefix}/include
	libdir=\${prefix}/lib

	Name: glut
	Description: A wrapper for FreeGLUT on MinGW
	Version: 1.0.0
	Cflags: -I\${includedir}
	Libs: -L\${libdir} -lglut
	EOF
fi
