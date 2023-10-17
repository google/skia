/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrThreadSafeCache_DEFINED
#define GrThreadSafeCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkSpinlock.h"
#include "src/base/SkTInternalLList.h"
#include "src/core/SkTDynamicHash.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"

// Ganesh creates a lot of utility textures (e.g., blurred-rrect masks) that need to be shared
// between the direct context and all the DDL recording contexts. This thread-safe cache
// allows this sharing.
//
// In operation, each thread will first check if the threaded cache possesses the required texture.
//
// If a DDL thread doesn't find a needed texture it will go off and create it on the cpu and then
// attempt to add it to the cache. If another thread had added it in the interim, the losing thread
// will discard its work and use the texture the winning thread had created.
//
// If the thread in possession of the direct context doesn't find the needed texture it should
// add a place holder view and then queue up the draw calls to complete it. In this way the
// gpu-thread has precedence over the recording threads.
//
// The invariants for this cache differ a bit from those of the proxy and resource caches.
// For this cache:
//
//   only this cache knows the unique key - neither the proxy nor backing resource should
//              be discoverable in any other cache by the unique key
//   if a backing resource resides in the resource cache then there should be an entry in this
//              cache
//   an entry in this cache, however, doesn't guarantee that there is a corresponding entry in
//              the resource cache - although the entry here should be able to generate that entry
//              (i.e., be a lazy proxy)
//
// Wrt interactions w/ GrContext/GrResourceCache purging, we have:
//
//    Both GrContext::abandonContext and GrContext::releaseResourcesAndAbandonContext will cause
//    all the refs held in this cache to be dropped prior to clearing out the resource cache.
//
//    For the size_t-variant of GrContext::purgeUnlockedResources, after an initial attempt
//    to purge the requested amount of resources fails, uniquely held resources in this cache
//    will be dropped in LRU to MRU order until the cache is under budget. Note that this
//    prioritizes the survival of resources in this cache over those just in the resource cache.
//
//    For the 'scratchResourcesOnly' variant of GrContext::purgeUnlockedResources, this cache
//    won't be modified in the scratch-only case unless the resource cache is over budget (in
//    which case it will purge uniquely-held resources in LRU to MRU order to get
//    back under budget). In the non-scratch-only case, all uniquely held resources in this cache
//    will be released prior to the resource cache being cleared out.
//
//    For GrContext::setResourceCacheLimit, if an initial pass through the resource cache doesn't
//    reach the budget, uniquely held resources in this cache will be released in LRU to MRU order.
//
//    For GrContext::performDeferredCleanup, any uniquely held resources that haven't been accessed
//    w/in 'msNotUsed' will be released from this cache prior to the resource cache being cleaned.
class GrThreadSafeCache {
public:
    GrThreadSafeCache();
    ~GrThreadSafeCache();

#if defined(GR_TEST_UTILS)
    int numEntries() const  SK_EXCLUDES(fSpinLock);

    size_t approxBytesUsedForHash() const  SK_EXCLUDES(fSpinLock);
#endif

    void dropAllRefs()  SK_EXCLUDES(fSpinLock);

    // Drop uniquely held refs until under the resource cache's budget.
    // A null parameter means drop all uniquely held refs.
    void dropUniqueRefs(GrResourceCache* resourceCache)  SK_EXCLUDES(fSpinLock);

    // Drop uniquely held refs that were last accessed before 'purgeTime'
    void dropUniqueRefsOlderThan(
            skgpu::StdSteadyClock::time_point purgeTime)  SK_EXCLUDES(fSpinLock);

    SkDEBUGCODE(bool has(const skgpu::UniqueKey&)  SK_EXCLUDES(fSpinLock);)

    GrSurfaceProxyView find(const skgpu::UniqueKey&)  SK_EXCLUDES(fSpinLock);
    std::tuple<GrSurfaceProxyView, sk_sp<SkData>> findWithData(
            const skgpu::UniqueKey&)  SK_EXCLUDES(fSpinLock);

    GrSurfaceProxyView add(
            const skgpu::UniqueKey&, const GrSurfaceProxyView&)  SK_EXCLUDES(fSpinLock);
    std::tuple<GrSurfaceProxyView, sk_sp<SkData>> addWithData(
            const skgpu::UniqueKey&, const GrSurfaceProxyView&)  SK_EXCLUDES(fSpinLock);

    GrSurfaceProxyView findOrAdd(const skgpu::UniqueKey&,
                                 const GrSurfaceProxyView&)  SK_EXCLUDES(fSpinLock);
    std::tuple<GrSurfaceProxyView, sk_sp<SkData>> findOrAddWithData(
            const skgpu::UniqueKey&, const GrSurfaceProxyView&)  SK_EXCLUDES(fSpinLock);

    // To hold vertex data in the cache and have it transparently transition from cpu-side to
    // gpu-side while being shared between all the threads we need a ref counted object that
    // keeps hold of the cpu-side data but allows deferred filling in of the mirroring gpu buffer.
    class VertexData : public SkNVRefCnt<VertexData> {
    public:
        ~VertexData();

        const void* vertices() const { return fVertices; }
        size_t size() const { return fNumVertices * fVertexSize; }

        int numVertices() const { return fNumVertices; }
        size_t vertexSize() const { return fVertexSize; }

        // TODO: make these return const GrGpuBuffers?
        GrGpuBuffer* gpuBuffer() { return fGpuBuffer.get(); }
        sk_sp<GrGpuBuffer> refGpuBuffer() { return fGpuBuffer; }

        void setGpuBuffer(sk_sp<GrGpuBuffer> gpuBuffer) {
            // TODO: once we add the gpuBuffer we could free 'fVertices'. Deinstantiable
            // DDLs could throw a monkey wrench into that plan though.
            SkASSERT(!fGpuBuffer);
            fGpuBuffer = std::move(gpuBuffer);
        }

        void reset() {
            sk_free(const_cast<void*>(fVertices));
            fVertices = nullptr;
            fNumVertices = 0;
            fVertexSize = 0;
            fGpuBuffer.reset();
        }

    private:
        friend class GrThreadSafeCache;  // for access to ctor

        VertexData(const void* vertices, int numVertices, size_t vertexSize)
                : fVertices(vertices)
                , fNumVertices(numVertices)
                , fVertexSize(vertexSize) {
        }

        VertexData(sk_sp<GrGpuBuffer> gpuBuffer, int numVertices, size_t vertexSize)
                : fVertices(nullptr)
                , fNumVertices(numVertices)
                , fVertexSize(vertexSize)
                , fGpuBuffer(std::move(gpuBuffer)) {
        }

        const void*        fVertices;
        int                fNumVertices;
        size_t             fVertexSize;

        sk_sp<GrGpuBuffer> fGpuBuffer;
    };

    // The returned VertexData object takes ownership of 'vertices' which had better have been
    // allocated with malloc!
    static sk_sp<VertexData> MakeVertexData(const void* vertices,
                                            int vertexCount,
                                            size_t vertexSize);
    static sk_sp<VertexData> MakeVertexData(sk_sp<GrGpuBuffer> buffer,
                                            int vertexCount,
                                            size_t vertexSize);

    std::tuple<sk_sp<VertexData>, sk_sp<SkData>> findVertsWithData(
            const skgpu::UniqueKey&)  SK_EXCLUDES(fSpinLock);

    typedef bool (*IsNewerBetter)(SkData* incumbent, SkData* challenger);

    std::tuple<sk_sp<VertexData>, sk_sp<SkData>> addVertsWithData(
                                                        const skgpu::UniqueKey&,
                                                        sk_sp<VertexData>,
                                                        IsNewerBetter)  SK_EXCLUDES(fSpinLock);

    void remove(const skgpu::UniqueKey&)  SK_EXCLUDES(fSpinLock);

    // To allow gpu-created resources to have priority, we pre-emptively place a lazy proxy
    // in the thread-safe cache (with findOrAdd). The Trampoline object allows that lazy proxy to
    // be instantiated with some later generated rendering result.
    class Trampoline : public SkRefCnt {
    public:
        sk_sp<GrTextureProxy> fProxy;
    };

    static std::tuple<GrSurfaceProxyView, sk_sp<Trampoline>> CreateLazyView(GrDirectContext*,
                                                                            GrColorType,
                                                                            SkISize dimensions,
                                                                            GrSurfaceOrigin,
                                                                            SkBackingFit);
private:
    struct Entry {
        Entry(const skgpu::UniqueKey& key, const GrSurfaceProxyView& view)
                : fKey(key)
                , fView(view)
                , fTag(Entry::kView) {
        }

        Entry(const skgpu::UniqueKey& key, sk_sp<VertexData> vertData)
                : fKey(key)
                , fVertData(std::move(vertData))
                , fTag(Entry::kVertData) {
        }

        ~Entry() {
            this->makeEmpty();
        }

        bool uniquelyHeld() const {
            SkASSERT(fTag != kEmpty);

            if (fTag == kView && fView.proxy()->unique()) {
                return true;
            } else if (fTag == kVertData && fVertData->unique()) {
                return true;
            }

            return false;
        }

        const skgpu::UniqueKey& key() const {
            SkASSERT(fTag != kEmpty);
            return fKey;
        }

        SkData* getCustomData() const {
            SkASSERT(fTag != kEmpty);
            return fKey.getCustomData();
        }

        sk_sp<SkData> refCustomData() const {
            SkASSERT(fTag != kEmpty);
            return fKey.refCustomData();
        }

        GrSurfaceProxyView view() {
            SkASSERT(fTag == kView);
            return fView;
        }

        sk_sp<VertexData> vertexData() {
            SkASSERT(fTag == kVertData);
            return fVertData;
        }

        void set(const skgpu::UniqueKey& key, const GrSurfaceProxyView& view) {
            SkASSERT(fTag == kEmpty);
            fKey = key;
            fView = view;
            fTag = kView;
        }

        void makeEmpty() {
            fKey.reset();
            if (fTag == kView) {
                fView.reset();
            } else if (fTag == kVertData) {
                fVertData.reset();
            }
            fTag = kEmpty;
        }

        void set(const skgpu::UniqueKey& key, sk_sp<VertexData> vertData) {
            SkASSERT(fTag == kEmpty || fTag == kVertData);
            fKey = key;
            fVertData = std::move(vertData);
            fTag = kVertData;
        }

        // The thread-safe cache gets to directly manipulate the llist and last-access members
        skgpu::StdSteadyClock::time_point fLastAccess;
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Entry);

        // for SkTDynamicHash
        static const skgpu::UniqueKey& GetKey(const Entry& e) {
            SkASSERT(e.fTag != kEmpty);
            return e.fKey;
        }
        static uint32_t Hash(const skgpu::UniqueKey& key) { return key.hash(); }

    private:
        // Note: the unique key is stored here bc it is never attached to a proxy or a GrTexture
        skgpu::UniqueKey             fKey;
        union {
            GrSurfaceProxyView  fView;
            sk_sp<VertexData>   fVertData;
        };

        enum {
            kEmpty,
            kView,
            kVertData,
        } fTag { kEmpty };
    };

    void makeExistingEntryMRU(Entry*)  SK_REQUIRES(fSpinLock);
    Entry* makeNewEntryMRU(Entry*)  SK_REQUIRES(fSpinLock);

    Entry* getEntry(const skgpu::UniqueKey&, const GrSurfaceProxyView&)  SK_REQUIRES(fSpinLock);
    Entry* getEntry(const skgpu::UniqueKey&, sk_sp<VertexData>)  SK_REQUIRES(fSpinLock);

    void recycleEntry(Entry*)  SK_REQUIRES(fSpinLock);

    std::tuple<GrSurfaceProxyView, sk_sp<SkData>> internalFind(
            const skgpu::UniqueKey&)  SK_REQUIRES(fSpinLock);
    std::tuple<GrSurfaceProxyView, sk_sp<SkData>> internalAdd(
            const skgpu::UniqueKey&, const GrSurfaceProxyView&)  SK_REQUIRES(fSpinLock);

    std::tuple<sk_sp<VertexData>, sk_sp<SkData>> internalFindVerts(
            const skgpu::UniqueKey&)  SK_REQUIRES(fSpinLock);
    std::tuple<sk_sp<VertexData>, sk_sp<SkData>> internalAddVerts(
            const skgpu::UniqueKey&, sk_sp<VertexData>, IsNewerBetter)  SK_REQUIRES(fSpinLock);

    mutable SkSpinlock fSpinLock;

    SkTDynamicHash<Entry, skgpu::UniqueKey> fUniquelyKeyedEntryMap  SK_GUARDED_BY(fSpinLock);
    // The head of this list is the MRU
    SkTInternalLList<Entry>            fUniquelyKeyedEntryList  SK_GUARDED_BY(fSpinLock);

    // TODO: empirically determine this from the skps
    static const int kInitialArenaSize = 64 * sizeof(Entry);

    char                         fStorage[kInitialArenaSize];
    SkArenaAlloc                 fEntryAllocator{fStorage, kInitialArenaSize, kInitialArenaSize};
    Entry*                       fFreeEntryList  SK_GUARDED_BY(fSpinLock);
};

#endif // GrThreadSafeCache_DEFINED
