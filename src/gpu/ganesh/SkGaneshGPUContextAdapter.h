/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGaneshGPUContextAdapter_DEFINED
#define SkGaneshGPUContextAdapter_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/SkAssert.h"
#include "include/private/SkGPUContext.h"

#include <chrono>
#include <cstddef>
#include <optional>
#include <utility>

class SkTraceMemoryDump;

// We need this adapter because we want SkContext to uniquely hold the GPU context
// but GrDirectContext is a legacy, yet already public API, that uses sk_sp and allows multiple refs
// The solution is to create an adapter that stores a GrDirectContext but can be held by a
// unique_ptr
class SkGaneshGPUContextAdapter final : public SkGPUContext {
public:
    explicit SkGaneshGPUContextAdapter(sk_sp<GrDirectContext> directContext)
            : fDirectContext(std::move(directContext)) {
        SkASSERT(fDirectContext);
    }

    ~SkGaneshGPUContextAdapter() override = default;

    GrDirectContext* directContext() const { return fDirectContext.get(); }

    bool submit() override { return fDirectContext->submit(); }

    void checkAsyncWorkCompletion() override { fDirectContext->checkAsyncWorkCompletion(); }

    void freeGpuResources() override { fDirectContext->freeGpuResources(); }

    void performDeferredCleanup(std::chrono::milliseconds msNotUsed,
                                std::optional<std::chrono::microseconds>) override {
        fDirectContext->performDeferredCleanup(msNotUsed);
    }

    size_t currentBudgetedBytes() const override {
        size_t bytes = 0;
        fDirectContext->getResourceCacheUsage(nullptr, &bytes);
        return bytes;
    }

    size_t currentPurgeableBytes() const override {
        return fDirectContext->getResourceCachePurgeableBytes();
    }

    size_t maxBudgetedBytes() const override { return fDirectContext->getResourceCacheLimit(); }

    void setMaxBudgetedBytes(size_t bytes) override {
        fDirectContext->setResourceCacheLimit(bytes);
    }

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override {
        fDirectContext->dumpMemoryStatistics(traceMemoryDump);
    }

    bool isDeviceLost() const override { return fDirectContext->isDeviceLost(); }

    int maxTextureSize() const override { return fDirectContext->maxTextureSize(); }

    bool supportsProtectedContent() const override {
        return fDirectContext->supportsProtectedContent();
    }

private:
    sk_sp<GrDirectContext> fDirectContext;
};

#endif  // SkGaneshGPUContextAdapter_DEFINED
