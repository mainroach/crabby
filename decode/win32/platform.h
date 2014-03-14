/*Copyright 2013 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
#limitations under the License.*/
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

