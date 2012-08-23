
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFunctions_DEFINED
#define GrGLFunctions_DEFINED

#include "GrGLConfig.h"

/**
 * Declares typedefs for all the GL functions used in GrGLInterface
 */

///////////////////////////////////////////////////////////////////////////////

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
typedef long GrGLintptr;
typedef long GrGLsizeiptr;

///////////////////////////////////////////////////////////////////////////////

extern "C" {
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLActiveTextureProc)(GrGLenum texture);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLAttachShaderProc)(GrGLuint program, GrGLuint shader);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBeginQueryProc)(GrGLenum target, GrGLuint id);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindAttribLocationProc)(GrGLuint program, GrGLuint index, const char* name);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindBufferProc)(GrGLenum target, GrGLuint buffer);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindFramebufferProc)(GrGLenum target, GrGLuint framebuffer);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindRenderbufferProc)(GrGLenum target, GrGLuint renderbuffer);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindTextureProc)(GrGLenum target, GrGLuint texture);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBlendColorProc)(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindFragDataLocationProc)(GrGLuint program, GrGLuint colorNumber, const GrGLchar* name);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBindFragDataLocationIndexedProc)(GrGLuint program, GrGLuint colorNumber, GrGLuint index, const GrGLchar * name);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBlendFuncProc)(GrGLenum sfactor, GrGLenum dfactor);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBlitFramebufferProc)(GrGLint srcX0, GrGLint srcY0, GrGLint srcX1, GrGLint srcY1, GrGLint dstX0, GrGLint dstY0, GrGLint dstX1, GrGLint dstY1, GrGLbitfield mask, GrGLenum filter);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBufferDataProc)(GrGLenum target, GrGLsizeiptr size, const GrGLvoid* data, GrGLenum usage);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLBufferSubDataProc)(GrGLenum target, GrGLintptr offset, GrGLsizeiptr size, const GrGLvoid* data);
    typedef GrGLenum (GR_GL_FUNCTION_TYPE* GrGLCheckFramebufferStatusProc)(GrGLenum target);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLClearProc)(GrGLbitfield mask);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLClearColorProc)(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLClearStencilProc)(GrGLint s);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLColorMaskProc)(GrGLboolean red, GrGLboolean green, GrGLboolean blue, GrGLboolean alpha);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCompileShaderProc)(GrGLuint shader);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCompressedTexImage2DProc)(GrGLenum target, GrGLint level, GrGLenum internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLsizei imageSize, const GrGLvoid* data);
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
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFramebufferRenderbufferProc)(GrGLenum target, GrGLenum attachment, GrGLenum renderbuffertarget, GrGLuint renderbuffer);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFramebufferTexture2DProc)(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLFrontFaceProc)(GrGLenum mode);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenBuffersProc)(GrGLsizei n, GrGLuint* buffers);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenFramebuffersProc)(GrGLsizei n, GrGLuint *framebuffers);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenQueriesProc)(GrGLsizei n, GrGLuint *ids);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenRenderbuffersProc)(GrGLsizei n, GrGLuint *renderbuffers);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGenTexturesProc)(GrGLsizei n, GrGLuint* textures);
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
    typedef const GrGLubyte* (GR_GL_FUNCTION_TYPE* GrGLGetStringProc)(GrGLenum name);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetTexLevelParameterivProc)(GrGLenum target, GrGLint level, GrGLenum pname, GrGLint* params);
    typedef GrGLint (GR_GL_FUNCTION_TYPE* GrGLGetUniformLocationProc)(GrGLuint program, const char* name);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLLineWidthProc)(GrGLfloat width);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLLinkProgramProc)(GrGLuint program);
    typedef GrGLvoid* (GR_GL_FUNCTION_TYPE* GrGLMapBufferProc)(GrGLenum target, GrGLenum access);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPixelStoreiProc)(GrGLenum pname, GrGLint param);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLQueryCounterProc)(GrGLuint id, GrGLenum target);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLReadBufferProc)(GrGLenum src);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLReadPixelsProc)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, GrGLvoid* pixels);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLRenderbufferStorageProc)(GrGLenum target, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLRenderbufferStorageMultisampleProc)(GrGLenum target, GrGLsizei samples, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLRenderbufferStorageMultisampleCoverageProc)(GrGLenum target, GrGLsizei coverageSamples, GrGLsizei colorSamples, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLResolveMultisampleFramebufferProc)();
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLScissorProc)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLShaderSourceProc)(GrGLuint shader, GrGLsizei count, const char** str, const GrGLint* length);
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
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTexSubImage2DProc)(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, const GrGLvoid* pixels);
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
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLUseProgramProc)(GrGLuint program);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexAttrib4fvProc)(GrGLuint indx, const GrGLfloat* values);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLVertexAttribPointerProc)(GrGLuint indx, GrGLint size, GrGLenum type, GrGLboolean normalized, GrGLsizei stride, const GrGLvoid* ptr);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLViewportProc)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);

    // Experimental: Functions for GL_NV_path_rendering. These will be
    // alphabetized with the above functions once this is fully supported
    // (and functions we are unlikely to use will possibly be omitted).
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLMatrixModeProc)(GrGLenum);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLLoadIdentityProc)();
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLLoadMatrixfProc)(const GrGLfloat* m);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathCommandsProc)(GrGLuint path, GrGLsizei numCommands, const GrGLubyte *commands, GrGLsizei numCoords, GrGLenum coordType, const GrGLvoid *coords);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathCoordsProc)(GrGLuint path, GrGLsizei numCoords, GrGLenum coordType, const GrGLvoid *coords);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathSubCommandsProc)(GrGLuint path, GrGLsizei commandStart, GrGLsizei commandsToDelete, GrGLsizei numCommands, const GrGLubyte *commands, GrGLsizei numCoords, GrGLenum coordType, const GrGLvoid *coords);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathSubCoordsProc)(GrGLuint path, GrGLsizei coordStart, GrGLsizei numCoords, GrGLenum coordType, const GrGLvoid *coords);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathStringProc)(GrGLuint path, GrGLenum format, GrGLsizei length, const GrGLvoid *pathString);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathGlyphsProc)(GrGLuint firstPathName, GrGLenum fontTarget, const GrGLvoid *fontName, GrGLbitfield fontStyle, GrGLsizei numGlyphs, GrGLenum type, const GrGLvoid *charcodes, GrGLenum handleMissingGlyphs, GrGLuint pathParameterTemplate, GrGLfloat emScale);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathGlyphRangeProc)(GrGLuint firstPathName, GrGLenum fontTarget, const GrGLvoid *fontName, GrGLbitfield fontStyle, GrGLuint firstGlyph, GrGLsizei numGlyphs, GrGLenum handleMissingGlyphs, GrGLuint pathParameterTemplate, GrGLfloat emScale);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLWeightPathsProc)(GrGLuint resultPath, GrGLsizei numPaths, const GrGLuint paths[], const GrGLfloat weights[]);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCopyPathProc)(GrGLuint resultPath, GrGLuint srcPath);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLInterpolatePathsProc)(GrGLuint resultPath, GrGLuint pathA, GrGLuint pathB, GrGLfloat weight);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLTransformPathProc)(GrGLuint resultPath, GrGLuint srcPath, GrGLenum transformType, const GrGLfloat *transformValues);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathParameterivProc)(GrGLuint path, GrGLenum pname, const GrGLint *value);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathParameteriProc)(GrGLuint path, GrGLenum pname, GrGLint value);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathParameterfvProc)(GrGLuint path, GrGLenum pname, const GrGLfloat *value);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathParameterfProc)(GrGLuint path, GrGLenum pname, GrGLfloat value);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathDashArrayProc)(GrGLuint path, GrGLsizei dashCount, const GrGLfloat *dashArray);
    typedef GrGLuint (GR_GL_FUNCTION_TYPE* GrGLGenPathsProc)(GrGLsizei range);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLDeletePathsProc)(GrGLuint path, GrGLsizei range);
    typedef GrGLboolean (GR_GL_FUNCTION_TYPE* GrGLIsPathProc)(GrGLuint path);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathStencilFuncProc)(GrGLenum func, GrGLint ref, GrGLuint mask);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathStencilDepthOffsetProc)(GrGLfloat factor, GrGLfloat units);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilFillPathProc)(GrGLuint path, GrGLenum fillMode, GrGLuint mask);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilStrokePathProc)(GrGLuint path, GrGLint reference, GrGLuint mask);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilFillPathInstancedProc)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum transformType, const GrGLfloat *transformValues);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLStencilStrokePathInstancedProc)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum transformType, const GrGLfloat *transformValues);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathCoverDepthFuncProc)(GrGLenum zfunc);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathColorGenProc)(GrGLenum color, GrGLenum genMode, GrGLenum colorFormat, const GrGLfloat *coeffs);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathTexGenProc)(GrGLenum texCoordSet, GrGLenum genMode, GrGLint components, const GrGLfloat *coeffs);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLPathFogGenProc)(GrGLenum genMode);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCoverFillPathProc)(GrGLuint path, GrGLenum coverMode);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCoverStrokePathProc)(GrGLuint name, GrGLenum coverMode);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCoverFillPathInstancedProc)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat *transformValues);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLCoverStrokePathInstancedProc)(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat* transformValues);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathParameterivProc)(GrGLuint name, GrGLenum param, GrGLint *value);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathParameterfvProc)(GrGLuint name, GrGLenum param, GrGLfloat *value);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathCommandsProc)(GrGLuint name, GrGLubyte *commands);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathCoordsProc)(GrGLuint name, GrGLfloat *coords);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathDashArrayProc)(GrGLuint name, GrGLfloat *dashArray);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathMetricsProc)(GrGLbitfield metricQueryMask, GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLsizei stride, GrGLfloat *metrics);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathMetricRangeProc)(GrGLbitfield metricQueryMask, GrGLuint fistPathName, GrGLsizei numPaths, GrGLsizei stride, GrGLfloat *metrics);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathSpacingProc)(GrGLenum pathListMode, GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLfloat advanceScale, GrGLfloat kerningScale, GrGLenum transformType, GrGLfloat *returnedSpacing);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathColorGenivProc)(GrGLenum color, GrGLenum pname, GrGLint *value);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathColorGenfvProc)(GrGLenum color, GrGLenum pname, GrGLfloat *value);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathTexGenivProc)(GrGLenum texCoordSet, GrGLenum pname, GrGLint *value);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GrGLGetPathTexGenfvProc)(GrGLenum texCoordSet, GrGLenum pname, GrGLfloat *value);
    typedef GrGLboolean (GR_GL_FUNCTION_TYPE* GrGLIsPointInFillPathProc)(GrGLuint path, GrGLuint mask, GrGLfloat x, GrGLfloat y);
    typedef GrGLboolean (GR_GL_FUNCTION_TYPE* GrGLIsPointInStrokePathProc)(GrGLuint path, GrGLfloat x, GrGLfloat y);
    typedef GrGLfloat (GR_GL_FUNCTION_TYPE* GrGLGetPathLengthProc)(GrGLuint path, GrGLsizei startSegment, GrGLsizei numSegments);
    typedef GrGLboolean (GR_GL_FUNCTION_TYPE* GrGLPointAlongPathProc)(GrGLuint path, GrGLsizei startSegment, GrGLsizei numSegments, GrGLfloat distance, GrGLfloat *x, GrGLfloat *y, GrGLfloat *tangentX, GrGLfloat *tangentY);
}  // extern "C"

#endif
