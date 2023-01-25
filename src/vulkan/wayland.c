#include <wayland-client.h>

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wayland-xdg-shell-client-protocol.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>

#include "wsi.h"

static struct wl_display *display;
static struct wl_compositor *compositor;
static struct xdg_wm_base *xdg_wm_base;

struct wl_surface *surface;
struct xdg_surface *xdg_surface;
struct xdg_toplevel *xdg_toplevel;
bool configured;

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
   if (strcmp(interface, wl_compositor_interface.name) == 0) {
      assert(!compositor);
      compositor =
         wl_registry_bind(registry, id, &wl_compositor_interface, 1);
   } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
      assert(!xdg_wm_base);
      xdg_wm_base =
         wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
      xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, NULL);
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

static void init_display()
{
   assert(!display);
   display = wl_display_connect(NULL);
   if (!display) {
      fprintf(stderr, "failed to connect to display");
      abort();
   }

   struct wl_registry *registry = wl_display_get_registry(display);
   wl_registry_add_listener(registry, &registry_listener, NULL);
   wl_display_roundtrip(display);
   wl_registry_destroy(registry);

   if (!compositor) {
      fprintf(stderr, "failed to bind compositor");
      abort();
   }

   if (!xdg_wm_base) {
      fprintf(stderr, "xdg-shell not supported");
      abort();
   }
}

void fini_display()
{
   xdg_wm_base_destroy(xdg_wm_base);
   wl_compositor_destroy(compositor);
   wl_display_flush(display);
   wl_display_disconnect(display);
}

static void
xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                      uint32_t serial)
{
   xdg_surface_ack_configure(xdg_surface, serial);
   configured = true;
}

static const struct xdg_surface_listener xdg_surface_listener = {
   xdg_surface_configure
};

static void
xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel,
                       int32_t width, int32_t height,
                       struct wl_array *states)
{
   if (width <= 0 || height <= 0) {
      return;
   }

   /* TODO: handle resize */
}

static void
xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel)
{
   /* TODO: handle close event */
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
   xdg_toplevel_configure,
   xdg_toplevel_close
};

static void init_window(const char *title)
{
   assert(xdg_wm_base && compositor);

   surface = wl_compositor_create_surface(compositor);
   xdg_surface =
      xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
   xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);

   xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
   xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);
   xdg_toplevel_set_title(xdg_toplevel, title);
   xdg_toplevel_set_app_id(xdg_toplevel, title);
   wl_surface_commit(surface);

   while (!configured)
      wl_display_dispatch(display);
}

static void fini_window()
{
   xdg_toplevel_destroy(xdg_toplevel);
   xdg_surface_destroy(xdg_surface);
}


static bool update_window()
{
   struct pollfd pollfd;
   int ret;

   pollfd.fd = wl_display_get_fd(display);
   pollfd.events = POLLIN;
   pollfd.revents = 0;

   while (1) {
      /* If we need to flush but can't, don't do anything at all which could
       * push further events into the socket. */
      if (!(pollfd.events & POLLOUT)) {
         wl_display_dispatch_pending(display);

         return false;
      }

      ret = wl_display_flush(display);
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
         ret = wl_display_dispatch(display);
         if (ret == -1)
            break;
      }

      ret = wl_display_flush(display);
      if (ret < 0 && errno != EAGAIN)
         break; /* fatal error; socket is broken */
      else if (ret < 0 && errno == EAGAIN)
         pollfd.events |= POLLOUT; /* need to wait until we can flush */
      else
         pollfd.events &= ~POLLOUT; /* successfully flushed */
   }

   return true;
}

#define GET_INSTANCE_PROC(name) \
   PFN_ ## name name = (PFN_ ## name)vkGetInstanceProcAddr(instance, #name);

static bool
create_surface(VkPhysicalDevice physical_device, VkInstance instance,
               VkSurfaceKHR *psurface)
{
   GET_INSTANCE_PROC(vkGetPhysicalDeviceWaylandPresentationSupportKHR)
   GET_INSTANCE_PROC(vkCreateWaylandSurfaceKHR)

   if (!vkGetPhysicalDeviceWaylandPresentationSupportKHR ||
       !vkCreateWaylandSurfaceKHR) {
      fprintf(stderr, "Failed to load extension functions\n");
      return VK_NULL_HANDLE;
   }

   if (!vkGetPhysicalDeviceWaylandPresentationSupportKHR(
         physical_device, 0, display)) {
      fprintf(stderr, "Vulkan not supported on given Wayland surface\n");
      return false;
   }

   VkResult result =
      vkCreateWaylandSurfaceKHR(instance,
                                &(VkWaylandSurfaceCreateInfoKHR) {
           .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
           .display = display,
           .surface = surface,
        }, NULL, psurface);

   return result == VK_SUCCESS;
}

struct wsi_interface
wayland_wsi_interface() {
   return (struct wsi_interface) {
      .required_extension_name = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,

      .init_display = init_display,
      .fini_display = fini_display,

      .init_window = init_window,
      .update_window = update_window,
      .fini_window = fini_window,

      .create_surface = create_surface,
   };
}
