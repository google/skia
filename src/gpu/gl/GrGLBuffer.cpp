/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLBuffer.h"
#include "GrGLGpu.h"
#include "SkTraceMemoryDump.h"

#define GL_CALL(X) GR_GL_CALL(this->glGpu()->glInterface(), X)
#define GL_CALL_RET(RET, X) GR_GL_CALL_RET(this->glGpu()->glInterface(), RET, X)

#if GR_GL_CHECK_ALLOC_WITH_GET_ERROR
    #define CLEAR_ERROR_BEFORE_ALLOC(iface)   GrGLClearErr(iface)
    #define GL_ALLOC_CALL(iface, call)        GR_GL_CALL_NOERRCHECK(iface, call)
    #define CHECK_ALLOC_ERROR(iface)          GR_GL_GET_ERROR(iface)
#else
    #define CLEAR_ERROR_BEFORE_ALLOC(iface)
    #define GL_ALLOC_CALL(iface, call)        GR_GL_CALL(iface, call)
    #define CHECK_ALLOC_ERROR(iface)          GR_GL_NO_ERROR
#endif

#ifdef SK_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

GrGLBuffer* GrGLBuffer::Create(GrGLGpu* gpu, size_t size, GrBufferType intendedType,
                               GrAccessPattern accessPattern, const void* data) {
    bool cpuBacked = gpu->glCaps().useNonVBOVertexAndIndexDynamicData() &&
                     GrBufferTypeIsVertexOrIndex(intendedType) &&
                     kDynamic_GrAccessPattern == accessPattern;
    SkAutoTUnref<GrGLBuffer> buffer(new GrGLBuffer(gpu, size, intendedType, accessPattern,
                                                   cpuBacked, data));
    if (!cpuBacked && 0 == buffer->bufferID()) {
        return nullptr;
    }
    return buffer.release();
}

// GL_STREAM_DRAW triggers an optimization in Chromium's GPU process where a client's vertex buffer
// objects are implemented as client-side-arrays on tile-deferred architectures.
#define DYNAMIC_DRAW_PARAM GR_GL_STREAM_DRAW

inline static GrGLenum gr_to_gl_access_pattern(GrBufferType bufferType,
                                               GrAccessPattern accessPattern) {
    static const GrGLenum drawUsages[] = {
        DYNAMIC_DRAW_PARAM,  // TODO: Do we really want to use STREAM_DRAW here on non-Chromium?
        GR_GL_STATIC_DRAW,   // kStatic_GrAccessPattern
        GR_GL_STREAM_DRAW    // kStream_GrAccessPattern
    };

    static const GrGLenum readUsages[] = {
        GR_GL_DYNAMIC_READ,  // kDynamic_GrAccessPattern
        GR_GL_STATIC_READ,   // kStatic_GrAccessPattern
        GR_GL_STREAM_READ    // kStream_GrAccessPattern
    };

    GR_STATIC_ASSERT(0 == kDynamic_GrAccessPattern);
    GR_STATIC_ASSERT(1 == kStatic_GrAccessPattern);
    GR_STATIC_ASSERT(2 == kStream_GrAccessPattern);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(drawUsages) == 1 + kLast_GrAccessPattern);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(readUsages) == 1 + kLast_GrAccessPattern);

    static GrGLenum const* const usageTypes[] = {
        drawUsages,  // kVertex_GrBufferType,
        drawUsages,  // kIndex_GrBufferType,
        drawUsages,  // kTexel_GrBufferType,
        drawUsages,  // kDrawIndirect_GrBufferType,
        drawUsages,  // kXferCpuToGpu_GrBufferType,
        readUsages   // kXferGpuToCpu_GrBufferType,
    };

    GR_STATIC_ASSERT(0 == kVertex_GrBufferType);
    GR_STATIC_ASSERT(1 == kIndex_GrBufferType);
    GR_STATIC_ASSERT(2 == kTexel_GrBufferType);
    GR_STATIC_ASSERT(3 == kDrawIndirect_GrBufferType);
    GR_STATIC_ASSERT(4 == kXferCpuToGpu_GrBufferType);
    GR_STATIC_ASSERT(5 == kXferGpuToCpu_GrBufferType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(usageTypes) == kGrBufferTypeCount);

    SkASSERT(bufferType >= 0 && bufferType <= kLast_GrBufferType);
    SkASSERT(accessPattern >= 0 && accessPattern <= kLast_GrAccessPattern);

    return usageTypes[bufferType][accessPattern];
}

GrGLBuffer::GrGLBuffer(GrGLGpu* gpu, size_t size, GrBufferType intendedType,
                       GrAccessPattern accessPattern, bool cpuBacked, const void* data)
    : INHERITED(gpu, size, intendedType, accessPattern, cpuBacked),
      fCPUData(nullptr),
      fIntendedType(intendedType),
      fBufferID(0),
      fSizeInBytes(size),
      fUsage(gr_to_gl_access_pattern(intendedType, accessPattern)),
      fGLSizeInBytes(0) {
    if (this->isCPUBacked()) {
        // Core profile uses vertex array objects, which disallow client side arrays.
        SkASSERT(!gpu->glCaps().isCoreProfile());
        if (gpu->caps()->mustClearUploadedBufferData()) {
            fCPUData = sk_calloc_throw(fSizeInBytes);
        } else {
            fCPUData = sk_malloc_flags(fSizeInBytes, SK_MALLOC_THROW);
        }
        if (data) {
            memcpy(fCPUData, data, fSizeInBytes);
        }
    } else {
        GL_CALL(GenBuffers(1, &fBufferID));
        if (fBufferID) {
            GrGLenum target = gpu->bindBuffer(fIntendedType, this);
            CLEAR_ERROR_BEFORE_ALLOC(gpu->glInterface());
            // make sure driver can allocate memory for this buffer
            GL_ALLOC_CALL(gpu->glInterface(), BufferData(target,
                                                         (GrGLsizeiptr) fSizeInBytes,
                                                         data,
                                                         fUsage));
            if (CHECK_ALLOC_ERROR(gpu->glInterface()) != GR_GL_NO_ERROR) {
                GL_CALL(DeleteBuffers(1, &fBufferID));
                fBufferID = 0;
            } else {
                fGLSizeInBytes = fSizeInBytes;
            }
        }
    }
    VALIDATE();
    this->registerWithCache();
}

inline GrGLGpu* GrGLBuffer::glGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrGLGpu*>(this->getGpu());
}

inline const GrGLCaps& GrGLBuffer::glCaps() const {
    return this->glGpu()->glCaps();
}

void GrGLBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        VALIDATE();
        // make sure we've not been abandoned or already released
        if (fCPUData) {
            SkASSERT(!fBufferID);
            sk_free(fCPUData);
            fCPUData = nullptr;
        } else if (fBufferID) {
            GL_CALL(DeleteBuffers(1, &fBufferID));
            fBufferID = 0;
            fGLSizeInBytes = 0;
        }
        fMapPtr = nullptr;
        VALIDATE();
    }

    INHERITED::onRelease();
}

void GrGLBuffer::onAbandon() {
    fBufferID = 0;
    fGLSizeInBytes = 0;
    fMapPtr = nullptr;
    sk_free(fCPUData);
    fCPUData = nullptr;
    VALIDATE();
    INHERITED::onAbandon();
}

void GrGLBuffer::onMap() {
    if (this->wasDestroyed()) {
        return;
    }

    VALIDATE();
    SkASSERT(!this->isMapped());

    if (0 == fBufferID) {
        fMapPtr = fCPUData;
        VALIDATE();
        return;
    }

    // TODO: Make this a function parameter.
    bool readOnly = (kXferGpuToCpu_GrBufferType == fIntendedType);

    // Handling dirty context is done in the bindBuffer call
    switch (this->glCaps().mapBufferType()) {
        case GrGLCaps::kNone_MapBufferType:
            break;
        case GrGLCaps::kMapBuffer_MapBufferType: {
            GrGLenum target = this->glGpu()->bindBuffer(fIntendedType, this);
            // Let driver know it can discard the old data
            if (GR_GL_USE_BUFFER_DATA_NULL_HINT || fGLSizeInBytes != fSizeInBytes) {
                GL_CALL(BufferData(target, fSizeInBytes, nullptr, fUsage));
            }
            GL_CALL_RET(fMapPtr, MapBuffer(target, readOnly ? GR_GL_READ_ONLY : GR_GL_WRITE_ONLY));
            break;
        }
        case GrGLCaps::kMapBufferRange_MapBufferType: {
            GrGLenum target = this->glGpu()->bindBuffer(fIntendedType, this);
            // Make sure the GL buffer size agrees with fDesc before mapping.
            if (fGLSizeInBytes != fSizeInBytes) {
                GL_CALL(BufferData(target, fSizeInBytes, nullptr, fUsage));
            }
            GrGLbitfield writeAccess = GR_GL_MAP_WRITE_BIT;
            if (kXferCpuToGpu_GrBufferType != fIntendedType) {
                // TODO: Make this a function parameter.
                writeAccess |= GR_GL_MAP_INVALIDATE_BUFFER_BIT;
            }
            GL_CALL_RET(fMapPtr, MapBufferRange(target, 0, fSizeInBytes,
                                                readOnly ?  GR_GL_MAP_READ_BIT : writeAccess));
            break;
        }
        case GrGLCaps::kChromium_MapBufferType: {
            GrGLenum target = this->glGpu()->bindBuffer(fIntendedType, this);
            // Make sure the GL buffer size agrees with fDesc before mapping.
            if (fGLSizeInBytes != fSizeInBytes) {
                GL_CALL(BufferData(target, fSizeInBytes, nullptr, fUsage));
            }
            GL_CALL_RET(fMapPtr, MapBufferSubData(target, 0, fSizeInBytes,
                                                  readOnly ?  GR_GL_READ_ONLY : GR_GL_WRITE_ONLY));
            break;
        }
    }
    fGLSizeInBytes = fSizeInBytes;
    VALIDATE();
}

void GrGLBuffer::onUnmap() {
    if (this->wasDestroyed()) {
        return;
    }

    VALIDATE();
    SkASSERT(this->isMapped());
    if (0 == fBufferID) {
        fMapPtr = nullptr;
        return;
    }
    // bind buffer handles the dirty context
    switch (this->glCaps().mapBufferType()) {
        case GrGLCaps::kNone_MapBufferType:
            SkDEBUGFAIL("Shouldn't get here.");
            return;
        case GrGLCaps::kMapBuffer_MapBufferType: // fall through
        case GrGLCaps::kMapBufferRange_MapBufferType: {
            GrGLenum target = this->glGpu()->bindBuffer(fIntendedType, this);
            GL_CALL(UnmapBuffer(target));
            break;
        }
        case GrGLCaps::kChromium_MapBufferType:
            this->glGpu()->bindBuffer(fIntendedType, this); // TODO: Is this needed?
            GL_CALL(UnmapBufferSubData(fMapPtr));
            break;
    }
    fMapPtr = nullptr;
}

bool GrGLBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    if (this->wasDestroyed()) {
        return false;
    }

    SkASSERT(!this->isMapped());
    VALIDATE();
    if (srcSizeInBytes > fSizeInBytes) {
        return false;
    }
    if (0 == fBufferID) {
        memcpy(fCPUData, src, srcSizeInBytes);
        return true;
    }
    SkASSERT(srcSizeInBytes <= fSizeInBytes);
    // bindbuffer handles dirty context
    GrGLenum target = this->glGpu()->bindBuffer(fIntendedType, this);

#if GR_GL_USE_BUFFER_DATA_NULL_HINT
    if (fSizeInBytes == srcSizeInBytes) {
        GL_CALL(BufferData(target, (GrGLsizeiptr) srcSizeInBytes, src, fUsage));
    } else {
        // Before we call glBufferSubData we give the driver a hint using
        // glBufferData with nullptr. This makes the old buffer contents
        // inaccessible to future draws. The GPU may still be processing
        // draws that reference the old contents. With this hint it can
        // assign a different allocation for the new contents to avoid
        // flushing the gpu past draws consuming the old contents.
        // TODO I think we actually want to try calling bufferData here
        GL_CALL(BufferData(target, fSizeInBytes, nullptr, fUsage));
        GL_CALL(BufferSubData(target, 0, (GrGLsizeiptr) srcSizeInBytes, src));
    }
    fGLSizeInBytes = fSizeInBytes;
#else
    // Note that we're cheating on the size here. Currently no methods
    // allow a partial update that preserves contents of non-updated
    // portions of the buffer (map() does a glBufferData(..size, nullptr..))
    GL_CALL(BufferData(target, srcSizeInBytes, src, fUsage));
    fGLSizeInBytes = srcSizeInBytes;
#endif
    VALIDATE();
    return true;
}

void GrGLBuffer::setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                                       const SkString& dumpName) const {
    SkString buffer_id;
    buffer_id.appendU32(this->bufferID());
    traceMemoryDump->setMemoryBacking(dumpName.c_str(), "gl_buffer",
                                      buffer_id.c_str());
}

#ifdef SK_DEBUG

void GrGLBuffer::validate() const {
    // The following assert isn't valid when the buffer has been abandoned:
    // SkASSERT((0 == fDesc.fID) == (fCPUData));
    SkASSERT(0 != fBufferID || 0 == fGLSizeInBytes);
    SkASSERT(nullptr == fMapPtr || fCPUData || fGLSizeInBytes <= fSizeInBytes);
    SkASSERT(nullptr == fCPUData || nullptr == fMapPtr || fCPUData == fMapPtr);
}

#endif
