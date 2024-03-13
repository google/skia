/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLCoreFunctions_DEFINED
#define GrGLCoreFunctions_DEFINED

//
// https://www.khronos.org/registry/EGL/extensions/KHR/EGL_KHR_get_all_proc_addresses.txt
// eglGetProcAddress() is not guaranteed to support the querying of non-extension EGL functions.
//
// This applies macro "M" to each core function name used by Skia.
//
#define GR_GL_CORE_FUNCTIONS_EACH(M) \
    M(eglGetCurrentDisplay) \
    M(eglQueryString) \
    M(glActiveTexture) \
    M(glAttachShader) \
    M(glBindAttribLocation) \
    M(glBindBuffer) \
    M(glBindFramebuffer) \
    M(glBindRenderbuffer) \
    M(glBindTexture) \
    M(glBlendColor) \
    M(glBlendEquation) \
    M(glBlendFunc) \
    M(glBufferData) \
    M(glBufferSubData) \
    M(glCheckFramebufferStatus) \
    M(glClear) \
    M(glClearColor) \
    M(glClearStencil) \
    M(glColorMask) \
    M(glCompileShader) \
    M(glCompressedTexImage2D) \
    M(glCompressedTexSubImage2D) \
    M(glCopyTexSubImage2D) \
    M(glCreateProgram) \
    M(glCreateShader) \
    M(glCullFace) \
    M(glDeleteBuffers) \
    M(glDeleteFramebuffers) \
    M(glDeleteProgram) \
    M(glDeleteRenderbuffers) \
    M(glDeleteShader) \
    M(glDeleteTextures) \
    M(glDepthMask) \
    M(glDisable) \
    M(glDisableVertexAttribArray) \
    M(glDrawArrays) \
    M(glDrawElements) \
    M(glEnable) \
    M(glEnableVertexAttribArray) \
    M(glFinish) \
    M(glFlush) \
    M(glFramebufferRenderbuffer) \
    M(glFramebufferTexture2D) \
    M(glFrontFace) \
    M(glGenBuffers) \
    M(glGenFramebuffers) \
    M(glGenRenderbuffers) \
    M(glGenTextures) \
    M(glGenerateMipmap) \
    M(glGetBufferParameteriv) \
    M(glGetError) \
    M(glGetFramebufferAttachmentParameteriv) \
    M(glGetIntegerv) \
    M(glGetProgramInfoLog) \
    M(glGetProgramiv) \
    M(glGetRenderbufferParameteriv) \
    M(glGetShaderInfoLog) \
    M(glGetShaderPrecisionFormat) \
    M(glGetShaderiv) \
    M(glGetString) \
    M(glGetUniformLocation) \
    M(glIsTexture) \
    M(glLineWidth) \
    M(glLinkProgram) \
    M(glPixelStorei) \
    M(glReadPixels) \
    M(glRenderbufferStorage) \
    M(glScissor) \
    M(glShaderSource) \
    M(glStencilFunc) \
    M(glStencilFuncSeparate) \
    M(glStencilMask) \
    M(glStencilMaskSeparate) \
    M(glStencilOp) \
    M(glStencilOpSeparate) \
    M(glTexImage2D) \
    M(glTexParameterf) \
    M(glTexParameterfv) \
    M(glTexParameteri) \
    M(glTexParameteriv) \
    M(glTexSubImage2D) \
    M(glUniform1f) \
    M(glUniform1fv) \
    M(glUniform1i) \
    M(glUniform1iv) \
    M(glUniform2f) \
    M(glUniform2fv) \
    M(glUniform2i) \
    M(glUniform2iv) \
    M(glUniform3f) \
    M(glUniform3fv) \
    M(glUniform3i) \
    M(glUniform3iv) \
    M(glUniform4f) \
    M(glUniform4fv) \
    M(glUniform4i) \
    M(glUniform4iv) \
    M(glUniformMatrix2fv) \
    M(glUniformMatrix3fv) \
    M(glUniformMatrix4fv) \
    M(glUseProgram) \
    M(glVertexAttrib1f) \
    M(glVertexAttrib2fv) \
    M(glVertexAttrib3fv) \
    M(glVertexAttrib4fv) \
    M(glVertexAttribPointer) \
    M(glViewport)

#endif
