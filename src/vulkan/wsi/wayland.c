#include <stdint.h>
#include <sys/poll.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "wayland-xdg-shell-client-protocol.h"
#include <wayland-util.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>

#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

#include "wsi.h"

static struct wsi_callbacks wsi_callbacks;

static struct wl_display *display;
static struct {
   struct wl_seat *seat;
   struct wl_keyboard *keyboard;
   struct xkb_context *xkb_context;
   struct xkb_keymap *xkb_keymap;
   struct xkb_state *xkb_state;
   int32_t rate, delay;
   int keyboard_timer_fd;
   xkb_keycode_t repeat_scancode;
} keyboard_data;
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
   } else if (strcmp(interface, wl_seat_interface.name) == 0) {
      keyboard_data.seat =
         wl_registry_bind(registry, id, &wl_seat_interface, 4);
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

static void
dispatch_key(xkb_keycode_t xkb_key, enum wl_keyboard_key_state state)
{
   xkb_keysym_t sym = xkb_state_key_get_one_sym(keyboard_data.xkb_state, xkb_key);
   enum wsi_key wsi_key = WSI_KEY_OTHER;
   switch (sym) {
   case XKB_KEY_Escape:
      wsi_key = WSI_KEY_ESC;
      break;
   case XKB_KEY_Up:
      wsi_key = WSI_KEY_UP;
      break;
   case XKB_KEY_Down:
      wsi_key = WSI_KEY_DOWN;
      break;
   case XKB_KEY_Left:
      wsi_key = WSI_KEY_LEFT;
      break;
   case XKB_KEY_Right:
      wsi_key =WSI_KEY_RIGHT;
      break;
   case XKB_KEY_A:
   case XKB_KEY_a:
      wsi_key = WSI_KEY_A;
      break;
   }
   wsi_callbacks.key_press(state, wsi_key);
}

static void
handle_key(uint key, enum wl_keyboard_key_state state)
{
   xkb_keycode_t xkb_key = key + 8;
   struct itimerspec timer = {0};
   if (keyboard_data.rate != 0 &&
       xkb_keymap_key_repeats(keyboard_data.xkb_keymap, xkb_key) &&
       state == WL_KEYBOARD_KEY_STATE_PRESSED) {
      keyboard_data.repeat_scancode = xkb_key;
      if (keyboard_data.rate > 1) {
         timer.it_interval.tv_nsec = 1000000000 / keyboard_data.rate;
      } else {
         timer.it_interval.tv_sec = 1;
      }

      timer.it_value.tv_sec = keyboard_data.delay / 1000;
      timer.it_value.tv_nsec = (keyboard_data.delay % 1000) * 1000000;
   }
   timerfd_settime(keyboard_data.keyboard_timer_fd, 0, &timer, NULL);

   dispatch_key(xkb_key, state);
}

static void
key(void *data, struct wl_keyboard *keyboard, uint serial,
    uint time, uint key, enum wl_keyboard_key_state state)
{
   handle_key(key, state);
}

static void
modifiers(void *data, struct wl_keyboard *keyboard, uint serial,
    uint mods_depressed, uint mods_latched, uint mods_locked, uint group)
{
   xkb_state_update_mask(keyboard_data.xkb_state, mods_depressed, mods_latched,
                         mods_locked, 0, 0, group);
}

static void
keymap(void *data, struct wl_keyboard *keyboard,
       enum wl_keyboard_keymap_format format, int32_t fd, uint32_t size)
{
   assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

   char *map_shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
   assert(map_shm != MAP_FAILED);

   keyboard_data.xkb_keymap = xkb_keymap_new_from_string(
      keyboard_data.xkb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
      XKB_KEYMAP_COMPILE_NO_FLAGS);
   munmap(map_shm, size);
   close(fd);

   keyboard_data.xkb_state = xkb_state_new(keyboard_data.xkb_keymap);
}

static void
enter(void *data, struct wl_keyboard *keyboard, uint serial,
      struct wl_surface *surface, struct wl_array *keys)
{
   uint32_t *key;
   wl_array_for_each(key, keys)
      handle_key(*key, WL_KEYBOARD_KEY_STATE_PRESSED);

}

static void
leave(void *data, struct wl_keyboard *keyboard, uint serial,
      struct wl_surface *surface)
{
   struct itimerspec timer = {0};
   timerfd_settime(keyboard_data.keyboard_timer_fd, 0, &timer, NULL);
}

static void
repeat_info_callback(void *data, struct wl_keyboard *wl_keyboard,
                        int32_t rate, int32_t delay)
{
   keyboard_data.rate = rate;
   keyboard_data.delay = delay;
}


static const struct wl_keyboard_listener keyboard_listener = {
   .key = key,
   .modifiers = modifiers,
   .keymap = keymap,
   .enter = enter,
   .leave = leave,
   .repeat_info = repeat_info_callback,
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

   keyboard_data.keyboard = wl_seat_get_keyboard(keyboard_data.seat);
   keyboard_data.keyboard_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);

   wl_keyboard_add_listener(keyboard_data.keyboard, &keyboard_listener, NULL);

   if (!compositor) {
      fprintf(stderr, "failed to bind compositor");
      abort();
   }

   if (!xdg_wm_base) {
      fprintf(stderr, "xdg-shell not supported");
      abort();
   }

   keyboard_data.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
}

void fini_display()
{
   wl_seat_destroy(keyboard_data.seat);
   xkb_context_unref(keyboard_data.xkb_context);
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

   wsi_callbacks.resize(width, height);
   xdg_surface_set_window_geometry(xdg_surface, 0, 0, width, height);
   wl_surface_commit(surface);
}

static void
xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel)
{
   wsi_callbacks.exit();
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
   xdg_toplevel_configure,
   xdg_toplevel_close
};

static void init_window(const char *title, int width, int height, bool fullscreen)
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
   if (fullscreen)
      xdg_toplevel_set_fullscreen(xdg_toplevel, NULL);
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
   int ret;

   struct pollfd pollfds[] = {
      {
         .fd = wl_display_get_fd(display),
         .events = POLLIN
      },
      {
         .fd = keyboard_data.keyboard_timer_fd,
         .events = POLLIN
      },
   };

   while (1) {
      /* If we need to flush but can't, don't do anything at all which could
       * push further events into the socket. */
      if (!(pollfds[0].events & POLLOUT))
         wl_display_dispatch_pending(display);

      ret = wl_display_flush(display);
      if (ret < 0 && errno != EAGAIN)
         break; /* fatal error; socket is broken */
      else if (ret < 0 && errno == EAGAIN)
         pollfds[0].events |= POLLOUT; /* need to wait until we can flush */
      else
         pollfds[0].events &= ~POLLOUT; /* successfully flushed */

      if (poll(pollfds, keyboard_data.rate > 0 ? 2 : 1, 0.0) == -1)
         break;

      if (pollfds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
         break;

      if (pollfds[0].events & POLLOUT) {
         if (!(pollfds[0].revents & POLLOUT))
            continue; /* block until we can flush */
         pollfds[0].events &= ~POLLOUT;
      }

      if (pollfds[0].revents & POLLIN) {
         ret = wl_display_dispatch(display);
         if (ret == -1)
            break;
      }

      ret = wl_display_flush(display);
      if (ret < 0 && errno != EAGAIN)
         break; /* fatal error; socket is broken */
      else if (ret < 0 && errno == EAGAIN)
         pollfds[0].events |= POLLOUT; /* need to wait until we can flush */
      else
         pollfds[0].events &= ~POLLOUT; /* successfully flushed */

      if (pollfds[1].revents & POLLIN) {
         uint64_t repeats;
         if(read(keyboard_data.keyboard_timer_fd, &repeats, sizeof(repeats)) == 8) {
            for(uint64_t i = 0; i < repeats; i++) {
               dispatch_key(keyboard_data.repeat_scancode, 1);
            }
         }
      }

      if (!(pollfds[0].events & POLLOUT))
         return false;
   }

   return true;
}

static void
set_wsi_callbacks(struct wsi_callbacks callbacks)
{
   wsi_callbacks = callbacks;
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

      .set_wsi_callbacks = set_wsi_callbacks,

      .create_surface = create_surface,
   };
}
