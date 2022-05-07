include_directories (
	${EGL_INCLUDE_DIRS}
)

if (X11_FOUND)
	add_library (eglut_x11 eglut.h eglut.c eglutint.h eglut_x11.c)
	target_link_libraries (eglut_x11 ${EGL_LIBRARIES} ${X11_X11_LIB})
	if (BUILD_SHARED_LIBS)
		install (TARGETS eglut_x11 DESTINATION ${LIBDIR})
	endif (BUILD_SHARED_LIBS)
endif (X11_FOUND)

if (BUILD_WAYLAND)
	ecm_add_wayland_client_protocol (eglut_wayland_proto_SOURCES
		PROTOCOL "${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml"
		BASENAME xdg-shell)

	add_library (eglut_wayland
		eglut.h eglut.c
		eglutint.h eglut_wayland.c
		${eglut_wayland_proto_SOURCES})
	target_compile_options (eglut_wayland PRIVATE ${Wayland_DEFINITIONS})
	target_include_directories (eglut_wayland PRIVATE
		${Wayland_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries (eglut_wayland ${EGL_LIBRARIES} ${Wayland_LIBRARIES})
	if (BUILD_SHARED_LIBS)
		install (TARGETS eglut_wayland DESTINATION ${LIBDIR})
	endif (BUILD_SHARED_LIBS)
endif (BUILD_WAYLAND)
