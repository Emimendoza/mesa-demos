#include <wayland-client.h>
#include <wayland-egl.h>

#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "eglutint.h"
#include "wayland-xdg-shell-client-protocol.h"

struct display {
   struct wl_display *display;
   struct wl_compositor *compositor;
   struct xdg_wm_base *xdg_wm_base;
   uint32_t mask;
};

struct window {
   struct wl_surface *surface;
   struct xdg_surface *xdg_surface;
   struct xdg_toplevel *xdg_toplevel;
   bool opaque;
   bool configured;
};

static struct display display = {0, };
static struct window window = {0, };

static void
xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
   xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
   xdg_wm_base_ping
};

static void
registry_handle_global(void *data, struct wl_registry *registry, uint32_t id,
                       const char *interface, uint32_t version)
{
   struct display *d = data;

   if (strcmp(interface, wl_compositor_interface.name) == 0) {
      d->compositor =
         wl_registry_bind(registry, id, &wl_compositor_interface, 1);
   } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
      d->xdg_wm_base =
         wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
      xdg_wm_base_add_listener(d->xdg_wm_base, &xdg_wm_base_listener, d);
   }
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
                              uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
   registry_handle_global,
   registry_handle_global_remove
};

static void
sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
   int *done = data;

   *done = 1;
   wl_callback_destroy(callback);
}

static const struct wl_callback_listener sync_listener = {
   sync_callback
};

static int
wayland_roundtrip(struct wl_display *display)
{
   struct wl_callback *callback;
   int done = 0, ret = 0;

   callback = wl_display_sync(display);
   wl_callback_add_listener(callback, &sync_listener, &done);
   while (ret != -1 && !done)
      ret = wl_display_dispatch(display);

   if (!done)
      wl_callback_destroy(callback);

   return ret;
}

void
_eglutNativeInitDisplay(void)
{
   struct wl_registry *registry;

   _eglut->native_dpy =  display.display = wl_display_connect(NULL);

   if (!_eglut->native_dpy)
      _eglutFatal("failed to initialize native display");

   registry = wl_display_get_registry(_eglut->native_dpy);
   wl_registry_add_listener(registry, &registry_listener, &display);
   wayland_roundtrip(_eglut->native_dpy);
   wl_registry_destroy(registry);

   if (!display.xdg_wm_base)
      _eglutFatal("xdg-shell not supported");

   _eglut->surface_type = EGL_WINDOW_BIT;
   _eglut->redisplay = 1;
}

void
_eglutNativeFiniDisplay(void)
{
   xdg_wm_base_destroy(display.xdg_wm_base);
   wl_compositor_destroy(display.compositor);
   wl_display_flush(_eglut->native_dpy);
   wl_display_disconnect(_eglut->native_dpy);
   _eglut->native_dpy = NULL;
}

static void
xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel,
                       int32_t width, int32_t height,
                       struct wl_array *states)
{
   struct eglut_window *win = data;

   if (width == 0 || height == 0) {
      /* Client should decide its own window dimensions.
       * Keep whatever we have. */
      return;
   }

   wl_egl_window_resize(win->native.u.window, width, height, 0, 0);
   win->native.width = width;
   win->native.height = height;
   if (win->reshape_cb)
      win->reshape_cb(win->native.width, win->native.height);
}

static void
xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel)
{
   struct eglut_window *win = data;
   eglutDestroyWindow(win->index);

   // FIXME: eglut does not terminate when all windows are closed.
   // eglut_x11 dies due to "X connection to $DISPLAY broken".
   // Since wl_display works fine with all windows closed, terminate ourselves.
   eglTerminate(_eglut->dpy);
   _eglutNativeFiniDisplay();
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
   xdg_toplevel_configure,
   xdg_toplevel_close
};

static void
xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                      uint32_t serial)
{
   struct window *window = data;
   xdg_surface_ack_configure(xdg_surface, serial);
   window->configured = true;
}

static const struct xdg_surface_listener xdg_surface_listener = {
   xdg_surface_configure
};

void
_eglutNativeInitWindow(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h)
{
   struct wl_egl_window *native;

   window.surface = wl_compositor_create_surface(display.compositor);

   EGLint alpha_size;
   if (!eglGetConfigAttrib(_eglut->dpy,
            win->config, EGL_ALPHA_SIZE, &alpha_size))
      _eglutFatal("failed to get alpha size");
   window.opaque = !alpha_size;

   window.xdg_surface =
      xdg_wm_base_get_xdg_surface(display.xdg_wm_base, window.surface);
   xdg_surface_add_listener(window.xdg_surface, &xdg_surface_listener, &window);

   window.xdg_toplevel = xdg_surface_get_toplevel(window.xdg_surface);
   xdg_toplevel_add_listener(window.xdg_toplevel, &xdg_toplevel_listener, win);
   xdg_toplevel_set_title(window.xdg_toplevel, title);
   xdg_toplevel_set_app_id(window.xdg_toplevel, title);
   wl_surface_commit(window.surface);

   native = wl_egl_window_create(window.surface, w, h);

   win->native.u.window = native;
   win->native.width = w;
   win->native.height = h;
}

void
_eglutNativeFiniWindow(struct eglut_window *win)
{
   wl_egl_window_destroy(win->native.u.window);

   xdg_toplevel_destroy(window.xdg_toplevel);
   xdg_surface_destroy(window.xdg_surface);
}

static void
draw(struct window *window)
{
   struct eglut_window *win = _eglut->current;

   /* Our client doesn't want to push another frame; go back to sleep. */
   if (!_eglut->redisplay)
      return;
   _eglut->redisplay = 0;

   if (win->display_cb)
      win->display_cb();

   if (window->opaque) {
      struct wl_region *region =
         wl_compositor_create_region(display.compositor);
      wl_region_add(region, 0, 0, win->native.width, win->native.height);
      wl_surface_set_opaque_region(window->surface, region);
      wl_region_destroy(region);
   }

   eglSwapBuffers(_eglut->dpy, win->surface);
}

void
_eglutNativeEventLoop(void)
{
   struct pollfd pollfd;
   int ret;

   pollfd.fd = wl_display_get_fd(display.display);
   pollfd.events = POLLIN;
   pollfd.revents = 0;

   while (!window.configured)
      wl_display_dispatch(display.display);

   while (_eglut->native_dpy != NULL) {
      /* If we need to flush but can't, don't do anything at all which could
       * push further events into the socket. */
      if (!(pollfd.events & POLLOUT)) {
         wl_display_dispatch_pending(display.display);

         if (_eglut->idle_cb)
            _eglut->idle_cb();

         /* Client wants to redraw, but we have no frame event to trigger the
          * redraw; kickstart it by redrawing immediately. */
         if (_eglut->redisplay)
            draw(&window);
      }

      ret = wl_display_flush(display.display);
      if (ret < 0 && errno != EAGAIN)
         break; /* fatal error; socket is broken */
      else if (ret < 0 && errno == EAGAIN)
         pollfd.events |= POLLOUT; /* need to wait until we can flush */
      else
         pollfd.events &= ~POLLOUT; /* successfully flushed */

      if (poll(&pollfd, 1, -1) == -1)
         break;

      if (pollfd.revents & (POLLERR | POLLHUP | POLLNVAL))
         break;

      if (pollfd.events & POLLOUT) {
	 if (!(pollfd.revents & POLLOUT))
            continue; /* block until we can flush */
         pollfd.events &= ~POLLOUT;
      }

      if (pollfd.revents & POLLIN) {
         ret = wl_display_dispatch(display.display);
         if (ret == -1)
            break;
      }

      ret = wl_display_flush(display.display);
      if (ret < 0 && errno != EAGAIN)
         break; /* fatal error; socket is broken */
      else if (ret < 0 && errno == EAGAIN)
         pollfd.events |= POLLOUT; /* need to wait until we can flush */
      else
         pollfd.events &= ~POLLOUT; /* successfully flushed */
   }
}
