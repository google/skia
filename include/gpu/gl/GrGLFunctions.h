
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFunctions_DEFINED
#define GrGLFunctions_DEFINED

#include "GrGLConfig.h"


extern "C" {

////////////////////////////////////////////////////////////////////////////////

/**
 * Classifies GL contexts by which standard they implement (currently as Desktop
 * vs. ES).
 */
enum GrGLStandard {
    kNone_GrGLStandard,
    kGL_GrGLStandard,
    kGLES_GrGLStandard,
};
static const int kGrGLStandardCnt = 3;

///////////////////////////////////////////////////////////////////////////////

/**
 * Declares typedefs for all the GL functions used in GrGLInterface
 */

typedef unsigned int GrGLenum;
typedef unsigned char GrGLboolean;
typedef unsigned int GrGLbitfield;
typedef signed char GrGLbyte;
typedef char GrGLchar;
typedef short GrGLshort;
typedef int GrGLint;
typedef int GrGLsizei;
typedef int64_t GrGLint64;
typedef unsigned char GrGLubyte;
typedef unsigned short GrGLushort;
typedef unsigned int GrGLuint;
typedef uint64_t GrGLuint64;
typedef float GrGLfloat;
typedef float GrGLclampf;
typedef double GrGLdouble;
typedef double GrGLclampd;
typedef void GrGLvoid;
#ifndef SK_IGNORE_64BIT_OPENGL_CHANGES
#ifdef _WIN64
typedef signed long long int GrGLintptr;
typedef signed long long int GrGLsizeiptr;
#else
typedef signed long int GrGLintptr;
typedef signed long int GrGLsizeiptr;
#endif
#else
typedef signed long int GrGLintptr;
typedef signed long int GrGLsizeiptr;
#endif

typedef void (GR_GL_FUNCTION_TYPE* GRGLDEBUGPROC)(GrGLenum source,
                                                  GrGLenum type,
                                                  GrGLuint id,
                                                  GrGLenum severity,
                                                  GrGLsizei length,
                                                  const GrGLchar* message,
                                                  const void* userParam);

///////////////////////////////////////////////////////////////////////////////

typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLActiveTextureProc)(GrGLenum texture);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLAttachShaderProc)(GrGLuint program, GrGLuint shader);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBeginQueryProc)(GrGLenum target, GrGLuint id);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindAttribLocationProc)(GrGLuint program, GrGLuint index, const char* name);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindBufferProc)(GrGLenum target, GrGLuint buffer);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindFramebufferProc)(GrGLenum target, GrGLuint framebuffer);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindRenderbufferProc)(GrGLenum target, GrGLuint renderbuffer);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindTextureProc)(GrGLenum target, GrGLuint texture);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindFragDataLocationProc)(GrGLuint program, GrGLuint colorNumber, const GrGLchar* name);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindFragDataLocationIndexedProc)(GrGLuint program, GrGLuint colorNumber, GrGLuint index, const GrGLchar * name);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindVertexArrayProc)(GrGLuint array);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBlendBarrierProc)();
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBlendColorProc)(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBlendEquationProc)(GrGLenum mode);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBlendFuncProc)(GrGLenum sfactor, GrGLenum dfactor);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBlitFramebufferProc)(GrGLint srcX0, GrGLint srcY0, GrGLint srcX1, GrGLint srcY1, GrGLint dstX0, GrGLint dstY0, GrGLint dstX1, GrGLint dstY1, GrGLbitfield mask, GrGLenum filter);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBufferDataProc)(GrGLenum target, GrGLsizeiptr size, const GrGLvoid* data, GrGLenum usage);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBufferSubDataProc)(GrGLenum target, GrGLintptr offset, GrGLsizeiptr size, const GrGLvoid* data);
typedef GrGLenum (GR_GL_FUNCTION_TYPE* GrGLCheckFramebufferStatusProc)(GrGLenum target);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLClearProc)(GrGLbitfield mask);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLClearColorProc)(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLClearStencilProc)(GrGLint s);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLClientActiveTextureProc)(GrGLenum texture);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLColorMaskProc)(GrGLboolean red, GrGLboolean green, GrGLboolean blue, GrGLboolean alpha);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCompileShaderProc)(GrGLuint shader);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCompressedTexImage2DProc)(GrGLenum target, GrGLint level, GrGLenum internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLsizei imageSize, const GrGLvoid* data);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCompressedTexSubImage2DProc)(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLsizei imageSize, const GrGLvoid* data);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCopyTexSubImage2DProc)(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCopyTextureCHROMIUMProc)(GrGLenum target, GrGLenum src, GrGLenum dst, GrGLint level, GrGLint format, GrGLenum type);
typedef GrGLuint (GR_GL_FUNCTION_TYPE* GrGLCreateProgramProc)(void);
typedef GrGLuint (GR_GL_FUNCTION_TYPE* GrGLCreateShaderProc)(GrGLenum type);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCullFaceProc)(GrGLenum mode);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDeleteBuffersProc)(GrGLsizei n, const GrGLuint* buffers);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDeleteFramebuffersProc)(GrGLsizei n, const GrGLuint *framebuffers);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDeleteProgramProc)(GrGLuint program);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDeleteQueriesProc)(GrGLsizei n, const GrGLuint *ids);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDeleteRenderbuffersProc)(GrGLsizei n, const GrGLuint *renderbuffers);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDeleteShaderProc)(GrGLuint shader);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDeleteTexturesProc)(GrGLsizei n, const GrGLuint* textures);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDeleteVertexArraysProc)(GrGLsizei n, const GrGLuint *arrays);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDepthMaskProc)(GrGLboolean flag);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDisableProc)(GrGLenum cap);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDisableVertexAttribArrayProc)(GrGLuint index);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDrawArraysProc)(GrGLenum mode, GrGLint first, GrGLsizei count);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDrawBufferProc)(GrGLenum mode);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDrawBuffersProc)(GrGLsizei n, const GrGLenum* bufs);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDrawElementsProc)(GrGLenum mode, GrGLsizei count, GrGLenum type, const GrGLvoid* indices);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLEnableProc)(GrGLenum cap);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLEnableVertexAttribArrayProc)(GrGLuint index);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLEndQueryProc)(GrGLenum target);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFinishProc)();
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFlushProc)();
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFlushMappedBufferRangeProc)(GrGLenum target, GrGLintptr offset, GrGLsizeiptr length);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFramebufferRenderbufferProc)(GrGLenum target, GrGLenum attachment, GrGLenum renderbuffertarget, GrGLuint renderbuffer);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFramebufferTexture2DProc)(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFramebufferTexture2DMultisampleProc)(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level, GrGLsizei samples);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFrontFaceProc)(GrGLenum mode);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenBuffersProc)(GrGLsizei n, GrGLuint* buffers);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenFramebuffersProc)(GrGLsizei n, GrGLuint *framebuffers);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenerateMipmapProc)(GrGLenum target);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenQueriesProc)(GrGLsizei n, GrGLuint *ids);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenRenderbuffersProc)(GrGLsizei n, GrGLuint *renderbuffers);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenTexturesProc)(GrGLsizei n, GrGLuint* textures);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenVertexArraysProc)(GrGLsizei n, GrGLuint *arrays);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetBufferParameterivProc)(GrGLenum target, GrGLenum pname, GrGLint* params);
typedef GrGLenum (GR_GL_FUNCTION_TYPE* GrGLGetErrorProc)();
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetFramebufferAttachmentParameterivProc)(GrGLenum target, GrGLenum attachment, GrGLenum pname, GrGLint* params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetIntegervProc)(GrGLenum pname, GrGLint* params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetProgramInfoLogProc)(GrGLuint program, GrGLsizei bufsize, GrGLsizei* length, char* infolog);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetProgramivProc)(GrGLuint program, GrGLenum pname, GrGLint* params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetQueryivProc)(GrGLenum GLtarget, GrGLenum pname, GrGLint *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetQueryObjecti64vProc)(GrGLuint id, GrGLenum pname, GrGLint64 *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetQueryObjectivProc)(GrGLuint id, GrGLenum pname, GrGLint *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetQueryObjectui64vProc)(GrGLuint id, GrGLenum pname, GrGLuint64 *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetQueryObjectuivProc)(GrGLuint id, GrGLenum pname, GrGLuint *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetRenderbufferParameterivProc)(GrGLenum target, GrGLenum pname, GrGLint* params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetShaderInfoLogProc)(GrGLuint shader, GrGLsizei bufsize, GrGLsizei* length, char* infolog);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetShaderivProc)(GrGLuint shader, GrGLenum pname, GrGLint* params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetShaderPrecisionFormatProc)(GrGLenum shadertype, GrGLenum precisiontype, GrGLint *range, GrGLint *precision);
typedef const GrGLubyte* (GR_GL_FUNCTION_TYPE* GrGLGetStringProc)(GrGLenum name);
typedef const GrGLubyte* (GR_GL_FUNCTION_TYPE* GrGLGetStringiProc)(GrGLenum name, GrGLuint index);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetTexLevelParameterivProc)(GrGLenum target, GrGLint level, GrGLenum pname, GrGLint* params);
typedef GrGLint (GR_GL_FUNCTION_TYPE* GrGLGetUniformLocationProc)(GrGLuint program, const char* name);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLInsertEventMarkerProc)(GrGLsizei length, const char* marker);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLInvalidateBufferDataProc)(GrGLuint buffer);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLInvalidateBufferSubDataProc)(GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr length);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLInvalidateFramebufferProc)(GrGLenum target, GrGLsizei numAttachments,  const GrGLenum *attachments);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLInvalidateSubFramebufferProc)(GrGLenum target, GrGLsizei numAttachments, const GrGLenum *attachments, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLInvalidateTexImageProc)(GrGLuint texture, GrGLint level);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLInvalidateTexSubImageProc)(GrGLuint texture, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint zoffset, GrGLsizei width, GrGLsizei height, GrGLsizei depth);
typedef GrGLboolean (GR_GL_FUNCTION_TYPE* GrGLIsTextureProc)(GrGLuint texture);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLLineWidthProc)(GrGLfloat width);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLLinkProgramProc)(GrGLuint program);
typedef GrGLvoid* (GR_GL_FUNCTION_TYPE* GrGLMapBufferProc)(GrGLenum target, GrGLenum access);
typedef GrGLvoid* (GR_GL_FUNCTION_TYPE* GrGLMapBufferRangeProc)(GrGLenum target, GrGLintptr offset, GrGLsizeiptr length, GrGLbitfield access);
typedef GrGLvoid* (GR_GL_FUNCTION_TYPE* GrGLMapBufferSubDataProc)(GrGLuint target, GrGLintptr offset, GrGLsizeiptr size, GrGLenum access);
typedef GrGLvoid* (GR_GL_FUNCTION_TYPE* GrGLMapTexSubImage2DProc)(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, GrGLenum access);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPixelStoreiProc)(GrGLenum pname, GrGLint param);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPopGroupMarkerProc)();
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPushGroupMarkerProc)(GrGLsizei length, const char* marker);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLQueryCounterProc)(GrGLuint id, GrGLenum target);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLRasterSamplesProc)(GrGLuint samples, GrGLboolean fixedsamplelocations);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLReadBufferProc)(GrGLenum src);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLReadPixelsProc)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, GrGLvoid* pixels);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLRenderbufferStorageProc)(GrGLenum target, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLRenderbufferStorageMultisampleProc)(GrGLenum target, GrGLsizei samples, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLResolveMultisampleFramebufferProc)();
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLScissorProc)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindUniformLocation)(GrGLuint program, GrGLint location, const char* name);

#if GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLShaderSourceProc)(GrGLuint shader, GrGLsizei count, const char* const * str, const GrGLint* length);
#else
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLShaderSourceProc)(GrGLuint shader, GrGLsizei count, const char** str, const GrGLint* length);
#endif
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilFuncProc)(GrGLenum func, GrGLint ref, GrGLuint mask);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilFuncSeparateProc)(GrGLenum face, GrGLenum func, GrGLint ref, GrGLuint mask);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilMaskProc)(GrGLuint mask);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilMaskSeparateProc)(GrGLenum face, GrGLuint mask);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilOpProc)(GrGLenum fail, GrGLenum zfail, GrGLenum zpass);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilOpSeparateProc)(GrGLenum face, GrGLenum fail, GrGLenum zfail, GrGLenum zpass);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTexImage2DProc)(GrGLenum target, GrGLint level, GrGLint internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid* pixels);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTexParameteriProc)(GrGLenum target, GrGLenum pname, GrGLint param);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTexParameterivProc)(GrGLenum target, GrGLenum pname, const GrGLint* params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTexStorage2DProc)(GrGLenum target, GrGLsizei levels, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDiscardFramebufferProc)(GrGLenum target, GrGLsizei numAttachments, const GrGLenum* attachments);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTexSubImage2DProc)(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, const GrGLvoid* pixels);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTextureBarrierProc)();
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform1fProc)(GrGLint location, GrGLfloat v0);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform1iProc)(GrGLint location, GrGLint v0);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform1fvProc)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform1ivProc)(GrGLint location, GrGLsizei count, const GrGLint* v);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform2fProc)(GrGLint location, GrGLfloat v0, GrGLfloat v1);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform2iProc)(GrGLint location, GrGLint v0, GrGLint v1);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform2fvProc)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform2ivProc)(GrGLint location, GrGLsizei count, const GrGLint* v);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform3fProc)(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform3iProc)(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform3fvProc)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform3ivProc)(GrGLint location, GrGLsizei count, const GrGLint* v);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform4fProc)(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2, GrGLfloat v3);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform4iProc)(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2, GrGLint v3);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform4fvProc)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniform4ivProc)(GrGLint location, GrGLsizei count, const GrGLint* v);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniformMatrix2fvProc)(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniformMatrix3fvProc)(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniformMatrix4fvProc)(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value);
typedef GrGLboolean (GR_GL_FUNCTION_TYPE* GrGLUnmapBufferProc)(GrGLenum target);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUnmapBufferSubDataProc)(const GrGLvoid* mem);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUnmapTexSubImage2DProc)(const GrGLvoid* mem);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUseProgramProc)(GrGLuint program);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexAttrib1fProc)(GrGLuint indx, const GrGLfloat value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexAttrib2fvProc)(GrGLuint indx, const GrGLfloat* values);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexAttrib3fvProc)(GrGLuint indx, const GrGLfloat* values);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexAttrib4fvProc)(GrGLuint indx, const GrGLfloat* values);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexAttribPointerProc)(GrGLuint indx, GrGLint size, GrGLenum type, GrGLboolean normalized, GrGLsizei stride, const GrGLvoid* ptr);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLViewportProc)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);

/* GL_NV_path_rendering */
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLMatrixLoadfProc)(GrGLenum matrixMode, const GrGLfloat* m);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLMatrixLoadIdentityProc)(GrGLenum);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathCommandsProc)(GrGLuint path, GrGLsizei numCommands, const GrGLubyte *commands, GrGLsizei numCoords, GrGLenum coordType, const GrGLvoid *coords);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathParameteriProc)(GrGLuint path, GrGLenum pname, GrGLint value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathParameterfProc)(GrGLuint path, GrGLenum pname, GrGLfloat value);
typedef GrGLuint (GR_GL_FUNCTION_TYPE* GrGLGenPathsProc)(GrGLsizei range);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDeletePathsProc)(GrGLuint path, GrGLsizei range);
typedef GrGLboolean (GR_GL_FUNCTION_TYPE* GrGLIsPathProc)(GrGLuint path);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathStencilFuncProc)(GrGLenum func, GrGLint ref, GrGLuint mask);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilFillPathProc)(GrGLuint path, GrGLenum fillMode, GrGLuint mask);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilStrokePathProc)(GrGLuint path, GrGLint reference, GrGLuint mask);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilFillPathInstancedProc)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum transformType, const GrGLfloat *transformValues);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilStrokePathInstancedProc)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum transformType, const GrGLfloat *transformValues);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCoverFillPathProc)(GrGLuint path, GrGLenum coverMode);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCoverStrokePathProc)(GrGLuint name, GrGLenum coverMode);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCoverFillPathInstancedProc)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat *transformValues);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCoverStrokePathInstancedProc)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat* transformValues);
// NV_path_rendering v1.2
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilThenCoverFillPathProc)(GrGLuint path, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilThenCoverStrokePathProc)(GrGLuint path, GrGLint reference, GrGLuint mask, GrGLenum coverMode);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilThenCoverFillPathInstancedProc)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat *transformValues);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilThenCoverStrokePathInstancedProc)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat *transformValues);
// NV_path_rendering v1.3
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramPathFragmentInputGenProc)(GrGLuint program, GrGLint location, GrGLenum genMode, GrGLint components,const GrGLfloat *coeffs);
// CHROMIUM_path_rendering
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindFragmentInputLocationProc)(GrGLuint program, GrGLint location, const GrGLchar* name);

/* ARB_program_interface_query */
typedef GrGLint (GR_GL_FUNCTION_TYPE* GrGLGetProgramResourceLocationProc)(GrGLuint program, GrGLenum programInterface, const GrGLchar *name);

/* GL_NV_framebuffer_mixed_samples */
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCoverageModulationProc)(GrGLenum components);

/* ARB_draw_instanced */
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDrawArraysInstancedProc)(GrGLenum mode, GrGLint first, GrGLsizei count, GrGLsizei primcount);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDrawElementsInstancedProc)(GrGLenum mode, GrGLsizei count, GrGLenum type, const GrGLvoid *indices, GrGLsizei primcount);

/* ARB_instanced_arrays */
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexAttribDivisorProc)(GrGLuint index, GrGLuint divisor);

/* NV_bindless_texture */
typedef GrGLuint64 (GR_GL_FUNCTION_TYPE* GrGLGetTextureHandleProc)(GrGLuint texture);
typedef GrGLuint64 (GR_GL_FUNCTION_TYPE* GrGLGetTextureSamplerHandleProc)(GrGLuint texture, GrGLuint sampler);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLMakeTextureHandleResidentProc)(GrGLuint64 handle);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLMakeTextureHandleNonResidentProc)(GrGLuint64 handle);
typedef GrGLuint64 (GR_GL_FUNCTION_TYPE* GrGLGetImageHandleProc)(GrGLuint texture, GrGLint level, GrGLboolean layered, GrGLint layer, GrGLint format);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLMakeImageHandleResidentProc)(GrGLuint64 handle, GrGLenum access);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLMakeImageHandleNonResidentProc)(GrGLuint64 handle);
typedef GrGLboolean (GR_GL_FUNCTION_TYPE* GrGLIsTextureHandleResidentProc)(GrGLuint64 handle);
typedef GrGLboolean (GR_GL_FUNCTION_TYPE* GrGLIsImageHandleResidentProc)(GrGLuint64 handle);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniformHandleui64Proc)(GrGLint location, GrGLuint64 v0);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUniformHandleui64vProc)(GrGLint location, GrGLsizei count, const GrGLuint64 *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniformHandleui64Proc)(GrGLuint program, GrGLint location, GrGLuint64 v0);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniformHandleui64vProc)(GrGLuint program, GrGLint location, GrGLsizei count, const GrGLuint64 *value);

/* EXT_direct_state_access */
// (In the future some of these methods may be omitted)
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTextureParameteriProc)(GrGLuint texture, GrGLenum target, GrGLenum pname, GrGLint param);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTextureParameterivProc)(GrGLuint texture, GrGLenum target, GrGLenum pname, const GrGLint *param);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTextureParameterfProc)(GrGLuint texture, GrGLenum target, GrGLenum pname, float param);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTextureParameterfvProc)(GrGLuint texture, GrGLenum target, GrGLenum pname, const float *param);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTextureImage1DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint GrGLinternalformat, GrGLsizei width, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid *pixels);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTextureImage2DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint GrGLinternalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid *pixels);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTextureSubImage1DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLsizei width, GrGLenum format, GrGLenum type, const GrGLvoid *pixels);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTextureSubImage2DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, const GrGLvoid *pixels);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCopyTextureImage1DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum GrGLinternalformat, GrGLint x, GrGLint y, GrGLsizei width, GrGLint border);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCopyTextureImage2DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum GrGLinternalformat, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height, GrGLint border);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCopyTextureSubImage1DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint x, GrGLint y, GrGLsizei width);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCopyTextureSubImage2DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetTextureImageProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum format, GrGLenum type, GrGLvoid *pixels);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetTextureParameterfvProc)(GrGLuint texture, GrGLenum target, GrGLenum pname, float *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetTextureParameterivProc)(GrGLuint texture, GrGLenum target, GrGLenum pname, GrGLint *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetTextureLevelParameterfvProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum pname, float *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetTextureLevelParameterivProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum pname, GrGLint *params);
// OpenGL 1.2
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTextureImage3DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint GrGLinternalformat, GrGLsizei width, GrGLsizei height, GrGLsizei depth, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid *pixels);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTextureSubImage3DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint zoffset, GrGLsizei width, GrGLsizei height, GrGLsizei depth, GrGLenum format, GrGLenum type, const GrGLvoid *pixels);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCopyTextureSubImage3DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint zoffset, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCompressedTextureImage3DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum GrGLinternalformat, GrGLsizei width, GrGLsizei height, GrGLsizei depth, GrGLint border, GrGLsizei imageSize, const GrGLvoid *data);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCompressedTextureImage2DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum GrGLinternalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLsizei imageSize, const GrGLvoid *data);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCompressedTextureImage1DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum GrGLinternalformat, GrGLsizei width, GrGLint border, GrGLsizei imageSize, const GrGLvoid *data);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCompressedTextureSubImage3DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint zoffset, GrGLsizei width, GrGLsizei height, GrGLsizei depth, GrGLenum format, GrGLsizei imageSize, const GrGLvoid *data);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCompressedTextureSubImage2DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLsizei imageSize, const GrGLvoid *data);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCompressedTextureSubImage1DProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLsizei width, GrGLenum format, GrGLsizei imageSize, const GrGLvoid *data);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetCompressedTextureImageProc)(GrGLuint texture, GrGLenum target, GrGLint level, GrGLvoid *img);
// OpenGL 1.5
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLNamedBufferDataProc)(GrGLuint buffer, GrGLsizeiptr size, const GrGLvoid *data, GrGLenum usage);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLNamedBufferSubDataProc)(GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr size, const GrGLvoid *data);
typedef GrGLvoid* (GR_GL_FUNCTION_TYPE* GrGLMapNamedBufferProc)(GrGLuint buffer, GrGLenum access);
typedef GrGLboolean (GR_GL_FUNCTION_TYPE* GrGLUnmapNamedBufferProc)(GrGLuint buffer);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetNamedBufferParameterivProc)(GrGLuint buffer, GrGLenum pname, GrGLint *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetNamedBufferPointervProc)(GrGLuint buffer, GrGLenum pname, GrGLvoid* *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetNamedBufferSubDataProc)(GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr size, GrGLvoid *data);
// OpenGL 2.0
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform1fProc)(GrGLuint program, GrGLint location, float v0);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform2fProc)(GrGLuint program, GrGLint location, float v0, float v1);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform3fProc)(GrGLuint program, GrGLint location, float v0, float v1, float v2);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform4fProc)(GrGLuint program, GrGLint location, float v0, float v1, float v2, float v3);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform1iProc)(GrGLuint program, GrGLint location, GrGLint v0);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform2iProc)(GrGLuint program, GrGLint location, GrGLint v0, GrGLint v1);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform3iProc)(GrGLuint program, GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform4iProc)(GrGLuint program, GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2, GrGLint v3);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform1fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, const float *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform2fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, const float *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform3fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, const float *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform4fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, const float *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform1ivProc)(GrGLuint program, GrGLint location, GrGLsizei count, const GrGLint *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform2ivProc)(GrGLuint program, GrGLint location, GrGLsizei count, const GrGLint *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform3ivProc)(GrGLuint program, GrGLint location, GrGLsizei count, const GrGLint *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniform4ivProc)(GrGLuint program, GrGLint location, GrGLsizei count, const GrGLint *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniformMatrix2fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniformMatrix3fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniformMatrix4fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value);
// OpenGL 2.1
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniformMatrix2x3fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniformMatrix3x2fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniformMatrix2x4fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniformMatrix4x2fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniformMatrix3x4fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLProgramUniformMatrix4x3fvProc)(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value);
// OpenGL 3.0
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLNamedRenderbufferStorageProc)(GrGLuint renderbuffer, GrGLenum GrGLinternalformat, GrGLsizei width, GrGLsizei height);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetNamedRenderbufferParameterivProc)(GrGLuint renderbuffer, GrGLenum pname, GrGLint *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLNamedRenderbufferStorageMultisampleProc)(GrGLuint renderbuffer, GrGLsizei samples, GrGLenum GrGLinternalformat, GrGLsizei width, GrGLsizei height);
typedef GrGLenum (GR_GL_FUNCTION_TYPE* GrGLCheckNamedFramebufferStatusProc)(GrGLuint framebuffer, GrGLenum target);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLNamedFramebufferTexture1DProc)(GrGLuint framebuffer, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLNamedFramebufferTexture2DProc)(GrGLuint framebuffer, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLNamedFramebufferTexture3DProc)(GrGLuint framebuffer, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level, GrGLint zoffset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLNamedFramebufferRenderbufferProc)(GrGLuint framebuffer, GrGLenum attachment, GrGLenum renderbuffertarget, GrGLuint renderbuffer);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetNamedFramebufferAttachmentParameterivProc)(GrGLuint framebuffer, GrGLenum attachment, GrGLenum pname, GrGLint *params);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenerateTextureMipmapProc)(GrGLuint texture, GrGLenum target);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFramebufferDrawBufferProc)(GrGLuint framebuffer, GrGLenum mode);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFramebufferDrawBuffersProc)(GrGLuint framebuffer, GrGLsizei n, const GrGLenum *bufs);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFramebufferReadBufferProc)(GrGLuint framebuffer, GrGLenum mode);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetFramebufferParameterivProc)(GrGLuint framebuffer, GrGLenum pname, GrGLint *param);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLNamedCopyBufferSubDataProc)(GrGLuint readBuffer, GrGLuint writeBuffer, GrGLintptr readOffset, GrGLintptr writeOffset, GrGLsizeiptr size);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexArrayVertexOffsetProc)(GrGLuint vaobj, GrGLuint buffer, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexArrayColorOffsetProc)(GrGLuint vaobj, GrGLuint buffer, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexArrayEdgeFlagOffsetProc)(GrGLuint vaobj, GrGLuint buffer, GrGLsizei stride, GrGLintptr offset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexArrayIndexOffsetProc)(GrGLuint vaobj, GrGLuint buffer, GrGLenum type, GrGLsizei stride, GrGLintptr offset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexArrayNormalOffsetProc)(GrGLuint vaobj, GrGLuint buffer, GrGLenum type, GrGLsizei stride, GrGLintptr offset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexArrayTexCoordOffsetProc)(GrGLuint vaobj, GrGLuint buffer, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexArrayMultiTexCoordOffsetProc)(GrGLuint vaobj, GrGLuint buffer, GrGLenum texunit, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexArrayFogCoordOffsetProc)(GrGLuint vaobj, GrGLuint buffer, GrGLenum type, GrGLsizei stride, GrGLintptr offset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexArraySecondaryColorOffsetProc)(GrGLuint vaobj, GrGLuint buffer, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexArrayVertexAttribOffsetProc)(GrGLuint vaobj, GrGLuint buffer, GrGLuint index, GrGLint size, GrGLenum type, GrGLboolean normalized, GrGLsizei stride, GrGLintptr offset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexArrayVertexAttribIOffsetProc)(GrGLuint vaobj, GrGLuint buffer, GrGLuint index, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLEnableVertexArrayProc)(GrGLuint vaobj, GrGLenum array);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDisableVertexArrayProc)(GrGLuint vaobj, GrGLenum array);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLEnableVertexArrayAttribProc)(GrGLuint vaobj, GrGLuint index);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDisableVertexArrayAttribProc)(GrGLuint vaobj, GrGLuint index);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetVertexArrayIntegervProc)(GrGLuint vaobj, GrGLenum pname, GrGLint *param);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetVertexArrayPointervProc)(GrGLuint vaobj, GrGLenum pname, GrGLvoid **param);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetVertexArrayIntegeri_vProc)(GrGLuint vaobj, GrGLuint index, GrGLenum pname, GrGLint *param);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetVertexArrayPointeri_vProc)(GrGLuint vaobj, GrGLuint index, GrGLenum pname, GrGLvoid **param);
typedef GrGLvoid* (GR_GL_FUNCTION_TYPE* GrGLMapNamedBufferRangeProc)(GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr length, GrGLbitfield access);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFlushMappedNamedBufferRangeProc)(GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr length);

/* KHR_debug */
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDebugMessageControlProc)(GrGLenum source, GrGLenum type, GrGLenum severity, GrGLsizei count, const GrGLuint* ids, GrGLboolean enabled);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDebugMessageInsertProc)(GrGLenum source, GrGLenum type, GrGLuint id, GrGLenum severity, GrGLsizei length,  const GrGLchar* buf);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDebugMessageCallbackProc)(GRGLDEBUGPROC callback, const GrGLvoid* userParam);
typedef GrGLuint (GR_GL_FUNCTION_TYPE* GrGLGetDebugMessageLogProc)(GrGLuint count, GrGLsizei bufSize, GrGLenum* sources, GrGLenum* types, GrGLuint* ids, GrGLenum* severities, GrGLsizei* lengths,  GrGLchar* messageLog);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPushDebugGroupProc)(GrGLenum source, GrGLuint id, GrGLsizei length,  const GrGLchar * message);
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPopDebugGroupProc)();
typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLObjectLabelProc)(GrGLenum identifier, GrGLuint name, GrGLsizei length, const GrGLchar *label);

}  // extern "C"

#endif
