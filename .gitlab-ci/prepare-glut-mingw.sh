#!/bin/bash

set -e -x -u

GLUT_INCLUDE_DIR="/tmp/freeglut-mingw/include"
GLUT_LIBRARY="/tmp/freeglut-mingw/lib/libglut.a"
if [ ! -f "$GLUT_INCLUDE_DIR/GL/glut.h" ]
then
	mkdir -p "$GLUT_INCLUDE_DIR/GL"
	for header in GL/glut.h GL/freeglut.h GL/freeglut_std.h GL/freeglut_ext.h
	do
		cp -av "/usr/include/$header" "$GLUT_INCLUDE_DIR/$header"
	done
fi
if [ ! -f "$GLUT_LIBRARY" ]
then
	mkdir -p /tmp/freeglut-mingw/lib
	i686-w64-mingw32-dlltool --kill-at --def .gitlab-ci/freeglut.def --output-lib "$GLUT_LIBRARY"
fi
