/*
 * eglinfo - like glxinfo but for EGL
 *
 * Brian Paul
 * 11 March 2005
 *
 * Copyright (C) 2005  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define EGL_EGLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gl_versions.h"
#include <glad/glad.h>

#define MAX_CONFIGS 1000
#define MAX_MODES 1000
#define MAX_SCREENS 10
#define MAX_COLUMN 70

#define ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

/* These are X visual types, so if you're running eglinfo under
 * something not X, they probably don't make sense. */
static const char *vnames[] = { "SG", "GS", "SC", "PC", "TC", "DC" };

struct platform {
   /* names as defined in the EGL spec */
   const char *names[2];
   /* used by parse_args */
   const char *short_name;
   /* a human-readable name */
   const char *human_name;
   EGLenum platform_enum;
};

enum api {
   OPENGL = 0,
   OPENGL_CORE = 1,
   OPENGL_ES = 2,
   /* OpenVG = 3, */
   ALL = ~0,
};

static const char *apis[3] = {
   [OPENGL] = "gl",
   [OPENGL_CORE] = "glcore",
   [OPENGL_ES] = "gles",
   /* [OpenVG] = "vg", */
};

static const struct platform platforms[] = {
   {
      .names = { "EGL_KHR_platform_android" },
      .short_name = "android",
      .human_name = "Android",
      .platform_enum = EGL_PLATFORM_ANDROID_KHR,
   },
   {
      .names = { "EGL_MESA_platform_gbm", "EGL_KHR_platform_gbm" },
      .short_name = "gbm",
      .human_name = "GBM",
      .platform_enum = EGL_PLATFORM_GBM_MESA,
   },
   {
      .names = { "EGL_EXT_platform_wayland", "EGL_KHR_platform_wayland" },
      .short_name = "wayland",
      .human_name = "Wayland",
      .platform_enum = EGL_PLATFORM_WAYLAND_EXT,
   },
   {
      .names = { "EGL_EXT_platform_x11", "EGL_KHR_platform_x11" },
      .short_name = "x11",
      .human_name = "X11",
      .platform_enum = EGL_PLATFORM_X11_EXT,
   },
   {
      .names = { "EGL_MESA_platform_surfaceless" },
      .short_name = "surfaceless",
      .human_name = "Surfaceless",
      .platform_enum = EGL_PLATFORM_SURFACELESS_MESA,
   },
};

struct options {
   unsigned platform;
   unsigned api;
   EGLBoolean brief;
};

/**
 * Print table of all available configurations.
 */
static void
PrintConfigs(EGLDisplay d)
{
   EGLConfig configs[MAX_CONFIGS];
   EGLint numConfigs, i;

   eglGetConfigs(d, configs, MAX_CONFIGS, &numConfigs);

   printf("Configurations:\n");
   printf("     bf lv colorbuffer dp st  ms    vis   cav bi  renderable  supported\n");
   printf("  id sz  l  r  g  b  a th cl ns b    id   eat nd gl es es2 vg surfaces \n");
   /*        ^  ^   ^  ^  ^  ^  ^ ^  ^  ^  ^    ^    ^   ^  ^  ^  ^   ^  ^
    *        |  |   |  |  |  |  | |  |  |  |    |    |   |  |  |  |   |  |
    *        |  |   |  |  |  |  | |  |  |  |    |    |   |  |  |  |   |  EGL_SURFACE_TYPE
    *        |  |   |  |  |  |  | |  |  |  |    |    |   |  EGL_RENDERABLE_TYPE
    *        |  |   |  |  |  |  | |  |  |  |    |    |   EGL_BIND_TO_TEXTURE_RGB/EGL_BIND_TO_TEXTURE_RGBA
    *        |  |   |  |  |  |  | |  |  |  |    |    EGL_CONFIG_CAVEAT
    *        |  |   |  |  |  |  | |  |  |  |    EGL_NATIVE_VISUAL_ID/EGL_NATIVE_VISUAL_TYPE
    *        |  |   |  |  |  |  | |  |  |  EGL_SAMPLE_BUFFERS
    *        |  |   |  |  |  |  | |  |  EGL_SAMPLES
    *        |  |   |  |  |  |  | |  EGL_STENCIL_SIZE
    *        |  |   |  |  |  |  | EGL_DEPTH_SIZE
    *        |  |   |  |  |  |  EGL_ALPHA_SIZE
    *        |  |   |  |  |  EGL_BLUE_SIZE
    *        |  |   |  |  EGL_GREEN_SIZE
    *        |  |   |  EGL_RED_SIZE
    *        |  |   EGL_LEVEL
    *        |  EGL_BUFFER_SIZE
    *        EGL_CONFIG_ID
    */
   printf("---------------------------------------------------------------------\n");
   for (i = 0; i < numConfigs; i++) {
      EGLint id, size, level;
      EGLint red, green, blue, alpha;
      EGLint depth, stencil;
      EGLint renderable, surfaces;
      EGLint vid, vtype, caveat, bindRgb, bindRgba;
      EGLint samples, sampleBuffers;
      char surfString[100] = "";

      eglGetConfigAttrib(d, configs[i], EGL_CONFIG_ID, &id);
      eglGetConfigAttrib(d, configs[i], EGL_BUFFER_SIZE, &size);
      eglGetConfigAttrib(d, configs[i], EGL_LEVEL, &level);

      eglGetConfigAttrib(d, configs[i], EGL_RED_SIZE, &red);
      eglGetConfigAttrib(d, configs[i], EGL_GREEN_SIZE, &green);
      eglGetConfigAttrib(d, configs[i], EGL_BLUE_SIZE, &blue);
      eglGetConfigAttrib(d, configs[i], EGL_ALPHA_SIZE, &alpha);
      eglGetConfigAttrib(d, configs[i], EGL_DEPTH_SIZE, &depth);
      eglGetConfigAttrib(d, configs[i], EGL_STENCIL_SIZE, &stencil);
      eglGetConfigAttrib(d, configs[i], EGL_NATIVE_VISUAL_ID, &vid);
      eglGetConfigAttrib(d, configs[i], EGL_NATIVE_VISUAL_TYPE, &vtype);

      eglGetConfigAttrib(d, configs[i], EGL_CONFIG_CAVEAT, &caveat);
      eglGetConfigAttrib(d, configs[i], EGL_BIND_TO_TEXTURE_RGB, &bindRgb);
      eglGetConfigAttrib(d, configs[i], EGL_BIND_TO_TEXTURE_RGBA, &bindRgba);
      eglGetConfigAttrib(d, configs[i], EGL_RENDERABLE_TYPE, &renderable);
      eglGetConfigAttrib(d, configs[i], EGL_SURFACE_TYPE, &surfaces);

      eglGetConfigAttrib(d, configs[i], EGL_SAMPLES, &samples);
      eglGetConfigAttrib(d, configs[i], EGL_SAMPLE_BUFFERS, &sampleBuffers);

      if (surfaces & EGL_WINDOW_BIT)
         strcat(surfString, "win,");
      if (surfaces & EGL_PBUFFER_BIT)
         strcat(surfString, "pb,");
      if (surfaces & EGL_PIXMAP_BIT)
         strcat(surfString, "pix,");
      if (surfaces & EGL_STREAM_BIT_KHR)
         strcat(surfString, "str,");
      if (strlen(surfString) > 0)
         surfString[strlen(surfString) - 1] = 0;

      printf("0x%02x %2d %2d %2d %2d %2d %2d %2d %2d %2d%2d 0x%02x%s ",
             id, size, level,
             red, green, blue, alpha,
             depth, stencil,
             samples, sampleBuffers, vid, vtype < 6 ? vnames[vtype] : "--");
      printf("  %c  %c  %c  %c  %c   %c %s\n",
             (caveat != EGL_NONE) ? 'y' : ' ',
             (bindRgba) ? 'a' : (bindRgb) ? 'y' : ' ',
             (renderable & EGL_OPENGL_BIT) ? 'y' : ' ',
             (renderable & EGL_OPENGL_ES_BIT) ? 'y' : ' ',
             (renderable & EGL_OPENGL_ES2_BIT) ? 'y' : ' ',
             (renderable & EGL_OPENVG_BIT) ? 'y' : ' ',
             surfString);
   }
}


static const char *
PrintExtensions(const char *extensions)
{
   const char *p, *end, *next;
   int column;

   column = 0;
   end = extensions + strlen(extensions);

   for (p = extensions; p < end; p = next + 1) {
      next = strchr(p, ' ');
      if (next == NULL)
         next = end;

      if (column > 0 && column + next - p + 1 > MAX_COLUMN) {
         printf("\n");
         column = 0;
      }
      if (column == 0)
         printf("    ");
      else
         printf(" ");
      column += next - p + 1;

      printf("%.*s", (int) (next - p), p);

      p = next + 1;
   }

   if (column > 0)
      printf("\n");

   return extensions;
}


static const char *
PrintDisplayExtensions(EGLDisplay d)
{
   const char *extensions;

   extensions = eglQueryString(d, EGL_EXTENSIONS);
   if (!extensions)
      return NULL;

#ifdef EGL_MESA_query_driver
   if (strstr(extensions, "EGL_MESA_query_driver")) {
      PFNEGLGETDISPLAYDRIVERNAMEPROC getDisplayDriverName =
         (PFNEGLGETDISPLAYDRIVERNAMEPROC)
            eglGetProcAddress("eglGetDisplayDriverName");
      printf("EGL driver name: %s\n", getDisplayDriverName(d));
   }
#endif

   puts(d == EGL_NO_DISPLAY ? "EGL client extensions string:" :
                              "EGL extensions string:");

   return PrintExtensions(extensions);
}


static const char *
PrintDeviceExtensions(EGLDeviceEXT d)
{
   PFNEGLQUERYDEVICESTRINGEXTPROC queryDeviceString =
     (PFNEGLQUERYDEVICESTRINGEXTPROC)
     eglGetProcAddress("eglQueryDeviceStringEXT");
   const char *extensions;

   puts("EGL device extensions string:");

   extensions = queryDeviceString(d, EGL_EXTENSIONS);
   if (!extensions)
      return NULL;

   return PrintExtensions(extensions);
}


/* Note that this function has a different return type than the other Print*
 * functions.
 */
static EGLBoolean
PrintContextExtensions(const char *api_name)
{
   if (!glGetStringi)
      return 0;

   int num_extensions;
   glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);

   printf("%s extensions:\n", api_name);

   unsigned column = 0;

   for (int i = 0; i < num_extensions; i++) {
      const char *extension = (const char *) glGetStringi(GL_EXTENSIONS, i);
      unsigned extension_len = strlen(extension);
      if (column > 0 && column + extension_len > MAX_COLUMN) {
         printf("\n");
         column = 0;
      }

      if (column == 0)
         printf("    ");
      else
         printf(" ");

      printf("%s", extension);

      column += extension_len;
   }

   if (column > 0)
      printf("\n");

   return 1;
}


static EGLConfig
chooseEGLConfig(EGLDisplay d, int api_bitmask)
{
   const int attribs[] = {
      EGL_CONFORMANT,      api_bitmask,
      EGL_RED_SIZE,        1,
      EGL_GREEN_SIZE,      1,
      EGL_BLUE_SIZE,       1,
      EGL_ALPHA_SIZE,      1,
      EGL_RENDERABLE_TYPE, api_bitmask,
      EGL_NONE
   };

   int num_configs = 0;
   EGLConfig configs[MAX_CONFIGS];
   eglChooseConfig(d, attribs, configs, MAX_CONFIGS, &num_configs);

   if (num_configs == 0) {
      return NULL;
   } else {
      return configs[0];
   }
}

static EGLContext
createEGLContext(EGLDisplay d, EGLConfig conf, int api,
                 EGLBoolean khr_create_context,
                 EGLBoolean core_profile)
{
   EGLContext ctx;

   /* can't create core GL context without KHR_create_context */
   if (core_profile && !khr_create_context) {
      return NULL;
   }

   if (api == EGL_OPENGL_API) {
      for (int i = 0; gl_versions[i].major > 0; i++) {
         /* if requesting for core profile, don't bother below GL 3.0 */
         if (core_profile &&
             gl_versions[i].major == 3 &&
             gl_versions[i].minor == 0)
            return NULL;

         const int attribs_new[] = {
            EGL_CONTEXT_MAJOR_VERSION, gl_versions[i].major,
            EGL_CONTEXT_MINOR_VERSION, gl_versions[i].minor,
            EGL_CONTEXT_OPENGL_PROFILE_MASK,
            core_profile ? EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT :
                           EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
            EGL_NONE,
         };

         const int attribs_old[] = {
            EGL_CONTEXT_CLIENT_VERSION, gl_versions[i].major,
            EGL_NONE,
         };

         const int *attribs = khr_create_context ? attribs_new : attribs_old;

         ctx = eglCreateContext(d, conf, EGL_NO_CONTEXT, attribs);

         if (ctx) {
            if (!eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
               eglDestroyContext(d, ctx);
               continue;
            }
            gladLoadGLLoader((GLADloadproc) eglGetProcAddress);
            return ctx;
         }
      }
      /* couldn't get core profile context */
      return NULL;
   }

   if (api == EGL_OPENGL_ES_API) {
      /* start from GLES3, then try to create context until we succeed
       * or reach GLES1
       */
      for (int i = 3; i > 0; i--) {
         const int attribs[] = {
            EGL_CONTEXT_MAJOR_VERSION, i,
            EGL_NONE,
         };
         ctx = eglCreateContext(d, conf, EGL_NO_CONTEXT, attribs);

         if (ctx) {
            if (!eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
               eglDestroyContext(d, ctx);
               continue;
            }

            if (i >= 2)
               gladLoadGLES2Loader((GLADloadproc) eglGetProcAddress);
            else
               gladLoadGLES1Loader((GLADloadproc) eglGetProcAddress);
            return ctx;
         }
      }
   }

   return NULL;
}

static int
doOneContext(EGLDisplay d,
             EGLContext ctx,
             const char *api_name,
             struct options opts)
{
   if (!glGetString || !glGetIntegerv)
      return 1;

   printf("%s vendor: %s\n", api_name, glGetString(GL_VENDOR));
   printf("%s renderer: %s\n", api_name, glGetString(GL_RENDERER));
   printf("%s version: %s\n", api_name, glGetString(GL_VERSION));
   printf("%s shading language version: %s\n", api_name,
          glGetString(GL_SHADING_LANGUAGE_VERSION));

   if (!opts.brief && !PrintContextExtensions(api_name))
      return 1;

   eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   return 0;
}


static int
doOneDisplay(EGLDisplay d, const char *name, struct options opts)
{
   int maj, min;

   printf("%s platform:\n", name);
   if (!eglInitialize(d, &maj, &min)) {
      printf("eglinfo: eglInitialize failed\n\n");
      return 1;
   }

   printf("EGL API version: %d.%d\n", maj, min);
   printf("EGL vendor string: %s\n", eglQueryString(d, EGL_VENDOR));
   printf("EGL version string: %s\n", eglQueryString(d, EGL_VERSION));
#ifdef EGL_VERSION_1_2
   const char *client_apis = eglQueryString(d, EGL_CLIENT_APIS);
   printf("EGL client APIs: %s\n", client_apis);
#endif

   const char *display_exts = eglQueryString(d, EGL_EXTENSIONS);
   
   if (!opts.brief)
      PrintDisplayExtensions(d);

#ifdef EGL_VERSION_1_4
   int khr_create_context = (maj == 1 && min >= 4) &&
      strstr(display_exts, "EGL_KHR_create_context") != 0;

   const char *has_opengl = strstr(client_apis, "OpenGL");
   const char *has_opengl_es = strstr(client_apis, "OpenGL_ES");
   const char *has_openvg = strstr(client_apis, "OpenVG");

   if (has_opengl == has_opengl_es) {
      int offset = strlen("OpenGL_ES");
      has_opengl = strstr(has_opengl_es + offset, "OpenGL");
   }

   EGLBoolean do_opengl_core =
      (opts.api == OPENGL || opts.api == OPENGL_CORE || opts.api == ALL);
   EGLBoolean do_opengl_compat = (opts.api == OPENGL || opts.api == ALL);
   EGLBoolean do_opengl_es = (opts.api == OPENGL_ES || opts.api == ALL);

   if (has_opengl && (do_opengl_core || do_opengl_compat)) 
   {
      EGLBoolean api_result = eglBindAPI(EGL_OPENGL_API);
      if (api_result) {
         EGLConfig config = chooseEGLConfig(d, EGL_OPENGL_BIT);
         EGLContext ctx = NULL;

         if (khr_create_context && do_opengl_core) {
            ctx = createEGLContext(d,
                                   config,
                                   EGL_OPENGL_API,
                                   EGL_TRUE,
                                   EGL_TRUE);

            if (ctx)
               if (doOneContext(d, ctx, "OpenGL core profile", opts) == 0)
                  if (!eglDestroyContext(d, ctx))
                     return 1;
         }

         if (do_opengl_compat) {
            ctx = createEGLContext(d,
                                   config,
                                   EGL_OPENGL_API,
                                   khr_create_context,
                                   EGL_FALSE);
            if (ctx)
               if (doOneContext(d, ctx, "OpenGL compatibility profile", opts) == 0)
                  if (!eglDestroyContext(d, ctx))
                     return 1;
         }
      }
   }

   if (has_opengl_es && do_opengl_es) {
      EGLBoolean api_result = eglBindAPI(EGL_OPENGL_ES_API);
      if (api_result) {
         EGLConfig config = chooseEGLConfig(d, EGL_OPENGL_ES_BIT);
         EGLContext ctx = createEGLContext(d,
                                           config,
                                           EGL_OPENGL_ES_API,
                                           khr_create_context,
                                           EGL_FALSE);

         if (ctx) {
            if (doOneContext(d, ctx, "OpenGL ES profile", opts) == 0)
               if (!eglDestroyContext(d, ctx))
                  return 1;
         }
      }
   }
   if (has_openvg) {
      /* TODO: support OpenVG? */
   }
#endif

   if (!opts.brief)
      PrintConfigs(d);

   eglTerminate(d);
   printf("\n");
   return 0;
}


static int
doOneDevice(EGLDeviceEXT d, int i, struct options opts)
{
   PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay =
     (PFNEGLGETPLATFORMDISPLAYEXTPROC)
     eglGetProcAddress("eglGetPlatformDisplayEXT");

   printf("Device #%d:\n\n", i);

   if (!opts.brief)
      PrintDeviceExtensions(d);

   return doOneDisplay(getPlatformDisplay(EGL_PLATFORM_DEVICE_EXT, d, NULL),
                       "Platform Device", opts);
}


static int
doDevices(const char *name, struct options opts)
{
   PFNEGLQUERYDEVICESEXTPROC queryDevices =
     (PFNEGLQUERYDEVICESEXTPROC) eglGetProcAddress("eglQueryDevicesEXT");
   EGLDeviceEXT *devices;
   EGLint max_devices, num_devices;
   EGLint i;
   int ret = 0;

   printf("%s:\n", name);

   if (!queryDevices(0, NULL, &max_devices))
      return 1;
   devices = calloc(sizeof(EGLDeviceEXT), max_devices);
   if (!devices)
      return 1;
   if (!queryDevices(max_devices, devices, &num_devices))
     num_devices = 0;

   for (i = 0; i < num_devices; ++i) {
       ret += doOneDevice(devices[i], i, opts);
   }

   free(devices);

   return ret;
}


static void
usage(void)
{
   /* 
    * Usage portion of the help message
    */

   printf("Usage: eglinfo [-h] [-B]");

#ifdef EGL_VERSION_1_2
   printf(" [-a <api>]");
#endif

   printf(" [-p <platform>]\n");

   /* 
    * Detailed portion of the help message
    */

   printf("\t -h \t This message.\n");
   printf("\t -B \t Brief output, print only the basics.\n");

#ifdef EGL_VERSION_1_2
   printf("\t -a \t Print information for a specific API, if supported.\n");
   printf("\t\t (");
   for (int i = 0; i < ELEMENTS(apis) - 1; i++) {
      printf("%s, ", apis[i]);
   }
   printf("%s)\n", apis[ELEMENTS(apis) - 1]);
#endif

   printf("\t -p \t Print information for a specific platform, if supported.\n");
   printf("\t\t (");
   for (int i = 0; i < ELEMENTS(platforms) - 1; i++) {
      printf("%s, ", platforms[i].short_name);
   }
   printf("%s)\n", platforms[ELEMENTS(platforms) - 1].short_name);
}

static void
parse_args(int argc, char *argv[], struct options *opts)
{
   opts->api = ALL;
   opts->platform = ALL; /* ALL == ~0 */
   opts->brief = 0;

   if (argc <= 1)
      return;

   for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-h") == 0) {
         usage();
         exit(0);
      }
   }

#ifdef EGL_VERSION_1_2
   for (int i = 1; i < argc; i++) {
      /* parse -a */
      if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
         const char *selected_api = argv[++i];

         for (int j = 0; j < ELEMENTS(apis); j++) {
            if (strcmp(selected_api, apis[j]) == 0) {
               opts->api = j;
               break;
            }
         }

         if (opts->api == ALL) {
            printf("Unknown API: %s\n", argv[i]);
            goto fail;
         }
      }

      /* parse -p */
      else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
         const char *selected_platform = argv[++i];

         for (int j = 0; j < ELEMENTS(platforms); j++) {
            if (strcmp(selected_platform, platforms[j].short_name) == 0) {
               opts->platform = j;
               break;
            }
         }

         if (opts->platform == ALL) {
            printf("Unknown platform: %s\n", argv[i]);
            goto fail;
         }
      }

      /* parse -B */
      else if (strcmp(argv[i], "-B") == 0) {
         opts->brief = 1;
      }

      /* unknown */
      else {
         printf("Unknown option: %s\n", argv[i]);
         goto fail;
      }
   }
#endif

   return;

fail:
   usage();
   exit(1);
}


int
main(int argc, char *argv[])
{
   struct options opts;
   parse_args(argc, argv, &opts);

   int ret = 0;
   const char *clientext;

   if (!opts.brief) {
      clientext = PrintDisplayExtensions(EGL_NO_DISPLAY);
      printf("\n");
   } else {
      clientext = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
   }

   if (strstr(clientext, "EGL_EXT_platform_base")) {
       PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay =
           (PFNEGLGETPLATFORMDISPLAYEXTPROC)
           eglGetProcAddress("eglGetPlatformDisplayEXT");

      for (int i = 0; i < ELEMENTS(platforms); i++) {
         if (opts.platform != ALL && i != opts.platform)
            continue;

         for (int j = 0; j < ELEMENTS(platforms[i].names); j++) {
            const char *name = platforms[i].names[j];

            if (!name)
               break;

            if (strstr(clientext, name)) {
               EGLDisplay d = getPlatformDisplay(platforms[i].platform_enum,
                                                EGL_DEFAULT_DISPLAY,
                                                NULL);
               ret += doOneDisplay(d, platforms[i].human_name, opts);
               break;
            }
         }
      }

      if (strstr(clientext, "EGL_EXT_device_enumeration") &&
          strstr(clientext, "EGL_EXT_platform_device") &&
          opts.platform == ALL)
         ret += doDevices("Device platform", opts);
   }
   else {
      ret = doOneDisplay(eglGetDisplay(EGL_DEFAULT_DISPLAY), "Default display", opts);
   }

   return ret;
}
