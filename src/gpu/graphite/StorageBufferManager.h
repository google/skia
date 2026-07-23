/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_StorageBufferManager_DEFINED
#define skgpu_graphite_StorageBufferManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkAlign.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

class SkGradientBaseShader;

namespace skgpu::graphite {

class DrawBufferManager;

/**
 * Manages allocation of storage buffer data, currently only handles gradient data
 */
class StorageBufferManager : public SkRefCnt {
public:
    StorageBufferManager() = default;
    ~StorageBufferManager() override;

    void reset();

    // Checks if data already exists for the requested gradient shader. If so, it returns a nullptr
    // and the existing offset. If not, it allocates space, caches the offset, and returns a pointer
    // to the start of the new data and the calculated offset.
    //
    // If it was not possible to store the gradient data, a nullptr and negative offset are returned
    // to signal the error state.
    std::pair<float*, int> allocateGradientData(int numStops, const SkGradientBaseShader* shader);

    bool finalize(DrawBufferManager* bufferMgr);

    BindBufferInfo getBufferInfo() const { return fBufferInfo.value_or(BindBufferInfo{}); }
    bool hasData() const { return fBufferInfo.has_value() && fBufferInfo->fBuffer != nullptr; }

#if defined(SK_DEBUG)
    bool isFinalized() const { return fBufferInfo.has_value(); }
    int gradientSize() const { return fGradientCache.fStorage.size_bytes(); }
#endif

private:
    struct GradientCache {
        static constexpr int kMaxGradientStops = 1024 * 1024;
        static constexpr int kMaxStorageFloats =
                static_cast<int>(std::numeric_limits<uint32_t>::max() / sizeof(float));

        skia_private::THashMap<const SkGradientBaseShader*, int> fOffsetCache;
        SkTDArray<float> fStorage;

        void reset();
    };

    GradientCache fGradientCache;
    std::optional<BindBufferInfo> fBufferInfo;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_StorageBufferManager_DEFINED
