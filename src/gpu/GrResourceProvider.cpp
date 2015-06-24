/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrResourceProvider.h"

#include "GrGpu.h"
#include "GrIndexBuffer.h"
#include "GrPathRendering.h"
#include "GrResourceCache.h"
#include "GrResourceKey.h"
#include "GrVertexBuffer.h"

GR_DECLARE_STATIC_UNIQUE_KEY(gQuadIndexBufferKey);

GrResourceProvider::GrResourceProvider(GrGpu* gpu, GrResourceCache* cache) : INHERITED(gpu, cache) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gQuadIndexBufferKey);
    fQuadIndexBufferKey = gQuadIndexBufferKey;
}

const GrIndexBuffer* GrResourceProvider::createInstancedIndexBuffer(const uint16_t* pattern,
                                                                    int patternSize,
                                                                    int reps,
                                                                    int vertCount,
                                                                    const GrUniqueKey& key) {
    size_t bufferSize = patternSize * reps * sizeof(uint16_t);

    GrIndexBuffer* buffer = this->getIndexBuffer(bufferSize, /* dynamic = */ false, true);
    if (!buffer) {
        return NULL;
    }
    uint16_t* data = (uint16_t*) buffer->map();
    bool useTempData = (NULL == data);
    if (useTempData) {
        data = SkNEW_ARRAY(uint16_t, reps * patternSize);
    }
    for (int i = 0; i < reps; ++i) {
        int baseIdx = i * patternSize;
        uint16_t baseVert = (uint16_t)(i * vertCount);
        for (int j = 0; j < patternSize; ++j) {
            data[baseIdx+j] = baseVert + pattern[j];
        }
    }
    if (useTempData) {
        if (!buffer->updateData(data, bufferSize)) {
            buffer->unref();
            return NULL;
        }
        SkDELETE_ARRAY(data);
    } else {
        buffer->unmap();
    }
    this->assignUniqueKeyToResource(key, buffer);
    return buffer;
}

const GrIndexBuffer* GrResourceProvider::createQuadIndexBuffer() {
    static const int kMaxQuads = 1 << 12; // max possible: (1 << 14) - 1;
    GR_STATIC_ASSERT(4 * kMaxQuads <= 65535);
    static const uint16_t kPattern[] = { 0, 1, 2, 0, 2, 3 };

    return this->createInstancedIndexBuffer(kPattern, 6, kMaxQuads, 4, fQuadIndexBufferKey);
}

GrPath* GrResourceProvider::createPath(const SkPath& path, const GrStrokeInfo& stroke) {
    SkASSERT(this->gpu()->pathRendering());
    return this->gpu()->pathRendering()->createPath(path, stroke);
}

GrPathRange* GrResourceProvider::createPathRange(GrPathRange::PathGenerator* gen,
                                                 const GrStrokeInfo& stroke) {
    SkASSERT(this->gpu()->pathRendering());
    return this->gpu()->pathRendering()->createPathRange(gen, stroke);
}

GrPathRange* GrResourceProvider::createGlyphs(const SkTypeface* tf, const SkDescriptor* desc,
                                              const GrStrokeInfo& stroke) {

    SkASSERT(this->gpu()->pathRendering());
    return this->gpu()->pathRendering()->createGlyphs(tf, desc, stroke);
}

GrIndexBuffer* GrResourceProvider::getIndexBuffer(size_t size, bool dynamic,
                                                  bool calledDuringFlush) {
    if (this->isAbandoned()) {
        return NULL;
    }

    if (dynamic) {
        // bin by pow2 with a reasonable min
        static const uint32_t MIN_SIZE = 1 << 12;
        size = SkTMax(MIN_SIZE, GrNextPow2(SkToUInt(size)));

        GrScratchKey key;
        GrIndexBuffer::ComputeScratchKey(size, dynamic, &key);
        uint32_t scratchFlags = 0;
        if (calledDuringFlush) {
            scratchFlags = GrResourceCache::kRequireNoPendingIO_ScratchFlag;
        } else {
            scratchFlags = GrResourceCache::kPreferNoPendingIO_ScratchFlag;
        }
        GrGpuResource* resource = this->cache()->findAndRefScratchResource(key, scratchFlags);
        if (resource) {
            return static_cast<GrIndexBuffer*>(resource);
        }
    }

    return this->gpu()->createIndexBuffer(size, dynamic);    
}

GrVertexBuffer* GrResourceProvider::getVertexBuffer(size_t size, bool dynamic, 
                                                    bool calledDuringFlush) {
    if (this->isAbandoned()) {
        return NULL;
    }

    if (dynamic) {
        // bin by pow2 with a reasonable min
        static const uint32_t MIN_SIZE = 1 << 15;
        size = SkTMax(MIN_SIZE, GrNextPow2(SkToUInt(size)));

        GrScratchKey key;
        GrVertexBuffer::ComputeScratchKey(size, dynamic, &key);
        uint32_t scratchFlags = 0;
        if (calledDuringFlush) {
            scratchFlags = GrResourceCache::kRequireNoPendingIO_ScratchFlag;
        } else {
            scratchFlags = GrResourceCache::kPreferNoPendingIO_ScratchFlag;
        }
        GrGpuResource* resource = this->cache()->findAndRefScratchResource(key, scratchFlags);
        if (resource) {
            return static_cast<GrVertexBuffer*>(resource);
        }
    }

    return this->gpu()->createVertexBuffer(size, dynamic);
}
