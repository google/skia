
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrVertexBuffer_DEFINED
#define GrVertexBuffer_DEFINED

#include "GrGeometryBuffer.h"

class GrVertexBuffer : public GrGeometryBuffer {
public:
    static void ComputeScratchKey(size_t size, bool dynamic, GrScratchKey* key) {
        static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();

        GrScratchKey::Builder builder(key, kType, 2);

        builder[0] = SkToUInt(size);
        builder[1] = dynamic ? 1 : 0;
    }

protected:
    GrVertexBuffer(GrGpu* gpu, size_t gpuMemorySize, bool dynamic, bool cpuBacked)
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
