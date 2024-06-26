
/*
 * GearTrain Simulator * Version:  1.00
 *
 * Copyright (C) 1999  Shobhan Kumar Dutta  All Rights Reserved.
 * <skdutta@del3.vsnl.net.in>
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
 * SHOBHAN KUMAR DUTTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 * OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "glut_wrap.h"
#include <string.h>
#include <stdio.h>

#ifndef min
#define min(x, y) ( x < y ? x : y )
#endif

typedef GLfloat TDA[4];

TDA background;


struct AXLE
  {
    char name[20];
      GLint id;
      GLfloat radius;
      GLint axis;
      TDA color;
      TDA position;
      GLfloat length;
      GLint motored;
      GLfloat angular_velocity;
      GLint direction;
  };


struct GEAR
  {
    char name[20];
    char type[7];
      GLint face;
      GLint id;
      GLfloat radius;
      GLfloat width;
      GLint teeth;
      GLfloat tooth_depth;
      GLfloat angle;
      GLfloat angular_velocity;
      TDA color;
      GLint relative_position;
      TDA position;
    char axle_name[20];
      GLint axis;
      GLint direction;
      GLint motored;
  };


struct BELT
  {
    char name[20];
      GLint id;
    char gear1_name[20];
    char gear2_name[20];
  };


FILE * mainfile;
struct GEAR g[10];
struct AXLE a[10];
struct BELT b[10];
int number_of_gears;
int number_of_axles;
int number_of_belts;


char Buf1[256], Buf2[256], Buf3[256], Buf4[256], Buf5[256];

static GLint T0 = 0;
static GLint Frames = 0;


static void
Clear_Buffers (void)
{
    memset (Buf1, '\0', 256);
    memset (Buf2, '\0', 256);
    memset (Buf3, '\0', 256);
    memset (Buf4, '\0', 256);
    memset (Buf5, '\0', 256);
}


static void
LoadTriplet (TDA A)
{
    int result;
    Clear_Buffers ();
    result = fscanf (mainfile, "%s %s %s %s", Buf1, Buf2, Buf3, Buf4);
    assert(result != EOF);
    A[0] = atof (Buf2);
    A[1] = atof (Buf3);
    A[2] = atof (Buf4);
}


static void
LoadReal (float *a)
{
    int result;
    Clear_Buffers ();
    result = fscanf (mainfile, "%s %s", Buf1, Buf2);
    assert(result != EOF);
    *a = atof (Buf2);
}


static void
LoadInteger (int *a)
{
    int result;
    Clear_Buffers ();
    result = fscanf (mainfile, "%s %s", Buf1, Buf2);
    assert(result != EOF);
    *a = atoi (Buf2);
}


static void
LoadText (char *a)
{
    int result;
    Clear_Buffers ();
    result = fscanf (mainfile, "%s %s", Buf1, Buf2);
    assert(result != EOF);
    strcpy (a, Buf2);
}


static void
getdata (char filename[])
{
    int gear_count = 0, axle_count = 0, belt_count = 0, i;

    mainfile = fopen (filename, "r");
    if (!mainfile) {
       printf("Error: couldn't open %s\n", filename);
       exit(-1);
    }

    do
    {
	int result;
	Clear_Buffers ();
	result = fscanf (mainfile, "%s", Buf1);
	(void) result;
	if (ferror (mainfile))
	{
	    printf ("\nError opening file !\n");
	    exit (1);
	}

	if (!(strcmp (Buf1, "BACKGROUND")))
	  LoadTriplet (background);

	if (!(strcmp (Buf1, "ANAME")))
	{
	    LoadText (a[axle_count].name);
	    axle_count++;
	}

	if (!(strcmp (Buf1, "ARADIUS")))
	  LoadReal (&a[axle_count - 1].radius);

	if (!(strcmp (Buf1, "AAXIS")))
	  LoadInteger (&a[axle_count - 1].axis);

	if (!(strcmp (Buf1, "ACOLOR")))
	  LoadTriplet (a[axle_count - 1].color);

	if (!(strcmp (Buf1, "APOSITION")))
	  LoadTriplet (a[axle_count - 1].position);

	if (!(strcmp (Buf1, "ALENGTH")))
	  LoadReal (&a[axle_count - 1].length);

	if (!(strcmp (Buf1, "AMOTORED")))
	  LoadInteger (&a[axle_count - 1].motored);

	if (!(strcmp (Buf1, "AANGULARVELOCITY")))
	  LoadReal (&a[axle_count - 1].angular_velocity);

	if (!(strcmp (Buf1, "ADIRECTION")))
	  LoadInteger (&a[axle_count - 1].direction);

	if (!(strcmp (Buf1, "GNAME")))
	{
	    LoadText (g[gear_count].name);
	    gear_count++;
	}

	if (!(strcmp (Buf1, "GTYPE")))
	  LoadText (g[gear_count - 1].type);

	if (!(strcmp (Buf1, "GFACE")))
	  LoadInteger (&g[gear_count - 1].face);

	if (!(strcmp (Buf1, "GRADIUS")))
	  LoadReal (&g[gear_count - 1].radius);

	if (!(strcmp (Buf1, "GWIDTH")))
	  LoadReal (&g[gear_count - 1].width);

	if (!(strcmp (Buf1, "GTEETH")))
	  LoadInteger (&g[gear_count - 1].teeth);

	if (!(strcmp (Buf1, "GTOOTHDEPTH")))
	  LoadReal (&g[gear_count - 1].tooth_depth);

	if (!(strcmp (Buf1, "GCOLOR")))
	  LoadTriplet (g[gear_count - 1].color);

	if (!(strcmp (Buf1, "GAXLE")))
	  LoadText (g[gear_count - 1].axle_name);

	if (!(strcmp (Buf1, "GPOSITION")))
	  LoadInteger (&g[gear_count - 1].relative_position);

	if (!(strcmp (Buf1, "BELTNAME")))
	{
	    LoadText (b[belt_count].name);
	    belt_count++;
	}

	if (!(strcmp (Buf1, "GEAR1NAME")))
	  LoadText (b[belt_count - 1].gear1_name);

	if (!(strcmp (Buf1, "GEAR2NAME")))
	  LoadText (b[belt_count - 1].gear2_name);
    }
    while (Buf1[0] != 0);

    number_of_gears = gear_count;
    number_of_axles = axle_count;
    number_of_belts = belt_count;

    for (i = 0; i < number_of_gears; i++)
    {
	g[i].axis = -1;
	g[i].direction = 0;
	g[i].angular_velocity = 0.0;
    }

    fclose (mainfile);
}


static void
axle (GLint j, GLfloat radius, GLfloat length)
{
    GLfloat angle, rad, incr = 10.0 * M_PI / 180.0;

    /* draw main cylinder */
    glBegin (GL_QUADS);
    for (angle = 0.0; angle < 360.0; angle += 5.0)
    {
	rad = angle * M_PI / 180.0;
	glNormal3f (cos (rad), sin (rad), 0.0);
	glVertex3f (radius * cos (rad), radius * sin (rad), length / 2);
	glVertex3f (radius * cos (rad), radius * sin (rad), -length / 2);
	glVertex3f (radius * cos (rad + incr), radius * sin (rad + incr), -length / 2);
	glVertex3f (radius * cos (rad + incr), radius * sin (rad + incr), length / 2);
    }
    glEnd ();

    /* draw front face */
    glNormal3f (0.0, 0.0, 1.0);
    glBegin (GL_TRIANGLES);
    for (angle = 0.0; angle < 360.0; angle += 5.0)
    {
	rad = angle * M_PI / 180.0;
	glVertex3f (0.0, 0.0, length / 2);
      	glVertex3f (radius * cos (rad), radius * sin (rad), length / 2);
	glVertex3f (radius * cos (rad + incr), radius * sin (rad + incr), length / 2);
	glVertex3f (0.0, 0.0, length / 2);
    }
    glEnd ();

    /* draw back face */
    glNormal3f (0.0, 0.0, -1.0);
    glBegin (GL_TRIANGLES);
    for (angle = 0.0; angle <= 360.0; angle += 5.0)
    {
	rad = angle * M_PI / 180.0;
	glVertex3f (0.0, 0.0, -length / 2);
	glVertex3f (radius * cos (rad), radius * sin (rad), -length / 2);
	glVertex3f (radius * cos (rad + incr), radius * sin (rad + incr), -length / 2);
	glVertex3f (0.0, 0.0, -length / 2);
    }
    glEnd ();
}



static void
gear (GLint j, char type[], GLfloat radius, GLfloat width,
      GLint teeth, GLfloat tooth_depth)
{
    GLint i;
    GLfloat r1, r2_front, r2_back;
    GLfloat angle, da;
    GLfloat u, v, len, fraction = 0.5;
    GLfloat n = 1.0;

    r1 = radius - tooth_depth;
    r2_front = r2_back = radius;

    fraction = 0.5;
    n = 1.0;

    da = 2.0 * M_PI / teeth / 4.0;
    if (!(strcmp (type, "BEVEL")))
    {
        if (g[j].face)
            r2_front = radius - width;
        else
            r2_back = radius - width;
    }

    /* draw front face */
	glNormal3f (0.0, 0.0, 1.0 * n);
	glBegin (GL_QUAD_STRIP);
	for (i = 0; i <= teeth; i++)
	{
	    angle = i * 2.0 * M_PI / teeth;
	    glVertex3f (0.0, 0.0, width * fraction);
	    glVertex3f (r1 * cos (angle), r1 * sin (angle), width * fraction);
	    glVertex3f (0.0, 0.0, width * fraction);
	    glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da), width * fraction);
	}
	glEnd ();

    /* draw front sides of teeth */
	glNormal3f (0.0, 0.0, 1.0 * n);
	glBegin (GL_QUADS);
	da = 2.0 * M_PI / teeth / 4.0;
	for (i = 0; i < teeth; i++)
	{
	    angle = i * 2.0 * M_PI / teeth;
	    glVertex3f (r1 * cos (angle), r1 * sin (angle), width * fraction);
	    glVertex3f (r2_front * cos (angle + da), r2_front * sin (angle + da), width * fraction);
	    glVertex3f (r2_front * cos (angle + 2 * da), r2_front * sin (angle + 2 * da), width * fraction);
	    glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da), width * fraction);
	}
	glEnd ();

    /* draw back face */
    glNormal3f (0.0, 0.0, -1.0 * n);
    glBegin (GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++)
    {
	angle = i * 2.0 * M_PI / teeth;
	glVertex3f (r1 * cos (angle), r1 * sin (angle), -width * fraction);
	glVertex3f (0.0, 0.0, -width * fraction);
	glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da), -width * fraction);
	glVertex3f (0.0, 0.0, -width * fraction);
    }
    glEnd ();

    /* draw back sides of teeth */
    glNormal3f (0.0, 0.0, -1.0 * n);
    glBegin (GL_QUADS);
    da = 2.0 * M_PI / teeth / 4.0;
    for (i = 0; i < teeth; i++)
    {
	angle = i * 2.0 * M_PI / teeth;
	glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da), -width * fraction);
	glVertex3f (r2_back * cos (angle + 2 * da), r2_back * sin (angle + 2 * da), -width * fraction);
	glVertex3f (r2_back * cos (angle + da), r2_back * sin (angle + da), -width * fraction);
	glVertex3f (r1 * cos (angle), r1 * sin (angle), -width * fraction);
    }
    glEnd ();


    /* draw outward faces of teeth. The visible faces are the backfaces, for testing purposes */
	glBegin (GL_QUAD_STRIP);
	for (i = 0; i < teeth; i++)
	{
	    angle = i * 2.0 * M_PI / teeth;

	    glNormal3f (-cos (angle - 0.5*da), -sin (angle - 0.5*da), 0.0);
	    glVertex3f (r1 * cos (angle), r1 * sin (angle), -width * fraction);
	    glVertex3f (r1 * cos (angle), r1 * sin (angle), width * fraction);
	    u = (r2_front+r2_back)/2.0 * cos (angle + da) - r1 * cos (angle);
	    v = (r2_front+r2_back)/2.0 * sin (angle + da) - r1 * sin (angle);
	    len = sqrt (u * u + v * v);
	    u /= len;
	    v /= len;
	    glNormal3f (-v, u, 0.0);
	    glVertex3f (r2_back * cos (angle + da), r2_back * sin (angle + da), -width * fraction);
	    glVertex3f (r2_front * cos (angle + da), r2_front * sin (angle + da), width * fraction);
	    glNormal3f (-cos (angle + 1.5*da), -sin (angle + 1.5*da), 0.0);
	    glVertex3f (r2_back * cos (angle + 2 * da), r2_back * sin (angle + 2 * da), -width * fraction);
	    glVertex3f (r2_front * cos (angle + 2 * da), r2_front * sin (angle + 2 * da), width * fraction);
	    u = r1 * cos (angle + 3 * da) - (r2_front+r2_back)/2.0 * cos (angle + 2 * da);
	    v = r1 * sin (angle + 3 * da) - (r2_front+r2_back)/2.0 * sin (angle + 2 * da);
	    len = sqrt (u * u + v * v);
	    u /= len;
	    v /= len;
	    glNormal3f (-v, u, 0.0);
	    glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da), -width * fraction);
	    glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da), width * fraction);
	    glNormal3f (-cos (angle + 3.5*da), -sin (angle + 3.5*da), 0.0);
	}

    glNormal3f (-cos (-0.5*da), -sin (-0.5*da), 0.0);
    glVertex3f (r1 * cos (0), r1 * sin (0), -width * fraction);
    glVertex3f (r1 * cos (0), r1 * sin (0), width * fraction);
    glEnd ();
}


static void
belt (struct GEAR g1, struct GEAR g2)
{
    GLfloat D, alpha, phi, angle, incr, width;
    GLint indexes[3] =
    {
       0, 0, 0
    };

    GLfloat col[3] =
    {
       0.0, 0.0, 0.0
    };

    width = min (g1.width, g2.width);
    D = sqrt (pow (g1.position[0] - g2.position[0], 2) + pow (g1.position[1] - g2.position[1], 2) + pow (g1.position[2] - g2.position[2], 2));
    alpha = acos ((g2.position[0] - g1.position[0]) / D);
    phi = acos ((g1.radius - g2.radius) / D);
    glBegin (GL_QUADS);
    glColor3fv (col);
    glMaterialiv (GL_FRONT, GL_COLOR_INDEXES, indexes);
    incr = 1.2 * 360.0 / g1.teeth * M_PI / 180.00;
    for (angle = alpha + phi; angle <= 2 * M_PI - phi + alpha; angle += 360.0 / g1.teeth * M_PI / 180.00)
    {
	glNormal3f (cos (angle), sin (angle), 0.0);
	glVertex3f (g1.radius * cos (angle), g1.radius * sin (angle), width * 0.5);
	glVertex3f (g1.radius * cos (angle), g1.radius * sin (angle), -width * 0.5);
	glVertex3f (g1.radius * cos (angle + incr), g1.radius * sin (angle + incr), -width * 0.5);
	glVertex3f (g1.radius * cos (angle + incr), g1.radius * sin (angle + incr), width * 0.5);
    }
    glEnd ();
    glBegin (GL_QUADS);
    glColor3fv (col);
    glMaterialiv (GL_FRONT, GL_COLOR_INDEXES, indexes);
    incr = 1.2 * 360.0 / g2.teeth * M_PI / 180.00;
    for (angle = -phi + alpha; angle <= phi + alpha; angle += 360.0 / g1.teeth * M_PI / 180.0)
    {
	glNormal3f (cos (angle), sin (angle), 0.0);
	glVertex3f (g2.radius * cos (angle) + g2.position[0] - g1.position[0], g2.radius * sin (angle) + g2.position[1] - g1.position[1], width * 0.5);
	glVertex3f (g2.radius * cos (angle) + g2.position[0] - g1.position[0], g2.radius * sin (angle) + g2.position[1] - g1.position[1], width * -0.5);
	glVertex3f (g2.radius * cos (angle + incr) + g2.position[0] - g1.position[0], g2.radius * sin (angle + incr) + g2.position[1] - g1.position[1], width * -0.5);
	glVertex3f (g2.radius * cos (angle + incr) + g2.position[0] - g1.position[0], g2.radius * sin (angle + incr) + g2.position[1] - g1.position[1], width * 0.5);
    }
    glEnd ();

    glBegin (GL_QUADS);
    glColor3fv (col);
    glMaterialiv (GL_FRONT, GL_COLOR_INDEXES, indexes);
    glVertex3f (g1.radius * cos (alpha + phi), g1.radius * sin (alpha + phi), width * 0.5);
    glVertex3f (g1.radius * cos (alpha + phi), g1.radius * sin (alpha + phi), width * -0.5);
    glVertex3f (g2.radius * cos (alpha + phi) + g2.position[0] - g1.position[0], g2.radius * sin (alpha + phi) + g2.position[1] - g1.position[1], width * -0.5);
    glVertex3f (g2.radius * cos (alpha + phi) + g2.position[0] - g1.position[0], g2.radius * sin (alpha + phi) + g2.position[1] - g1.position[1], width * 0.5);
    glVertex3f (g1.radius * cos (alpha - phi), g1.radius * sin (alpha - phi), width * 0.5);
    glVertex3f (g1.radius * cos (alpha - phi), g1.radius * sin (alpha - phi), width * -0.5);
    glVertex3f (g2.radius * cos (alpha - phi) + g2.position[0] - g1.position[0], g2.radius * sin (alpha - phi) + g2.position[1] - g1.position[1], width * -0.5);
    glVertex3f (g2.radius * cos (alpha - phi) + g2.position[0] - g1.position[0], g2.radius * sin (alpha - phi) + g2.position[1] - g1.position[1], width * 0.5);
    glEnd ();
}


static int
axle_find (char axle_name[])
{
    int i;

    for (i = 0; i < number_of_axles; i++)
    {
	if (!(strcmp (axle_name, a[i].name)))
           break;
    }
    return i;
}


static int
gear_find (char gear_name[])
{
    int i;

    for (i = 0; i < number_of_gears; i++)
    {
	if (!(strcmp (gear_name, g[i].name)))
           break;
    }
    return i;
}


static void
process (void)
{
    GLfloat x, y, z, D, dist;
    GLint axle_index, i, j, g1, g2, k;

    for (i = 0; i < number_of_gears; i++)
    {
	x = 0.0;
	y = 0.0;
	z = 0.0;
	axle_index = axle_find (g[i].axle_name);
	g[i].axis = a[axle_index].axis;
	g[i].motored = a[axle_index].motored;
	if (a[axle_index].motored)
	{
	    g[i].direction = a[axle_index].direction;
	    g[i].angular_velocity = a[axle_index].angular_velocity;
	}
	if (g[i].axis == 0)
           x = 1.0;
        else if (g[i].axis == 1)
           y = 1.0;
        else
           z = 1.0;

	g[i].position[0] = a[axle_index].position[0] + x * g[i].relative_position;
	g[i].position[1] = a[axle_index].position[1] + y * g[i].relative_position;
	g[i].position[2] = a[axle_index].position[2] + z * g[i].relative_position;
    }

    for (k = 0; k < number_of_axles; k++)
    {
	for (i = 0; i < number_of_gears - 1; i++)
	{
	    for (j = 0; j < number_of_gears; j++)
	    {
		if (!(strcmp (g[i].type, g[j].type)) && (!(strcmp (g[i].type, "NORMAL"))) && ((strcmp (g[i].axle_name, g[j].axle_name) != 0)) && (g[i].axis == g[j].axis))
		{
		    D = sqrt (pow (g[i].position[0] - g[j].position[0], 2) + pow (g[i].position[1] - g[j].position[1], 2) + pow (g[i].position[2] - g[j].position[2], 2));
		    if (D < 1.1 * (g[i].radius - g[i].tooth_depth + g[j].radius - g[j].tooth_depth))
		    {
			printf ("Gear %s and %s are too close to each other.", g[i].name, g[j].name);
			exit (1);
		    }

		    if (g[i].axis == 0)
		    {
			dist = g[i].position[0] - g[j].position[0];
		    }
		    else if (g[i].axis == 1)
		    {
			dist = g[i].position[1] - g[j].position[1];
		    }
		    else
                       dist = g[i].position[2] - g[j].position[2];

		    dist = fabs (dist);

		    if (dist < (g[i].width / 2 + g[j].width / 2))
		    {
			if ((g[i].motored) && (!(g[j].motored)) && (D < 0.95 * (g[i].radius + g[j].radius)))
			{
			    axle_index = axle_find (g[j].axle_name);
			    if ((a[axle_index].direction != 0) && (g[j].angular_velocity != g[i].angular_velocity * g[i].teeth / g[j].teeth * g[i].radius / g[j].radius))
			    {
				printf ("Error in tooth linkage of gears %s and %s.", g[i].name, g[j].name);
				exit (1);
			    }

			    g[j].motored = (a[axle_index].motored = 1);
			    g[j].direction = (a[axle_index].direction = -g[i].direction);
			    a[axle_index].angular_velocity = g[i].angular_velocity * g[i].teeth / g[j].teeth;
			    g[j].angular_velocity = (a[axle_index].angular_velocity *= g[i].radius / g[j].radius);
			}

			if ((!(g[i].motored)) && (g[j].motored) && (D < 0.95 * (g[i].radius + g[j].radius)))
			{
			    axle_index = axle_find (g[i].axle_name);
			    if ((a[axle_index].direction != 0) && (g[i].angular_velocity != g[j].angular_velocity * g[j].teeth / g[i].teeth * g[j].radius / g[i].radius))
			    {
				printf ("Error in tooth linkage of gears %s and %s.", g[i].name, g[j].name);
				exit (1);
			    }

			    g[i].motored = (a[axle_index].motored = 1);
			    g[i].direction = (a[axle_index].direction = -g[j].direction);
			    a[axle_index].angular_velocity = g[j].angular_velocity * g[j].teeth / g[i].teeth;
			    g[i].angular_velocity = (a[axle_index].angular_velocity *= g[j].radius / g[i].radius);

			}
		    }
		}

		if (!(strcmp (g[i].type, g[j].type)) && (!(strcmp (g[i].type, "BEVEL"))) && ((strcmp (g[i].axle_name, g[j].axle_name) != 0)) && (g[i].axis != g[j].axis))
		{
		    D = sqrt (pow (g[i].position[0] - g[j].position[0], 2) + pow (g[i].position[1] - g[j].position[1], 2) + pow (g[i].position[2] - g[j].position[2], 2));
		    if ((g[i].motored) && (!(g[j].motored)) && (D < 0.95 * sqrt (g[i].radius * g[i].radius + g[j].radius * g[j].radius)))
		    {
			axle_index = axle_find (g[j].axle_name);
			if ((a[axle_index].direction != 0) && (g[j].angular_velocity != g[i].angular_velocity * g[i].teeth / g[j].teeth * g[i].radius / g[j].radius))
			{
			    printf ("Error in tooth linkage of gears %s and %s.", g[i].name, g[j].name);
			    exit (1);
			}
			g[j].motored = (a[axle_index].motored = 1);
			g[j].direction = (a[axle_index].direction = -g[i].direction);
			a[axle_index].angular_velocity = g[i].angular_velocity * g[i].teeth / g[j].teeth;
			g[j].angular_velocity = (a[axle_index].angular_velocity *= g[i].radius / g[j].radius);
		    }


		    if ((!(g[i].motored)) && (g[j].motored) && (D < 0.95 * sqrt (g[i].radius * g[i].radius + g[j].radius * g[j].radius)))
		    {
			axle_index = axle_find (g[i].axle_name);
			if ((a[axle_index].direction != 0) && (g[i].angular_velocity != g[j].angular_velocity * g[j].teeth / g[i].teeth * g[j].radius / g[i].radius))
			{
			    printf ("Error in tooth linkage of gears %s and %s.", g[i].name, g[j].name);
			    exit (1);
			}
			g[i].motored = (a[axle_index].motored = 1);
			g[i].direction = (a[axle_index].direction = -g[j].direction);
			a[axle_index].angular_velocity = g[j].angular_velocity * g[j].teeth / g[i].teeth;
			g[i].angular_velocity = (a[axle_index].angular_velocity *= g[j].radius / g[i].radius);
		    }
		}
	    }
	}

	for (i = 0; i < number_of_gears; i++)
	{
	    axle_index = axle_find (g[i].axle_name);
	    g[i].motored = a[axle_index].motored;
	    if (a[axle_index].motored)
	    {
		g[i].direction = a[axle_index].direction;
		g[i].angular_velocity = a[axle_index].angular_velocity;
	    }
	}

	for (i = 0; i < number_of_belts; i++)
	{
	    g1 = gear_find (b[i].gear1_name);
	    g2 = gear_find (b[i].gear2_name);
	    D = sqrt (pow (g[g1].position[0] - g[g2].position[0], 2) + pow (g[g1].position[1] - g[g2].position[1], 2) + pow (g[g1].position[2] - g[g2].position[2], 2));
	    if (!((g[g1].axis == g[g2].axis) && (!strcmp (g[g1].type, g[g2].type)) && (!strcmp (g[g1].type, "NORMAL"))))
	    {
		printf ("Belt %s invalid.", b[i].name);
		exit (1);
	    }

	    if ((g[g1].axis == g[g2].axis) && (!strcmp (g[g1].type, g[g2].type)) && (!strcmp (g[g1].type, "NORMAL")))
	    {
	      /*
	         if((g[g1].motored)&&(g[g2].motored))
	         if(g[g2].angular_velocity!=(g[g1].angular_velocity*g[g1].radius/g[g2].radius))
	         {
	         printf("Error in belt linkage of gears %s and %s".,g[g1].name,g[g2].name);
	         exit(1);
	         }
              */
               if (g[g1].axis == 0)
                  {
		    dist = g[g1].position[0] - g[g2].position[0];
		}
		else if (g[i].axis == 1)
		{
		    dist = g[g1].position[1] - g[g2].position[1];
		}
		else
                   dist = g[g1].position[2] - g[g2].position[2];

		dist = fabs (dist);

		if (dist > (g[g1].width / 2 + g[g2].width / 2))
		{
		    printf ("Belt %s invalid.", b[i].name);
		    exit (1);
		}

		if (dist < (g[g1].width / 2 + g[g2].width / 2))
		{
		    if (D < g[g1].radius + g[g2].radius)
		    {
			printf ("Gears %s and %s too close to be linked with belts", g[g1].name, g[g2].name);
			exit (1);
		    }

		    if ((g[g1].motored) && (!(g[g2].motored)))
		    {
			axle_index = axle_find (g[g2].axle_name);
			g[g2].motored = (a[axle_index].motored = 1);
			g[g2].direction = (a[axle_index].direction = g[g1].direction);
			g[g2].angular_velocity = (a[axle_index].angular_velocity = g[g1].angular_velocity * g[g1].radius / g[g2].radius);
		    }

		    if ((!(g[g1].motored)) && (g[g2].motored))
		    {
			axle_index = axle_find (g[g1].axle_name);
			g[g1].motored = (a[axle_index].motored = 1);
			g[g1].direction = (a[axle_index].direction = g[g2].direction);
			g[g1].angular_velocity = (a[axle_index].angular_velocity = g[g2].angular_velocity * g[g2].radius / g[g1].radius);
		    }
		}
	    }
	}

	for (i = 0; i < number_of_gears; i++)
	{
	    axle_index = axle_find (g[i].axle_name);
	    g[i].motored = a[axle_index].motored;
	    if (a[axle_index].motored)
	    {
		g[i].direction = a[axle_index].direction;
		g[i].angular_velocity = a[axle_index].angular_velocity;
	    }
	}
    }
}



GLfloat view_rotx = 20.0, view_roty = 30.0, view_rotz = 10.0;


static void
draw (void)
{
    int i;
    GLfloat x, y, z;
    int index;

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix ();
    glRotatef (view_rotx, 1.0, 0.0, 0.0);
    glRotatef (view_roty, 0.0, 1.0, 0.0);
    glRotatef (view_rotz, 0.0, 0.0, 1.0);

    for (i = 0; i < number_of_gears; i++)
    {
	x = 0.0;
	y = 0.0;
	z = 0.0;
	glPushMatrix ();
/*glTranslatef( -3.0, -2.0, 0.0 );*/
	glTranslatef (g[i].position[0], g[i].position[1], g[i].position[2]);
	if (g[i].axis == 0)
           y = 1.0;
        else if (g[i].axis == 1)
           x = 1.0;
        else
           z = 1.0;

	if (z != 1.0)
           glRotatef (90.0, x, y, z);

	glRotatef (g[i].direction * g[i].angle, 0.0, 0.0, 1.0);
	glCallList (g[i].id);
	glPopMatrix ();
    }

    for (i = 0; i < number_of_axles; i++)
    {
	x = 0.0;
	y = 0.0;
	z = 0.0;
	glPushMatrix ();
	glTranslatef (a[i].position[0], a[i].position[1], a[i].position[2]);
	if (a[i].axis == 0)
           y = 1.0;
        else if (a[i].axis == 1)
           x = 1.0;
        else
           z = 1.0;

	if (z != 1.0)
           glRotatef (90.0, x, y, z);

	glCallList (a[i].id);
	glPopMatrix ();
    }

    for (i = 0; i < number_of_belts; i++)
    {
	x = 0.0;
	y = 0.0;
	z = 0.0;
	glPushMatrix ();
	index = gear_find (b[i].gear1_name);
	glTranslatef (g[index].position[0], g[index].position[1], g[index].position[2]);
	if (g[index].axis == 0)
           y = 1.0;
        else if (g[index].axis == 1)
           x = 1.0;
        else
           z = 1.0;

	if (z != 1.0)
           glRotatef (90.0, x, y, z);

	glCallList (b[i].id);
	glPopMatrix ();
    }

    glPopMatrix ();
    glutSwapBuffers ();

    {
	GLint t = glutGet(GLUT_ELAPSED_TIME);
	Frames++;
	if (t - T0 >= 5000) {
	    GLfloat seconds = (t - T0) / 1000.0;
	    GLfloat fps = Frames / seconds;
	    printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
            fflush(stdout);
	    T0 = t;
	    Frames = 0;
	}
    }
}


static void
idle (void)
{
    int i;
    static double t0 = -1.;
    double dt, t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    if (t0 < 0.0)
       t0 = t;
    dt = t - t0;
    t0 = t;
    for (i = 0; i < number_of_gears; i++)
      g[i].angle += g[i].angular_velocity * dt;
    glutPostRedisplay();
}




/* change view angle, exit upon ESC */
static void
key (unsigned char k, int x, int y)
{
    switch (k)
    {
    case 'x':
	view_rotx += 5.0;
        break;
    case 'X':
	view_rotx -= 5.0;
        break;
    case 'y':
	view_roty += 5.0;
        break;
    case 'Y':
	view_roty -= 5.0;
        break;
    case 'z':
	view_rotz += 5.0;
        break;
    case 'Z':
	view_rotz -= 5.0;
        break;
    case 0x1B:
	exit(0);
    }
}




/* new window size or exposure */
static void
reshape (int width, int height)
{
    glViewport (0, 0, (GLint) width, (GLint) height);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    if (width > height)
    {
	GLfloat w = (GLfloat) width / (GLfloat) height;
	glFrustum (-w, w, -1.0, 1.0, 5.0, 60.0);
    }
    else
    {
	GLfloat h = (GLfloat) height / (GLfloat) width;
	glFrustum (-1.0, 1.0, -h, h, 5.0, 60.0);
    }

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glTranslatef (0.0, 0.0, -40.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}



static void
init (void)
{
    GLfloat matShine = 20.00F;
    const GLfloat light0Pos[4] =
    {
       0.70F, 0.70F, 1.25F, 0.50F
    };
    int i;

    glShadeModel(GL_FLAT);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

    glClearColor (background[0], background[1], background[2], 1.0F);
    glClearIndex ((GLfloat) 0.0);

    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, matShine);
    glLightfv (GL_LIGHT0, GL_POSITION, light0Pos);
    glEnable (GL_LIGHT0);

    glEnable (GL_LIGHTING);
    glEnable (GL_DEPTH_TEST);
    for (i = 0; i < number_of_gears; i++)
      g[i].angle = 0.0;

    for (i = 0; i < number_of_gears; i++)
    {
	g[i].id = glGenLists (1);
	glNewList (g[i].id, GL_COMPILE);
	glColor3fv (g[i].color);
	glMaterialfv (GL_FRONT, GL_SPECULAR, g[i].color);
	gear (i, g[i].type, g[i].radius, g[i].width, g[i].teeth, g[i].tooth_depth);
	glEndList ();
    }

    for (i = 0; i < number_of_axles; i++)
    {
	a[i].id = glGenLists (1);
	glNewList (a[i].id, GL_COMPILE);
	glColor3fv (a[i].color);
	glMaterialfv (GL_FRONT, GL_SPECULAR, a[i].color);
	axle (i, a[i].radius, a[i].length);
	glEndList ();
    }

    for (i = 0; i < number_of_belts; i++)
    {
	b[i].id = glGenLists (1);
	glNewList (b[i].id, GL_COMPILE);
	belt (g[gear_find (b[i].gear1_name)], g[gear_find (b[i].gear2_name)]);
	glEndList ();
    }

    glEnable (GL_COLOR_MATERIAL);
}



int
main (int argc, char *argv[])
{
    char *file;

    glutInitWindowSize(640,480);
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE );

    if (glutCreateWindow ("Gear Train Simulation") == GL_FALSE)
      exit (1);

    if (argc < 2)
       file = DEMOS_DATA_DIR "geartrain.dat";
    else
       file = argv[1];

    getdata (file);
    process ();
    init ();

    glutDisplayFunc (draw);
    glutReshapeFunc (reshape);
    glutKeyboardFunc (key);
    glutIdleFunc (idle);
    glutMainLoop ();
    return 0;
}
