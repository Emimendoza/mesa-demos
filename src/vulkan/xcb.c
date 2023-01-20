#include <xcb/xcb.h>

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xcb.h>
#include <xcb/xproto.h>


#include "wsi.h"

static xcb_connection_t *connection;
static xcb_screen_iterator_t screen_iterator;
static xcb_window_t window;

static xcb_atom_t
get_atom(struct xcb_connection_t *conn, const char *name)
{
   xcb_intern_atom_cookie_t cookie;
   xcb_intern_atom_reply_t *reply;
   xcb_atom_t atom;

   cookie = xcb_intern_atom(conn, 0, strlen(name), name);
   reply = xcb_intern_atom_reply(conn, cookie, NULL);
   if (reply)
      atom = reply->atom;
   else
      atom = XCB_NONE;

   free(reply);
   return atom;
}

static void
init_display()
{
   connection = xcb_connect(NULL, NULL);
   screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(connection));
}

static void
fini_display()
{

}

static void
init_window(const char *title)
{
   window = xcb_generate_id(connection);
   xcb_create_window(connection,
                     XCB_COPY_FROM_PARENT,
                     window,
                     screen_iterator.data->root,
                     0, 0, 300, 300,
                     0,
                     XCB_WINDOW_CLASS_INPUT_OUTPUT,
                     screen_iterator.data->root_visual,
                     XCB_CW_EVENT_MASK,
                     (uint32_t[]){
                         XCB_EVENT_MASK_EXPOSURE |
                         XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                         XCB_EVENT_MASK_KEY_PRESS
                     });

   xcb_change_property(connection,
                       XCB_PROP_MODE_REPLACE,
                       window,
                       get_atom(connection, "_NET_WM_NAME"),
                       get_atom(connection, "UTF8_STRING"),
                       8,
                       strlen(title), title);

   xcb_map_window(connection, window);
}

static bool
update_window()
{
   xcb_generic_event_t *event;
   event = xcb_wait_for_event(connection);

   xcb_client_message_event_t client_message;

   client_message.response_type = XCB_CLIENT_MESSAGE;
   client_message.format = 32;
   client_message.window = window;
   client_message.type = XCB_ATOM_NOTICE;

   xcb_send_event(connection, 0, window,
                  0, (char *) &client_message);

   xcb_flush(connection);

   return false;
}

static void
fini_window()
{

}

#define GET_INSTANCE_PROC(name) \
   PFN_ ## name name = (PFN_ ## name)vkGetInstanceProcAddr(instance, #name);

static bool
create_surface(VkPhysicalDevice physical_device, VkInstance instance,
               VkSurfaceKHR *surface)
{

   GET_INSTANCE_PROC(vkGetPhysicalDeviceXcbPresentationSupportKHR)
   GET_INSTANCE_PROC(vkCreateXcbSurfaceKHR)

   if (!vkGetPhysicalDeviceXcbPresentationSupportKHR ||
       !vkCreateXcbSurfaceKHR) {
      fprintf(stderr, "Failed to load extension functions\n");
      return VK_NULL_HANDLE;
   }

   if (!vkGetPhysicalDeviceXcbPresentationSupportKHR(physical_device,
                                                     0,
                                                     connection,
                                                     screen_iterator.data->root_visual)) {
      fprintf(stderr, "Vulkan not supported on given XCB surface\n");
      return false;
   }

   vkCreateXcbSurfaceKHR(instance,
                         &(VkXcbSurfaceCreateInfoKHR){
                            .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
                            .connection = connection,
                            .window = window,
                         },
                         NULL,
                         surface);

   return true;
}

struct wsi_interface
xcb_wsi_interface() {
   return (struct wsi_interface) {
      .required_extension_name = VK_KHR_XCB_SURFACE_EXTENSION_NAME,

      .init_display = init_display,
      .fini_display = fini_display,

      .init_window = init_window,
      .update_window = update_window,
      .fini_window = fini_window,

      .create_surface = create_surface,
   };
}
