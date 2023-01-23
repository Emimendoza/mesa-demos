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

static struct wsi_callbacks wsi_callbacks;

static xcb_connection_t *connection;
static xcb_screen_iterator_t screen_iterator;
static xcb_window_t window;

static xcb_atom_t wm_protocols_atom, delete_atom;

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

   wm_protocols_atom = get_atom(connection, "WM_PROTOCOLS");
   delete_atom = get_atom(connection, "WM_DELETE_WINDOW");
}

static void
fini_display()
{

}

static void
init_window(const char *title, int width, int height, bool fullscreen)
{
   window = xcb_generate_id(connection);
   xcb_create_window(connection,
                     XCB_COPY_FROM_PARENT,
                     window,
                     screen_iterator.data->root,
                     0, 0, width, height,
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

   xcb_change_property(connection,
                       XCB_PROP_MODE_REPLACE,
                       window,
                       wm_protocols_atom,
                       XCB_ATOM_ATOM,
                       sizeof(xcb_atom_t) * 8,
                       1, &delete_atom);

   if (fullscreen) {
       xcb_atom_t fullscreen_atom = get_atom(connection, "_NET_WM_STATE_FULLSCREEN");
       xcb_change_property(connection,
                           XCB_PROP_MODE_REPLACE,
                           window,
                           get_atom(connection, "_NET_WM_STATE"),
                           XCB_ATOM_ATOM,
                           sizeof(xcb_atom_t) * 8,
                           1, &fullscreen_atom);
   }

   xcb_map_window(connection, window);
   xcb_flush(connection);
}

static bool
update_window()
{
   union {
      xcb_generic_event_t *generic;
      xcb_configure_notify_event_t *configure_event;
      xcb_client_message_event_t *client_message;
      xcb_key_press_event_t *key_press;
   } event;

   event.generic = xcb_wait_for_event(connection);
   while(event.generic) {
      switch (event.generic->response_type & 0x7f) {
      case XCB_CONFIGURE_NOTIFY:
         wsi_callbacks.resize(event.configure_event->width, event.configure_event->height);
         break;

      case XCB_CLIENT_MESSAGE:
         if(event.client_message->window == window &&
            event.client_message->type == wm_protocols_atom &&
            event.client_message->data.data32[0] == delete_atom)
         {
            wsi_callbacks.exit();
         }
         break;
      case XCB_KEY_PRESS:
      case XCB_KEY_RELEASE: {
         enum wsi_key key = WSI_KEY_OTHER;
         switch (event.key_press->detail) {
         case 9:
            key = WSI_KEY_ESC;
            break;
         case 111:
            key = WSI_KEY_UP;
            break;
         case 116:
            key = WSI_KEY_DOWN;
            break;
         case 113:
            key = WSI_KEY_LEFT;
            break;
         case 114:
            key =WSI_KEY_RIGHT;
            break;
         case 38:
            key = WSI_KEY_A;
            break;
         }
         wsi_callbacks.key_press(event.generic->response_type == XCB_KEY_PRESS, key);
         break;
      }
      }

      free(event.generic);
      event.generic = xcb_poll_for_event(connection);
   }
   if(event.generic) {
      free(event.generic);
   }

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

static void
set_wsi_callbacks(struct wsi_callbacks callbacks)
{
   wsi_callbacks = callbacks;
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

      .set_wsi_callbacks = set_wsi_callbacks,

      .create_surface = create_surface,
   };
}
