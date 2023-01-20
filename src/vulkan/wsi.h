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

struct wsi_interface {
   const char *required_extension_name;

   void (*init_display)();
   void (*fini_display)();

   void (*init_window)(const char *title);
   bool (*update_window)();
   void (*fini_window)();

   bool (*create_surface)(VkPhysicalDevice physical_device, VkInstance instance,
               VkSurfaceKHR *surface);
};

#endif // WSI_H_
