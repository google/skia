
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFunctions_DEFINED
#define GrGLFunctions_DEFINED

#include "../private/SkTLogic.h"
#include "GrGLTypes.h"

#include <cstring>

extern "C" {

///////////////////////////////////////////////////////////////////////////////

using GrGLActiveTextureProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum texture);
using GrGLAttachShaderProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLuint shader);
using GrGLBeginQueryProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLuint id);
using GrGLBindAttribLocationProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLuint index, const char* name);
using GrGLBindBufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLuint buffer);
using GrGLBindFramebufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLuint framebuffer);
using GrGLBindRenderbufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLuint renderbuffer);
using GrGLBindTextureProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLuint texture);
using GrGLBindFragDataLocationProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLuint colorNumber, const GrGLchar* name);
using GrGLBindFragDataLocationIndexedProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLuint colorNumber, GrGLuint index, const GrGLchar* name);
using GrGLBindVertexArrayProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint array);
using GrGLBlendBarrierProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)();
using GrGLBlendColorProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha);
using GrGLBlendEquationProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode);
using GrGLBlendFuncProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum sfactor, GrGLenum dfactor);
using GrGLBlitFramebufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint srcX0, GrGLint srcY0, GrGLint srcX1, GrGLint srcY1, GrGLint dstX0, GrGLint dstY0, GrGLint dstX1, GrGLint dstY1, GrGLbitfield mask, GrGLenum filter);
using GrGLBufferDataProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLsizeiptr size, const GrGLvoid* data, GrGLenum usage);
using GrGLBufferSubDataProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLintptr offset, GrGLsizeiptr size, const GrGLvoid* data);
using GrGLCheckFramebufferStatusProc = GrGLenum (GR_GL_FUNCTION_TYPE*)(GrGLenum target);
using GrGLClearProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLbitfield mask);
using GrGLClearColorProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha);
using GrGLClearStencilProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint s);
using GrGLClearTexImageProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint texture, GrGLint level, GrGLenum format, GrGLenum type, const GrGLvoid* data);
using GrGLClearTexSubImageProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint texture, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint zoffset, GrGLsizei width, GrGLsizei height, GrGLsizei depth, GrGLenum format, GrGLenum type, const GrGLvoid* data);
using GrGLColorMaskProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLboolean red, GrGLboolean green, GrGLboolean blue, GrGLboolean alpha);
using GrGLCompileShaderProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint shader);
using GrGLCompressedTexImage2DProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLint level, GrGLenum internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLsizei imageSize, const GrGLvoid* data);
using GrGLCompressedTexSubImage2DProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLsizei imageSize, const GrGLvoid* data);
using GrGLCopyTexSubImage2DProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);
using GrGLCreateProgramProc = GrGLuint (GR_GL_FUNCTION_TYPE*)();
using GrGLCreateShaderProc = GrGLuint (GR_GL_FUNCTION_TYPE*)(GrGLenum type);
using GrGLCullFaceProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode);
using GrGLDeleteBuffersProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, const GrGLuint* buffers);
using GrGLDeleteFramebuffersProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, const GrGLuint* framebuffers);
using GrGLDeleteProgramProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program);
using GrGLDeleteQueriesProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, const GrGLuint* ids);
using GrGLDeleteRenderbuffersProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, const GrGLuint* renderbuffers);
using GrGLDeleteShaderProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint shader);
using GrGLDeleteTexturesProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, const GrGLuint* textures);
using GrGLDeleteVertexArraysProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, const GrGLuint* arrays);
using GrGLDepthMaskProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLboolean flag);
using GrGLDisableProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum cap);
using GrGLDisableVertexAttribArrayProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint index);
using GrGLDrawArraysProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode, GrGLint first, GrGLsizei count);
using GrGLDrawArraysInstancedProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode, GrGLint first, GrGLsizei count, GrGLsizei primcount);
using GrGLDrawArraysIndirectProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode, const GrGLvoid* indirect);
using GrGLDrawBufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode);
using GrGLDrawBuffersProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, const GrGLenum* bufs);
using GrGLDrawElementsProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode, GrGLsizei count, GrGLenum type, const GrGLvoid* indices);
using GrGLDrawElementsInstancedProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode, GrGLsizei count, GrGLenum type, const GrGLvoid* indices, GrGLsizei primcount);
using GrGLDrawElementsIndirectProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode, GrGLenum type, const GrGLvoid* indirect);
using GrGLDrawRangeElementsProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode, GrGLuint start, GrGLuint end, GrGLsizei count, GrGLenum type, const GrGLvoid* indices);
using GrGLEnableProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum cap);
using GrGLEnableVertexAttribArrayProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint index);
using GrGLEndQueryProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target);
using GrGLFinishProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)();
using GrGLFlushProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)();
using GrGLFlushMappedBufferRangeProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLintptr offset, GrGLsizeiptr length);
using GrGLFramebufferRenderbufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum attachment, GrGLenum renderbuffertarget, GrGLuint renderbuffer);
using GrGLFramebufferTexture2DProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level);
using GrGLFramebufferTexture2DMultisampleProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level, GrGLsizei samples);
using GrGLFrontFaceProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode);
using GrGLGenBuffersProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, GrGLuint* buffers);
using GrGLGenFramebuffersProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, GrGLuint* framebuffers);
using GrGLGenerateMipmapProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target);
using GrGLGenQueriesProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, GrGLuint* ids);
using GrGLGenRenderbuffersProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, GrGLuint* renderbuffers);
using GrGLGenTexturesProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, GrGLuint* textures);
using GrGLGenVertexArraysProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei n, GrGLuint* arrays);
using GrGLGetBufferParameterivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum pname, GrGLint* params);
using GrGLGetErrorProc = GrGLenum (GR_GL_FUNCTION_TYPE*)();
using GrGLGetFramebufferAttachmentParameterivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum attachment, GrGLenum pname, GrGLint* params);
using GrGLGetIntegervProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum pname, GrGLint* params);
using GrGLGetMultisamplefvProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum pname, GrGLuint index, GrGLfloat* val);
using GrGLGetProgramBinaryProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLsizei bufsize, GrGLsizei* length, GrGLenum* binaryFormat, void* binary);
using GrGLGetProgramInfoLogProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLsizei bufsize, GrGLsizei* length, char* infolog);
using GrGLGetProgramivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLenum pname, GrGLint* params);
using GrGLGetQueryivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum GLtarget, GrGLenum pname, GrGLint* params);
using GrGLGetQueryObjecti64vProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint id, GrGLenum pname, GrGLint64* params);
using GrGLGetQueryObjectivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint id, GrGLenum pname, GrGLint* params);
using GrGLGetQueryObjectui64vProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint id, GrGLenum pname, GrGLuint64* params);
using GrGLGetQueryObjectuivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint id, GrGLenum pname, GrGLuint* params);
using GrGLGetRenderbufferParameterivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum pname, GrGLint* params);
using GrGLGetShaderInfoLogProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint shader, GrGLsizei bufsize, GrGLsizei* length, char* infolog);
using GrGLGetShaderivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint shader, GrGLenum pname, GrGLint* params);
using GrGLGetShaderPrecisionFormatProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum shadertype, GrGLenum precisiontype, GrGLint* range, GrGLint* precision);
using GrGLGetStringProc = const GrGLubyte* (GR_GL_FUNCTION_TYPE*)(GrGLenum name);
using GrGLGetStringiProc = const GrGLubyte* (GR_GL_FUNCTION_TYPE*)(GrGLenum name, GrGLuint index);
using GrGLGetTexLevelParameterivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLint level, GrGLenum pname, GrGLint* params);
using GrGLGetUniformLocationProc = GrGLint (GR_GL_FUNCTION_TYPE*)(GrGLuint program, const char* name);
using GrGLInsertEventMarkerProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei length, const char* marker);
using GrGLInvalidateBufferDataProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint buffer);
using GrGLInvalidateBufferSubDataProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr length);
using GrGLInvalidateFramebufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLsizei numAttachments, const GrGLenum* attachments);
using GrGLInvalidateSubFramebufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLsizei numAttachments, const GrGLenum* attachments, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);
using GrGLInvalidateTexImageProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint texture, GrGLint level);
using GrGLInvalidateTexSubImageProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint texture, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint zoffset, GrGLsizei width, GrGLsizei height, GrGLsizei depth);
using GrGLIsTextureProc = GrGLboolean (GR_GL_FUNCTION_TYPE*)(GrGLuint texture);
using GrGLLineWidthProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLfloat width);
using GrGLLinkProgramProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program);
using GrGLMapBufferProc = GrGLvoid* (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum access);
using GrGLMapBufferRangeProc = GrGLvoid* (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLintptr offset, GrGLsizeiptr length, GrGLbitfield access);
using GrGLMapBufferSubDataProc = GrGLvoid* (GR_GL_FUNCTION_TYPE*)(GrGLuint target, GrGLintptr offset, GrGLsizeiptr size, GrGLenum access);
using GrGLMapTexSubImage2DProc = GrGLvoid* (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, GrGLenum access);
using GrGLPixelStoreiProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum pname, GrGLint param);
using GrGLPolygonModeProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum face, GrGLenum mode);
using GrGLPopGroupMarkerProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)();
using GrGLProgramBinaryProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLenum binaryFormat, void* binary, GrGLsizei length);
using GrGLProgramParameteriProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLenum pname, GrGLint value);
using GrGLPushGroupMarkerProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei length, const char* marker);
using GrGLQueryCounterProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint id, GrGLenum target);
using GrGLRasterSamplesProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint samples, GrGLboolean fixedsamplelocations);
using GrGLReadBufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum src);
using GrGLReadPixelsProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, GrGLvoid* pixels);
using GrGLRenderbufferStorageProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
using GrGLRenderbufferStorageMultisampleProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLsizei samples, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
using GrGLResolveMultisampleFramebufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)();
using GrGLScissorProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);
// GL_CHROMIUM_bind_uniform_location
using GrGLBindUniformLocationProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLint location, const char* name);

#if GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE
using GrGLShaderSourceProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint shader, GrGLsizei count, const char* const* str, const GrGLint* length);
#else
using GrGLShaderSourceProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint shader, GrGLsizei count, const char** str, const GrGLint* length);
#endif
using GrGLStencilFuncProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum func, GrGLint ref, GrGLuint mask);
using GrGLStencilFuncSeparateProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum face, GrGLenum func, GrGLint ref, GrGLuint mask);
using GrGLStencilMaskProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint mask);
using GrGLStencilMaskSeparateProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum face, GrGLuint mask);
using GrGLStencilOpProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum fail, GrGLenum zfail, GrGLenum zpass);
using GrGLStencilOpSeparateProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum face, GrGLenum fail, GrGLenum zfail, GrGLenum zpass);
using GrGLTexBufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum internalformat, GrGLuint buffer);
using GrGLTexBufferRangeProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum internalformat, GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr size);
using GrGLTexImage2DProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLint level, GrGLint internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid* pixels);
using GrGLTexParameteriProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum pname, GrGLint param);
using GrGLTexParameterivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum pname, const GrGLint* params);
using GrGLTexStorage2DProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLsizei levels, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
using GrGLDiscardFramebufferProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLsizei numAttachments, const GrGLenum* attachments);
using GrGLTexSubImage2DProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, const GrGLvoid* pixels);
using GrGLTextureBarrierProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)();
using GrGLUniform1fProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLfloat v0);
using GrGLUniform1iProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLint v0);
using GrGLUniform1fvProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
using GrGLUniform1ivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLsizei count, const GrGLint* v);
using GrGLUniform2fProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLfloat v0, GrGLfloat v1);
using GrGLUniform2iProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLint v0, GrGLint v1);
using GrGLUniform2fvProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
using GrGLUniform2ivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLsizei count, const GrGLint* v);
using GrGLUniform3fProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2);
using GrGLUniform3iProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2);
using GrGLUniform3fvProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
using GrGLUniform3ivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLsizei count, const GrGLint* v);
using GrGLUniform4fProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2, GrGLfloat v3);
using GrGLUniform4iProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2, GrGLint v3);
using GrGLUniform4fvProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
using GrGLUniform4ivProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLsizei count, const GrGLint* v);
using GrGLUniformMatrix2fvProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value);
using GrGLUniformMatrix3fvProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value);
using GrGLUniformMatrix4fvProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value);
using GrGLUnmapBufferProc = GrGLboolean (GR_GL_FUNCTION_TYPE*)(GrGLenum target);
using GrGLUnmapBufferSubDataProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(const GrGLvoid* mem);
using GrGLUnmapTexSubImage2DProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(const GrGLvoid* mem);
using GrGLUseProgramProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program);
using GrGLVertexAttrib1fProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint indx, const GrGLfloat value);
using GrGLVertexAttrib2fvProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint indx, const GrGLfloat* values);
using GrGLVertexAttrib3fvProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint indx, const GrGLfloat* values);
using GrGLVertexAttrib4fvProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint indx, const GrGLfloat* values);
using GrGLVertexAttribDivisorProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint index, GrGLuint divisor);
using GrGLVertexAttribIPointerProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint indx, GrGLint size, GrGLenum type, GrGLsizei stride, const GrGLvoid* ptr);
using GrGLVertexAttribPointerProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint indx, GrGLint size, GrGLenum type, GrGLboolean normalized, GrGLsizei stride, const GrGLvoid* ptr);
using GrGLViewportProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);

/* GL_NV_path_rendering */
using GrGLMatrixLoadfProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum matrixMode, const GrGLfloat* m);
using GrGLMatrixLoadIdentityProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum);
using GrGLPathCommandsProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint path, GrGLsizei numCommands, const GrGLubyte* commands, GrGLsizei numCoords, GrGLenum coordType, const GrGLvoid* coords);
using GrGLPathParameteriProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint path, GrGLenum pname, GrGLint value);
using GrGLPathParameterfProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint path, GrGLenum pname, GrGLfloat value);
using GrGLGenPathsProc = GrGLuint (GR_GL_FUNCTION_TYPE*)(GrGLsizei range);
using GrGLDeletePathsProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint path, GrGLsizei range);
using GrGLIsPathProc = GrGLboolean (GR_GL_FUNCTION_TYPE*)(GrGLuint path);
using GrGLPathStencilFuncProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum func, GrGLint ref, GrGLuint mask);
using GrGLStencilFillPathProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint path, GrGLenum fillMode, GrGLuint mask);
using GrGLStencilStrokePathProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint path, GrGLint reference, GrGLuint mask);
using GrGLStencilFillPathInstancedProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid* paths, GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum transformType, const GrGLfloat* transformValues);
using GrGLStencilStrokePathInstancedProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid* paths, GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum transformType, const GrGLfloat* transformValues);
using GrGLCoverFillPathProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint path, GrGLenum coverMode);
using GrGLCoverStrokePathProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint name, GrGLenum coverMode);
using GrGLCoverFillPathInstancedProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid* paths, GrGLuint pathBase, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat* transformValues);
using GrGLCoverStrokePathInstancedProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid* paths, GrGLuint pathBase, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat* transformValues);
// NV_path_rendering v1.2
using GrGLStencilThenCoverFillPathProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint path, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode);
using GrGLStencilThenCoverStrokePathProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint path, GrGLint reference, GrGLuint mask, GrGLenum coverMode);
using GrGLStencilThenCoverFillPathInstancedProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid* paths, GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat* transformValues);
using GrGLStencilThenCoverStrokePathInstancedProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid* paths, GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat* transformValues);
// NV_path_rendering v1.3
using GrGLProgramPathFragmentInputGenProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLint location, GrGLenum genMode, GrGLint components, const GrGLfloat* coeffs);
// CHROMIUM_path_rendering
using GrGLBindFragmentInputLocationProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLint location, const GrGLchar* name);

/* ARB_program_interface_query */
using GrGLGetProgramResourceLocationProc = GrGLint (GR_GL_FUNCTION_TYPE*)(GrGLuint program, GrGLenum programInterface, const GrGLchar* name);

/* GL_NV_framebuffer_mixed_samples */
using GrGLCoverageModulationProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum components);

/* EXT_multi_draw_indirect */
using GrGLMultiDrawArraysIndirectProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode, const GrGLvoid* indirect, GrGLsizei drawcount, GrGLsizei stride);
using GrGLMultiDrawElementsIndirectProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode, GrGLenum type, const GrGLvoid* indirect, GrGLsizei drawcount, GrGLsizei stride);

/* ARB_sample_shading */
using GrGLMinSampleShadingProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLfloat value);

/* ARB_sync */
using GrGLFenceSyncProc = GrGLsync (GR_GL_FUNCTION_TYPE*)(GrGLenum condition, GrGLbitfield flags);
using GrGLIsSyncProc = GrGLboolean (GR_GL_FUNCTION_TYPE*)(GrGLsync sync);
using GrGLClientWaitSyncProc = GrGLenum (GR_GL_FUNCTION_TYPE*)(GrGLsync sync, GrGLbitfield flags, GrGLuint64 timeout);
using GrGLWaitSyncProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsync sync, GrGLbitfield flags, GrGLuint64 timeout);
using GrGLDeleteSyncProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLsync sync);

/* ARB_internalformat_query */
using GrGLGetInternalformativProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum target, GrGLenum internalformat, GrGLenum pname, GrGLsizei bufSize, GrGLint* params);

/* KHR_debug */
using GrGLDebugMessageControlProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum source, GrGLenum type, GrGLenum severity, GrGLsizei count, const GrGLuint* ids, GrGLboolean enabled);
using GrGLDebugMessageInsertProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum source, GrGLenum type, GrGLuint id, GrGLenum severity, GrGLsizei length, const GrGLchar* buf);
using GrGLDebugMessageCallbackProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GRGLDEBUGPROC callback, const GrGLvoid* userParam);
using GrGLGetDebugMessageLogProc = GrGLuint (GR_GL_FUNCTION_TYPE*)(GrGLuint count, GrGLsizei bufSize, GrGLenum* sources, GrGLenum* types, GrGLuint* ids, GrGLenum* severities, GrGLsizei* lengths, GrGLchar* messageLog);
using GrGLPushDebugGroupProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum source, GrGLuint id, GrGLsizei length, const GrGLchar* message);
using GrGLPopDebugGroupProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)();
using GrGLObjectLabelProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum identifier, GrGLuint name, GrGLsizei length, const GrGLchar* label);

/** EXT_window_rectangles */
using GrGLWindowRectanglesProc = GrGLvoid (GR_GL_FUNCTION_TYPE*)(GrGLenum mode, GrGLsizei count, const GrGLint box[]);

/** EGL functions */
using GrEGLQueryStringProc = const char* (GR_GL_FUNCTION_TYPE*)(GrEGLDisplay dpy, GrEGLint name);
using GrEGLGetCurrentDisplayProc = GrEGLDisplay (GR_GL_FUNCTION_TYPE*)();
using GrEGLCreateImageProc = GrEGLImage (GR_GL_FUNCTION_TYPE*)(GrEGLDisplay dpy, GrEGLContext ctx, GrEGLenum target, GrEGLClientBuffer buffer, const GrEGLint* attrib_list);
using GrEGLDestroyImageProc = GrEGLBoolean (GR_GL_FUNCTION_TYPE*)(GrEGLDisplay dpy, GrEGLImage image);
}  // extern "C"

// This is a lighter-weight std::function, trying to reduce code size and compile time
// by only supporting the exact use cases we require.
template <typename T> class GrGLFunction;

template <typename R, typename... Args>
class GrGLFunction<R (*)(Args...)> {
public:
    using Fn = R GR_GL_FUNCTION_TYPE(Args...);
    // Construct empty.
    GrGLFunction() = default;
    GrGLFunction(std::nullptr_t) {}

    // Construct from a simple function pointer.
    GrGLFunction(Fn* fn_ptr) {
        if (fn_ptr) {
            static_assert(sizeof(fn_ptr) <= sizeof(fBuf), "fBuf is too small");
            memcpy(fBuf, &fn_ptr, sizeof(fn_ptr));
            fCall = [](const void* buf, Args... args) {
                return (*(Fn**)buf)(std::forward<Args>(args)...);
            };
        }
    }

    // Construct from a small closure.
    template <typename Closure>
    GrGLFunction(Closure closure) : GrGLFunction() {
        static_assert(sizeof(Closure) <= sizeof(fBuf), "fBuf is too small");
#if defined(__APPLE__)  // I am having serious trouble getting these to work with all STLs...
        static_assert(std::is_trivially_copyable<Closure>::value, "");
        static_assert(std::is_trivially_destructible<Closure>::value, "");
#endif

        memcpy(fBuf, &closure, sizeof(closure));
        fCall = [](const void* buf, Args... args) {
            auto closure = (const Closure*)buf;
            return (*closure)(args...);
        };
    }

    R operator()(Args... args) const {
        SkASSERT(fCall);
        return fCall(fBuf, std::forward<Args>(args)...);
    }

    explicit operator bool() const { return fCall != nullptr; }

    void reset() { fCall = nullptr; }

private:
    using Call = R(const void* buf, Args...);
    Call* fCall = nullptr;
    size_t fBuf[4];
};

#endif
