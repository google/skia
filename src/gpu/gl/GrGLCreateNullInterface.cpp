/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "GrGLTestInterface.h"
#include "SkMutex.h"
#include "SkTDArray.h"

// added to suppress 'no previous prototype' warning and because this code is duplicated in
// SkNullGLContext.cpp
namespace {

class BufferObj {
public:
    BufferObj(GrGLuint id) : fID(id), fDataPtr(nullptr), fSize(0), fMapped(false) {}
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
    BufferManager() : fFreeListHead(kFreeListEnd) {
        *fBuffers.append() = nullptr; // 0 is not a valid GL buffer id.
    }

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
        SkASSERT(buffer);
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

/** Null interface implementation */
class NullInterface : public GrGLTestInterface {
public:
    NullInterface()
        : fCurrArrayBuffer(0)
        , fCurrElementArrayBuffer(0)
        , fCurrPixelPackBuffer(0)
        , fCurrPixelUnpackBuffer(0)
        , fCurrProgramID(0)
        , fCurrShaderID(0)
        , fCurrGenericID(0)
        , fCurrUniformLocation(0) {
        this->init(kGL_GrGLStandard);
    }

    GrGLenum checkFramebufferStatus(GrGLenum target) override {
        return GR_GL_FRAMEBUFFER_COMPLETE;
    }

    GrGLvoid genBuffers(GrGLsizei n, GrGLuint* ids) override {
        for (int i = 0; i < n; ++i) {
            BufferObj* buffer = fBufferManager.create();
            ids[i] = buffer->id();
        }
    }

    GrGLvoid bufferData(GrGLenum target, GrGLsizeiptr size, const GrGLvoid* data,
                        GrGLenum usage) override {
        GrGLuint id = 0;

        switch (target) {
            case GR_GL_ARRAY_BUFFER:
                id = fCurrArrayBuffer;
                break;
            case GR_GL_ELEMENT_ARRAY_BUFFER:
                id = fCurrElementArrayBuffer;
                break;
            case GR_GL_PIXEL_PACK_BUFFER:
                id = fCurrPixelPackBuffer;
                break;
            case GR_GL_PIXEL_UNPACK_BUFFER:
                id = fCurrPixelUnpackBuffer;
                break;
            default:
                SkFAIL("Unexpected target to nullGLBufferData");
                break;
        }

        if (id > 0) {
            BufferObj* buffer = fBufferManager.lookUp(id);
            buffer->allocate(size, (const GrGLchar*) data);
        }
    }

    GrGLuint createProgram() override {
        return ++fCurrProgramID;
    }

    GrGLuint createShader(GrGLenum type) override {
        return ++fCurrShaderID;
    }

    GrGLvoid bindBuffer(GrGLenum target, GrGLuint buffer) override {
        switch (target) {
            case GR_GL_ARRAY_BUFFER:
                fCurrArrayBuffer = buffer;
                break;
            case GR_GL_ELEMENT_ARRAY_BUFFER:
                fCurrElementArrayBuffer = buffer;
                break;
            case GR_GL_PIXEL_PACK_BUFFER:
                fCurrPixelPackBuffer = buffer;
                break;
            case GR_GL_PIXEL_UNPACK_BUFFER:
                fCurrPixelUnpackBuffer = buffer;
                break;
        }
    }

   // deleting a bound buffer has the side effect of binding 0
    GrGLvoid deleteBuffers(GrGLsizei n, const GrGLuint* ids) override {
        for (int i = 0; i < n; ++i) {
            if (ids[i] == fCurrArrayBuffer) {
                fCurrArrayBuffer = 0;
            }
            if (ids[i] == fCurrElementArrayBuffer) {
                fCurrElementArrayBuffer = 0;
            }
            if (ids[i] == fCurrPixelPackBuffer) {
                fCurrPixelPackBuffer = 0;
            }
            if (ids[i] == fCurrPixelUnpackBuffer) {
                fCurrPixelUnpackBuffer = 0;
            }

            if (ids[i] > 0) {
                BufferObj* buffer = fBufferManager.lookUp(ids[i]);
                fBufferManager.free(buffer);
            }
        }
    }

    GrGLvoid genFramebuffers(GrGLsizei n, GrGLuint *framebuffers) override {
        this->genGenericIds(n, framebuffers);
    }

    GrGLvoid genQueries(GrGLsizei n, GrGLuint *ids) override { this->genGenericIds(n, ids); }

    GrGLvoid genRenderbuffers(GrGLsizei n, GrGLuint *renderbuffers) override {
        this->genGenericIds(n, renderbuffers);
    }

    GrGLvoid genTextures(GrGLsizei n, GrGLuint *textures) override {
        this->genGenericIds(n, textures);
    }

    GrGLvoid genVertexArrays(GrGLsizei n, GrGLuint *arrays) override {
        this->genGenericIds(n, arrays);
    }

    GrGLenum getError() override { return GR_GL_NO_ERROR; }

    GrGLvoid getIntegerv(GrGLenum pname, GrGLint* params) override {
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
            case GR_GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
            case GR_GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS:
            case GR_GL_MAX_TEXTURE_IMAGE_UNITS:
            case GR_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
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
            case GR_GL_NUM_EXTENSIONS: {
                GrGLint i = 0;
                while (kExtensions[i++]);
                *params = i;
                break;
            }
            default:
                SkFAIL("Unexpected pname to GetIntegerv");
        }
    }

    GrGLvoid getProgramiv(GrGLuint program, GrGLenum pname, GrGLint* params) override {
        this->getShaderOrProgramiv(program, pname, params);
    }

    GrGLvoid getProgramInfoLog(GrGLuint program, GrGLsizei bufsize, GrGLsizei* length,
                               char* infolog) override {
        this->getInfoLog(program, bufsize, length, infolog);
    }

    GrGLvoid getMultisamplefv(GrGLenum pname, GrGLuint index, GrGLfloat* val) override {
        val[0] = val[1] = 0.5f;
    }

    GrGLvoid getQueryiv(GrGLenum GLtarget, GrGLenum pname, GrGLint *params) override {
        switch (pname) {
            case GR_GL_CURRENT_QUERY:
                *params = 0;
                break;
            case GR_GL_QUERY_COUNTER_BITS:
                *params = 32;
                break;
            default:
                SkFAIL("Unexpected pname passed GetQueryiv.");
        }
    }

    GrGLvoid getQueryObjecti64v(GrGLuint id, GrGLenum pname, GrGLint64 *params) override {
        this->queryResult(id, pname, params);
    }

    GrGLvoid getQueryObjectiv(GrGLuint id, GrGLenum pname, GrGLint *params) override {
        this->queryResult(id, pname, params);
    }

    GrGLvoid getQueryObjectui64v(GrGLuint id, GrGLenum pname, GrGLuint64 *params) override {
        this->queryResult(id, pname, params);
    }

    GrGLvoid getQueryObjectuiv(GrGLuint id, GrGLenum pname, GrGLuint *params) override {
        this->queryResult(id, pname, params);
    }

    GrGLvoid getShaderiv(GrGLuint shader, GrGLenum pname, GrGLint* params) override {
        this->getShaderOrProgramiv(shader, pname, params);
    }

    GrGLvoid getShaderInfoLog(GrGLuint shader, GrGLsizei bufsize, GrGLsizei* length,
                              char* infolog) override {
        this->getInfoLog(shader, bufsize, length, infolog);
    }

    const GrGLubyte* getString(GrGLenum name) override {
        switch (name) {
            case GR_GL_EXTENSIONS:
                return CombinedExtensionString();
            case GR_GL_VERSION:
                return (const GrGLubyte*)"4.0 Null GL";
            case GR_GL_SHADING_LANGUAGE_VERSION:
                return (const GrGLubyte*)"4.20.8 Null GLSL";
            case GR_GL_VENDOR:
                return (const GrGLubyte*)"Null Vendor";
            case GR_GL_RENDERER:
                return (const GrGLubyte*)"The Null (Non-)Renderer";
            default:
                SkFAIL("Unexpected name passed to GetString");
                return nullptr;
        }
    }

    const GrGLubyte* getStringi(GrGLenum name, GrGLuint i) override {
        switch (name) {
            case GR_GL_EXTENSIONS: {
                GrGLint count;
                this->getIntegerv(GR_GL_NUM_EXTENSIONS, &count);
                if ((GrGLint)i <= count) {
                    return (const GrGLubyte*) kExtensions[i];
                } else {
                    return nullptr;
                }
            }
            default:
                SkFAIL("Unexpected name passed to GetStringi");
                return nullptr;
        }
    }

    GrGLint getUniformLocation(GrGLuint program, const char* name) override {
        return ++fCurrUniformLocation;
    }

    GrGLvoid* mapBufferRange(GrGLenum target, GrGLintptr offset, GrGLsizeiptr length,
                             GrGLbitfield access) override {
        GrGLuint id = 0;
        switch (target) {
            case GR_GL_ARRAY_BUFFER:
                id = fCurrArrayBuffer;
                break;
            case GR_GL_ELEMENT_ARRAY_BUFFER:
                id = fCurrElementArrayBuffer;
                break;
            case GR_GL_PIXEL_PACK_BUFFER:
                id = fCurrPixelPackBuffer;
                break;
            case GR_GL_PIXEL_UNPACK_BUFFER:
                id = fCurrPixelUnpackBuffer;
                break;
        }

        if (id > 0) {
            // We just ignore the offset and length here.
            BufferObj* buffer = fBufferManager.lookUp(id);
            SkASSERT(!buffer->mapped());
            buffer->setMapped(true);
            return buffer->dataPtr();
        }
        return nullptr;
    }

    GrGLvoid* mapBuffer(GrGLenum target, GrGLenum access) override {
        GrGLuint id = 0;
        switch (target) {
            case GR_GL_ARRAY_BUFFER:
                id = fCurrArrayBuffer;
                break;
            case GR_GL_ELEMENT_ARRAY_BUFFER:
                id = fCurrElementArrayBuffer;
                break;
            case GR_GL_PIXEL_PACK_BUFFER:
                id = fCurrPixelPackBuffer;
                break;
            case GR_GL_PIXEL_UNPACK_BUFFER:
                id = fCurrPixelUnpackBuffer;
                break;
        }

        if (id > 0) {
            BufferObj* buffer = fBufferManager.lookUp(id);
            SkASSERT(!buffer->mapped());
            buffer->setMapped(true);
            return buffer->dataPtr();
        }

        SkASSERT(false);
        return nullptr;            // no buffer bound to target
    }

    GrGLboolean unmapBuffer(GrGLenum target) override {
        GrGLuint id = 0;
        switch (target) {
            case GR_GL_ARRAY_BUFFER:
                id = fCurrArrayBuffer;
                break;
            case GR_GL_ELEMENT_ARRAY_BUFFER:
                id = fCurrElementArrayBuffer;
                break;
            case GR_GL_PIXEL_PACK_BUFFER:
                id = fCurrPixelPackBuffer;
                break;
            case GR_GL_PIXEL_UNPACK_BUFFER:
                id = fCurrPixelUnpackBuffer;
                break;
        }
        if (id > 0) {
            BufferObj* buffer = fBufferManager.lookUp(id);
            SkASSERT(buffer->mapped());
            buffer->setMapped(false);
            return GR_GL_TRUE;
        }

        GrAlwaysAssert(false);
        return GR_GL_FALSE; // GR_GL_INVALID_OPERATION;
    }

    GrGLvoid getBufferParameteriv(GrGLenum target, GrGLenum pname, GrGLint* params) override {
        switch (pname) {
            case GR_GL_BUFFER_MAPPED: {
                *params = GR_GL_FALSE;
                GrGLuint id = 0;
                switch (target) {
                    case GR_GL_ARRAY_BUFFER:
                        id = fCurrArrayBuffer;
                        break;
                    case GR_GL_ELEMENT_ARRAY_BUFFER:
                        id = fCurrElementArrayBuffer;
                        break;
                    case GR_GL_PIXEL_PACK_BUFFER:
                        id = fCurrPixelPackBuffer;
                        break;
                    case GR_GL_PIXEL_UNPACK_BUFFER:
                        id = fCurrPixelUnpackBuffer;
                        break;
                }
                if (id > 0) {
                    BufferObj* buffer = fBufferManager.lookUp(id);
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

private:
    BufferManager   fBufferManager;
    GrGLuint        fCurrArrayBuffer;
    GrGLuint        fCurrElementArrayBuffer;
    GrGLuint        fCurrPixelPackBuffer;
    GrGLuint        fCurrPixelUnpackBuffer;
    GrGLuint        fCurrProgramID;
    GrGLuint        fCurrShaderID;
    GrGLuint        fCurrGenericID;
    GrGLuint        fCurrUniformLocation;

    // the OpenGLES 2.0 spec says this must be >= 128
    static const GrGLint kDefaultMaxVertexUniformVectors = 128;

    // the OpenGLES 2.0 spec says this must be >=16
    static const GrGLint kDefaultMaxFragmentUniformVectors = 16;

    // the OpenGLES 2.0 spec says this must be >= 8
    static const GrGLint kDefaultMaxVertexAttribs = 8;

    // the OpenGLES 2.0 spec says this must be >= 8
    static const GrGLint kDefaultMaxVaryingVectors = 8;

    static const char* kExtensions[];

    static const GrGLubyte* CombinedExtensionString() {
        static SkString gExtString;
        static SkMutex gMutex;
        gMutex.acquire();
        if (0 == gExtString.size()) {
            int i = 0;
            while (kExtensions[i]) {
                if (i > 0) {
                    gExtString.append(" ");
                }
                gExtString.append(kExtensions[i]);
                ++i;
            }
        }
        gMutex.release();
        return (const GrGLubyte*) gExtString.c_str();
    }

    GrGLvoid genGenericIds(GrGLsizei n, GrGLuint* ids) {
        for (int i = 0; i < n; ++i) {
            ids[i] = ++fCurrGenericID;
        }
    }

    GrGLvoid getInfoLog(GrGLuint object, GrGLsizei bufsize, GrGLsizei* length,
                        char* infolog) {
        if (length) {
            *length = 0;
        }
        if (bufsize > 0) {
            *infolog = 0;
        }
    }

    GrGLvoid getShaderOrProgramiv(GrGLuint object,  GrGLenum pname, GrGLint* params) {
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
                SkFAIL("Unexpected pname to GetProgramiv");
                break;
        }
    }

    template <typename T>
    void queryResult(GrGLenum GLtarget, GrGLenum pname, T *params) {
        switch (pname) {
            case GR_GL_QUERY_RESULT_AVAILABLE:
                *params = GR_GL_TRUE;
                break;
            case GR_GL_QUERY_RESULT:
                *params = 0;
                break;
            default:
                SkFAIL("Unexpected pname passed to GetQueryObject.");
                break;
        }
    }

    typedef GrGLTestInterface INHERITED;
};

const char* NullInterface::kExtensions[] = {
    "GL_ARB_framebuffer_object",
    "GL_ARB_blend_func_extended",
    "GL_ARB_timer_query",
    "GL_ARB_draw_buffers",
    "GL_ARB_occlusion_query",
    "GL_EXT_stencil_wrap",
    nullptr, // signifies the end of the array.
};

}  // anonymous namespace

const GrGLInterface* GrGLCreateNullInterface() { return new NullInterface; }
