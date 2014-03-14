
#pragma once

#include <string>
#include <windows.h>
#include <stdio.h>


#define  LOGI(...)  printf(__VA_ARGS__)
#define  LOGE(...)  printf(__VA_ARGS__)


#include <GL/gl.h>
#include <GL/glu.h>
#include "opengl/glext.h"


// GL_ARB_shader_objects
extern PFNGLCREATEPROGRAMOBJECTARBPROC  glCreateProgram;
extern PFNGLDELETEOBJECTARBPROC         glDeleteObject;
extern PFNGLUSEPROGRAMOBJECTARBPROC     glUseProgram;
extern PFNGLCREATESHADEROBJECTARBPROC   glCreateShader;
extern PFNGLSHADERSOURCEARBPROC         glShaderSource;
extern PFNGLCOMPILESHADERARBPROC        glCompileShader;
//extern PFNGLGETOBJECTPARAMETERIVARBPROC glGetShaderiv;
extern PFNGLATTACHOBJECTARBPROC         glAttachShader;
extern PFNGLGETINFOLOGARBPROC           glGetInfoLog;
extern PFNGLLINKPROGRAMARBPROC          glLinkProgram;
extern PFNGLGETUNIFORMLOCATIONARBPROC   glGetUniformLocation;
extern PFNGLUNIFORM4FARBPROC            glUniform4f;
extern PFNGLUNIFORM1IARBPROC            glUniform1i;
extern PFNGLUNIFORMMATRIX4FVARBPROC				 glUniformMatrix4fv;
extern PFNGLACTIVETEXTUREARBPROC			glActiveTexture;
extern PFNGLISSHADERPROC					glIsShader;
extern PFNGLISPROGRAMPROC					glIsProgram;
extern PFNGLGETSHADERIVPROC							glGetShaderiv;
extern PFNGLGETPROGRAMIVPROC				glGetProgramiv;
extern PFNGLUNIFORM1FVPROC glUniform1fv;
extern PFNGLGETPROGRAMINFOLOGPROC		glGetProgramInfoLog;


#define glGetShaderInfoLog glGetInfoLog 

