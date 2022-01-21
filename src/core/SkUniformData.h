/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUniformData_DEFINED
#define SkUniformData_DEFINED

#include "include/core/SkRefCnt.h"

class SkUniform;

/*
 * TODO: Here is the plan of record for SkUniformData
 *    allocate them (and their ancillary data (offsets & data)) in an arena - and rm SkRefCnt
 *    remove the offsets - these should be recomputable as/if needed
 *    allow a scratch buffer to be used to collect the uniform data when processing a PaintParams
 *       - this can be reset for each draw and be copied into a cache if it needs to be preserved
 *    if possible, clear out the cache (and delete the arena) once the DrawPass is created/the
 *               uniforms are copied into a buffer
 *       - this will mean we'll get w/in DrawPass reuse but not cross-DrawPass reuse
 *       - we could also explore getting w/in a Recording (i.e. cross-DrawPass) and w/in a
 *               Recorder reuse (i.e., cross-Recordings, but single-threaded)
 *    have the SkUniformData's data layout match what is required by the DrawList so we can just
 *               copy it into the buffer
 */

class SkUniformData : public SkRefCnt {
public:

    // TODO: should we require a name (e.g., "gradient_uniforms") for each uniform block so
    // we can better name the Metal FS uniform struct?
    static sk_sp<SkUniformData> Make(int count,
                                     const SkUniform* uniforms,
                                     size_t dataSize);

    ~SkUniformData() override {
        // TODO: fOffsets and fData should just be allocated right after UniformData in an arena
        delete [] fOffsets;
        delete [] fData;
    }

    int count() const { return fCount; }
    const SkUniform* uniforms() const { return fUniforms; }
    uint32_t* offsets() { return fOffsets; }
    uint32_t offset(int index) {
        SkASSERT(index >= 0 && index < fCount);
        return fOffsets[index];
    }
    char* data() { return fData; }
    size_t dataSize() const { return fDataSize; }

private:
    SkUniformData(int count,
                  const SkUniform* uniforms,
                  uint32_t* offsets,
                  char* data,
                  size_t dataSize)
            : fCount(count)
            , fUniforms(uniforms)
            , fOffsets(offsets)
            , fData(data)
            , fDataSize(dataSize) {
    }

    const int fCount;
    const SkUniform* fUniforms;
    uint32_t* fOffsets; // offset of each uniform in 'fData'
    char* fData;
    const size_t fDataSize;
};

#endif // SkUniformData_DEFINED
