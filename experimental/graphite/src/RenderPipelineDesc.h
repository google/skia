/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RenderPipelineDesc_DEFINED
#define skgpu_RenderPipelineDesc_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"

namespace skgpu {

class RenderPipelineDesc {
public:
    RenderPipelineDesc();

    // Returns this as a uint32_t array to be used as a key in the pipeline cache.
    // TODO: Do we want to do anything here with a tuple or an SkSpan?
    const uint32_t* asKey() const {
        return fKey.data();
    }

    // Gets the number of bytes in asKey(). It will be a 4-byte aligned value.
    uint32_t keyLength() const {
        return fKey.size() * sizeof(uint32_t);
    }

    bool operator== (const RenderPipelineDesc& that) const {
        return this->fKey == that.fKey;
    }

    bool operator!= (const RenderPipelineDesc& other) const {
        return !(*this == other);
    }

private:
    // Estimate of max expected key size
    // TODO: flesh this out
    static constexpr int kPreAllocSize = 1;

    SkSTArray<kPreAllocSize, uint32_t, true> fKey;
};

} // namespace skgpu

#endif // skgpu_RenderPipelineDesc_DEFINED
