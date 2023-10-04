/*
 * Copyright (c) 2021-2023 Blaž Tomažič
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

/*
 * An example of how to share a GL/GLES texture in EGL context between
 * different processes by using DMA-BUF. The code for sharing is in the
 * `share_texture` function.
 *
 * Runtime prerequisites:
 *   - EGL extensions:
 *     * EGL_MESA_image_dma_buf_export
 *     * EGL_EXT_image_dma_buf_import
 *   - One of the next GL/GLES extensions:
 *     * GL_EXT_EGL_image_storage
 *     * GL_OES_EGL_image
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "glad/gl.h"

#include "eglut.h"


static int is_server;

const size_t TEXTURE_DATA_LENGTH = 256;
static GLuint texture; /* GL texture that will be shared */
static int *texture_data;

/* Timing for updating (rotating) the texture */
static double last_time = 0;

/* Dir for socket path for sending/receiving file descriptor and image storage
 * data */
static char temp_dir[50];

int extension_GL_EXT_EGL_image_storage_supported = 0;
int extension_GL_OES_EGL_image_supported = 0;


static int
socket_create(const char *path)
{
   int sock = socket(AF_UNIX, SOCK_DGRAM, 0);

   struct sockaddr_un addr;
   memset(&addr, 0, sizeof(addr));
   addr.sun_family = AF_UNIX;
   strcpy(addr.sun_path, path);
   if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
      fprintf(stderr, "Failed to create socket at %s.\n", path);
      exit(1);
   }

   return sock;
}

static int
socket_connect(int sock, const char *path)
{
   struct sockaddr_un addr;
   memset(&addr, 0, sizeof(addr));
   addr.sun_family = AF_UNIX;
   strcpy(addr.sun_path, path);
   return connect(sock, (struct sockaddr *) &addr, sizeof(addr));
}

static void
socket_write(int sock, int fd, void *data, size_t data_len)
{
   struct msghdr msg = {0};
   char buf[CMSG_SPACE(sizeof(fd))];
   memset(buf, '\0', sizeof(buf));

   struct iovec io = {.iov_base = data, .iov_len = data_len};

   msg.msg_iov = &io;
   msg.msg_iovlen = 1;
   msg.msg_control = buf;
   msg.msg_controllen = sizeof(buf);

   struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
   cmsg->cmsg_level = SOL_SOCKET;
   cmsg->cmsg_type = SCM_RIGHTS;
   cmsg->cmsg_len = CMSG_LEN(sizeof(fd));

   *(int *) CMSG_DATA(cmsg) = fd;

   msg.msg_controllen = CMSG_SPACE(sizeof(fd));

   if (sendmsg(sock, &msg, 0) < 0) {
      fprintf(stderr, "Failed to send message to socket.\n");
      exit(1);
   }
}

static void
socket_read(int sock, int *fd, void *data, size_t data_len)
{
   struct msghdr msg = {0};

   struct iovec io = {.iov_base = data, .iov_len = data_len};
   msg.msg_iov = &io;
   msg.msg_iovlen = 1;

   char c_buffer[256];
   msg.msg_control = c_buffer;
   msg.msg_controllen = sizeof(c_buffer);

   if (recvmsg(sock, &msg, 0) < 0) {
      fprintf(stderr, "Failed to read message from socket.\n");
      exit(1);
   }

   struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

   *fd = *(int *) CMSG_DATA(cmsg);
}

static void
rm_temp_dir()
{
   rmdir(temp_dir);
}

static void
make_temp_socket_names(char server_file[], char client_file[])
{
   strcpy(temp_dir, "/tmp/dmabufshare-XXXXXX");
   mkdtemp(temp_dir);
   atexit(rm_temp_dir);

   strcpy(server_file, temp_dir);
   strcat(server_file, "/server");
   strcpy(client_file, temp_dir);
   strcat(client_file, "/client");
}

static int *
update_texture_data(int *data, size_t length)
{
   size_t size = length * length;
   size_t half_length = length / 2;

   /* Paint the texture in a pattern like so:
    * RG
    * BW
    * where R - red, G - green, B - blue, W - white */
   const int RGBW[] = {0x000000FF, 0x0000FF00, 0X00FF0000, 0x00FFFFFF};
   const unsigned int num_elements = sizeof(RGBW) / sizeof(RGBW[0]);
   /* But rotate clockwise each time the function is called */
   static unsigned int iteration = 0;
   const unsigned int offset = num_elements - (iteration % num_elements);
   iteration++;
   for (size_t i = 0; i < size; i++) {
      size_t x = i % length;
      size_t y = i / length;

      if (x < half_length) {
         if (y < half_length) {
            data[i] = RGBW[(0 + offset) % num_elements];
         } else {
            data[i] = RGBW[(3 + offset) % num_elements];
         }
      } else {
         if (y < half_length) {
            data[i] = RGBW[(1 + offset) % num_elements];
         } else {
            data[i] = RGBW[(2 + offset) % num_elements];
         }
      }
   }

   return data;
}

static int
has_extension(const char *extensions, const char *ext)
{
   const char *loc;
   const char *terminator;
   if (extensions == NULL) {
      return 0;
   }
   while (1) {
      loc = strstr(extensions, ext);
      if (loc == NULL) {
         return 0;
      }
      terminator = loc + strlen(ext);
      if ((loc == extensions || *(loc - 1) == ' ') &&
          (*terminator == ' ' || *terminator == '\0')) {
         return 1;
      }
      extensions = terminator;
   }
}

static void
report_missing_extension(const char *api, const char *ext)
{
   fprintf(stderr, "%s does not support %s.\n", api, ext);
}

static void
require_extension(const char *api, const char *extensions, const char *ext)
{
   if (!has_extension(extensions, ext)) {
      report_missing_extension(api, ext);
      exit(1);
   }
}

static void
init()
{
   /* Shader source that draws a textures quad */
   const char *vertex_shader_source =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "layout (location = 1) in vec2 aTexCoords;\n"

      "out vec2 TexCoords;\n"

      "void main()\n"
      "{\n"
      "   TexCoords = aTexCoords;\n"
      "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}\0";
   const char *fragment_shader_source =
      "#version 330 core\n"
      "out vec4 FragColor;\n"

      "in vec2 TexCoords;\n"

      "uniform sampler2D Texture1;\n"

      "void main()\n"
      "{\n"
      "   FragColor = texture(Texture1, TexCoords);\n"
      "}\0";

   /* vertex shader */
   int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
   glCompileShader(vertex_shader);
   /* fragment shader */
   int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
   glCompileShader(fragment_shader);
   /* link shaders */
   int shader_program = glCreateProgram();
   glAttachShader(shader_program, vertex_shader);
   glAttachShader(shader_program, fragment_shader);
   glLinkProgram(shader_program);
   /* delete shaders */
   glDeleteShader(vertex_shader);
   glDeleteShader(fragment_shader);

   /* quad */
   float vertices[] = {
      0.5f,  0.5f,  0.0f, 1.0f, 0.0f, /* top right */
      0.5f,  -0.5f, 0.0f, 1.0f, 1.0f, /* bottom right */
      -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, /* bottom left */
      -0.5f, 0.5f,  0.0f, 0.0f, 0.0f  /* top left */
   };
   unsigned int indices[] = {
      0, 1, 3, /* first triangle */
      1, 2, 3  /* second triangle */
   };

   unsigned int VBO, VAO, EBO;
   glGenVertexArrays(1, &VAO);
   glGenBuffers(1, &VBO);
   glGenBuffers(1, &EBO);
   glBindVertexArray(VAO);

   glBindBuffer(GL_ARRAY_BUFFER, VBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                         (void *) 0);
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                         (void *) (3 * sizeof(float)));

   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glBindVertexArray(0);

   /* Prebind stuff needed for drawing */
   glUseProgram(shader_program);
   glBindVertexArray(VAO);

   /* Init timer for updating the texture data */
   last_time = time(NULL);
}

static void
idle()
{
   /* Update texture data each second to see that the client didn't just copy
    * the texture and is indeed referencing the same texture data. */
   if (is_server) {
      time_t cur_time = time(NULL);
      if (last_time < cur_time) {
         last_time = cur_time;
         update_texture_data(texture_data, TEXTURE_DATA_LENGTH);
         glBindTexture(GL_TEXTURE_2D, texture);
         glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXTURE_DATA_LENGTH,
                         TEXTURE_DATA_LENGTH, GL_RGBA, GL_UNSIGNED_BYTE,
                         texture_data);
      }
   }

   eglutPostRedisplay();
}

static void
draw()
{
   /* clear */
   glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);

   /* draw quad */
   /* VAO and shader program are already bound from the call to init() */
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, texture);
   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

static void
share_texture(const char *server_file, const char *client_file)
{
   EGLDisplay egl_display = eglGetCurrentDisplay();

   /* Custom image storage data description to transfer over socket */
   struct texture_storage_metadata_t {
      int fourcc;
      EGLuint64KHR modifiers;
      EGLint stride;
      EGLint offset;
   };

   /* The next `if` block contains server code in the `true` branch and client
    * code in the `false` branch. The `true` branch is always executed first
    * and the `false` branch after it (in a different process). This is
    * because the server loops at the end of the branch until it can send a
    * message to the client and the client blocks at the start of the branch
    * until it has a message to read. This way the whole `if` block from top
    * to bottom represents (mostly) the order of events as they happen. */
   if (is_server) {
      /* Create texture data and populate it */
      texture_data =
         malloc(TEXTURE_DATA_LENGTH * TEXTURE_DATA_LENGTH * sizeof(int));
      update_texture_data(texture_data, TEXTURE_DATA_LENGTH);

      /* GL: Create and populate the texture */
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_DATA_LENGTH,
                   TEXTURE_DATA_LENGTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXTURE_DATA_LENGTH,
                      TEXTURE_DATA_LENGTH, GL_RGBA, GL_UNSIGNED_BYTE,
                      texture_data);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      /* EGL: Create EGL image from the GL texture */
      EGLImage image = eglCreateImage(
         egl_display, eglGetCurrentContext(), EGL_GL_TEXTURE_2D,
         (EGLClientBuffer) (uint64_t) texture, NULL);
      assert(image != EGL_NO_IMAGE);

      /* EGL (extension: EGL_MESA_image_dma_buf_export): Get file descriptor
       * (texture_dmabuf_fd) for the EGL image and get its storage data
       * (texture_storage_metadata) */
      int texture_dmabuf_fd;
      struct texture_storage_metadata_t texture_storage_metadata;

      int num_planes;
      PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC eglExportDMABUFImageQueryMESA =
         (PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC) eglGetProcAddress(
            "eglExportDMABUFImageQueryMESA");
      assert(eglExportDMABUFImageQueryMESA != NULL);
      EGLBoolean queried = eglExportDMABUFImageQueryMESA(
         egl_display, image, &texture_storage_metadata.fourcc, &num_planes,
         &texture_storage_metadata.modifiers);
      assert(queried);
      assert(num_planes == 1);
      PFNEGLEXPORTDMABUFIMAGEMESAPROC eglExportDMABUFImageMESA =
         (PFNEGLEXPORTDMABUFIMAGEMESAPROC) eglGetProcAddress(
            "eglExportDMABUFImageMESA");
      assert(eglExportDMABUFImageMESA != NULL);
      EGLBoolean exported = eglExportDMABUFImageMESA(
         egl_display, image, &texture_dmabuf_fd,
         &texture_storage_metadata.stride, &texture_storage_metadata.offset);
      assert(exported);

      /* Unix Domain Socket: Send file descriptor (texture_dmabuf_fd) and
       * texture storage data (texture_storage_metadata) */
      int sock = socket_create(server_file);
      while (socket_connect(sock, client_file) != 0)
         ;
      socket_write(sock, texture_dmabuf_fd, &texture_storage_metadata,
                   sizeof(texture_storage_metadata));
      close(sock);
      close(texture_dmabuf_fd);
      unlink(server_file);
   } else {
      /* Unix Domain Socket: Receive file descriptor (texture_dmabuf_fd) and
       * texture storage data (texture_storage_metadata) */
      int texture_dmabuf_fd;
      struct texture_storage_metadata_t texture_storage_metadata;

      int sock = socket_create(client_file);
      socket_read(sock, &texture_dmabuf_fd, &texture_storage_metadata,
                  sizeof(texture_storage_metadata));
      close(sock);
      unlink(client_file);

      /* EGL (extension: EGL_EXT_image_dma_buf_import): Create EGL image from
       * file descriptor (texture_dmabuf_fd) and storage data
       * (texture_storage_metadata) */
      EGLAttrib const attribute_list[] = {
         EGL_WIDTH,
         TEXTURE_DATA_LENGTH,
         EGL_HEIGHT,
         TEXTURE_DATA_LENGTH,
         EGL_LINUX_DRM_FOURCC_EXT,
         texture_storage_metadata.fourcc,
         EGL_DMA_BUF_PLANE0_FD_EXT,
         texture_dmabuf_fd,
         EGL_DMA_BUF_PLANE0_OFFSET_EXT,
         texture_storage_metadata.offset,
         EGL_DMA_BUF_PLANE0_PITCH_EXT,
         texture_storage_metadata.stride,
         EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT,
         (uint32_t) texture_storage_metadata.modifiers,
         EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT,
         (uint32_t) (texture_storage_metadata.modifiers >> 32),
         EGL_NONE};
      EGLImage image =
         eglCreateImage(egl_display, NULL, EGL_LINUX_DMA_BUF_EXT,
                        (EGLClientBuffer) NULL, attribute_list);
      assert(image != EGL_NO_IMAGE);
      close(texture_dmabuf_fd);

      /* GL (extension: GL_EXT_EGL_image_storage) or GLES (extension:
       * GL_OES_EGL_image): Create GL texture from EGL image */
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      if (extension_GL_EXT_EGL_image_storage_supported) {
         glEGLImageTargetTexStorageEXT(GL_TEXTURE_2D, image, NULL);
      } else if (extension_GL_OES_EGL_image_supported) {
         PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES =
            (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress(
               "glEGLImageTargetTexture2DOES");
         assert(glEGLImageTargetTexture2DOES != NULL);
         glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
      } else {
         assert(0);
      }
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   }
}

int
main(int argc, char **argv)
{
   /* Generate socket filenames */
   char server_file[50];
   char client_file[50];
   make_temp_socket_names(server_file, client_file);

   /* Fork off a child (client) that will receive the created texture */
   pid_t pid = fork();
   assert(pid != -1);
   is_server = pid != 0;

   /* Initialize EGL and GL contexts */
   eglutInitWindowSize(TEXTURE_DATA_LENGTH + 64, TEXTURE_DATA_LENGTH + 64);
   eglutInitAPIMask(EGLUT_OPENGL_BIT);
   eglutInit(argc, argv);

   /* Create window and register callback functions */
   eglutCreateWindow(is_server ? "dmabufshare server" : "dmabufshare client");
   eglutIdleFunc(idle);
   eglutDisplayFunc(draw);

   /* Load GL functions */
   gladLoadGL((GLADloadfunc) eglGetProcAddress);

   /* Check for required EGL extensions */
   const char *egl_extensions =
      eglQueryString(eglGetCurrentDisplay(), EGL_EXTENSIONS);
   require_extension("EGL", egl_extensions, "EGL_MESA_image_dma_buf_export");
   require_extension("EGL", egl_extensions, "EGL_EXT_image_dma_buf_import");
   /* Check for required GL/GLES extensions */
   const char *gl_extensions = (const char *) glGetString(GL_EXTENSIONS);
   extension_GL_EXT_EGL_image_storage_supported =
      has_extension(gl_extensions, "GL_EXT_EGL_image_storage");
   extension_GL_OES_EGL_image_supported =
      has_extension(gl_extensions, "GL_OES_EGL_image");
   if (!extension_GL_EXT_EGL_image_storage_supported &&
       !extension_GL_OES_EGL_image_supported) {
      report_missing_extension(
         "GL", "GL_EXT_EGL_image_storage or GL_OES_EGL_image");
      exit(1);
   }

   /* Setup GL scene */
   init();

   /* Create and share a texture */
   share_texture(server_file, client_file);

   /* Main loop */
   eglutMainLoop();

   return 0;
}
