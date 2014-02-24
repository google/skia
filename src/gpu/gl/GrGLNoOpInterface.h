/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLNoOpInterface_DEFINED
#define GrGLNoOpInterface_DEFINED

#include "gl/GrGLDefines.h"
#include "gl/GrGLFunctions.h"

// These are constants/functions that are common to the Null and Debug GL interface implementations.

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBlendColor(GrGLclampf red,
                                              GrGLclampf green,
                                              GrGLclampf blue,
                                              GrGLclampf alpha);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBindFragDataLocation(GrGLuint program,
                                                        GrGLuint colorNumber,
                                                        const GrGLchar* name);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBlendFunc(GrGLenum sfactor,
                                             GrGLenum dfactor);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBufferSubData(GrGLenum target,
                                                 GrGLintptr offset,
                                                 GrGLsizeiptr size,
                                                 const GrGLvoid* data);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLClear(GrGLbitfield mask);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLClearColor(GrGLclampf red,
                                              GrGLclampf green,
                                              GrGLclampf blue,
                                              GrGLclampf alpha);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLClearStencil(GrGLint s);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLColorMask(GrGLboolean red,
                                             GrGLboolean green,
                                             GrGLboolean blue,
                                             GrGLboolean alpha);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLCompileShader(GrGLuint shader);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLCompressedTexImage2D(GrGLenum target,
                                                        GrGLint level,
                                                        GrGLenum internalformat,
                                                        GrGLsizei width,
                                                        GrGLsizei height,
                                                        GrGLint border,
                                                        GrGLsizei imageSize,
                                                        const GrGLvoid* data);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLCopyTexSubImage2D(GrGLenum target,
                                                     GrGLint level,
                                                     GrGLint xoffset,
                                                     GrGLint yoffset,
                                                     GrGLint x,
                                                     GrGLint y,
                                                     GrGLsizei width,
                                                     GrGLsizei height);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLCullFace(GrGLenum mode);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDepthMask(GrGLboolean flag);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDisable(GrGLenum cap);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDisableVertexAttribArray(GrGLuint index);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDrawArrays(GrGLenum mode, GrGLint first, GrGLsizei count);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDrawBuffer(GrGLenum mode);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDrawBuffers(GrGLsizei n,
                                                const GrGLenum* bufs);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDrawElements(GrGLenum mode,
                                                GrGLsizei count,
                                                GrGLenum type,
                                                const GrGLvoid* indices);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLEnable(GrGLenum cap);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLEnableVertexAttribArray(GrGLuint index);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLEndQuery(GrGLenum target);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLFinish();

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLFlush();

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLFrontFace(GrGLenum mode);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLLoadIdentity();

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLLoadMatrixf(const GrGLfloat*);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLLineWidth(GrGLfloat width);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLLinkProgram(GrGLuint program);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLMatrixMode(GrGLenum);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLQueryCounter(GrGLuint id,
                                                GrGLenum target);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLReadBuffer(GrGLenum src);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLScissor(GrGLint x,
                                           GrGLint y,
                                           GrGLsizei width,
                                           GrGLsizei height);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLShaderSource(GrGLuint shader,
                                                GrGLsizei count,
#if GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE
                                                const char* const * str,
#else
                                                const char** str,
#endif
                                                const GrGLint* length);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilFunc(GrGLenum func, GrGLint ref, GrGLuint mask);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilFuncSeparate(GrGLenum face,
                                                       GrGLenum func,
                                                       GrGLint ref,
                                                       GrGLuint mask);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilMask(GrGLuint mask);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilMaskSeparate(GrGLenum face, GrGLuint mask);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilOp(GrGLenum fail, GrGLenum zfail, GrGLenum zpass);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilOpSeparate(GrGLenum face,
                                                     GrGLenum fail,
                                                     GrGLenum zfail,
                                                     GrGLenum zpass);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexImage2D(GrGLenum target,
                                              GrGLint level,
                                              GrGLint internalformat,
                                              GrGLsizei width,
                                              GrGLsizei height,
                                              GrGLint border,
                                              GrGLenum format,
                                              GrGLenum type,
                                              const GrGLvoid* pixels);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexParameteri(GrGLenum target, GrGLenum pname, GrGLint param);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexParameteriv(GrGLenum target,
                                                  GrGLenum pname,
                                                  const GrGLint* params);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexStorage2D(GrGLenum target,
                                                GrGLsizei levels,
                                                GrGLenum internalformat,
                                                GrGLsizei width,
                                                GrGLsizei height);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexGenf(GrGLenum, GrGLenum, GrGLfloat);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexGenfv(GrGLenum, GrGLenum, const GrGLfloat*);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexGeni(GrGLenum, GrGLenum, GrGLint);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDiscardFramebuffer(GrGLenum target,
                                                      GrGLsizei numAttachments,
                                                      const GrGLenum* attachments);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexSubImage2D(GrGLenum target,
                                                 GrGLint level,
                                                 GrGLint xoffset,
                                                 GrGLint yoffset,
                                                 GrGLsizei width,
                                                 GrGLsizei height,
                                                 GrGLenum format,
                                                 GrGLenum type,
                                                 const GrGLvoid* pixels);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform1f(GrGLint location, GrGLfloat v0);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform1i(GrGLint location, GrGLint v0);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform1fv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLfloat* v);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform1iv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLint* v);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform2f(GrGLint location,
                                             GrGLfloat v0,
                                             GrGLfloat v1);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform2i(GrGLint location, GrGLint v0, GrGLint v1);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform2fv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLfloat* v);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform2iv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLint* v);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform3f(GrGLint location,
                                             GrGLfloat v0,
                                             GrGLfloat v1,
                                             GrGLfloat v2);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform3i(GrGLint location,
                                             GrGLint v0,
                                             GrGLint v1,
                                             GrGLint v2);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform3fv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLfloat* v);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform3iv(GrGLint location,
                                               GrGLsizei count,
                                               const GrGLint* v);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform4f(GrGLint location,
                                              GrGLfloat v0,
                                              GrGLfloat v1,
                                              GrGLfloat v2,
                                              GrGLfloat v3);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform4i(GrGLint location,
                                             GrGLint v0,
                                             GrGLint v1,
                                             GrGLint v2,
                                             GrGLint v3);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform4fv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLfloat* v);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform4iv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLint* v);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniformMatrix2fv(GrGLint location,
                                                    GrGLsizei count,
                                                    GrGLboolean transpose,
                                                    const GrGLfloat* value);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniformMatrix3fv(GrGLint location,
                                                    GrGLsizei count,
                                                    GrGLboolean transpose,
                                                    const GrGLfloat* value);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniformMatrix4fv(GrGLint location,
                                                    GrGLsizei count,
                                                    GrGLboolean transpose,
                                                    const GrGLfloat* value);

 GrGLvoid GR_GL_FUNCTION_TYPE noOpGLVertexAttrib4fv(GrGLuint indx, const GrGLfloat* values);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLVertexAttribPointer(GrGLuint indx,
                                                       GrGLint size,
                                                       GrGLenum type,
                                                       GrGLboolean normalized,
                                                       GrGLsizei stride,
                                                       const GrGLvoid* ptr);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLViewport(GrGLint x,
                                            GrGLint y,
                                            GrGLsizei width,
                                            GrGLsizei height);

  GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetFramebufferAttachmentParameteriv(GrGLenum target,
                                                                         GrGLenum attachment,
                                                                         GrGLenum pname,
                                                                         GrGLint* params);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetRenderbufferParameteriv(GrGLenum target,
                                                              GrGLenum pname,
                                                              GrGLint* params);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLRenderbufferStorage(GrGLenum target,
                                                       GrGLenum internalformat,
                                                       GrGLsizei width,
                                                       GrGLsizei height);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLRenderbufferStorageMultisample(GrGLenum target,
                                                                  GrGLsizei samples,
                                                                  GrGLenum internalformat,
                                                                  GrGLsizei width,
                                                                  GrGLsizei height);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBlitFramebuffer(GrGLint srcX0,
                                                   GrGLint srcY0,
                                                   GrGLint srcX1,
                                                   GrGLint srcY1,
                                                   GrGLint dstX0,
                                                   GrGLint dstY0,
                                                   GrGLint dstX1,
                                                   GrGLint dstY1,
                                                   GrGLbitfield mask,
                                                   GrGLenum filter);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLResolveMultisampleFramebuffer();

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBindFragDataLocationIndexed(GrGLuint program,
                                                               GrGLuint colorNumber,
                                                               GrGLuint index,
                                                               const GrGLchar * name);

GrGLenum GR_GL_FUNCTION_TYPE noOpGLCheckFramebufferStatus(GrGLenum target);

// this function can be used for all glGen*(GLsize i, GLuint*) functions
GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGenIds(GrGLsizei n, GrGLuint* ids);

// this function function can be used for all glDelete*(GLsize i, const GLuint*)
GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDeleteIds(GrGLsizei n, const GrGLuint* ids);

GrGLenum GR_GL_FUNCTION_TYPE noOpGLGetError();

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetIntegerv(GrGLenum pname, GrGLint* params);

// can be used for both the program and shader info logs
GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetInfoLog(GrGLuint program,
                                              GrGLsizei bufsize,
                                              GrGLsizei* length,
                                              char* infolog);

// can be used for both the program and shader params
GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetShaderOrProgramiv(GrGLuint program,
                                                        GrGLenum pname,
                                                        GrGLint* params);

// Queries on bogus GLs just don't do anything at all. We could potentially make the timers work.
GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetQueryiv(GrGLenum GLtarget,
                                              GrGLenum pname,
                                              GrGLint *params);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetQueryObjecti64v(GrGLuint id,
                                                      GrGLenum pname,
                                                      GrGLint64 *params);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetQueryObjectiv(GrGLuint id, GrGLenum pname, GrGLint *params);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetQueryObjectui64v(GrGLuint id,
                                                       GrGLenum pname,
                                                       GrGLuint64 *params);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetQueryObjectuiv(GrGLuint id,
                                                     GrGLenum pname,
                                                     GrGLuint *params);

const GrGLubyte* GR_GL_FUNCTION_TYPE noOpGLGetString(GrGLenum name);

const GrGLubyte* GR_GL_FUNCTION_TYPE noOpGLGetStringi(GrGLenum name, GrGLuint i);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetTexLevelParameteriv(GrGLenum target,
                                                          GrGLint level,
                                                          GrGLenum pname,
                                                          GrGLint* params);

GrGLint GR_GL_FUNCTION_TYPE noOpGLGetUniformLocation(GrGLuint program, const char* name);

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLInsertEventMarker(GrGLsizei length, const char* marker);
GrGLvoid GR_GL_FUNCTION_TYPE noOpGLPushGroupMarker(GrGLsizei length  , const char* marker);
GrGLvoid GR_GL_FUNCTION_TYPE noOpGLPopGroupMarker();

#endif
