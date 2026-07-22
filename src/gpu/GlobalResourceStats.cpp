/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GlobalResourceStats.h"

#include "include/gpu/GpuTypes.h"
#include "src/core/SkTraceEvent.h"

namespace skgpu {

void GlobalResourceStats::recordNewResource(size_t size, Budgeted budgeted) {
    if (budgeted == Budgeted::kYes) {
        fBudgetedBytes += size;
    } else {
        fUnbudgetedBytes += size;
    }
}

void GlobalResourceStats::recordPurgeResource(size_t size) {
    // This assumes all unbudgeted resources transition to budgeted resources before they are purged
    fBudgetedBytes -= size;
}

void GlobalResourceStats::recordResourceBudgetChange(size_t size, Budgeted newBudget) {
    // We're not worrying about synchronizing the update between unbudgeted and budgeted, tracing
    // will eventually show the right state.
    if (newBudget == Budgeted::kYes) {
        // We're assuming that this budget change will not happen on a wrapped resource, so no need
        // to adjust fWrappedBytes.
        fUnbudgetedBytes -= size;
        fBudgetedBytes += size;
    } else {
        fBudgetedBytes -= size;
        fUnbudgetedBytes += size;
    }
}

void GlobalResourceStats::recordResourceUpdatedSize(size_t newSize,
                                                    size_t oldSize,
                                                    Budgeted budgeted) {
    // We could structure this as subtracting oldSize and then adding newSize, but we can keep it
    // atomic by choosing += or -=.
    if (budgeted == Budgeted::kYes) {
        if (oldSize > newSize) {
            fBudgetedBytes -= (oldSize - newSize);
        } else {
            fBudgetedBytes += (newSize - oldSize);
        }
    } else {
        // We're assuming that this size change does not happen on a wrapped resource, so no need
        // to adjust fWrappedBytes (this is possible because Ganesh does not query for changed
        // resource sizes and Graphite does not cache wrapped resources so fWrappedBytes is 0).
        if (oldSize > newSize) {
            fUnbudgetedBytes -= (oldSize - newSize);
        } else {
            fUnbudgetedBytes += (newSize - oldSize);
        }
    }
}

void GlobalResourceStats::recordResourcePurgeable(size_t size) {
    fPurgeableBytes += size;
}

void GlobalResourceStats::recordResourceNonpurgeable(size_t size) {
    fPurgeableBytes -= size;
}

void GlobalResourceStats::recordCreateBackendTexture(size_t size) {
    fBackendTexBytes += size;
    // Trigger a trace for backend texture records since they happen outside the regular
    // flush/submit flow.
    this->traceStatsSummary();
}

void GlobalResourceStats::recordDeleteBackendTexture(size_t size) {
    fBackendTexBytes -= size;
    this->traceStatsSummary();
}

void GlobalResourceStats::traceStatsSummary() const {
    // Grab a snapshot (these won't change, but they aren't loaded atomically)
          size_t budgeted   = fBudgetedBytes;
    const size_t unbudgeted = fUnbudgetedBytes;
    const size_t purgeable  = fPurgeableBytes;
    const size_t backendTex = fBackendTexBytes;

    // Separate budgeted and purgeable into a non-overlapping categories
    if (budgeted > purgeable) {
        budgeted -= purgeable;
    } else {
        budgeted = 0;
    }

    // While the total could be derived within the trace data, it is helpful to trace the total
    // as a visual anchor.
    const size_t total = backendTex + unbudgeted + budgeted + purgeable;

    // This pushes the protected label to the front since Perfetto sorts tracks alphabetically.
    #define TRACE_STATS(label) \
        TRACE_COUNTER1_ALWAYS("skia.gpu.cache", label " Total GPU Bytes",           total);      \
        TRACE_COUNTER1_ALWAYS("skia.gpu.cache", label " Purgeable GPU Bytes",       purgeable);  \
        TRACE_COUNTER1_ALWAYS("skia.gpu.cache", label " Budgeted GPU Bytes",        budgeted);   \
        TRACE_COUNTER1_ALWAYS("skia.gpu.cache", label " Unbudgeted GPU Bytes",      unbudgeted); \
        TRACE_COUNTER1_ALWAYS("skia.gpu.cache", label " Backend Texture GPU Bytes", backendTex);

    // Issue traces with different counter names depending on protected or unprotected so they
    // appear in separate tracks.
    if (fProtected == Protected::kYes) {
        TRACE_STATS("Skia Protected")
    } else {
        TRACE_STATS("Skia Unprotected")
    }

    #undef TRACE_STATS
}

} // namespace skgpu
