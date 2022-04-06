/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUniformData_DEFINED
#define SkUniformData_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "src/core/SkUniform.h"

/*
 * TODO: Here is the plan of record for SkUniformData
 *    allocate them (and their ancillary data in an arena - and rm SkRefCnt
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

    static sk_sp<SkUniformData> Make(const char* data, size_t size);

    ~SkUniformData() override {
        // TODO: fData should just be allocated right after UniformData in an arena
        delete [] fData;
    }

    char* data() { return fData; }
    const char* data() const { return fData; }
    size_t dataSize() const { return fDataSize; }

    bool operator==(const SkUniformData&) const;
    bool operator!=(const SkUniformData& other) const { return !(*this == other);  }

private:
    SkUniformData(char* data, size_t dataSize)
            : fData(data)
            , fDataSize(dataSize) {
    }

    char* fData;
    const size_t fDataSize;
};

#endif // SkUniformData_DEFINED
