/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLBufferImpl.h"
#include "GrGLGpu.h"

#define GL_CALL(GPU, X) GR_GL_CALL(GPU->glInterface(), X)

#ifdef SK_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

// GL_STREAM_DRAW triggers an optimization in Chromium's GPU process where a client's vertex buffer
// objects are implemented as client-side-arrays on tile-deferred architectures.
#define DYNAMIC_USAGE_PARAM GR_GL_STREAM_DRAW

GrGLBufferImpl::GrGLBufferImpl(GrGLGpu* gpu, const Desc& desc, GrGLenum bufferType)
    : fDesc(desc)
    , fBufferType(bufferType)
    , fMapPtr(NULL) {
    if (0 == desc.fID) {
        fCPUData = sk_malloc_flags(desc.fSizeInBytes, SK_MALLOC_THROW);
        fGLSizeInBytes = 0;
    } else {
        fCPUData = NULL;
        // We assume that the GL buffer was created at the desc's size initially.
        fGLSizeInBytes = fDesc.fSizeInBytes;
    }
    VALIDATE();
}

void GrGLBufferImpl::release(GrGLGpu* gpu) {
    VALIDATE();
    // make sure we've not been abandoned or already released
    if (fCPUData) {
        sk_free(fCPUData);
        fCPUData = NULL;
    } else if (fDesc.fID) {
        GL_CALL(gpu, DeleteBuffers(1, &fDesc.fID));
        if (GR_GL_ARRAY_BUFFER == fBufferType) {
            gpu->notifyVertexBufferDelete(fDesc.fID);
        } else {
            SkASSERT(GR_GL_ELEMENT_ARRAY_BUFFER == fBufferType);
            gpu->notifyIndexBufferDelete(fDesc.fID);
        }
        fDesc.fID = 0;
        fGLSizeInBytes = 0;
    }
    fMapPtr = NULL;
    VALIDATE();
}

void GrGLBufferImpl::abandon() {
    fDesc.fID = 0;
    fGLSizeInBytes = 0;
    fMapPtr = NULL;
    sk_free(fCPUData);
    fCPUData = NULL;
    VALIDATE();
}

void GrGLBufferImpl::bind(GrGLGpu* gpu) const {
    VALIDATE();
    if (GR_GL_ARRAY_BUFFER == fBufferType) {
        gpu->bindVertexBuffer(fDesc.fID);
    } else {
        SkASSERT(GR_GL_ELEMENT_ARRAY_BUFFER == fBufferType);
        gpu->bindIndexBufferAndDefaultVertexArray(fDesc.fID);
    }
    VALIDATE();
}

void* GrGLBufferImpl::map(GrGLGpu* gpu) {
    VALIDATE();
    SkASSERT(!this->isMapped());
    if (0 == fDesc.fID) {
        fMapPtr = fCPUData;
    } else {
        switch (gpu->glCaps().mapBufferType()) {
            case GrGLCaps::kNone_MapBufferType:
                VALIDATE();
                return NULL;
            case GrGLCaps::kMapBuffer_MapBufferType:
                this->bind(gpu);
                // Let driver know it can discard the old data
                if (GR_GL_USE_BUFFER_DATA_NULL_HINT || fDesc.fSizeInBytes != fGLSizeInBytes) {
                    fGLSizeInBytes = fDesc.fSizeInBytes;
                    GL_CALL(gpu,
                            BufferData(fBufferType, fGLSizeInBytes, NULL,
                                       fDesc.fDynamic ? DYNAMIC_USAGE_PARAM : GR_GL_STATIC_DRAW));
                }
                GR_GL_CALL_RET(gpu->glInterface(), fMapPtr,
                               MapBuffer(fBufferType, GR_GL_WRITE_ONLY));
                break;
            case GrGLCaps::kMapBufferRange_MapBufferType: {
                this->bind(gpu);
                // Make sure the GL buffer size agrees with fDesc before mapping.
                if (fDesc.fSizeInBytes != fGLSizeInBytes) {
                    fGLSizeInBytes = fDesc.fSizeInBytes;
                    GL_CALL(gpu,
                            BufferData(fBufferType, fGLSizeInBytes, NULL,
                                       fDesc.fDynamic ? DYNAMIC_USAGE_PARAM : GR_GL_STATIC_DRAW));
                }
                static const GrGLbitfield kAccess = GR_GL_MAP_INVALIDATE_BUFFER_BIT |
                                                    GR_GL_MAP_WRITE_BIT;
                GR_GL_CALL_RET(gpu->glInterface(),
                               fMapPtr,
                               MapBufferRange(fBufferType, 0, fGLSizeInBytes, kAccess));
                break;
            }
            case GrGLCaps::kChromium_MapBufferType:
                this->bind(gpu);
                // Make sure the GL buffer size agrees with fDesc before mapping.
                if (fDesc.fSizeInBytes != fGLSizeInBytes) {
                    fGLSizeInBytes = fDesc.fSizeInBytes;
                    GL_CALL(gpu,
                            BufferData(fBufferType, fGLSizeInBytes, NULL,
                                       fDesc.fDynamic ? DYNAMIC_USAGE_PARAM : GR_GL_STATIC_DRAW));
                }
                GR_GL_CALL_RET(gpu->glInterface(),
                               fMapPtr,
                               MapBufferSubData(fBufferType, 0, fGLSizeInBytes, GR_GL_WRITE_ONLY));
                break;
        }
    }
    VALIDATE();
    return fMapPtr;
}

void GrGLBufferImpl::unmap(GrGLGpu* gpu) {
    VALIDATE();
    SkASSERT(this->isMapped());
    if (0 != fDesc.fID) {
        switch (gpu->glCaps().mapBufferType()) {
            case GrGLCaps::kNone_MapBufferType:
                SkDEBUGFAIL("Shouldn't get here.");
                return;
            case GrGLCaps::kMapBuffer_MapBufferType: // fall through
            case GrGLCaps::kMapBufferRange_MapBufferType:
                this->bind(gpu);
                GL_CALL(gpu, UnmapBuffer(fBufferType));
                break;
            case GrGLCaps::kChromium_MapBufferType:
                this->bind(gpu);
                GR_GL_CALL(gpu->glInterface(), UnmapBufferSubData(fMapPtr));
                break;
        }
    }
    fMapPtr = NULL;
}

bool GrGLBufferImpl::isMapped() const {
    VALIDATE();
    return SkToBool(fMapPtr);
}

bool GrGLBufferImpl::updateData(GrGLGpu* gpu, const void* src, size_t srcSizeInBytes) {
    SkASSERT(!this->isMapped());
    VALIDATE();
    if (srcSizeInBytes > fDesc.fSizeInBytes) {
        return false;
    }
    if (0 == fDesc.fID) {
        memcpy(fCPUData, src, srcSizeInBytes);
        return true;
    }
    this->bind(gpu);
    GrGLenum usage = fDesc.fDynamic ? DYNAMIC_USAGE_PARAM : GR_GL_STATIC_DRAW;

#if GR_GL_USE_BUFFER_DATA_NULL_HINT
    if (fDesc.fSizeInBytes == srcSizeInBytes) {
        GL_CALL(gpu, BufferData(fBufferType, (GrGLsizeiptr) srcSizeInBytes, src, usage));
    } else {
        // Before we call glBufferSubData we give the driver a hint using
        // glBufferData with NULL. This makes the old buffer contents
        // inaccessible to future draws. The GPU may still be processing
        // draws that reference the old contents. With this hint it can
        // assign a different allocation for the new contents to avoid
        // flushing the gpu past draws consuming the old contents.
        fGLSizeInBytes = fDesc.fSizeInBytes;
        GL_CALL(gpu, BufferData(fBufferType, fGLSizeInBytes, NULL, usage));
        GL_CALL(gpu, BufferSubData(fBufferType, 0, (GrGLsizeiptr) srcSizeInBytes, src));
    }
#else
    // Note that we're cheating on the size here. Currently no methods
    // allow a partial update that preserves contents of non-updated
    // portions of the buffer (map() does a glBufferData(..size, NULL..))
    bool doSubData = false;
#if GR_GL_MAC_BUFFER_OBJECT_PERFOMANCE_WORKAROUND
    static int N = 0;
    // 128 was chosen experimentally. At 256 a slight hitchiness was noticed
    // when dragging a Chromium window around with a canvas tab backgrounded.
    doSubData = 0 == (N % 128);
    ++N;
#endif
    if (doSubData) {
        // The workaround is to do a glBufferData followed by glBufferSubData.
        // Chromium's command buffer may turn a glBufferSubData where the size
        // exactly matches the buffer size into a glBufferData. So we tack 1
        // extra byte onto the glBufferData.
        fGLSizeInBytes = srcSizeInBytes + 1;
        GL_CALL(gpu, BufferData(fBufferType, fGLSizeInBytes, NULL, usage));
        GL_CALL(gpu, BufferSubData(fBufferType, 0, srcSizeInBytes, src));
    } else {
        fGLSizeInBytes = srcSizeInBytes;
        GL_CALL(gpu, BufferData(fBufferType, fGLSizeInBytes, src, usage));
    }
#endif
    return true;
}

void GrGLBufferImpl::validate() const {
    SkASSERT(GR_GL_ARRAY_BUFFER == fBufferType || GR_GL_ELEMENT_ARRAY_BUFFER == fBufferType);
    // The following assert isn't valid when the buffer has been abandoned:
    // SkASSERT((0 == fDesc.fID) == (fCPUData));
    SkASSERT(NULL == fCPUData || 0 == fGLSizeInBytes);
    SkASSERT(NULL == fMapPtr || fCPUData || fGLSizeInBytes == fDesc.fSizeInBytes);
    SkASSERT(NULL == fCPUData || NULL == fMapPtr || fCPUData == fMapPtr);
}
