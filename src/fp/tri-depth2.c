
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "glad/gl.h"
#include "glut_wrap.h"



static void Init( void )
{
   /* scale of 10.0 gives me a visible result on nv hardware.
    */
   static const char *modulate2D =
      "!!ARBfp1.0\n"
      "TEMP R0;\n"
      "MUL R0, fragment.position.z, {10.0}.x;\n"
      "MOV result.color, R0; \n"
      "END"
      ;
   GLuint modulateProg;

   if (!GLAD_GL_ARB_fragment_program) {
      printf("Error: GL_ARB_fragment_program not supported!\n");
      exit(1);
   }
   printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));

   /* Setup the fragment program */
   glGenProgramsARB(1, &modulateProg);
   glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, modulateProg);
   glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                        strlen(modulate2D), (const GLubyte *)modulate2D);

   printf("glGetError = 0x%x\n", (int) glGetError());
   printf("glError(GL_PROGRAM_ERROR_STRING_ARB) = %s\n",
          (char *) glGetString(GL_PROGRAM_ERROR_STRING_ARB));

   glEnable(GL_FRAGMENT_PROGRAM_ARB);

   glClearColor(.3, .3, .3, 0);
}

static void Reshape(int width, int height)
{

    glViewport(0, 0, (GLint)width, (GLint)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

static void Key(unsigned char key, int x, int y)
{

    switch (key) {
      case 27:
	exit(1);
      default:
	break;
    }

    glutPostRedisplay();
}

static void Draw(void)
{
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);

   glBegin(GL_TRIANGLES);
   glColor3f(0,0,1);
	glVertex3f( 0.9, -0.9, -30.0);
	glVertex3f( 0.9,  0.9, -30.0);
	glVertex3f(-0.9,  0.0, -30.0);
   glColor3f(0,1,0);
	glVertex3f(-0.9, -0.9, -40.0);
	glVertex3f(-0.9,  0.9, -40.0);
	glVertex3f( 0.9,  0.0, -25.0);
   glEnd();

   glFlush();


}


int main(int argc, char **argv)
{
    GLenum type;

    glutInit(&argc, argv);



    glutInitWindowPosition(0, 0); glutInitWindowSize( 250, 250);

    type = GLUT_RGB | GLUT_DEPTH;
    type |= GLUT_SINGLE;
    glutInitDisplayMode(type);

    if (glutCreateWindow("First Tri") == GL_FALSE) {
	exit(1);
    }

    gladLoaderLoadGL();

    Init();

    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Key);
    glutDisplayFunc(Draw);
    glutMainLoop();
    gladLoaderUnloadGL();
	return 0;
}
