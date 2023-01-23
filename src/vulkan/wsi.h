#ifndef WSI_H_
#define WSI_H_

#include <stdbool.h>

#include<vulkan/vulkan.h>

#ifdef WAYLAND_SUPPORT
struct wsi_interface
wayland_wsi_interface();
#endif

#ifdef XCB_SUPPORT
struct wsi_interface
xcb_wsi_interface();
#endif

enum wsi_key {
   WSI_KEY_UP,
   WSI_KEY_DOWN,
   WSI_KEY_LEFT,
   WSI_KEY_RIGHT,
   WSI_KEY_A,
   WSI_KEY_ESC,
   WSI_KEY_OTHER,
};

struct wsi_callbacks {
   void (*resize)(int new_width, int new_height);
   void (*key_press)(bool down, enum wsi_key key);
   void (*exit)();
};

struct wsi_interface {
   const char *required_extension_name;

   void (*init_display)();
   void (*fini_display)();

   void (*init_window)(const char *title, int width, int height, bool fullscreen);
   bool (*update_window)();
   void (*fini_window)();

   void (*set_wsi_callbacks)(struct wsi_callbacks);

   bool (*create_surface)(VkPhysicalDevice physical_device, VkInstance instance,
               VkSurfaceKHR *surface);
};

#endif // WSI_H_
