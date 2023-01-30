#include "wsi.h"

#include <stdlib.h>

struct wsi_interface
wsi_interface()
{
#if defined(WAYLAND_SUPPORT) && defined(XCB_SUPPORT)
   return getenv("WAYLAND_DISPLAY") ? wayland_wsi_interface() :
                                      xcb_wsi_interface();
#elif defined(WAYLAND_SUPPORT)
   return wayland_wsi_interface();
#elif defined(X11_SUPPORT)
   return xcb_wsi_interface();
#endif
}
