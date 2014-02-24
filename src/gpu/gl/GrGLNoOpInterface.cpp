/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLNoOpInterface.h"
#include "SkString.h"
#include "SkThread.h"

// the OpenGLES 2.0 spec says this must be >= 128
static const GrGLint kDefaultMaxVertexUniformVectors = 128;

// the OpenGLES 2.0 spec says this must be >=16
static const GrGLint kDefaultMaxFragmentUniformVectors = 16;

// the OpenGLES 2.0 spec says this must be >= 8
static const GrGLint kDefaultMaxVertexAttribs = 8;

// the OpenGLES 2.0 spec says this must be >= 8
static const GrGLint kDefaultMaxVaryingVectors = 8;

static const char* kExtensions[] = {
    "GL_ARB_framebuffer_object",
    "GL_ARB_blend_func_extended",
    "GL_ARB_timer_query",
    "GL_ARB_draw_buffers",
    "GL_ARB_occlusion_query",
    "GL_EXT_blend_color",
    "GL_EXT_stencil_wrap"
};

namespace {
const GrGLubyte* combined_extensions_string() {
    static SkString gExtString;
    static SkMutex gMutex;
    gMutex.acquire();
    if (0 == gExtString.size()) {
        for (size_t i = 0; i < GR_ARRAY_COUNT(kExtensions) - 1; ++i) {
            gExtString.append(kExtensions[i]);
            gExtString.append(" ");
        }
        gExtString.append(kExtensions[GR_ARRAY_COUNT(kExtensions) - 1]);
    }
    gMutex.release();
    return (const GrGLubyte*) gExtString.c_str();
}
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBlendColor(GrGLclampf red,
                                              GrGLclampf green,
                                              GrGLclampf blue,
                                              GrGLclampf alpha) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBindFragDataLocation(GrGLuint program,
                                                        GrGLuint colorNumber,
                                                        const GrGLchar* name) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBlendFunc(GrGLenum sfactor,
                                              GrGLenum dfactor) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBufferSubData(GrGLenum target,
                                                 GrGLintptr offset,
                                                 GrGLsizeiptr size,
                                                 const GrGLvoid* data) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLClear(GrGLbitfield mask) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLClearColor(GrGLclampf red,
                                              GrGLclampf green,
                                              GrGLclampf blue,
                                              GrGLclampf alpha) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLClearStencil(GrGLint s) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLColorMask(GrGLboolean red,
                                             GrGLboolean green,
                                             GrGLboolean blue,
                                             GrGLboolean alpha) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLCompileShader(GrGLuint shader) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLCompressedTexImage2D(GrGLenum target,
                                                        GrGLint level,
                                                        GrGLenum internalformat,
                                                        GrGLsizei width,
                                                        GrGLsizei height,
                                                        GrGLint border,
                                                        GrGLsizei imageSize,
                                                        const GrGLvoid* data) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLCopyTexSubImage2D(GrGLenum target,
                                                     GrGLint level,
                                                     GrGLint xoffset,
                                                     GrGLint yoffset,
                                                     GrGLint x,
                                                     GrGLint y,
                                                     GrGLsizei width,
                                                     GrGLsizei height) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLCullFace(GrGLenum mode) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDepthMask(GrGLboolean flag) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDisable(GrGLenum cap) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDisableVertexAttribArray(GrGLuint index) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDrawArrays(GrGLenum mode,
                                              GrGLint first,
                                              GrGLsizei count) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDrawBuffer(GrGLenum mode) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDrawBuffers(GrGLsizei n,
                                               const GrGLenum* bufs) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDrawElements(GrGLenum mode,
                                                GrGLsizei count,
                                                GrGLenum type,
                                                const GrGLvoid* indices) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLEnable(GrGLenum cap) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLEnableVertexAttribArray(GrGLuint index) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLEndQuery(GrGLenum target) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLFinish() {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLFlush() {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLFrontFace(GrGLenum mode) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLLineWidth(GrGLfloat width) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLLinkProgram(GrGLuint program) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLLoadIdentity() {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLLoadMatrixf(const GrGLfloat*) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLMatrixMode(GrGLenum) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLQueryCounter(GrGLuint id, GrGLenum target) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLReadBuffer(GrGLenum src) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLScissor(GrGLint x,
                                           GrGLint y,
                                           GrGLsizei width,
                                           GrGLsizei height) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLShaderSource(GrGLuint shader,
                                                GrGLsizei count,
#if GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE
                                                const char* const * str,
#else
                                                const char** str,
#endif
                                                const GrGLint* length) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilFunc(GrGLenum func, GrGLint ref, GrGLuint mask) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilFuncSeparate(GrGLenum face,
                                                       GrGLenum func,
                                                       GrGLint ref,
                                                       GrGLuint mask) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilMask(GrGLuint mask) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilMaskSeparate(GrGLenum face, GrGLuint mask) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilOp(GrGLenum fail, GrGLenum zfail, GrGLenum zpass) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLStencilOpSeparate(GrGLenum face,
                                                     GrGLenum fail,
                                                     GrGLenum zfail,
                                                     GrGLenum zpass) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexGenf(GrGLenum, GrGLenum, float) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexGenfv(GrGLenum, GrGLenum, const float*) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexGeni(GrGLenum, GrGLenum, GrGLint) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexImage2D(GrGLenum target,
                                              GrGLint level,
                                              GrGLint internalformat,
                                              GrGLsizei width,
                                              GrGLsizei height,
                                              GrGLint border,
                                              GrGLenum format,
                                              GrGLenum type,
                                              const GrGLvoid* pixels) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexParameteri(GrGLenum target,
                                                 GrGLenum pname,
                                                 GrGLint param) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexParameteriv(GrGLenum target,
                                                  GrGLenum pname,
                                                  const GrGLint* params) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexStorage2D(GrGLenum target,
                                                GrGLsizei levels,
                                                GrGLenum internalformat,
                                                GrGLsizei width,
                                                GrGLsizei height) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDiscardFramebuffer(GrGLenum target,
                                                      GrGLsizei numAttachments,
                                                      const GrGLenum* attachments) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLTexSubImage2D(GrGLenum target,
                                                 GrGLint level,
                                                 GrGLint xoffset,
                                                 GrGLint yoffset,
                                                 GrGLsizei width,
                                                 GrGLsizei height,
                                                 GrGLenum format,
                                                 GrGLenum type,
                                                 const GrGLvoid* pixels) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform1f(GrGLint location, GrGLfloat v0) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform1i(GrGLint location, GrGLint v0) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform1fv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLfloat* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform1iv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLint* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform2f(GrGLint location, GrGLfloat v0, GrGLfloat v1) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform2i(GrGLint location, GrGLint v0, GrGLint v1) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform2fv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLfloat* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform2iv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLint* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform3f(GrGLint location,
                                              GrGLfloat v0,
                                              GrGLfloat v1,
                                              GrGLfloat v2) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform3i(GrGLint location,
                                              GrGLint v0,
                                              GrGLint v1,
                                              GrGLint v2) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform3fv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLfloat* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform3iv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLint* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform4f(GrGLint location,
                                              GrGLfloat v0,
                                              GrGLfloat v1,
                                              GrGLfloat v2,
                                              GrGLfloat v3) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform4i(GrGLint location,
                                              GrGLint v0,
                                              GrGLint v1,
                                              GrGLint v2,
                                              GrGLint v3) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform4fv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLfloat* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniform4iv(GrGLint location,
                                              GrGLsizei count,
                                              const GrGLint* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniformMatrix2fv(GrGLint location,
                                                    GrGLsizei count,
                                                    GrGLboolean transpose,
                                                    const GrGLfloat* value) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniformMatrix3fv(GrGLint location,
                                                    GrGLsizei count,
                                                    GrGLboolean transpose,
                                                    const GrGLfloat* value) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLUniformMatrix4fv(GrGLint location,
                                                    GrGLsizei count,
                                                    GrGLboolean transpose,
                                                    const GrGLfloat* value) {
}

 GrGLvoid GR_GL_FUNCTION_TYPE noOpGLVertexAttrib4fv(GrGLuint indx, const GrGLfloat* values) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLVertexAttribPointer(GrGLuint indx,
                                                       GrGLint size,
                                                       GrGLenum type,
                                                       GrGLboolean normalized,
                                                       GrGLsizei stride,
                                                       const GrGLvoid* ptr) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLViewport(GrGLint x,
                                            GrGLint y,
                                            GrGLsizei width,
                                            GrGLsizei height) {
}

  GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetFramebufferAttachmentParameteriv(GrGLenum target,
                                                                         GrGLenum attachment,
                                                                         GrGLenum pname,
                                                                         GrGLint* params) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetRenderbufferParameteriv(GrGLenum target,
                                                              GrGLenum pname,
                                                              GrGLint* params) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLRenderbufferStorage(GrGLenum target,
                                                       GrGLenum internalformat,
                                                       GrGLsizei width,
                                                       GrGLsizei height) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLRenderbufferStorageMultisample(GrGLenum target,
                                                                  GrGLsizei samples,
                                                                  GrGLenum internalformat,
                                                                  GrGLsizei width,
                                                                  GrGLsizei height) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBlitFramebuffer(GrGLint srcX0,
                                                   GrGLint srcY0,
                                                   GrGLint srcX1,
                                                   GrGLint srcY1,
                                                   GrGLint dstX0,
                                                   GrGLint dstY0,
                                                   GrGLint dstX1,
                                                   GrGLint dstY1,
                                                   GrGLbitfield mask,
                                                   GrGLenum filter) {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLResolveMultisampleFramebuffer() {
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLBindFragDataLocationIndexed(GrGLuint program,
                                                               GrGLuint colorNumber,
                                                               GrGLuint index,
                                                               const GrGLchar * name) {
}

GrGLenum GR_GL_FUNCTION_TYPE noOpGLCheckFramebufferStatus(GrGLenum target) {

    GrAlwaysAssert(GR_GL_FRAMEBUFFER == target);

    return GR_GL_FRAMEBUFFER_COMPLETE;
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGenIds(GrGLsizei n, GrGLuint* ids) {
    static int gCurrID = 1;
    for (int i = 0; i < n; ++i) {
        ids[i] = ++gCurrID;
   }
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLDeleteIds(GrGLsizei n, const GrGLuint* ids) {
}

GrGLenum GR_GL_FUNCTION_TYPE noOpGLGetError() {
    return GR_GL_NO_ERROR;
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetIntegerv(GrGLenum pname, GrGLint* params) {
    // TODO: remove from Ganesh the #defines for gets we don't use.
    // We would like to minimize gets overall due to performance issues
    switch (pname) {
        case GR_GL_CONTEXT_PROFILE_MASK:
            *params = GR_GL_CONTEXT_COMPATIBILITY_PROFILE_BIT;
            break;
        case GR_GL_STENCIL_BITS:
            *params = 8;
            break;
        case GR_GL_SAMPLES:
            *params = 1;
            break;
        case GR_GL_FRAMEBUFFER_BINDING:
            *params = 0;
            break;
        case GR_GL_VIEWPORT:
            params[0] = 0;
            params[1] = 0;
            params[2] = 800;
            params[3] = 600;
            break;
        case GR_GL_MAX_TEXTURE_IMAGE_UNITS:
            *params = 8;
            break;
        case GR_GL_MAX_TEXTURE_COORDS:
            *params = 8;
            break;
        case GR_GL_MAX_VERTEX_UNIFORM_VECTORS:
            *params = kDefaultMaxVertexUniformVectors;
            break;
        case GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS:
            *params = kDefaultMaxFragmentUniformVectors;
            break;
        case GR_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
            *params = 16 * 4;
            break;
        case GR_GL_NUM_COMPRESSED_TEXTURE_FORMATS:
            *params = 0;
            break;
        case GR_GL_COMPRESSED_TEXTURE_FORMATS:
            break;
        case GR_GL_MAX_TEXTURE_SIZE:
            *params = 8192;
            break;
        case GR_GL_MAX_RENDERBUFFER_SIZE:
            *params = 8192;
            break;
        case GR_GL_MAX_SAMPLES:
            *params = 32;
            break;
        case GR_GL_MAX_VERTEX_ATTRIBS:
            *params = kDefaultMaxVertexAttribs;
            break;
        case GR_GL_MAX_VARYING_VECTORS:
            *params = kDefaultMaxVaryingVectors;
            break;
        case GR_GL_NUM_EXTENSIONS:
            *params = GR_ARRAY_COUNT(kExtensions);
            break;
        default:
            GrCrash("Unexpected pname to GetIntegerv");
   }
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetInfoLog(GrGLuint program,
                                              GrGLsizei bufsize,
                                              GrGLsizei* length,
                                              char* infolog) {
    if (length) {
        *length = 0;
   }
    if (bufsize > 0) {
        *infolog = 0;
   }
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetShaderOrProgramiv(GrGLuint program,
                                                        GrGLenum pname,
                                                        GrGLint* params) {
    switch (pname) {
        case GR_GL_LINK_STATUS:  // fallthru
        case GR_GL_COMPILE_STATUS:
            *params = GR_GL_TRUE;
            break;
        case GR_GL_INFO_LOG_LENGTH:
            *params = 0;
            break;
        // we don't expect any other pnames
        default:
            GrCrash("Unexpected pname to GetProgramiv");
            break;
   }
}

namespace {
template <typename T>
void query_result(GrGLenum GLtarget, GrGLenum pname, T *params) {
    switch (pname) {
        case GR_GL_QUERY_RESULT_AVAILABLE:
            *params = GR_GL_TRUE;
            break;
        case GR_GL_QUERY_RESULT:
            *params = 0;
            break;
        default:
            GrCrash("Unexpected pname passed to GetQueryObject.");
            break;
   }
}
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetQueryiv(GrGLenum GLtarget,
                                              GrGLenum pname,
                                              GrGLint *params) {
    switch (pname) {
        case GR_GL_CURRENT_QUERY:
            *params = 0;
            break;
        case GR_GL_QUERY_COUNTER_BITS:
            *params = 32;
            break;
        default:
            GrCrash("Unexpected pname passed GetQueryiv.");
   }
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetQueryObjecti64v(GrGLuint id,
                                                      GrGLenum pname,
                                                      GrGLint64 *params) {
    query_result(id, pname, params);
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetQueryObjectiv(GrGLuint id,
                                                    GrGLenum pname,
                                                    GrGLint *params) {
    query_result(id, pname, params);
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetQueryObjectui64v(GrGLuint id,
                                                       GrGLenum pname,
                                                       GrGLuint64 *params) {
    query_result(id, pname, params);
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetQueryObjectuiv(GrGLuint id,
                                                     GrGLenum pname,
                                                     GrGLuint *params) {
    query_result(id, pname, params);
}

const GrGLubyte* GR_GL_FUNCTION_TYPE noOpGLGetString(GrGLenum name) {
    switch (name) {
        case GR_GL_EXTENSIONS:
            return combined_extensions_string();
        case GR_GL_VERSION:
            return (const GrGLubyte*)"4.0 Debug GL";
        case GR_GL_SHADING_LANGUAGE_VERSION:
            return (const GrGLubyte*)"4.20.8 Debug GLSL";
        case GR_GL_VENDOR:
            return (const GrGLubyte*)"Debug Vendor";
        case GR_GL_RENDERER:
            return (const GrGLubyte*)"The Debug (Non-)Renderer";
        default:
            GrCrash("Unexpected name passed to GetString");
            return NULL;
   }
}

const GrGLubyte* GR_GL_FUNCTION_TYPE noOpGLGetStringi(GrGLenum name, GrGLuint i) {
    switch (name) {
        case GR_GL_EXTENSIONS:
            if (static_cast<size_t>(i) <= GR_ARRAY_COUNT(kExtensions)) {
                return (const GrGLubyte*) kExtensions[i];
            } else {
                return NULL;
            }
        default:
            GrCrash("Unexpected name passed to GetStringi");
            return NULL;
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLGetTexLevelParameteriv(GrGLenum target,
                                                          GrGLint level,
                                                          GrGLenum pname,
                                                          GrGLint* params) {
    // we used to use this to query stuff about externally created textures,
    // now we just require clients to tell us everything about the texture.
    GrCrash("Should never query texture parameters.");
}

GrGLint GR_GL_FUNCTION_TYPE noOpGLGetUniformLocation(GrGLuint program, const char* name) {
    static int gUniLocation = 0;
    return ++gUniLocation;
}

GrGLvoid GR_GL_FUNCTION_TYPE noOpGLInsertEventMarker(GrGLsizei length, const char* marker) {
}
GrGLvoid GR_GL_FUNCTION_TYPE noOpGLPushGroupMarker(GrGLsizei length  , const char* marker) {
}
GrGLvoid GR_GL_FUNCTION_TYPE noOpGLPopGroupMarker() {
}
