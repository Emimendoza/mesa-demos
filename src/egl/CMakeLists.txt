
add_subdirectory(eglut)

add_subdirectory(opengl)

if (GLESV1_FOUND)
	add_subdirectory(opengles1)
endif ()

if (GLESV2_FOUND)
	add_subdirectory(opengles2)
endif ()
