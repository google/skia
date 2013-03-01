/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "GrGLDefines.h"
#include "SkTDArray.h"
#include "GrGLNoOpInterface.h"

// Functions not declared in GrGLBogusInterface.h (not common with the Debug GL interface).

namespace { // added to suppress 'no previous prototype' warning

GrGLvoid GR_GL_FUNCTION_TYPE nullGLActiveTexture(GrGLenum texture) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLAttachShader(GrGLuint program, GrGLuint shader) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBeginQuery(GrGLenum target, GrGLuint id) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindAttribLocation(GrGLuint program, GrGLuint index, const char* name) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindTexture(GrGLenum target, GrGLuint texture) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindVertexArray(GrGLuint id) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBufferData(GrGLenum target, GrGLsizeiptr size, const GrGLvoid* data, GrGLenum usage) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLPixelStorei(GrGLenum pname, GrGLint param) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLReadPixels(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, GrGLvoid* pixels) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLUseProgram(GrGLuint program) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLViewport(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindFramebuffer(GrGLenum target, GrGLuint framebuffer) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindRenderbuffer(GrGLenum target, GrGLuint renderbuffer) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDeleteFramebuffers(GrGLsizei n, const GrGLuint *framebuffers) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDeleteRenderbuffers(GrGLsizei n, const GrGLuint *renderbuffers) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLFramebufferRenderbuffer(GrGLenum target, GrGLenum attachment, GrGLenum renderbuffertarget, GrGLuint renderbuffer) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLFramebufferTexture2D(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level) {}

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

// In debug builds we do asserts that ensure we agree with GL about when a buffer
// is mapped.
static SkTDArray<GrGLuint> gMappedBuffers;
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

} // end anonymous namespace

const GrGLInterface* GrGLCreateNullInterface() {
    // The gl functions are not context-specific so we create one global
    // interface
    static SkAutoTUnref<GrGLInterface> glInterface;
    if (!glInterface.get()) {
        GrGLInterface* interface = SkNEW(GrGLInterface);
        glInterface.reset(interface);
        interface->fBindingsExported = kDesktop_GrGLBinding;
        interface->fActiveTexture = nullGLActiveTexture;
        interface->fAttachShader = nullGLAttachShader;
        interface->fBeginQuery = nullGLBeginQuery;
        interface->fBindAttribLocation = nullGLBindAttribLocation;
        interface->fBindBuffer = nullGLBindBuffer;
        interface->fBindFragDataLocation = noOpGLBindFragDataLocation;
        interface->fBindTexture = nullGLBindTexture;
        interface->fBindVertexArray = nullGLBindVertexArray;
        interface->fBlendColor = noOpGLBlendColor;
        interface->fBlendFunc = noOpGLBlendFunc;
        interface->fBufferData = nullGLBufferData;
        interface->fBufferSubData = noOpGLBufferSubData;
        interface->fClear = noOpGLClear;
        interface->fClearColor = noOpGLClearColor;
        interface->fClearStencil = noOpGLClearStencil;
        interface->fColorMask = noOpGLColorMask;
        interface->fCompileShader = noOpGLCompileShader;
        interface->fCompressedTexImage2D = noOpGLCompressedTexImage2D;
        interface->fCreateProgram = nullGLCreateProgram;
        interface->fCreateShader = nullGLCreateShader;
        interface->fCullFace = noOpGLCullFace;
        interface->fDeleteBuffers = nullGLDeleteBuffers;
        interface->fDeleteProgram = nullGLDelete;
        interface->fDeleteQueries = noOpGLDeleteIds;
        interface->fDeleteShader = nullGLDelete;
        interface->fDeleteTextures = noOpGLDeleteIds;
        interface->fDeleteVertexArrays = noOpGLDeleteIds;
        interface->fDepthMask = noOpGLDepthMask;
        interface->fDisable = noOpGLDisable;
        interface->fDisableVertexAttribArray = noOpGLDisableVertexAttribArray;
        interface->fDrawArrays = noOpGLDrawArrays;
        interface->fDrawBuffer = noOpGLDrawBuffer;
        interface->fDrawBuffers = noOpGLDrawBuffers;
        interface->fDrawElements = noOpGLDrawElements;
        interface->fEnable = noOpGLEnable;
        interface->fEnableVertexAttribArray = noOpGLEnableVertexAttribArray;
        interface->fEndQuery = noOpGLEndQuery;
        interface->fFinish = noOpGLFinish;
        interface->fFlush = noOpGLFlush;
        interface->fFrontFace = noOpGLFrontFace;
        interface->fGenBuffers = noOpGLGenIds;
        interface->fGenQueries = noOpGLGenIds;
        interface->fGenTextures = noOpGLGenIds;
        interface->fGenVertexArrays = noOpGLGenIds;
        interface->fGetBufferParameteriv = nullGLGetBufferParameteriv;
        interface->fGetError = noOpGLGetError;
        interface->fGetIntegerv = noOpGLGetIntegerv;
        interface->fGetQueryObjecti64v = noOpGLGetQueryObjecti64v;
        interface->fGetQueryObjectiv = noOpGLGetQueryObjectiv;
        interface->fGetQueryObjectui64v = noOpGLGetQueryObjectui64v;
        interface->fGetQueryObjectuiv = noOpGLGetQueryObjectuiv;
        interface->fGetQueryiv = noOpGLGetQueryiv;
        interface->fGetProgramInfoLog = noOpGLGetInfoLog;
        interface->fGetProgramiv = noOpGLGetShaderOrProgramiv;
        interface->fGetShaderInfoLog = noOpGLGetInfoLog;
        interface->fGetShaderiv = noOpGLGetShaderOrProgramiv;
        interface->fGetString = noOpGLGetString;
        interface->fGetStringi = noOpGLGetStringi;
        interface->fGetTexLevelParameteriv = noOpGLGetTexLevelParameteriv;
        interface->fGetUniformLocation = noOpGLGetUniformLocation;
        interface->fLineWidth = noOpGLLineWidth;
        interface->fLinkProgram = noOpGLLinkProgram;
        interface->fPixelStorei = nullGLPixelStorei;
        interface->fQueryCounter = noOpGLQueryCounter;
        interface->fReadBuffer = noOpGLReadBuffer;
        interface->fReadPixels = nullGLReadPixels;
        interface->fScissor = noOpGLScissor;
        interface->fShaderSource = noOpGLShaderSource;
        interface->fStencilFunc = noOpGLStencilFunc;
        interface->fStencilFuncSeparate = noOpGLStencilFuncSeparate;
        interface->fStencilMask = noOpGLStencilMask;
        interface->fStencilMaskSeparate = noOpGLStencilMaskSeparate;
        interface->fStencilOp = noOpGLStencilOp;
        interface->fStencilOpSeparate = noOpGLStencilOpSeparate;
        interface->fTexImage2D = noOpGLTexImage2D;
        interface->fTexParameteri = noOpGLTexParameteri;
        interface->fTexParameteriv = noOpGLTexParameteriv;
        interface->fTexSubImage2D = noOpGLTexSubImage2D;
        interface->fTexStorage2D = noOpGLTexStorage2D;
        interface->fUniform1f = noOpGLUniform1f;
        interface->fUniform1i = noOpGLUniform1i;
        interface->fUniform1fv = noOpGLUniform1fv;
        interface->fUniform1iv = noOpGLUniform1iv;
        interface->fUniform2f = noOpGLUniform2f;
        interface->fUniform2i = noOpGLUniform2i;
        interface->fUniform2fv = noOpGLUniform2fv;
        interface->fUniform2iv = noOpGLUniform2iv;
        interface->fUniform3f = noOpGLUniform3f;
        interface->fUniform3i = noOpGLUniform3i;
        interface->fUniform3fv = noOpGLUniform3fv;
        interface->fUniform3iv = noOpGLUniform3iv;
        interface->fUniform4f = noOpGLUniform4f;
        interface->fUniform4i = noOpGLUniform4i;
        interface->fUniform4fv = noOpGLUniform4fv;
        interface->fUniform4iv = noOpGLUniform4iv;
        interface->fUniformMatrix2fv = noOpGLUniformMatrix2fv;
        interface->fUniformMatrix3fv = noOpGLUniformMatrix3fv;
        interface->fUniformMatrix4fv = noOpGLUniformMatrix4fv;
        interface->fUseProgram = nullGLUseProgram;
        interface->fVertexAttrib4fv = noOpGLVertexAttrib4fv;
        interface->fVertexAttribPointer = noOpGLVertexAttribPointer;
        interface->fViewport = nullGLViewport;
        interface->fBindFramebuffer = nullGLBindFramebuffer;
        interface->fBindRenderbuffer = nullGLBindRenderbuffer;
        interface->fCheckFramebufferStatus = noOpGLCheckFramebufferStatus;
        interface->fDeleteFramebuffers = nullGLDeleteFramebuffers;
        interface->fDeleteRenderbuffers = nullGLDeleteRenderbuffers;
        interface->fFramebufferRenderbuffer = nullGLFramebufferRenderbuffer;
        interface->fFramebufferTexture2D = nullGLFramebufferTexture2D;
        interface->fGenFramebuffers = noOpGLGenIds;
        interface->fGenRenderbuffers = noOpGLGenIds;
        interface->fGetFramebufferAttachmentParameteriv = noOpGLGetFramebufferAttachmentParameteriv;
        interface->fGetRenderbufferParameteriv = noOpGLGetRenderbufferParameteriv;
        interface->fRenderbufferStorage = noOpGLRenderbufferStorage;
        interface->fRenderbufferStorageMultisample = noOpGLRenderbufferStorageMultisample;
        interface->fBlitFramebuffer = noOpGLBlitFramebuffer;
        interface->fResolveMultisampleFramebuffer = noOpGLResolveMultisampleFramebuffer;
        interface->fMapBuffer = nullGLMapBuffer;
        interface->fUnmapBuffer = nullGLUnmapBuffer;
        interface->fBindFragDataLocationIndexed = noOpGLBindFragDataLocationIndexed;
    }
    glInterface.get()->ref();
    return glInterface.get();
}
