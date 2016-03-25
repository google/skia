
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrIndexBuffer_DEFINED
#define GrIndexBuffer_DEFINED

#include "GrGeometryBuffer.h"


class GrIndexBuffer : public GrGeometryBuffer {
public:
    static void ComputeScratchKey(size_t size, bool dynamic, GrScratchKey* key) {
        static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();

        GrScratchKey::Builder builder(key, kType, 2);

        builder[0] = SkToUInt(size);
        builder[1] = dynamic ? 1 : 0;
    }

    /**
     * Retrieves the maximum number of quads that could be rendered
     * from the index buffer (using kTriangles_GrPrimitiveType).
     * @return the maximum number of quads using full size of index buffer.
     */
    int maxQuads() const {
        return static_cast<int>(this->gpuMemorySize() / (sizeof(uint16_t) * 6));
    }
protected:
    GrIndexBuffer(GrGpu* gpu, size_t gpuMemorySize, bool dynamic, bool cpuBacked)
        : INHERITED(gpu, gpuMemorySize, dynamic, cpuBacked) {
        // We currently only make buffers scratch if they're both pow2 sized and not cpuBacked.
        if (!cpuBacked && SkIsPow2(gpuMemorySize)) {
            GrScratchKey key;
            ComputeScratchKey(gpuMemorySize, dynamic, &key);
            this->setScratchKey(key);
        }
    }

private:
    typedef GrGeometryBuffer INHERITED;
};

#endif
