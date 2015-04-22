
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLUtil.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

static GrGLFuncPtr android_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(NULL == ctx);
    // Some older drivers on Android have busted eglGetProcAdddress Functions that
    // will return the wrong pointer for built in GLES2 functions. This set of functions
    // was generated on a Xoom by finding mismatches between the function pulled in via gl2.h and
    // the address returned by eglGetProcAddress.
    if (0 == strcmp("glActiveTexture", name)) {
        return (GrGLFuncPtr) glActiveTexture;
    } else if (0 == strcmp("glAttachShader", name)) {
        return (GrGLFuncPtr) glAttachShader;
    } else if (0 == strcmp("glBindAttribLocation", name)) {
        return (GrGLFuncPtr) glBindAttribLocation;
    } else if (0 == strcmp("glBindBuffer", name)) {
        return (GrGLFuncPtr) glBindBuffer;
    } else if (0 == strcmp("glBindTexture", name)) {
        return (GrGLFuncPtr) glBindTexture;
    } else if (0 == strcmp("glBlendColor", name)) {
        return (GrGLFuncPtr) glBlendColor;
    } else if (0 == strcmp("glBlendEquation", name)) {
        return (GrGLFuncPtr) glBlendEquation;
    } else if (0 == strcmp("glBlendFunc", name)) {
        return (GrGLFuncPtr) glBlendFunc;
    } else if (0 == strcmp("glBufferData", name)) {
        return (GrGLFuncPtr) glBufferData;
    } else if (0 == strcmp("glBufferSubData", name)) {
        return (GrGLFuncPtr) glBufferSubData;
    } else if (0 == strcmp("glClear", name)) {
        return (GrGLFuncPtr) glClear;
    } else if (0 == strcmp("glClearColor", name)) {
        return (GrGLFuncPtr) glClearColor;
    } else if (0 == strcmp("glClearStencil", name)) {
        return (GrGLFuncPtr) glClearStencil;
    } else if (0 == strcmp("glColorMask", name)) {
        return (GrGLFuncPtr) glColorMask;
    } else if (0 == strcmp("glCompileShader", name)) {
        return (GrGLFuncPtr) glCompileShader;
    } else if (0 == strcmp("glCompressedTexImage2D", name)) {
        return (GrGLFuncPtr) glCompressedTexImage2D;
    } else if (0 == strcmp("glCompressedTexSubImage2D", name)) {
        return (GrGLFuncPtr) glCompressedTexSubImage2D;
    } else if (0 == strcmp("glCopyTexSubImage2D", name)) {
        return (GrGLFuncPtr) glCopyTexSubImage2D;
    } else if (0 == strcmp("glCreateProgram", name)) {
        return (GrGLFuncPtr) glCreateProgram;
    } else if (0 == strcmp("glCreateShader", name)) {
        return (GrGLFuncPtr) glCreateShader;
    } else if (0 == strcmp("glCullFace", name)) {
        return (GrGLFuncPtr) glCullFace;
    } else if (0 == strcmp("glDeleteBuffers", name)) {
        return (GrGLFuncPtr) glDeleteBuffers;
    } else if (0 == strcmp("glDeleteProgram", name)) {
        return (GrGLFuncPtr) glDeleteProgram;
    } else if (0 == strcmp("glDeleteShader", name)) {
        return (GrGLFuncPtr) glDeleteShader;
    } else if (0 == strcmp("glDeleteTextures", name)) {
        return (GrGLFuncPtr) glDeleteTextures;
    } else if (0 == strcmp("glDepthMask", name)) {
        return (GrGLFuncPtr) glDepthMask;
    } else if (0 == strcmp("glDisable", name)) {
        return (GrGLFuncPtr) glDisable;
    } else if (0 == strcmp("glDisableVertexAttribArray", name)) {
        return (GrGLFuncPtr) glDisableVertexAttribArray;
    } else if (0 == strcmp("glDrawArrays", name)) {
        return (GrGLFuncPtr) glDrawArrays;
    } else if (0 == strcmp("glDrawElements", name)) {
        return (GrGLFuncPtr) glDrawElements;
    } else if (0 == strcmp("glEnable", name)) {
        return (GrGLFuncPtr) glEnable;
    } else if (0 == strcmp("glEnableVertexAttribArray", name)) {
        return (GrGLFuncPtr) glEnableVertexAttribArray;
    } else if (0 == strcmp("glFinish", name)) {
        return (GrGLFuncPtr) glFinish;
    } else if (0 == strcmp("glFlush", name)) {
        return (GrGLFuncPtr) glFlush;
    } else if (0 == strcmp("glFrontFace", name)) {
        return (GrGLFuncPtr) glFrontFace;
    } else if (0 == strcmp("glGenBuffers", name)) {
        return (GrGLFuncPtr) glGenBuffers;
    } else if (0 == strcmp("glGenerateMipmap", name)) {
        return (GrGLFuncPtr) glGenerateMipmap;
    } else if (0 == strcmp("glGenTextures", name)) {
        return (GrGLFuncPtr) glGenTextures;
    } else if (0 == strcmp("glGetBufferParameteriv", name)) {
        return (GrGLFuncPtr) glGetBufferParameteriv;
    } else if (0 == strcmp("glGetError", name)) {
        return (GrGLFuncPtr) glGetError;
    } else if (0 == strcmp("glGetIntegerv", name)) {
        return (GrGLFuncPtr) glGetIntegerv;
    } else if (0 == strcmp("glGetProgramInfoLog", name)) {
        return (GrGLFuncPtr) glGetProgramInfoLog;
    } else if (0 == strcmp("glGetProgramiv", name)) {
        return (GrGLFuncPtr) glGetProgramiv;
    } else if (0 == strcmp("glGetShaderInfoLog", name)) {
        return (GrGLFuncPtr) glGetShaderInfoLog;
    } else if (0 == strcmp("glGetShaderiv", name)) {
        return (GrGLFuncPtr) glGetShaderiv;
    } else if (0 == strcmp("glGetString", name)) {
        return (GrGLFuncPtr) glGetString;
    } else if (0 == strcmp("glGetUniformLocation", name)) {
        return (GrGLFuncPtr) glGetUniformLocation;
    } else if (0 == strcmp("glLineWidth", name)) {
        return (GrGLFuncPtr) glLineWidth;
    } else if (0 == strcmp("glLinkProgram", name)) {
        return (GrGLFuncPtr) glLinkProgram;
    } else if (0 == strcmp("glPixelStorei", name)) {
        return (GrGLFuncPtr) glPixelStorei;
    } else if (0 == strcmp("glReadPixels", name)) {
        return (GrGLFuncPtr) glReadPixels;
    } else if (0 == strcmp("glScissor", name)) {
        return (GrGLFuncPtr) glScissor;
    } else if (0 == strcmp("glShaderSource", name)) {
        return (GrGLFuncPtr) glShaderSource;
    } else if (0 == strcmp("glStencilFunc", name)) {
        return (GrGLFuncPtr) glStencilFunc;
    } else if (0 == strcmp("glStencilFuncSeparate", name)) {
        return (GrGLFuncPtr) glStencilFuncSeparate;
    } else if (0 == strcmp("glStencilMask", name)) {
        return (GrGLFuncPtr) glStencilMask;
    } else if (0 == strcmp("glStencilMaskSeparate", name)) {
        return (GrGLFuncPtr) glStencilMaskSeparate;
    } else if (0 == strcmp("glStencilOp", name)) {
        return (GrGLFuncPtr) glStencilOp;
    } else if (0 == strcmp("glStencilOpSeparate", name)) {
        return (GrGLFuncPtr) glStencilOpSeparate;
    } else if (0 == strcmp("glTexImage2D", name)) {
        return (GrGLFuncPtr) glTexImage2D;
    } else if (0 == strcmp("glTexParameteri", name)) {
        return (GrGLFuncPtr) glTexParameteri;
    } else if (0 == strcmp("glTexParameteriv", name)) {
        return (GrGLFuncPtr) glTexParameteriv;
    } else if (0 == strcmp("glTexSubImage2D", name)) {
        return (GrGLFuncPtr) glTexSubImage2D;
    } else if (0 == strcmp("glUniform1f", name)) {
        return (GrGLFuncPtr) glUniform1f;
    } else if (0 == strcmp("glUniform1i", name)) {
        return (GrGLFuncPtr) glUniform1i;
    } else if (0 == strcmp("glUniform1fv", name)) {
        return (GrGLFuncPtr) glUniform1fv;
    } else if (0 == strcmp("glUniform1iv", name)) {
        return (GrGLFuncPtr) glUniform1iv;
    } else if (0 == strcmp("glUniform2f", name)) {
        return (GrGLFuncPtr) glUniform2f;
    } else if (0 == strcmp("glUniform2i", name)) {
        return (GrGLFuncPtr) glUniform2i;
    } else if (0 == strcmp("glUniform2fv", name)) {
        return (GrGLFuncPtr) glUniform2fv;
    } else if (0 == strcmp("glUniform2iv", name)) {
        return (GrGLFuncPtr) glUniform2iv;
    } else if (0 == strcmp("glUniform3f", name)) {
        return (GrGLFuncPtr) glUniform3f;
    } else if (0 == strcmp("glUniform3i", name)) {
        return (GrGLFuncPtr) glUniform3i;
    } else if (0 == strcmp("glUniform3fv", name)) {
        return (GrGLFuncPtr) glUniform3fv;
    } else if (0 == strcmp("glUniform3iv", name)) {
        return (GrGLFuncPtr) glUniform3iv;
    } else if (0 == strcmp("glUniform4f", name)) {
        return (GrGLFuncPtr) glUniform4f;
    } else if (0 == strcmp("glUniform4i", name)) {
        return (GrGLFuncPtr) glUniform4i;
    } else if (0 == strcmp("glUniform4fv", name)) {
        return (GrGLFuncPtr) glUniform4fv;
    } else if (0 == strcmp("glUniform4iv", name)) {
        return (GrGLFuncPtr) glUniform4iv;
    } else if (0 == strcmp("glUniformMatrix2fv", name)) {
        return (GrGLFuncPtr) glUniformMatrix2fv;
    } else if (0 == strcmp("glUniformMatrix3fv", name)) {
        return (GrGLFuncPtr) glUniformMatrix3fv;
    } else if (0 == strcmp("glUniformMatrix4fv", name)) {
        return (GrGLFuncPtr) glUniformMatrix4fv;
    } else if (0 == strcmp("glUseProgram", name)) {
        return (GrGLFuncPtr) glUseProgram;
    } else if (0 == strcmp("glVertexAttrib1f", name)) {
        return (GrGLFuncPtr) glVertexAttrib1f;
    } else if (0 == strcmp("glVertexAttrib2fv", name)) {
        return (GrGLFuncPtr) glVertexAttrib2fv;
    } else if (0 == strcmp("glVertexAttrib3fv", name)) {
        return (GrGLFuncPtr) glVertexAttrib3fv;
    } else if (0 == strcmp("glVertexAttrib4fv", name)) {
        return (GrGLFuncPtr) glVertexAttrib4fv;
    } else if (0 == strcmp("glVertexAttribPointer", name)) {
        return (GrGLFuncPtr) glVertexAttribPointer;
    } else if (0 == strcmp("glViewport", name)) {
        return (GrGLFuncPtr) glViewport;
    } else if (0 == strcmp("glBindFramebuffer", name)) {
        return (GrGLFuncPtr) glBindFramebuffer;
    } else if (0 == strcmp("glBindRenderbuffer", name)) {
        return (GrGLFuncPtr) glBindRenderbuffer;
    } else if (0 == strcmp("glCheckFramebufferStatus", name)) {
        return (GrGLFuncPtr) glCheckFramebufferStatus;
    } else if (0 == strcmp("glDeleteFramebuffers", name)) {
        return (GrGLFuncPtr) glDeleteFramebuffers;
    } else if (0 == strcmp("glDeleteRenderbuffers", name)) {
        return (GrGLFuncPtr) glDeleteRenderbuffers;
    } else if (0 == strcmp("glFramebufferRenderbuffer", name)) {
        return (GrGLFuncPtr) glFramebufferRenderbuffer;
    } else if (0 == strcmp("glFramebufferTexture2D", name)) {
        return (GrGLFuncPtr) glFramebufferTexture2D;
    } else if (0 == strcmp("glGenFramebuffers", name)) {
        return (GrGLFuncPtr) glGenFramebuffers;
    } else if (0 == strcmp("glGenRenderbuffers", name)) {
        return (GrGLFuncPtr) glGenRenderbuffers;
    } else if (0 == strcmp("glGetFramebufferAttachmentParameteriv", name)) {
        return (GrGLFuncPtr) glGetFramebufferAttachmentParameteriv;
    } else if (0 == strcmp("glGetRenderbufferParameteriv", name)) {
        return (GrGLFuncPtr) glGetRenderbufferParameteriv;
    } else if (0 == strcmp("glRenderbufferStorage", name)) {
        return (GrGLFuncPtr) glRenderbufferStorage;
    }
    return eglGetProcAddress(name);
}

const GrGLInterface* GrGLCreateNativeInterface() {
    return GrGLAssembleInterface(NULL, android_get_gl_proc);
}
