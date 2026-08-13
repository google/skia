/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_StorageContext_DEFINED
#define skgpu_graphite_StorageContext_DEFINED

#include "include/private/SkTDArray.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <cstdint>
#include <limits>
#include <utility>

class SkGradientBaseShader;

namespace skgpu::graphite {

class DrawBufferManager;

class StorageContext {
public:
    // Placeholder alignment, to be replaced by LCM across rendersteps in future.
    static constexpr size_t kStructAlignment = 16;

    StorageContext();
    ~StorageContext();

    // Resets cached gradient data. Should only occur at "organic" flush time (e.g. canvas/recorder
    // flush).
    void resetCache();

    // Stub, will call more than resetCache() in the future
    void reset() {
        this->resetCache();
    }

    std::pair<float*, int> allocateGradientData(int numStops, const SkGradientBaseShader* shader);

    // Called at the beginning of snapping a draw pass. It aligns the cached data so that appended
    // data may have a valid offset. Note, this extends and clears the CPU side copy of the cached
    // data but does not make uploads.
    void finalizePrecachedStorageData();

    // Called after snapping the draw pass. It creates a single mapped buffer containing the
    // concatenated cached and append data. Currently, appending is not implemented, so it only
    // uploads cached data.
    BindBufferInfo finalize(DrawBufferManager* bufferMgr);

    bool isEmpty() const { return fGradientCache.isEmpty(); }

    SkDEBUGCODE(int size() const { return fGradientCache.fGradientData.size(); })

private:
    struct GradientCache {
        static constexpr int kMaxGradientStops = 1024 * 1024;
        static constexpr int kMaxStorageFloats =
                static_cast<int>(std::numeric_limits<uint32_t>::max() / sizeof(float));

        skia_private::THashMap<const SkGradientBaseShader*, int> fLocalGradientOffsetCache;
        SkTDArray<float> fGradientData;
        size_t fGradientDataSize = 0;

        void reset();

        bool isEmpty() const { return fGradientData.empty(); }
    };

    SkDEBUGCODE(bool fGradientsFinalized = false;)

    GradientCache fGradientCache;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_StorageContext_DEFINED
