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
#include "SkTLS.h"

// TODO: Delete this file after chrome starts using SkNullGLContext.

// added to suppress 'no previous prototype' warning and because this code is duplicated in
// SkNullGLContext.cpp
namespace {      

class BufferObj {
public:
    

    BufferObj(GrGLuint id) : fID(id), fDataPtr(nullptr), fSize(0), fMapped(false) {
    }
    ~BufferObj() { delete[] fDataPtr; }

    void allocate(GrGLsizeiptr size, const GrGLchar* dataPtr) {
        if (fDataPtr) {
            SkASSERT(0 != fSize);
            delete[] fDataPtr;
        }

        fSize = size;
        fDataPtr = new char[size];
    }

    GrGLuint id() const          { return fID; }
    GrGLchar* dataPtr()          { return fDataPtr; }
    GrGLsizeiptr size() const    { return fSize; }

    void setMapped(bool mapped)  { fMapped = mapped; }
    bool mapped() const          { return fMapped; }

private:
    GrGLuint     fID;
    GrGLchar*    fDataPtr;
    GrGLsizeiptr fSize;         // size in bytes
    bool         fMapped;
};

// This class maintains a sparsely populated array of buffer pointers.
class BufferManager {
public:
    

    BufferManager() : fFreeListHead(kFreeListEnd) {}

    ~BufferManager() {
        // nullptr out the entries that are really free list links rather than ptrs before deleting.
        intptr_t curr = fFreeListHead;
        while (kFreeListEnd != curr) {
            intptr_t next = reinterpret_cast<intptr_t>(fBuffers[SkToS32(curr)]);
            fBuffers[SkToS32(curr)] = nullptr;
            curr = next;
        }

        fBuffers.deleteAll();
    }

    BufferObj* lookUp(GrGLuint id) {
        BufferObj* buffer = fBuffers[id];
        SkASSERT(buffer && buffer->id() == id);
        return buffer;
    }

    BufferObj* create() {
        GrGLuint id;
        BufferObj* buffer;

        if (kFreeListEnd == fFreeListHead) {
            // no free slots - create a new one
            id = fBuffers.count();
            buffer = new BufferObj(id);
            *fBuffers.append() = buffer;
        } else {
            // grab the head of the free list and advance the head to the next free slot.
            id = static_cast<GrGLuint>(fFreeListHead);
            fFreeListHead = reinterpret_cast<intptr_t>(fBuffers[id]);

            buffer = new BufferObj(id);
            fBuffers[id] = buffer;
        }

        return buffer;
    }

    void free(BufferObj* buffer) {
        SkASSERT(fBuffers.count() > 0);

        GrGLuint id = buffer->id();
        delete buffer;

        fBuffers[id] = reinterpret_cast<BufferObj*>(fFreeListHead);
        fFreeListHead = id;
    }

private:
    static const intptr_t kFreeListEnd = -1;
    // Index of the first entry of fBuffers in the free list. Free slots in fBuffers are indices to
    // the next free slot. The last free slot has a value of kFreeListEnd.
    intptr_t                fFreeListHead;
    SkTDArray<BufferObj*>   fBuffers;
};

/**
 * The global-to-thread state object for the null interface. All null interfaces on the
 * same thread currently share one of these. This means two null contexts on the same thread
 * can interfere with each other. It may make sense to more integrate this into SkNullGLContext
 * and use it's makeCurrent mechanism.
 */
struct ThreadContext {
public:
    

    BufferManager   fBufferManager;
    GrGLuint        fCurrArrayBuffer;
    GrGLuint        fCurrElementArrayBuffer;
    GrGLuint        fCurrProgramID;
    GrGLuint        fCurrShaderID;

    static ThreadContext* Get() {
        return reinterpret_cast<ThreadContext*>(SkTLS::Get(Create, Delete));
    }

    ThreadContext()
        : fCurrArrayBuffer(0)
        , fCurrElementArrayBuffer(0)
        , fCurrProgramID(0)
        , fCurrShaderID(0) {}

private:
    static void* Create() { return new ThreadContext; }
    static void Delete(void* context) { delete reinterpret_cast<ThreadContext*>(context); }
};

// Functions not declared in GrGLBogusInterface.h (not common with the Debug GL interface).

GrGLvoid GR_GL_FUNCTION_TYPE nullGLActiveTexture(GrGLenum texture) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLAttachShader(GrGLuint program, GrGLuint shader) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBeginQuery(GrGLenum target, GrGLuint id) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindAttribLocation(GrGLuint program, GrGLuint index, const char* name) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindTexture(GrGLenum target, GrGLuint texture) {}
GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindVertexArray(GrGLuint id) {}

GrGLvoid GR_GL_FUNCTION_TYPE nullGLGenBuffers(GrGLsizei n, GrGLuint* ids) {
    ThreadContext* ctx = ThreadContext::Get();
    for (int i = 0; i < n; ++i) {
        BufferObj* buffer = ctx->fBufferManager.create();
        ids[i] = buffer->id();
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE nullGLGenerateMipmap(GrGLenum target) {}

GrGLvoid GR_GL_FUNCTION_TYPE nullGLBufferData(GrGLenum target,
                                              GrGLsizeiptr size,
                                              const GrGLvoid* data,
                                              GrGLenum usage) {
    ThreadContext* ctx = ThreadContext::Get();
    GrGLuint id = 0;

    switch (target) {
    case GR_GL_ARRAY_BUFFER:
        id = ctx->fCurrArrayBuffer;
        break;
    case GR_GL_ELEMENT_ARRAY_BUFFER:
        id = ctx->fCurrElementArrayBuffer;
        break;
    default:
        SkFAIL("Unexpected target to nullGLBufferData");
        break;
    }

    if (id > 0) {
        BufferObj* buffer = ctx->fBufferManager.lookUp(id);
        buffer->allocate(size, (const GrGLchar*) data);
    }
}

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
    return ++ThreadContext::Get()->fCurrProgramID;
}

GrGLuint GR_GL_FUNCTION_TYPE nullGLCreateShader(GrGLenum type) {
    return ++ThreadContext::Get()->fCurrShaderID;
}

// same delete used for shaders and programs
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDelete(GrGLuint program) {
}

GrGLvoid GR_GL_FUNCTION_TYPE nullGLBindBuffer(GrGLenum target, GrGLuint buffer) {
    ThreadContext* ctx = ThreadContext::Get();
    switch (target) {
    case GR_GL_ARRAY_BUFFER:
        ctx->fCurrArrayBuffer = buffer;
        break;
    case GR_GL_ELEMENT_ARRAY_BUFFER:
        ctx->fCurrElementArrayBuffer = buffer;
        break;
    }
}

// deleting a bound buffer has the side effect of binding 0
GrGLvoid GR_GL_FUNCTION_TYPE nullGLDeleteBuffers(GrGLsizei n, const GrGLuint* ids) {
    ThreadContext* ctx = ThreadContext::Get();
    for (int i = 0; i < n; ++i) {
        if (ids[i] == ctx->fCurrArrayBuffer) {
            ctx->fCurrArrayBuffer = 0;
        }
        if (ids[i] == ctx->fCurrElementArrayBuffer) {
            ctx->fCurrElementArrayBuffer = 0;
        }

        BufferObj* buffer = ctx->fBufferManager.lookUp(ids[i]);
        ctx->fBufferManager.free(buffer);
    }
}

GrGLvoid* GR_GL_FUNCTION_TYPE nullGLMapBufferRange(GrGLenum target, GrGLintptr offset,
                                                   GrGLsizeiptr length, GrGLbitfield access) {
    ThreadContext* ctx = ThreadContext::Get();
    GrGLuint id = 0;
    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            id = ctx->fCurrArrayBuffer;
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            id = ctx->fCurrElementArrayBuffer;
            break;
    }

    if (id > 0) {
        // We just ignore the offset and length here.
        BufferObj* buffer = ctx->fBufferManager.lookUp(id);
        SkASSERT(!buffer->mapped());
        buffer->setMapped(true);
        return buffer->dataPtr();
    }
    return nullptr;
}

GrGLvoid* GR_GL_FUNCTION_TYPE nullGLMapBuffer(GrGLenum target, GrGLenum access) {
    ThreadContext* ctx = ThreadContext::Get();
    GrGLuint id = 0;
    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            id = ctx->fCurrArrayBuffer;
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            id = ctx->fCurrElementArrayBuffer;
            break;
    }

    if (id > 0) {
        BufferObj* buffer = ctx->fBufferManager.lookUp(id);
        SkASSERT(!buffer->mapped());
        buffer->setMapped(true);
        return buffer->dataPtr();
    }

    SkASSERT(false);
    return nullptr;            // no buffer bound to target
}

GrGLvoid GR_GL_FUNCTION_TYPE nullGLFlushMappedBufferRange(GrGLenum target,
                                                          GrGLintptr offset,
                                                          GrGLsizeiptr length) {}


GrGLboolean GR_GL_FUNCTION_TYPE nullGLUnmapBuffer(GrGLenum target) {
    ThreadContext* ctx = ThreadContext::Get();
    GrGLuint id = 0;
    switch (target) {
    case GR_GL_ARRAY_BUFFER:
        id = ctx->fCurrArrayBuffer;
        break;
    case GR_GL_ELEMENT_ARRAY_BUFFER:
        id = ctx->fCurrElementArrayBuffer;
        break;
    }
    if (id > 0) {
        BufferObj* buffer = ctx->fBufferManager.lookUp(id);
        SkASSERT(buffer->mapped());
        buffer->setMapped(false);
        return GR_GL_TRUE;
    }

    GrAlwaysAssert(false);
    return GR_GL_FALSE; // GR_GL_INVALID_OPERATION;
}

GrGLvoid GR_GL_FUNCTION_TYPE nullGLGetBufferParameteriv(GrGLenum target, GrGLenum pname, GrGLint* params) {
    ThreadContext* ctx = ThreadContext::Get();
    switch (pname) {
        case GR_GL_BUFFER_MAPPED: {
            *params = GR_GL_FALSE;
            GrGLuint id = 0;
            switch (target) {
                case GR_GL_ARRAY_BUFFER:
                    id = ctx->fCurrArrayBuffer;
                    break;
                case GR_GL_ELEMENT_ARRAY_BUFFER:
                    id = ctx->fCurrElementArrayBuffer;
                    break;
            }
            if (id > 0) {
                BufferObj* buffer = ctx->fBufferManager.lookUp(id);
                if (buffer->mapped()) {
                    *params = GR_GL_TRUE;
                }
            }
            break; }
        default:
            SkFAIL("Unexpected pname to GetBufferParamateriv");
            break;
    }
};

} // end anonymous namespace

const GrGLInterface* GrGLCreateNullInterface() {
    GrGLInterface* interface = new GrGLInterface;

    interface->fStandard = kGL_GrGLStandard;

    GrGLInterface::Functions* functions = &interface->fFunctions;
    functions->fActiveTexture = nullGLActiveTexture;
    functions->fAttachShader = nullGLAttachShader;
    functions->fBeginQuery = nullGLBeginQuery;
    functions->fBindAttribLocation = nullGLBindAttribLocation;
    functions->fBindBuffer = nullGLBindBuffer;
    functions->fBindFragDataLocation = noOpGLBindFragDataLocation;
    functions->fBindTexture = nullGLBindTexture;
    functions->fBindVertexArray = nullGLBindVertexArray;
    functions->fBlendColor = noOpGLBlendColor;
    functions->fBlendEquation = noOpGLBlendEquation;
    functions->fBlendFunc = noOpGLBlendFunc;
    functions->fBufferData = nullGLBufferData;
    functions->fBufferSubData = noOpGLBufferSubData;
    functions->fClear = noOpGLClear;
    functions->fClearColor = noOpGLClearColor;
    functions->fClearStencil = noOpGLClearStencil;
    functions->fColorMask = noOpGLColorMask;
    functions->fCompileShader = noOpGLCompileShader;
    functions->fCompressedTexImage2D = noOpGLCompressedTexImage2D;
    functions->fCompressedTexSubImage2D = noOpGLCompressedTexSubImage2D;
    functions->fCopyTexSubImage2D = noOpGLCopyTexSubImage2D;
    functions->fCreateProgram = nullGLCreateProgram;
    functions->fCreateShader = nullGLCreateShader;
    functions->fCullFace = noOpGLCullFace;
    functions->fDeleteBuffers = nullGLDeleteBuffers;
    functions->fDeleteProgram = nullGLDelete;
    functions->fDeleteQueries = noOpGLDeleteIds;
    functions->fDeleteShader = nullGLDelete;
    functions->fDeleteTextures = noOpGLDeleteIds;
    functions->fDeleteVertexArrays = noOpGLDeleteIds;
    functions->fDepthMask = noOpGLDepthMask;
    functions->fDisable = noOpGLDisable;
    functions->fDisableVertexAttribArray = noOpGLDisableVertexAttribArray;
    functions->fDrawArrays = noOpGLDrawArrays;
    functions->fDrawArraysInstanced = noOpGLDrawArraysInstanced;
    functions->fDrawBuffer = noOpGLDrawBuffer;
    functions->fDrawBuffers = noOpGLDrawBuffers;
    functions->fDrawElements = noOpGLDrawElements;
    functions->fDrawElementsInstanced = noOpGLDrawElementsInstanced;
    functions->fEnable = noOpGLEnable;
    functions->fEnableVertexAttribArray = noOpGLEnableVertexAttribArray;
    functions->fEndQuery = noOpGLEndQuery;
    functions->fFinish = noOpGLFinish;
    functions->fFlush = noOpGLFlush;
    functions->fFlushMappedBufferRange = nullGLFlushMappedBufferRange;
    functions->fFrontFace = noOpGLFrontFace;
    functions->fGenBuffers = nullGLGenBuffers;
    functions->fGenerateMipmap = nullGLGenerateMipmap;
    functions->fGenQueries = noOpGLGenIds;
    functions->fGenTextures = noOpGLGenIds;
    functions->fGenVertexArrays = noOpGLGenIds;
    functions->fGetBufferParameteriv = nullGLGetBufferParameteriv;
    functions->fGetError = noOpGLGetError;
    functions->fGetIntegerv = noOpGLGetIntegerv;
    functions->fGetQueryObjecti64v = noOpGLGetQueryObjecti64v;
    functions->fGetQueryObjectiv = noOpGLGetQueryObjectiv;
    functions->fGetQueryObjectui64v = noOpGLGetQueryObjectui64v;
    functions->fGetQueryObjectuiv = noOpGLGetQueryObjectuiv;
    functions->fGetQueryiv = noOpGLGetQueryiv;
    functions->fGetProgramInfoLog = noOpGLGetInfoLog;
    functions->fGetProgramiv = noOpGLGetShaderOrProgramiv;
    functions->fGetShaderInfoLog = noOpGLGetInfoLog;
    functions->fGetShaderiv = noOpGLGetShaderOrProgramiv;
    functions->fGetString = noOpGLGetString;
    functions->fGetStringi = noOpGLGetStringi;
    functions->fGetTexLevelParameteriv = noOpGLGetTexLevelParameteriv;
    functions->fGetUniformLocation = noOpGLGetUniformLocation;
    functions->fInsertEventMarker = noOpGLInsertEventMarker;
    functions->fLineWidth = noOpGLLineWidth;
    functions->fLinkProgram = noOpGLLinkProgram;
    functions->fMapBuffer = nullGLMapBuffer;
    functions->fMapBufferRange = nullGLMapBufferRange;
    functions->fPixelStorei = nullGLPixelStorei;
    functions->fPopGroupMarker = noOpGLPopGroupMarker;
    functions->fPushGroupMarker = noOpGLPushGroupMarker;
    functions->fQueryCounter = noOpGLQueryCounter;
    functions->fReadBuffer = noOpGLReadBuffer;
    functions->fReadPixels = nullGLReadPixels;
    functions->fScissor = noOpGLScissor;
    functions->fShaderSource = noOpGLShaderSource;
    functions->fStencilFunc = noOpGLStencilFunc;
    functions->fStencilFuncSeparate = noOpGLStencilFuncSeparate;
    functions->fStencilMask = noOpGLStencilMask;
    functions->fStencilMaskSeparate = noOpGLStencilMaskSeparate;
    functions->fStencilOp = noOpGLStencilOp;
    functions->fStencilOpSeparate = noOpGLStencilOpSeparate;
    functions->fTexImage2D = noOpGLTexImage2D;
    functions->fTexParameteri = noOpGLTexParameteri;
    functions->fTexParameteriv = noOpGLTexParameteriv;
    functions->fTexSubImage2D = noOpGLTexSubImage2D;
    functions->fTexStorage2D = noOpGLTexStorage2D;
    functions->fDiscardFramebuffer = noOpGLDiscardFramebuffer;
    functions->fUniform1f = noOpGLUniform1f;
    functions->fUniform1i = noOpGLUniform1i;
    functions->fUniform1fv = noOpGLUniform1fv;
    functions->fUniform1iv = noOpGLUniform1iv;
    functions->fUniform2f = noOpGLUniform2f;
    functions->fUniform2i = noOpGLUniform2i;
    functions->fUniform2fv = noOpGLUniform2fv;
    functions->fUniform2iv = noOpGLUniform2iv;
    functions->fUniform3f = noOpGLUniform3f;
    functions->fUniform3i = noOpGLUniform3i;
    functions->fUniform3fv = noOpGLUniform3fv;
    functions->fUniform3iv = noOpGLUniform3iv;
    functions->fUniform4f = noOpGLUniform4f;
    functions->fUniform4i = noOpGLUniform4i;
    functions->fUniform4fv = noOpGLUniform4fv;
    functions->fUniform4iv = noOpGLUniform4iv;
    functions->fUniformMatrix2fv = noOpGLUniformMatrix2fv;
    functions->fUniformMatrix3fv = noOpGLUniformMatrix3fv;
    functions->fUniformMatrix4fv = noOpGLUniformMatrix4fv;
    functions->fUnmapBuffer = nullGLUnmapBuffer;
    functions->fUseProgram = nullGLUseProgram;
    functions->fVertexAttrib1f = noOpGLVertexAttrib1f;
    functions->fVertexAttrib2fv = noOpGLVertexAttrib2fv;
    functions->fVertexAttrib3fv = noOpGLVertexAttrib3fv;
    functions->fVertexAttrib4fv = noOpGLVertexAttrib4fv;
    functions->fVertexAttribPointer = noOpGLVertexAttribPointer;
    functions->fVertexAttribDivisor = noOpGLVertexAttribDivisor;
    functions->fViewport = nullGLViewport;
    functions->fBindFramebuffer = nullGLBindFramebuffer;
    functions->fBindRenderbuffer = nullGLBindRenderbuffer;
    functions->fCheckFramebufferStatus = noOpGLCheckFramebufferStatus;
    functions->fDeleteFramebuffers = nullGLDeleteFramebuffers;
    functions->fDeleteRenderbuffers = nullGLDeleteRenderbuffers;
    functions->fFramebufferRenderbuffer = nullGLFramebufferRenderbuffer;
    functions->fFramebufferTexture2D = nullGLFramebufferTexture2D;
    functions->fGenFramebuffers = noOpGLGenIds;
    functions->fGenRenderbuffers = noOpGLGenIds;
    functions->fGetFramebufferAttachmentParameteriv = noOpGLGetFramebufferAttachmentParameteriv;
    functions->fGetRenderbufferParameteriv = noOpGLGetRenderbufferParameteriv;
    functions->fRenderbufferStorage = noOpGLRenderbufferStorage;
    functions->fRenderbufferStorageMultisample = noOpGLRenderbufferStorageMultisample;
    functions->fBlitFramebuffer = noOpGLBlitFramebuffer;
    functions->fResolveMultisampleFramebuffer = noOpGLResolveMultisampleFramebuffer;
    functions->fMatrixLoadf = noOpGLMatrixLoadf;
    functions->fMatrixLoadIdentity = noOpGLMatrixLoadIdentity;
    functions->fBindFragDataLocationIndexed = noOpGLBindFragDataLocationIndexed;

    interface->fExtensions.init(kGL_GrGLStandard, functions->fGetString, functions->fGetStringi,
                                functions->fGetIntegerv, nullptr, GR_EGL_NO_DISPLAY);
    return interface;
}
