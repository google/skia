/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Gpu_DEFINED
#define skgpu_Gpu_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkDeque.h"

#include "experimental/graphite/include/GraphiteTypes.h"

namespace skgpu {

class Caps;
class ResourceProvider;
class CommandBuffer;
class GpuWorkSubmission;

class Gpu : public SkRefCnt {
public:
    ~Gpu() override;

    /**
     * Gets the capabilities of the draw target.
     */
    const Caps* caps() const { return fCaps.get(); }
    sk_sp<const Caps> refCaps() const;

    ResourceProvider* resourceProvider() const { return fResourceProvider.get(); }

    bool submit(sk_sp<CommandBuffer>);
    void checkForFinishedWork(SyncToCpu);

#if GRAPHITE_TEST_UTILS
    virtual void testingOnly_startCapture() {}
    virtual void testingOnly_endCapture() {}
#endif

protected:
    Gpu(sk_sp<const Caps>);

    std::unique_ptr<ResourceProvider> fResourceProvider;

    using OutstandingSubmission = std::unique_ptr<GpuWorkSubmission>;
    SkDeque fOutstandingSubmissions;

private:
    virtual bool onSubmit(sk_sp<CommandBuffer>) = 0;

    sk_sp<const Caps> fCaps;
};

} // namespace skgpu

#endif // skgpu_Gpu_DEFINED
