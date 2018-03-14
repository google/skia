/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"
#if defined(SK_BUILD_FOR_ANDROID)

#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLUtil.h"

#include <EGL/egl.h>
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2.h>

static GrGLFuncPtr android_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(nullptr == ctx);
    // Some older drivers on Android have busted eglGetProcAdddress Functions that
    // will return the wrong pointer for built in GLES2 functions. This set of functions
    // was generated on a Xoom by finding mismatches between the function pulled in via gl2.h and
    // the address returned by eglGetProcAddress.

    #define M(X) if (0 == strcmp(#X, name)) { return (GrGLFuncPtr) X; }
    M(eglGetCurrentDisplay);
    M(eglQueryString);
    M(glActiveTexture);
    M(glAttachShader);
    M(glBindAttribLocation);
    M(glBindBuffer);
    M(glBindFramebuffer);
    M(glBindRenderbuffer);
    M(glBindTexture);
    M(glBlendColor);
    M(glBlendEquation);
    M(glBlendFunc);
    M(glBufferData);
    M(glBufferSubData);
    M(glCheckFramebufferStatus);
    M(glClear);
    M(glClearColor);
    M(glClearStencil);
    M(glColorMask);
    M(glCompileShader);
    M(glCompressedTexImage2D);
    M(glCompressedTexSubImage2D);
    M(glCopyTexSubImage2D);
    M(glCreateProgram);
    M(glCreateShader);
    M(glCullFace);
    M(glDeleteBuffers);
    M(glDeleteFramebuffers);
    M(glDeleteProgram);
    M(glDeleteRenderbuffers);
    M(glDeleteShader);
    M(glDeleteTextures);
    M(glDepthMask);
    M(glDisable);
    M(glDisableVertexAttribArray);
    M(glDrawArrays);
    M(glDrawElements);
    M(glEnable);
    M(glEnableVertexAttribArray);
    M(glFinish);
    M(glFlush);
    M(glFramebufferRenderbuffer);
    M(glFramebufferTexture2D);
    M(glFrontFace);
    M(glGenBuffers);
    M(glGenFramebuffers);
    M(glGenRenderbuffers);
    M(glGenTextures);
    M(glGenerateMipmap);
    M(glGetBufferParameteriv);
    M(glGetError);
    M(glGetFramebufferAttachmentParameteriv);
    M(glGetIntegerv);
    M(glGetProgramInfoLog);
    M(glGetProgramiv);
    M(glGetRenderbufferParameteriv);
    M(glGetShaderInfoLog);
    M(glGetShaderPrecisionFormat);
    M(glGetShaderiv);
    M(glGetString);
    M(glGetUniformLocation);
    M(glIsTexture);
    M(glLineWidth);
    M(glLinkProgram);
    M(glPixelStorei);
    M(glReadPixels);
    M(glRenderbufferStorage);
    M(glScissor);
    M(glShaderSource);
    M(glStencilFunc);
    M(glStencilFuncSeparate);
    M(glStencilMask);
    M(glStencilMaskSeparate);
    M(glStencilOp);
    M(glStencilOpSeparate);
    M(glTexImage2D);
    M(glTexParameteri);
    M(glTexParameteriv);
    M(glTexSubImage2D);
    M(glUniform1f);
    M(glUniform1fv);
    M(glUniform1i);
    M(glUniform1iv);
    M(glUniform2f);
    M(glUniform2fv);
    M(glUniform2i);
    M(glUniform2iv);
    M(glUniform3f);
    M(glUniform3fv);
    M(glUniform3i);
    M(glUniform3iv);
    M(glUniform4f);
    M(glUniform4fv);
    M(glUniform4i);
    M(glUniform4iv);
    M(glUniformMatrix2fv);
    M(glUniformMatrix3fv);
    M(glUniformMatrix4fv);
    M(glUseProgram);
    M(glVertexAttrib1f);
    M(glVertexAttrib2fv);
    M(glVertexAttrib3fv);
    M(glVertexAttrib4fv);
    M(glVertexAttribPointer);
    M(glViewport);
    #undef M
    return eglGetProcAddress(name);
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    return GrGLMakeAssembledInterface(nullptr, android_get_gl_proc);
}

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }

#endif//defined(SK_BUILD_FOR_ANDROID)
