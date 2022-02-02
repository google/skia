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
#include <vector>

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
    static sk_sp<SkUniformData> Make(SkSpan<const SkUniform>, size_t dataSize);

    ~SkUniformData() override {
        // TODO: fOffsets and fData should just be allocated right after UniformData in an arena
        delete [] fOffsets;
        delete [] fData;
    }

    int count() const { return static_cast<unsigned int>(fUniforms.size()); }
    SkSpan<const SkUniform> uniforms() const { return fUniforms; }
    uint32_t* offsets() { return fOffsets; }
    const uint32_t* offsets() const { return fOffsets; }
    uint32_t offset(int index) {
        SkASSERT(index >= 0 && static_cast<size_t>(index) < fUniforms.size());
        return fOffsets[index];
    }
    char* data() { return fData; }
    const char* data() const { return fData; }
    size_t dataSize() const { return fDataSize; }

    bool operator==(const SkUniformData&) const;
    bool operator!=(const SkUniformData& other) const { return !(*this == other);  }

private:
    SkUniformData(SkSpan<const SkUniform> uniforms,
                  uint32_t* offsets,
                  char* data,
                  size_t dataSize)
            : fUniforms(uniforms)
            , fOffsets(offsets)
            , fData(data)
            , fDataSize(dataSize) {
    }

    SkSpan<const SkUniform> fUniforms;
    uint32_t* fOffsets; // offset of each uniform in 'fData'
    char* fData;
    const size_t fDataSize;
};

class SkUniformBlock {
public:
    SkUniformBlock() = default;
    SkUniformBlock(sk_sp<SkUniformData> initial) {
        fUniformData.push_back(std::move(initial));
    }

    void add(sk_sp<SkUniformData>);

    bool empty() const { return fUniformData.empty(); }
    size_t totalSize() const;  // TODO: cache this?
    int count() const;         // TODO: cache this?

    bool operator==(const SkUniformBlock&) const;
    bool operator!=(const SkUniformBlock& other) const { return !(*this == other);  }
    size_t hash() const;

    using container = std::vector<sk_sp<SkUniformData>>;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;

    inline iterator begin() noexcept { return fUniformData.begin(); }
    inline const_iterator cbegin() const noexcept { return fUniformData.cbegin(); }
    inline iterator end() noexcept { return fUniformData.end(); }
    inline const_iterator cend() const noexcept { return fUniformData.cend(); }

private:
    // TODO: SkUniformData should be held uniquely
    std::vector<sk_sp<SkUniformData>> fUniformData;
};

#endif // SkUniformData_DEFINED
