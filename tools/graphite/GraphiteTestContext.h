/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_GraphiteTestContext_DEFINED
#define skiatest_graphite_GraphiteTestContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/GraphiteTypes.h"

namespace skgpu {
enum class BackendApi : unsigned;
enum class ContextType;
}
namespace skgpu::graphite {
class Context;
struct ContextOptions;
class Recording;
}

namespace sk_gpu_test { class FlushFinishTracker; }

namespace skiatest::graphite {

/**
 * An offscreen 3D context. This class is intended for Skia's internal testing needs and not
 * for general use.
 */
class GraphiteTestContext {
public:
    GraphiteTestContext(const GraphiteTestContext&) = delete;
    GraphiteTestContext& operator=(const GraphiteTestContext&) = delete;

    virtual ~GraphiteTestContext();

    virtual skgpu::BackendApi backend() = 0;

    virtual skgpu::ContextType contextType() = 0;

    virtual std::unique_ptr<skgpu::graphite::Context> makeContext(
            const skgpu::graphite::ContextOptions&) = 0;

    bool getMaxGpuFrameLag(int *maxFrameLag) const {
        *maxFrameLag = kMaxFrameLag;
        return true;
    }

    /**
     * This will insert a Recording and submit work to the GPU. Additionally, we will add a finished
     * callback to our insert recording call. We allow ourselves to have kMaxFrameLag number of
     * unfinished flushes active on the GPU at a time. If we have 2 outstanding flushes then we will
     * wait on the CPU until one has finished.
     */
    void submitRecordingAndWaitOnSync(skgpu::graphite::Context*, skgpu::graphite::Recording*);

    /**
     * Allow the GPU API to make or detect forward progress on submitted work. For most APIs this is
     * a no-op as the API can do this on another thread.
     */
    virtual void tick() {}

protected:
    static constexpr int kMaxFrameLag = 3;

    sk_sp<sk_gpu_test::FlushFinishTracker> fFinishTrackers[kMaxFrameLag - 1];
    int fCurrentFlushIdx = 0;

    GraphiteTestContext();
};


}  // namespace skiatest::graphite

#endif // skiatest_graphite_GraphiteTestContext_DEFINED
