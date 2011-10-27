
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLInterface.h"

GrGLvoid GR_GL_FUNCTION_TYPE nullGLActiveTexture(GrGLenum texture) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLAttachShader(GrGLuint program, GrGLuint shader) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBeginQuery(GrGLenum target, GrGLuint id) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindAttribLocation(GrGLuint program, GrGLuint index, const char* name) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindTexture(GrGLenum target, GrGLuint texture) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBlendColor(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindFragDataLocation(GrGLuint program, GrGLuint colorNumber, const GrGLchar* name) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBlendFunc(GrGLenum sfactor, GrGLenum dfactor) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBufferData(GrGLenum target, GrGLsizeiptr size, const GrGLvoid* data, GrGLenum usage) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBufferSubData(GrGLenum target, GrGLintptr offset, GrGLsizeiptr size, const GrGLvoid* data) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLClear(GrGLbitfield mask) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLClearColor(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLClearStencil(GrGLint s) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLClientActiveTexture(GrGLenum texture) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLColor4ub(GrGLubyte red, GrGLubyte green, GrGLubyte blue, GrGLubyte alpha) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLColorMask(GrGLboolean red, GrGLboolean green, GrGLboolean blue, GrGLboolean alpha) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLColorPointer(GrGLint size, GrGLenum type, GrGLsizei stride, const GrGLvoid* pointer) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLCompileShader(GrGLuint shader) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLCompressedTexImage2D(GrGLenum target, GrGLint level, GrGLenum internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLsizei imageSize, const GrGLvoid* data) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLCullFace(GrGLenum mode) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDepthMask(GrGLboolean flag) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDisable(GrGLenum cap) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDisableClientState(GrGLenum array) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDisableVertexAttribArray(GrGLuint index) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDrawArrays(GrGLenum mode, GrGLint first, GrGLsizei count) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDrawBuffer(GrGLenum mode) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDrawBuffers(GrGLsizei n, const GrGLenum* bufs) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDrawElements(GrGLenum mode, GrGLsizei count, GrGLenum type, const GrGLvoid* indices) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLEnable(GrGLenum cap) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLEnableClientState(GrGLenum cap) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLEnableVertexAttribArray(GrGLuint index) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLEndQuery(GrGLenum target) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLFinish() {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLFlush() {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLFrontFace(GrGLenum mode) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLLineWidth(GrGLfloat width) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLLinkProgram(GrGLuint program) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLLoadMatrixf(const GrGLfloat* m) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLMatrixMode(GrGLenum mode) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLPixelStorei(GrGLenum pname, GrGLint param) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLPointSize(GrGLfloat size) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLQueryCounter(GrGLuint id, GrGLenum target) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLReadBuffer(GrGLenum src) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLReadPixels(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, GrGLvoid* pixels) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLScissor(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLShadeModel(GrGLenum mode) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLShaderSource(GrGLuint shader, GrGLsizei count, const char** str, const GrGLint* length) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLStencilFunc(GrGLenum func, GrGLint ref, GrGLuint mask) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLStencilFuncSeparate(GrGLenum face, GrGLenum func, GrGLint ref, GrGLuint mask) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLStencilMask(GrGLuint mask) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLStencilMaskSeparate(GrGLenum face, GrGLuint mask) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLStencilOp(GrGLenum fail, GrGLenum zfail, GrGLenum zpass) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLStencilOpSeparate(GrGLenum face, GrGLenum fail, GrGLenum zfail, GrGLenum zpass) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLTexCoordPointer(GrGLint size, GrGLenum type, GrGLsizei stride, const GrGLvoid* pointer) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLTexEnvi(GrGLenum target, GrGLenum pname, GrGLint param) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLTexImage2D(GrGLenum target, GrGLint level, GrGLint internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid* pixels) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLTexParameteri(GrGLenum target, GrGLenum pname, GrGLint param) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLTexSubImage2D(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, const GrGLvoid* pixels) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform1f(GrGLint location, GrGLfloat v0) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform1i(GrGLint location, GrGLint v0) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform1fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform1iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform2f(GrGLint location, GrGLfloat v0, GrGLfloat v1) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform2i(GrGLint location, GrGLint v0, GrGLint v1) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform2fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform2iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform3f(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform3i(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform3fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform3iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform4f(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2, GrGLfloat v3) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform4i(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2, GrGLint v3) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform4fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniform4iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniformMatrix2fv(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniformMatrix3fv(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUniformMatrix4fv(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUseProgram(GrGLuint program) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLVertexAttrib4fv(GrGLuint indx, const GrGLfloat* values) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLVertexAttribPointer(GrGLuint indx, GrGLint size, GrGLenum type, GrGLboolean normalized, GrGLsizei stride, const GrGLvoid* ptr) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLVertexPointer(GrGLint size, GrGLenum type, GrGLsizei stride, const GrGLvoid* pointer) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLViewport(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindFramebuffer(GrGLenum target, GrGLuint framebuffer) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindRenderbuffer(GrGLenum target, GrGLuint renderbuffer) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDeleteFramebuffers(GrGLsizei n, const GrGLuint *framebuffers) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDeleteRenderbuffers(GrGLsizei n, const GrGLuint *renderbuffers) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLFramebufferRenderbuffer(GrGLenum target, GrGLenum attachment, GrGLenum renderbuffertarget, GrGLuint renderbuffer) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLFramebufferTexture2D(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetFramebufferAttachmentParameteriv(GrGLenum target, GrGLenum attachment, GrGLenum pname, GrGLint* params) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetRenderbufferParameteriv(GrGLenum target, GrGLenum pname, GrGLint* params) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLRenderbufferStorage(GrGLenum target, GrGLenum internalformat, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLRenderbufferStorageMultisample(GrGLenum target, GrGLsizei samples, GrGLenum internalformat, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBlitFramebuffer(GrGLint srcX0, GrGLint srcY0, GrGLint srcX1, GrGLint srcY1, GrGLint dstX0, GrGLint dstY0, GrGLint dstX1, GrGLint dstY1, GrGLbitfield mask, GrGLenum filter) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLResolveMultisampleFramebuffer() {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindFragDataLocationIndexed(GrGLuint program, GrGLuint colorNumber, GrGLuint index, const GrGLchar * name) {}

GrGLenum GR_GL_FUNCTION_TYPE nullGLCheckFramebufferStatus(GrGLenum target) {
    return GR_GL_FRAMEBUFFER_COMPLETE;
}

GrGLuint GR_GL_FUNCTION_TYPE nullGLCreateProgram() {
    static int gCurrID = 0;
    return ++gCurrID;
}

GrGLuint GR_GL_FUNCTION_TYPE nullGLCreateShader(GrGLenum type) {
    static int gCurrID = 0;
    return ++gCurrID;
}

// same delete used for shaders and programs
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDelete(GrGLuint program) {
}

// same function used for all glGen*(GLsize i, GLuint*) functions
GrGLvoid GR_GL_FUNCTION_TYPE nullGLGenIds(GrGLsizei n, GrGLuint* ids) {
    static int gCurrID = 0;
    for (int i = 0; i < n; ++i) {
        ids[i] = ++gCurrID;
    }
}
// same delete function for all glDelete*(GLsize i, const GLuint*) except buffers
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDeleteIds(GrGLsizei n, const GrGLuint* ids) {}

// In debug builds we do asserts that ensure we agree with GL about when a buffer
// is mapped.
#include "GrTDArray.h"
static GrTDArray<GrGLuint> gMappedBuffers;
static GrGLuint gCurrArrayBuffer;
static GrGLuint gCurrElementArrayBuffer;

GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindBuffer(GrGLenum target, GrGLuint buffer) {
    switch (target) {
    case GR_GL_ARRAY_BUFFER:
        gCurrArrayBuffer = buffer;
        break;
    case GR_GL_ELEMENT_ARRAY_BUFFER:
        gCurrElementArrayBuffer = buffer;
        break;
    }
}

// deleting a bound buffer has the side effect of binding 0
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDeleteBuffers(GrGLsizei n, const GrGLuint* ids) {
    for (int i = 0; i < n; ++i) {
        if (ids[i] == gCurrArrayBuffer) {
            gCurrArrayBuffer = 0;
        }
        if (ids[i] == gCurrElementArrayBuffer) {
            gCurrElementArrayBuffer = 0;
        }
        for (int j = 0; j < gMappedBuffers.count(); ++j) {
            if (gMappedBuffers[j] == ids[i]) {
                gMappedBuffers.remove(j);
                // don't break b/c we didn't check for dupes on insert
                --j;
            }
        }
    }
}

GrGLvoid* GR_GL_FUNCTION_TYPE nullGLMapBuffer(GrGLenum target, GrGLenum access) {
    // We just reserve 32MB of RAM for all locks and hope its big enough
    static SkAutoMalloc gBufferData(32 * (1 << 20));
    GrGLuint buf = 0;
    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            buf = gCurrArrayBuffer;
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            buf = gCurrElementArrayBuffer;
            break;
    }
    if (buf) {
        *gMappedBuffers.append() = buf;
    }
    return gBufferData.get();
}

GrGLboolean GR_GL_FUNCTION_TYPE nullGLUnmapBuffer(GrGLenum target) {
    GrGLuint buf = 0;
    switch (target) {
    case GR_GL_ARRAY_BUFFER:
        buf = gCurrArrayBuffer;
        break;
    case GR_GL_ELEMENT_ARRAY_BUFFER:
        buf = gCurrElementArrayBuffer;
        break;
    }
    if (buf) {
        for (int i = 0; i < gMappedBuffers.count(); ++i) {
            if (gMappedBuffers[i] == buf) {
                gMappedBuffers.remove(i);
                // don't break b/c we didn't check for dupes on insert
                --i;
            }
        }
    }
    return GR_GL_TRUE;
}

GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetBufferParameteriv(GrGLenum target, GrGLenum pname, GrGLint* params) {
    switch (pname) {
        case GR_GL_BUFFER_MAPPED: {
            *params = GR_GL_FALSE;
            GrGLuint buf = 0;
            switch (target) {
                case GR_GL_ARRAY_BUFFER:
                    buf = gCurrArrayBuffer;
                    break;
                case GR_GL_ELEMENT_ARRAY_BUFFER:
                    buf = gCurrElementArrayBuffer;
                    break;  
            }
            if (buf) {
                for (int i = 0; i < gMappedBuffers.count(); ++i) {
                    if (gMappedBuffers[i] == buf) {
                        *params = GR_GL_TRUE;
                        break;
                    }
                }
            }
            break; }
        default:
            GrCrash("Unexpected pname to GetBufferParamateriv");
            break;
    }
};

GrGLenum GR_GL_FUNCTION_TYPE nullGLGetError() {
    return GR_GL_NO_ERROR;
}

GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetIntegerv(GrGLenum pname, GrGLint* params) {
    switch (pname) {
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
        case GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS:
            *params = 16;
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
            *params = 16;
            break;
        case GR_GL_MAX_TEXTURE_UNITS:
            *params = 8;
            break;
        default:
            GrCrash("Unexpected pname to GetIntegerv");
    }
}
// used for both the program and shader info logs
GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetInfoLog(GrGLuint program, GrGLsizei bufsize, GrGLsizei* length, char* infolog) {
    if (length) {
        *length = 0;
    }
    if (bufsize > 0) {
        *infolog = 0;
    }
}

// used for both the program and shader params
GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetShaderOrProgramiv(GrGLuint program, GrGLenum pname, GrGLint* params) {
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

// Queries on the null GL just don't do anything at all. We could potentially make
// the timers work.
GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetQueryiv(GrGLenum GLtarget, GrGLenum pname, GrGLint *params) {
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
GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetQueryObjecti64v(GrGLuint id, GrGLenum pname, GrGLint64 *params) {
    query_result(id, pname, params);
}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetQueryObjectiv(GrGLuint id, GrGLenum pname, GrGLint *params) {
    query_result(id, pname, params);
}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetQueryObjectui64v(GrGLuint id, GrGLenum pname, GrGLuint64 *params) {
    query_result(id, pname, params);
}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetQueryObjectuiv(GrGLuint id, GrGLenum pname, GrGLuint *params) {
    query_result(id, pname, params);
}

const GrGLubyte* GR_GL_FUNCTION_TYPE nullGLGetString(GrGLenum name) {
    switch (name) {
        case GR_GL_EXTENSIONS:
            return (const GrGLubyte*)"GL_ARB_framebuffer_object GL_ARB_blend_func_extended GL_ARB_timer_query GL_ARB_draw_buffers GL_ARB_occlusion_query GL_EXT_blend_color GL_EXT_stencil_wrap";
        case GR_GL_VERSION:
            return (const GrGLubyte*)"4.0 Null GL";
        case GR_GL_SHADING_LANGUAGE_VERSION:
            return (const GrGLubyte*)"4.20.8 Null GLSL";
        default:
            GrCrash("Unexpected name to GetString");
            return NULL;
    }
}

// we used to use this to query stuff about externally created textures, now we just
// require clients to tell us everything about the texture.
GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetTexLevelParameteriv(GrGLenum target, GrGLint level, GrGLenum pname, GrGLint* params) {
    GrCrash("Should never query texture parameters.");
}

GrGLint GR_GL_FUNCTION_TYPE nullGLGetUniformLocation(GrGLuint program, const char* name) {
    static int gUniLocation = 0;
    return ++gUniLocation;
}

const GrGLInterface* GrGLCreateNullInterface() {
    // The gl functions are not context-specific so we create one global 
    // interface
    static SkAutoTUnref<GrGLInterface> glInterface;
    if (!glInterface.get()) {
        GrGLInterface* interface = new GrGLInterface;
        glInterface.reset(interface);
        interface->fBindingsExported = kDesktop_GrGLBinding;
        interface->fActiveTexture = nullGLActiveTexture;
        interface->fAttachShader = nullGLAttachShader;
        interface->fBeginQuery = nullGLBeginQuery;
        interface->fBindAttribLocation = nullGLBindAttribLocation;
        interface->fBindBuffer = nullGLBindBuffer;
        interface->fBindFragDataLocation = nullGLBindFragDataLocation;
        interface->fBindTexture = nullGLBindTexture;
        interface->fBlendColor = nullGLBlendColor;
        interface->fBlendFunc = nullGLBlendFunc;
        interface->fBufferData = nullGLBufferData;
        interface->fBufferSubData = nullGLBufferSubData;
        interface->fClear = nullGLClear;
        interface->fClearColor = nullGLClearColor;
        interface->fClearStencil = nullGLClearStencil;
        interface->fClientActiveTexture = nullGLClientActiveTexture;
        interface->fColor4ub = nullGLColor4ub;
        interface->fColorMask = nullGLColorMask;
        interface->fColorPointer = nullGLColorPointer;
        interface->fCompileShader = nullGLCompileShader;
        interface->fCompressedTexImage2D = nullGLCompressedTexImage2D;
        interface->fCreateProgram = nullGLCreateProgram;
        interface->fCreateShader = nullGLCreateShader;
        interface->fCullFace = nullGLCullFace;
        interface->fDeleteBuffers = nullGLDeleteBuffers;
        interface->fDeleteProgram = nullGLDelete;
        interface->fDeleteQueries = nullGLDeleteIds;
        interface->fDeleteShader = nullGLDelete;
        interface->fDeleteTextures = nullGLDeleteIds;
        interface->fDepthMask = nullGLDepthMask;
        interface->fDisable = nullGLDisable;
        interface->fDisableClientState = nullGLDisableClientState;
        interface->fDisableVertexAttribArray = nullGLDisableVertexAttribArray;
        interface->fDrawArrays = nullGLDrawArrays;
        interface->fDrawBuffer = nullGLDrawBuffer;
        interface->fDrawBuffers = nullGLDrawBuffers;
        interface->fDrawElements = nullGLDrawElements;
        interface->fEnable = nullGLEnable;
        interface->fEnableClientState = nullGLEnableClientState;
        interface->fEnableVertexAttribArray = nullGLEnableVertexAttribArray;
        interface->fEndQuery = nullGLEndQuery;
        interface->fFinish = nullGLFinish;
        interface->fFlush = nullGLFlush;
        interface->fFrontFace = nullGLFrontFace;
        interface->fGenBuffers = nullGLGenIds;
        interface->fGenQueries = nullGLGenIds;
        interface->fGenTextures = nullGLGenIds;
        interface->fGetBufferParameteriv = nullGLGetBufferParameteriv;
        interface->fGetError = nullGLGetError;
        interface->fGetIntegerv = nullGLGetIntegerv;
        interface->fGetQueryObjecti64v = nullGLGetQueryObjecti64v;
        interface->fGetQueryObjectiv = nullGLGetQueryObjectiv;
        interface->fGetQueryObjectui64v = nullGLGetQueryObjectui64v;
        interface->fGetQueryObjectuiv = nullGLGetQueryObjectuiv;
        interface->fGetQueryiv = nullGLGetQueryiv;
        interface->fGetProgramInfoLog = nullGLGetInfoLog;
        interface->fGetProgramiv = nullGLGetShaderOrProgramiv;
        interface->fGetShaderInfoLog = nullGLGetInfoLog;
        interface->fGetShaderiv = nullGLGetShaderOrProgramiv;
        interface->fGetString = nullGLGetString;
        interface->fGetTexLevelParameteriv = nullGLGetTexLevelParameteriv;
        interface->fGetUniformLocation = nullGLGetUniformLocation;
        interface->fLineWidth = nullGLLineWidth;
        interface->fLinkProgram = nullGLLinkProgram;
        interface->fLoadMatrixf = nullGLLoadMatrixf;
        interface->fMatrixMode = nullGLMatrixMode;
        interface->fPixelStorei = nullGLPixelStorei;
        interface->fPointSize = nullGLPointSize;
        interface->fQueryCounter = nullGLQueryCounter;
        interface->fReadBuffer = nullGLReadBuffer;
        interface->fReadPixels = nullGLReadPixels;
        interface->fScissor = nullGLScissor;
        interface->fShadeModel = nullGLShadeModel;
        interface->fShaderSource = nullGLShaderSource;
        interface->fStencilFunc = nullGLStencilFunc;
        interface->fStencilFuncSeparate = nullGLStencilFuncSeparate;
        interface->fStencilMask = nullGLStencilMask;
        interface->fStencilMaskSeparate = nullGLStencilMaskSeparate;
        interface->fStencilOp = nullGLStencilOp;
        interface->fStencilOpSeparate = nullGLStencilOpSeparate;
        interface->fTexCoordPointer = nullGLTexCoordPointer;
        interface->fTexEnvi = nullGLTexEnvi;
        interface->fTexImage2D = nullGLTexImage2D;
        interface->fTexParameteri = nullGLTexParameteri;
        interface->fTexSubImage2D = nullGLTexSubImage2D;
        interface->fUniform1f = nullGLUniform1f;
        interface->fUniform1i = nullGLUniform1i;
        interface->fUniform1fv = nullGLUniform1fv;
        interface->fUniform1iv = nullGLUniform1iv;
        interface->fUniform2f = nullGLUniform2f;
        interface->fUniform2i = nullGLUniform2i;
        interface->fUniform2fv = nullGLUniform2fv;
        interface->fUniform2iv = nullGLUniform2iv;
        interface->fUniform3f = nullGLUniform3f;
        interface->fUniform3i = nullGLUniform3i;
        interface->fUniform3fv = nullGLUniform3fv;
        interface->fUniform3iv = nullGLUniform3iv;
        interface->fUniform4f = nullGLUniform4f;
        interface->fUniform4i = nullGLUniform4i;
        interface->fUniform4fv = nullGLUniform4fv;
        interface->fUniform4iv = nullGLUniform4iv;
        interface->fUniformMatrix2fv = nullGLUniformMatrix2fv;
        interface->fUniformMatrix3fv = nullGLUniformMatrix3fv;
        interface->fUniformMatrix4fv = nullGLUniformMatrix4fv;
        interface->fUseProgram = nullGLUseProgram;
        interface->fVertexAttrib4fv = nullGLVertexAttrib4fv;
        interface->fVertexAttribPointer = nullGLVertexAttribPointer;
        interface->fVertexPointer = nullGLVertexPointer;
        interface->fViewport = nullGLViewport;
        interface->fBindFramebuffer = nullGLBindFramebuffer;
        interface->fBindRenderbuffer = nullGLBindRenderbuffer;
        interface->fCheckFramebufferStatus = nullGLCheckFramebufferStatus;
        interface->fDeleteFramebuffers = nullGLDeleteFramebuffers;
        interface->fDeleteRenderbuffers = nullGLDeleteRenderbuffers;
        interface->fFramebufferRenderbuffer = nullGLFramebufferRenderbuffer;
        interface->fFramebufferTexture2D = nullGLFramebufferTexture2D;
        interface->fGenFramebuffers = nullGLGenIds;
        interface->fGenRenderbuffers = nullGLGenIds;
        interface->fGetFramebufferAttachmentParameteriv = nullGLGetFramebufferAttachmentParameteriv;
        interface->fGetRenderbufferParameteriv = nullGLGetRenderbufferParameteriv;
        interface->fRenderbufferStorage = nullGLRenderbufferStorage;
        interface->fRenderbufferStorageMultisample = nullGLRenderbufferStorageMultisample;
        interface->fBlitFramebuffer = nullGLBlitFramebuffer;
        interface->fResolveMultisampleFramebuffer = nullGLResolveMultisampleFramebuffer;
        interface->fMapBuffer = nullGLMapBuffer;
        interface->fUnmapBuffer = nullGLUnmapBuffer;
        interface->fBindFragDataLocationIndexed = nullGLBindFragDataLocationIndexed;
    }
    glInterface.get()->ref();
    return glInterface.get();
}
