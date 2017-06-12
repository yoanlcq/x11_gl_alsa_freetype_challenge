#pragma once

#include <dsky/vec.h>

#ifndef _WIN32
#define GL_GLEXT_PROTOTYPES
#endif
#include <GL/gl.h>
#include <GL/glext.h>

#ifdef __linux__
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

GLuint GLTexture_from_power_of_two_rgba32(const Rgba32 *data, Extent2u size);
GLuint GLProgram_quick_load(const char *vert, const char *frag);
