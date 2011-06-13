// Modified from chromium/src/webkit/glue/gl_bindings_skia_cmd_buffer.cc

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GrGLInterface.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include "gl2.h"
#include "gl2ext.h"
/*
#include "gpu/GLES2/gl2.h"
#include "gpu/GLES2/gl2ext.h"
*/
void GrGLSetDefaultGLInterface() {
  static GrGLInterface cmd_buffer_interface = {
    kES2_GrGLBinding,

    glActiveTexture,
    glAttachShader,
    glBindAttribLocation,
    glBindBuffer,
    glBindTexture,
    glBlendColor,
    glBlendFunc,
    glBufferData,
    glBufferSubData,
    glClear,
    glClearColor,
    glClearStencil,
    NULL,  // glClientActiveTexture
    NULL,  // glColor4ub
    glColorMask,
    NULL,  // glColorPointer
    glCompileShader,
    glCompressedTexImage2D,
    glCreateProgram,
    glCreateShader,
    glCullFace,
    glDeleteBuffers,
    glDeleteProgram,
    glDeleteShader,
    glDeleteTextures,
    glDepthMask,
    glDisable,
    NULL,  // glDisableClientState
    glDisableVertexAttribArray,
    glDrawArrays,
    glDrawElements,
    glEnable,
    NULL,  // glEnableClientState
    glEnableVertexAttribArray,
    glFrontFace,
    glGenBuffers,
    glGenTextures,
    glGetBufferParameteriv,
    glGetError,
    glGetIntegerv,
    glGetProgramInfoLog,
    glGetProgramiv,
    glGetShaderInfoLog,
    glGetShaderiv,
    glGetString,
    glGetUniformLocation,
    glLineWidth,
    glLinkProgram,
    NULL,  // glLoadMatrixf
    NULL,  // glMatrixMode
    glPixelStorei,
    NULL,  // glPointSize
    glReadPixels,
    glScissor,
    NULL,  // glShadeModel
    glShaderSource,
    glStencilFunc,
    glStencilFuncSeparate,
    glStencilMask,
    glStencilMaskSeparate,
    glStencilOp,
    glStencilOpSeparate,
    NULL,  // glTexCoordPointer
    NULL,  // glTexEnvi
    glTexImage2D,
    glTexParameteri,
    glTexSubImage2D,
    glUniform1f,
    glUniform1i,
    glUniform1fv,
    glUniform1iv,
    glUniform2f,
    glUniform2i,
    glUniform2fv,
    glUniform2iv,
    glUniform3f,
    glUniform3i,
    glUniform3fv,
    glUniform3iv,
    glUniform4f,
    glUniform4i,
    glUniform4fv,
    glUniform4iv,
    glUniformMatrix2fv,
    glUniformMatrix3fv,
    glUniformMatrix4fv,
    glUseProgram,
    glVertexAttrib4fv,
    glVertexAttribPointer,
    NULL,  // glVertexPointer
    glViewport,
    glBindFramebuffer,
    glBindRenderbuffer,
    glCheckFramebufferStatus,
    glDeleteFramebuffers,
    glDeleteRenderbuffers,
    glFramebufferRenderbuffer,
    glFramebufferTexture2D,
    glGenFramebuffers,
    glGenRenderbuffers,
    glRenderbufferStorage,
    NULL,  // glRenderbufferStorageMultisampleEXT,
    NULL,  // glBlitFramebufferEXT,
    NULL,  // glResolveMultisampleFramebuffer
    glMapBufferOES,
    glUnmapBufferOES,
    NULL,
    GrGLInterface::kStaticInitEndGuard
  };
  static bool host_StubGL_initialized = false;
  if (!host_StubGL_initialized) {
    GrGLSetGLInterface(&cmd_buffer_interface);
    host_StubGL_initialized = true;
  }
}

