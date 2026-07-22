/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GlobalResourceStats_DEFINED

#include "include/gpu/GpuTypes.h"

#include <atomic>

namespace skgpu {

// Both Ganesh and Graphite track resources in similar categories (e.g. budgeted and unbudgeted,
// wrapped vs. not-wrapped, protected vs. unprotected) as well as letting clients create
// BackendTextures.
//
// It's helpful to collect all of these details in a single place to trace counters. This is done
// as two singletons, one for protected and one for unprotected memory. As singletons, the summaries
// automatically merges memory use across multiple GrDirectContexts, or Graphite Contexts. It also
// automatically merges the memory across Graphite's multiple ResourceCaches (between the Context
// and an arbitrary number of Recorders).
//
// This allows the summary to be traced to a stable track (e.g. TRACE_COUNTER) without needing to
// embed context IDs into the counter names. Globally accessibility also makes it easier to
// report when Ganesh creates backend textures.
//
// NOTE: This could go away once Graphite has a single thread-safe ResourceCache and RenderEngine
// and HWUI no longer rely on multiple contexts. Then the tracing logic can exist directly within
// ResourceCache or GrResourceCache.
class GlobalResourceStats {
public:

    // Push stats out to TRACE_COUNTER. This could instead be used to produce a snapshot struct that
    // could be returned to a client for custom consumption.
    static void TraceStatsSummary() {
        Singleton(Protected::kNo).traceStatsSummary();
        Singleton(Protected::kYes).traceStatsSummary();
    }

    // -- The remaining functions should only be used by skgpu::graphite::ResourceCache and
    //    ResourceProvider, or GrResourceCache and the various Ganesh backend texture factories.

    // Called when a resource is created or newly wrapped
    static void RecordNewResource(Protected isProtected,
                                  size_t size,
                                  Budgeted budgeted) {
        Singleton(isProtected).recordNewResource(size, budgeted);
    }
    // Called when a resource is purged, which are always assumed to be budgeted when not wrapped
    static void RecordPurgeResource(Protected isProtected, size_t size) {
        Singleton(isProtected).recordPurgeResource(size);
    }

    // Called when a resource changes its Budgeted policy, moving `size` bytes from its old budget
    // (assumed opposite of `newBudget`) to `newBudget`'s total.
    static void RecordResourceBudgetChange(Protected isProtected, size_t size, Budgeted newBudget) {
        Singleton(isProtected).recordResourceBudgetChange(size, newBudget);
    }
    // Called when a resource's underlying size has changed.
    static void RecordResourceUpdatedSize(Protected isProtected,
                                          size_t newSize,
                                          size_t oldSize,
                                          Budgeted budgeted) {
        Singleton(isProtected).recordResourceUpdatedSize(newSize, oldSize, budgeted);
    }

    // Called when a resource of the given byte size changes from non-purgeable to purgeable.
    static void RecordResourcePurgeable(Protected isProtected, size_t size) {
        Singleton(isProtected).recordResourcePurgeable(size);
    }
    // Called when a resource of the given byte size changes from purgeable to non-purgeable.
    static void RecordResourceNonpurgeable(Protected isProtected, size_t size) {
        Singleton(isProtected).recordResourceNonpurgeable(size);
    }

    // Called when a GPU BackendTexture is created (at the point of creation, likely before it is
    // wrapped into an SkImage or SkSurface (e.g. resource)).
    static void RecordCreateBackendTexture(Protected isProtected, size_t size) {
        Singleton(isProtected).recordCreateBackendTexture(size);
    }
    // Called when a GPU BackendTexture is explicitly destroyed by the client.
    static void RecordDeleteBackendTexture(Protected isProtected, size_t size) {
        Singleton(isProtected).recordDeleteBackendTexture(size);
    }

private:
    GlobalResourceStats(Protected isProtected) : fProtected(isProtected) {}

    static GlobalResourceStats& Singleton(Protected isProtected) {
        static GlobalResourceStats kUnprotected{Protected::kNo};
        static GlobalResourceStats kProtected{Protected::kYes};
        return isProtected == Protected::kYes ? kProtected : kUnprotected;
    }

    void recordNewResource(size_t size, Budgeted);
    void recordPurgeResource(size_t size);

    void recordResourceBudgetChange(size_t size, Budgeted newBudget);
    void recordResourceUpdatedSize(size_t newSize, size_t oldSize, Budgeted);

    void recordResourcePurgeable(size_t);
    void recordResourceNonpurgeable(size_t);

    void recordCreateBackendTexture(size_t size);
    void recordDeleteBackendTexture(size_t size);

    void traceStatsSummary() const;

    const Protected fProtected;

    std::atomic<size_t> fBudgetedBytes = 0; // Includes purgeable bytes
    std::atomic<size_t> fPurgeableBytes = 0; // Resources that can be safely deleted at any time

    // Resources created via Skia APIs for an SkImage or SkSurface that are owned by the client and
    // not counted against any budget. Ganesh defaults its public factories to still creating
    // budgeted resources whereas Graphite creates these as unbudgeted.
    //
    // This does *not* include wrapped resources (e.g. a BackendTexture wrapped in an SkImage).
    // Those bytes are either already counted in fBackendTexBytes, in which case whether or not it's
    // also viewed as an SkImage is noise, or the wrapped bytes are 100% the client's responsibility
    // so Skia doesn't want to trace them out in its resource memory summary.
    std::atomic<size_t> fUnbudgetedBytes = 0;

    // Skia-created BackendTextures and wrapped AHBs. These are owned by the client and the client
    // must invoke special functions to tell Skia when to destroy them, but are distinguished from
    // fully wrapped BackendTextures whose creation and destruction happen outside of Skia.
    //
    // Wrapping an AHB is considered creating a BackendTexture and included in this category even
    // though the AHB itself exists outside of Skia; this is because the GL texture or VkImage
    // wrapping the AHB are created and destroyed with Skia's codepaths.
    std::atomic<size_t> fBackendTexBytes = 0;
};

} // namespace skgpu

#endif // skgpu_GlobalResourceStats_DEFINED
