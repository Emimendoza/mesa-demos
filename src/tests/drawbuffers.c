/*
 * Test GL_ARB_draw_buffers, GL_EXT_framebuffer_object
 * and GLSL's gl_FragData[].
 *
 * Brian Paul
 * 11 March 2007
 */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "glad/gl.h"
#include "glut_wrap.h"


static int Win;
static int Width = 400, Height = 400;
static GLuint FBobject, RBobjects[3];
static GLfloat Xrot = 0.0, Yrot = 0.0;
static GLuint Program;


static void
CheckError(int line)
{
   GLenum err = glGetError();
   if (err) {
      printf("GL Error 0x%x at line %d\n", (int) err, line);
   }
}


static void
Display(void)
{
   GLubyte *buffer = malloc(Width * Height * 4);
   static const GLenum buffers[2] = {
      GL_COLOR_ATTACHMENT0_EXT,
      GL_COLOR_ATTACHMENT1_EXT
   };

   glUseProgram(Program);

   glEnable(GL_DEPTH_TEST);

   /* draw to user framebuffer */
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBobject);

   /* Clear color buffer 0 (blue) */
   glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
   glClearColor(0.5, 0.5, 1.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT);

   /* Clear color buffer 1 (1 - blue) */
   glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
   glClearColor(0.5, 0.5, 0.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT);

   glClear(GL_DEPTH_BUFFER_BIT);

   /* draw to two buffers w/ fragment shader */
   glDrawBuffersARB(2, buffers);

   glPushMatrix();
   glRotatef(Xrot, 1, 0, 0);
   glRotatef(Yrot, 0, 1, 0);
   glutSolidTorus(0.75, 2.0, 10, 20);
   glPopMatrix();

   /* read from user framebuffer */
   /* left half = colorbuffer 0 */
   glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
   glPixelStorei(GL_PACK_ROW_LENGTH, Width);
   glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
   glReadPixels(0, 0, Width / 2, Height, GL_RGBA, GL_UNSIGNED_BYTE,
                buffer);

   /* right half = colorbuffer 1 */
   glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
   glPixelStorei(GL_PACK_SKIP_PIXELS, Width / 2);
   glReadPixels(Width / 2, 0, Width - Width / 2, Height,
                GL_RGBA, GL_UNSIGNED_BYTE,
                buffer);

   /* draw to window */
   glUseProgram(0);
   glDisable(GL_DEPTH_TEST);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
   glWindowPos2iARB(0, 0);
   glDrawPixels(Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

   free(buffer);
   glutSwapBuffers();
   CheckError(__LINE__);
}


static void
Reshape(int width, int height)
{
   float ar = (float) width / (float) height;

   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-ar, ar, -1.0, 1.0, 5.0, 35.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0.0, 0.0, -20.0);

   Width = width;
   Height = height;

   glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, RBobjects[0]);
   glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB, Width, Height);
   glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, RBobjects[1]);
   glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB, Width, Height);
   glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, RBobjects[2]);
   glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
                            Width, Height);
}


static void
CleanUp(void)
{
   glDeleteFramebuffersEXT(1, &FBobject);
   glDeleteRenderbuffersEXT(3, RBobjects);
   glutDestroyWindow(Win);
   exit(0);
}


static void
Key(unsigned char key, int x, int y)
{
   (void) x;
   (void) y;
   switch (key) {
      case 'x':
         Xrot += 5.0;
         break;
      case 'y':
         Yrot += 5.0;
         break;
      case 27:
         CleanUp();
         break;
   }
   glutPostRedisplay();
}


static void
CheckExtensions(void)
{
   GLint numBuf;

   if (!GLAD_GL_EXT_framebuffer_object) {
      printf("Sorry, GL_EXT_framebuffer_object is required!\n");
      exit(1);
   }
   if (!GLAD_GL_ARB_draw_buffers) {
      printf("Sorry, GL_ARB_draw_buffers is required!\n");
      exit(1);
   }
   if (!GLAD_GL_VERSION_2_0) {
      printf("Sorry, OpenGL 2.0 is required!\n");
      exit(1);
   }

   glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &numBuf);
   printf("GL_MAX_DRAW_BUFFERS_ARB = %d\n", numBuf);
   if (numBuf < 2) {
      printf("Sorry, GL_MAX_DRAW_BUFFERS_ARB needs to be >= 2\n");
      exit(1);
   }

   printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
}


static void
SetupRenderbuffers(void)
{
   glGenFramebuffersEXT(1, &FBobject);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBobject);

   glGenRenderbuffersEXT(3, RBobjects);

   glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, RBobjects[0]);
   glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB, Width, Height);

   glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, RBobjects[1]);
   glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB, Width, Height);

   glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, RBobjects[2]);
   glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
                            Width, Height);

   glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                GL_RENDERBUFFER_EXT, RBobjects[0]);
   glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT,
                                GL_RENDERBUFFER_EXT, RBobjects[1]);
   glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                GL_RENDERBUFFER_EXT, RBobjects[2]);

   CheckError(__LINE__);
}


static GLuint
LoadAndCompileShader(GLenum target, const char *text)
{
   GLint stat;
   GLuint shader = glCreateShader(target);
   glShaderSource(shader, 1, (const GLchar **) &text, NULL);
   glCompileShader(shader);
   glGetShaderiv(shader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      GLchar log[1000];
      GLsizei len;
      glGetShaderInfoLog(shader, 1000, &len, log);
      fprintf(stderr, "drawbuffers: problem compiling shader:\n%s\n", log);
      exit(1);
   }
   return shader;
}


static void
CheckLink(GLuint prog)
{
   GLint stat;
   glGetProgramiv(prog, GL_LINK_STATUS, &stat);
   if (!stat) {
      GLchar log[1000];
      GLsizei len;
      glGetProgramInfoLog(prog, 1000, &len, log);
      fprintf(stderr, "drawbuffers: shader link error:\n%s\n", log);
   }
}


static void
SetupShaders(void)
{
   /* second color output = 1 - first color */
   static const char *fragShaderText =
      "void main() {\n"
      "   gl_FragData[0] = gl_Color; \n"
      "   gl_FragData[1] = vec4(1.0) - gl_Color; \n"
      "}\n";

   GLuint fragShader;

   fragShader = LoadAndCompileShader(GL_FRAGMENT_SHADER, fragShaderText);
   Program = glCreateProgram();

   glAttachShader(Program, fragShader);
   glLinkProgram(Program);
   CheckLink(Program);
   glUseProgram(Program);
}


static void
SetupLighting(void)
{
   static const GLfloat frontMat[4] = { 1.0, 0.5, 0.5, 1.0 };
   static const GLfloat backMat[4] = { 1.0, 0.5, 0.5, 1.0 };

   glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, frontMat);
   glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, backMat);
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
   glEnable(GL_LIGHT0);
   glEnable(GL_LIGHTING);
}


static void
Init(void)
{
   CheckExtensions();
   SetupRenderbuffers();
   SetupShaders();
   SetupLighting();
   glEnable(GL_DEPTH_TEST);
}


int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowPosition(0, 0);
   glutInitWindowSize(Width, Height);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   Win = glutCreateWindow(argv[0]);
   gladLoaderLoadGL();
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Display);
   Init();
   glutMainLoop();
   gladLoaderUnloadGL();
   return 0;
}
