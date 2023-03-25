/*
 * Copyright © 2023 Hoe Hao Cheng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "wsi.h"

#include <stdlib.h>

struct eglut_wsi_interface
_eglutGetWindowSystemInterface(void)
{
#if defined(WAYLAND_SUPPORT) && defined(X11_SUPPORT)
   const char *wayland_dpy = getenv("WAYLAND_DISPLAY");
   return wayland_dpy && *wayland_dpy ? wayland_wsi_interface() :
                                        x11_wsi_interface();
#elif defined(WAYLAND_SUPPORT)
   return wayland_wsi_interface();
#elif defined(X11_SUPPORT)
   return x11_wsi_interface();
#endif
}
