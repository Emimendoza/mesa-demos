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

#ifndef WSI_H_
#define WSI_H_

#include <stdbool.h>

struct eglut_window;

struct eglut_wsi_interface {
   void (*init_display)();
   void (*fini_display)();

   void (*init_window)(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h);
   void (*event_loop)();
   void (*fini_window)(struct eglut_window *win);
};

#ifdef WAYLAND_SUPPORT
struct eglut_wsi_interface
wayland_wsi_interface(void);
#endif

#ifdef X11_SUPPORT
struct eglut_wsi_interface
x11_wsi_interface(void);
#endif

struct eglut_wsi_interface
_eglutGetWindowSystemInterface(void);

#endif /* WSI_H_ */
