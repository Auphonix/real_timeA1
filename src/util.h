#pragma once

#include "vec.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#if _WIN32
#  include <Windows.h>
#endif
#if __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define clamp(x, a, b) min(max((x), (a)), (b))

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif

#define RADDEG(x) (180 / M_PI * x)

#define UNUSED(x) (void)(x)

// we can save a little bit of typing with these macros
#define submitColor(v) (glColor3fv((const GLfloat*)&v))
#define submitNormal(v) (glNormal3fv((const GLfloat*)&v))
#define submitTexCoord(v) (glTexCoord2fv((const GLfloat*)&v))
#define submitVertex(v) (glVertex3fv((const GLfloat*)&v))

// some useful colors that we can pass around
const Vec3f WHITE;
const Vec3f RED;
const Vec3f GREEN;
const Vec3f BLUE;
const Vec3f YELLOW;
const Vec3f CYAN;
const Vec3f PURPLE;

float getRand();
float getNRand();
float getTRand(float min, float max);

void drawAxes();

unsigned int loadTexture(const char* filename);
