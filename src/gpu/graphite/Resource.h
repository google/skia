/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Resource_DEFINED
#define skgpu_graphite_Resource_DEFINED

#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkMutex.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <atomic>
#include <functional>
#include <string>
#include <string_view>

class SkMutex;
class SkTraceMemoryDump;

namespace skgpu::graphite {

class GlobalCache;
class ResourceCache;
class SharedContext;
class Texture;

/**
 * Base class for objects that can be kept in the ResourceCache and created or fetched by a
 * ResourceProvider. A ResourceProvider controls the creation of a Resource and determines how
 * its lifecycle is managed (e.g. is it cached, shareable, etc.). Once created, however, the
 * Resource's lifecycle is controlled by the provider's ResourceCache so that GPU resources can
 * be reused when ref counts reach 0; a Resource that has no more refs and does not belong to a
 * cache is simply deleted.
 *
 * Resource Threading Model
 * ========================
 *
 * A Resource is allowed to be used in multiple threads and be deleted from any thread safely.
 * Resource refs are broken into several categories: usage (public C++ references), command buffer
 * (representing active GPU work), and internal (managed by the ResourceCache). Non-cache refs can
 * be removed and reach zero from any thread and be safely returned to the ResourceCache. Unlike
 * Resource, ResourceCache and ResourceProvider are not thread safe and are only used by a single
 * thread.
 *
 * Active Threads:
 *   1. Context thread: this is the primary thread where Recordings are inserted and GPU work is
 *      scheduled.
 *   2. Cache thread: this is the thread that the ResourceCache is associated with. This can be the
 *      same as the context thread if the ResourceCache and ResourceProvider belong to the Context.
 *      However, a Recorder's ResourceCache/Provider can be used on an arbitrary single thread the
 *      client is using for that specific Recorder.
 *   3. Usage threads: once created on the cache thread, a Resource is not restricted to just the
 *      cache thread and context thread. The Resource can have references held on any other
 *      Recorder's thread if multiple work streams use the resource, or may end up held in other
 *      client threads naturally as part of their lifecycle management of Skia's API objects.
 *         - A Resource could be accessed by an arbitrary number of usage threads.
 *
 * Reference Types:
 *   1. Usage refs: a Resource starts with a single usage ref when created by the ResourceProvider,
 *      which is then given to the caller as an `sk_sp<R>`. Once it escapes the ResourceProvider,
 *      operations on the `sk_sp` can add more usage refs. Usage refs track the liveness of the
 *      Resource from C++ code external to the ResourceCache (either higher-level Skia/Graphite
 *      objects or client objects keeping it alive).
 *   2. Command buffer refs: every Resource used in a command buffer has command buffer refs added
 *      when referencing work (e.g. a Recording) is inserted into the Context. These refs are
 *      released when the command buffer's finish procs are called and it's known that the GPU-side
 *      operations are completely done with the Resources.
 *   3. Cache ref: A ResourceProvider can choose to register a Resource with its ResourceCache at
 *      creation time before returning the Resource. The ResourceCache holds a single ref on every
 *      Resource registered with it until the cache is shutdown.
 *   4. Return queue ref: At most one ref held while the Resource is being added to the cache's
 *      return queue and held until it's removed from the queue on the cache thread.

 *
 * Lifecycle:
 *   1. Initialization: all Resources are created via the ResourceProvider, which also coordinates
 *      with its ResourceCache. If an existing resource can't be used, a new one is created. The
 *      initialization phase is the period between when the provider creates the Resource object and
 *      when it's returned to the caller. The resource can be assigned a key and added to the cache
 *      at this point.
 *   2. Reusable: when all usage refs are removed, some Resources can become reusable and move into
 *      the return queue (on any thread). On the cache thread, the queue is regularly processed and
 *      resources that remain reusable are tracked appropriately within the cache.
 *         - Resources that are only modified via command buffer operations can be reusable when
 *           usage refs reach 0 because future modifications won't conflict with any outstanding
 *           work that is holding current command buffer refs.
 *         - Resources that can be modified directly (e.g. mapped buffers) are not reusable until
 *           there are no usage or command buffer refs to ensure modifications don't become visible
 *           to the GPU before intended.
 *   3. Purgeable: when all usage refs and command buffer refs are removed, a Resource is purgeable.
 *      Resources that weren't considered reusable while having outstanding command buffer refs are
 *      now also reusable. Purgeable resources are also added to the return queue so that the cache
 *      can decide whether or not to keep the resource alive (based on budget limits).
 *   4. Destruction: if a Resource has no refs at all, it can be destroyed. A purgeable resource
 *      that was not registered with a cache will never have a cache ref or return queue ref so is
 *      immediately destroyed. When a cache processes a purgeable resource from the return queue, it
 *      can choose to drop its cache ref (and return queue ref) so the resource will be destroyed.
 *      Purgeable resources that had been kept alive for reuse can also be dropped by the cache when
 *      it is being purged (for budget or idleness reasons).
 *   5. Shutdown: when a Context or Recorder are deleted, its ResourceCache is shutdown. The cache
 *      removes all of its cache refs and blocks any further changes to the return queue. This
 *      allows Resources to be destroyed ASAP when they become purgeable instead of going back to
 *      the shutdown cache. However, every Resource registered with the cache keeps the cache alive.
 *      Once every cached resource is destroyed, the cache itself will be destroyed.
 *
 * Ref Counting and State Transition Properties:
 *   - The cache ref can only be added during initialization (it is uniquely held and on the cache
 *     thread).
 *   - The cache ref can only be removed by the ResourceCache on the cache thread (purging
 *     operations and shutdown are all single-threaded functions on the cache thread).
 *   - The return queue ref can be added by any thread when the Resource transitions to reusable
 *     and/or purgeable.
 *   - The return queue ref cannot be added more than once; if the Resource goes through multiple
 *     transitions before being processed by the cache, the second transition does not need to be
 *     put into the queue.
 *   - The return queue ref cannot be added directly, but only as part of a usage/CB unref that
 *     transitions the resource to being reusable or purgeable. The return queue ref is added
 *     atomically with the other unref.
 *   - The return queue ref can only be removed by the resource cache on the cache thread, unless
 *     the cache was shutdown already. In that case the ref is removed by the adding thread. The
 *     cache will only remove the return queue ref *after* the resource is removed from the queue.
 *   - If the resource was not registered with a cache during initialization, its cache and return
 *     queue refs will always be zero.
 *   - Regular usage refs can only be added when there was prior usage ref held by the caller. This
 *     can happen on any thread.
 *   - Command buffer refs can only be added if the resource has a usage ref (that is held through
 *     the CB ref'ing operation). This can happen on any thread (although in practice just the
 *     context thread).
 *   - A command buffer ref cannot be promoted back to a usage ref.
 *   - Both command buffer and usage refs can be removed from any thread (although CB refs are
 *     likely to be unreffed on the context thread).
 *   - When the usage ref count reaches zero, it can only become non-zero via the cache thread by
 *     actions of its ResourceCache.
 *        - Actions predicated on usage refs being 0 on the cache thread are safe.
 *        - Such actions on other threads are not safe because the cache thread could process the
 *          return queue and reuse the resource, or skip the return queue entirely for shareable
 *          resources.
 *   - When the resource becomes purgeable, it can only become non-purgeable on the cache thread
 *     by action of the ResourceCache.
 *        - Since there are no other usage refs (beyond what the cache might re-add), no other
 *          thread can re-add a command buffer ref.
 *
 */
class Resource {
    enum class RefType {
        kUsage, // Counts controlled by `sk_sp` and tracks liveness from external C++ code.
        kCommandBuffer, // Incremented in Context::insertRecording, decremented by finish procs.
        kCache, // At most 1 ref, added in registerWithCache(), removed on cache shutdown or purge.
        kReturnQueue, // At most 1 ref, held while in the cache's return queue.
    };

public:
    Resource(const Resource&) = delete;
    Resource(Resource&&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource& operator=(Resource&&) = delete;

    // Adds a usage ref to the resource. Named ref so we can easily manage usage refs with sk_sp.
    void ref() const {
        // Only the cache should be able to add the first usage ref to a resource.
        this->addRef<RefType::kUsage>();
    }

    // Removes a usage ref from the resource
    void unref() const {
        this->removeRef<RefType::kUsage>();
    }

    // Adds a command buffer ref to the resource
    void refCommandBuffer() const {
        this->addRef<RefType::kCommandBuffer>();
    }

    // Removes a command buffer ref from the resource
    void unrefCommandBuffer() const {
        this->removeRef<RefType::kCommandBuffer>();
    }

    Ownership ownership() const { return fOwnership; }

    Budgeted budgeted() const { return fBudgeted; }
    Shareable shareable() const { return fShareable; }
    const GraphiteResourceKey& key() const { return fKey; }

    // Retrieves the amount of GPU memory used by this resource in bytes. It is approximate since we
    // aren't aware of additional padding or copies made by the driver.
    size_t gpuMemorySize() const { return fGpuMemorySize; }

    class UniqueID {
    public:
        UniqueID() = default;

        explicit UniqueID(uint32_t id) : fID(id) {}

        uint32_t asUInt() const { return fID; }

        bool operator==(const UniqueID& other) const { return fID == other.fID; }
        bool operator!=(const UniqueID& other) const { return !(*this == other); }

    private:
        uint32_t fID = SK_InvalidUniqueID;
    };

    // Gets an id that is unique for this Resource object. It is static in that it does not change
    // when the content of the Resource object changes. This will never return 0.
    UniqueID uniqueID() const { return fUniqueID; }

    std::string getLabel() const { return fLabel; }

    // We allow the label on a Resource to change when used for a different function. For example
    // when reusing a scratch Texture we can change the label to match callers current use.
    void setLabel(std::string_view label) {
        fLabel = label;

        if (!fLabel.empty()) {
            const std::string fullLabel = "Skia_" + fLabel;
            this->setBackendLabel(fullLabel.c_str());
        }
    }

    // Tests whether a object has been abandoned or released. All objects will be in this state
    // after their creating Context is destroyed or abandoned.
    //
    // @return true if the object has been released or abandoned,
    //         false otherwise.
    // TODO: As of now this function isn't really needed because in freeGpuData we are always
    // deleting this object. However, I want to implement all the purging logic first to make sure
    // we don't have a use case for calling internalDispose but not wanting to delete the actual
    // object yet.
    bool wasDestroyed() const { return fSharedContext == nullptr; }

    // Describes the type of gpu resource that is represented by the implementing
    // class (e.g. texture, buffer, etc).  This data is used for diagnostic
    // purposes by dumpMemoryStatistics().
    //
    // The value returned is expected to be long lived and will not be copied by the caller.
    virtual const char* getResourceType() const = 0;

    virtual const Texture* asTexture() const { return nullptr; }

#if defined(GPU_TEST_UTILS)
    bool testingShouldDeleteASAP() const { return fDeleteASAP == DeleteASAP::kYes; }
#endif

protected:
    Resource(const SharedContext*,
             Ownership,
             size_t gpuMemorySize,
             bool reusableRequiresPurgeable = false);
    virtual ~Resource();

    const SharedContext* sharedContext() const { return fSharedContext; }

    // Needs to be protected for DawnBuffer's emscripten prepareForReturnToCache
    void setDeleteASAP() { fDeleteASAP = DeleteASAP::kYes; }

private:
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // The following set of functions are only meant to be called by the [Global|Proxy]Cache. We
    // don't want them public general users of a Resource, but they also aren't purely internal.
    ///////////////////////////////////////////////////////////////////////////////////////////////
    friend class ProxyCache; // for setDeleteASAP and updateAccessTime
    friend GlobalCache; // for lastAccessTime and updateAccessTime

    enum class DeleteASAP : bool {
        kNo = false,
        kYes = true,
    };

    DeleteASAP shouldDeleteASAP() const { return fDeleteASAP; }

    // In the ResourceCache this is called whenever a Resource is moved into the purgeableQueue. It
    // may also be called by the ProxyCache and GlobalCache to track the time on Resources they are
    // holding on to.
    void updateAccessTime() { fLastAccess = skgpu::StdSteadyClock::now(); }
    skgpu::StdSteadyClock::time_point lastAccessTime() const { return fLastAccess; }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // The following set of functions are only meant to be called by the ResourceCache. We don't
    // want them public general users of a Resource, but they also aren't purely internal calls.
    ///////////////////////////////////////////////////////////////////////////////////////////////
    friend class ResourceCache;

    void setBudgeted(Budgeted budgeted) {
        SkASSERT(budgeted == Budgeted::kNo || fOwnership == Ownership::kOwned);
        fBudgeted = budgeted;
    }
    void setShareable(Shareable shareable) {
        SkASSERT(shareable == Shareable::kNo || fBudgeted == Budgeted::kYes);
        fShareable = shareable;
    }

    void setAvailableForReuse(bool avail) { fAvailableForReuse = avail; }
    bool isAvailableForReuse() const { return fAvailableForReuse; }

    uint32_t lastUseToken() const { return fLastUseToken; }
    void setLastUseToken(uint32_t token) { fLastUseToken = token; }

    void setNextInReturnQueue(Resource* next) {
        SkASSERT(this->hasReturnQueueRef());
        fNextInReturnQueue = next;
    }

    int* accessCacheIndex() const { return &fCacheArrayIndex; }
    const ResourceCache* cache() const { return fReturnCache.get(); }

    // If possible, queries the backend API to check the current allocation size of the gpu
    // resource and updates the tracked value. This is specifically useful for Vulkan backends which
    // use lazy allocated memory for "memoryless" resources. Ideally that memory should stay zero
    // throughout its usage, but certain usage patterns can trigger the device to commit real memory
    // to the resource. So this will allow us to have a more accurate tracking of our memory usage.
    void updateGpuMemorySize() { fGpuMemorySize = this->onUpdateGpuMemorySize(); }

    // Dumps memory usage information for this Resource to traceMemoryDump.
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump, bool inPurgeableQueue) const;

    /**
     * If the resource has a non-shareable key then this gives the resource subclass an opportunity
     * to prepare itself to re-enter the cache. The ResourceCache extends its privilege to take the
     * first UsageRef to this function via takeRef. If takeRef is called this resource will not
     * immediately enter the cache but will be re-reprocessed when the usage ref count again reaches
     * zero.
     *
     * Return true if takeRef() was invoked.
     */
    virtual bool prepareForReturnToCache(const std::function<void()>& takeRef) { return false; }

    // Adds a cache ref to the resource. May only be called once.
    void registerWithCache(sk_sp<ResourceCache>, const GraphiteResourceKey&, Budgeted, Shareable);

    // This version of ref allows adding a ref when the usage count is 0. This should only be called
    // from the ResourceCache.
    void initialUsageRef() const {
        this->addRef<RefType::kUsage, /*MustHaveUsageRefs=*/false>();
    }

    // Removes a cache ref from the resource. The unref here should only ever be called from the
    // ResourceCache and only in the Recorder/Context thread the ResourceCache is part of.
    void unrefCache() const {
        SkASSERT(fReturnCache);
        this->removeRef<RefType::kCache>();
    }

    // Removes the return queue ref that was held while the Resource was in the queue. This can only
    // be called by the ResourceCache on its thread. It should not be called after unrefCache().
    // It must only be called after the cache has removed the resource from its return queue.
    //
    // Returns {isReusable, isPurgeable} atomically based on the reference state when the return
    // queue ref was removed. `isReusable` is true if all refs affecting reusability were zero
    // when the queue ref was removed. `isPurgeable` is true if all usage and command buffer refs
    // were zero.
    //
    // If true is returned there are no other refs that could trigger a reusable or purgeable state
    // change. A resource that entered the return queue due to becoming reusable or purgeable only
    // happens if that ref count reached zero, so there should be no external ref holder (other than
    // the ResourceCache with its separate cache ref). However, unrefReturnQueue() is only called by
    // the ResourceCache so it won't be simultaneously handing out usage refs.
    //
    // If false is returned, it is possible for the resource to immediately become purgeable on
    // another thread but since this thread has released its return queue ref, the Resource will
    // simply go back in the next return queue.
    //
    // The cache should track the Resource based on this return value instead of re-checking the
    // ref counts as that would not be an atomic operation.
    std::tuple<bool, bool, Resource*> unrefReturnQueue() {
        // We must reset the fNextInReturnQueue value *before* removing the return queue ref, but we
        // need to return the old value to the ResourceCache so that it can continue iterating over
        // the linked list.
        Resource* next = fNextInReturnQueue;
        fNextInReturnQueue = nullptr;

        uint64_t origRefs = this->removeRef<RefType::kReturnQueue>();

        // Since we should always have a cache ref when this is called, the Resource will never be
        // transitioning to having zero refs, although if `true` is returned the cache may choose to
        // then drop its cache ref.
        SkASSERT((origRefs & RefMask(RefType::kCache)) != 0);
        // `fReusableRefMask` always includes the ReturnQueue ref mask, and since we just removed
        // the return ref value, `origRefs` also includes the the ReturnQueue ref mask bit. We have
        // to compare to the ref mask to detect the case when the actual reusable refs are all zero.
        // Since PurgeableMask() does not add the ReturnQueue ref mask, it *can* compare to zero.
        return {(origRefs & fReusableRefMask) == RefMask(RefType::kReturnQueue),
                (origRefs & PurgeableMask()) == 0,
                next};
    }

#if defined(SK_DEBUG) || defined(GPU_TEST_UTILS)
    bool hasCacheRef() const {
        return (fRefs.load(std::memory_order_acquire) & RefMask(RefType::kCache)) != 0;
    }

    bool hasReturnQueueRef() const {
        return (fRefs.load(std::memory_order_acquire) & RefMask(RefType::kReturnQueue)) != 0;
    }

    bool inReturnQueue() const {
        return this->hasReturnQueueRef() && SkToBool(fNextInReturnQueue);
    }

    bool isUsableAsScratch() const {
        // This is only called by the ResourceCache, so the state of the Resource's refs won't
        // be changed by another thread when isReusable is true.
        uint64_t origRefs = fRefs.load(std::memory_order_acquire) & ~RefMask(RefType::kReturnQueue);
        bool isReusable = (origRefs & fReusableRefMask) == 0;
        return fShareable == Shareable::kScratch || (fShareable == Shareable::kNo && isReusable);
    }

    bool isPurgeable() const {
        // This is only called by the ResourceCache on its thread; if the usage and CB ref counts
        // are 0, the ResourceCache is the only way in which they can become non-zero again.
        return (fRefs.load(std::memory_order_acquire) & PurgeableMask()) == 0;
    }

    bool isUniquelyHeld() const {
        // This intentionally checks that the cache ref and return queue refs are 0, so that fRefs
        // is compared to the value it is initialized with.
        return fRefs.load(std::memory_order_acquire) == RefIncrement(RefType::kUsage);
    }

    bool hasAnyRefs() const {
        // Because all ref counts are packed into the same atomic, when this load is actually 0
        // there are no other threads that can reach the object and add new refs (assuming a raw
        // pointer has never leaked).
        return fRefs.load(std::memory_order_acquire) != 0;
    }
#endif

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // The remaining calls are meant to be truely private (including virtuals for subclasses)
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Overridden to free GPU resources in the backend API.
    virtual void freeGpuData() = 0;

    // Overridden to call any release callbacks, if necessary
    virtual void invokeReleaseProc() {}

    // Overridden to set the label on the underlying GPU resource
    virtual void setBackendLabel(char const* label) {}

    // Overridden to add extra information to the memory dump.
    virtual void onDumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump,
                                        const char* dumpName) const {}


    // Overridden to calculate a more up-to-date size in bytes.
    virtual size_t onUpdateGpuMemorySize() { return fGpuMemorySize; }

    // Try to add the Resource to the cache's return queue for pending reuse.
    // This should only be called when there is a cache to return to, and the calling thread
    // successfully transitioned from no "return queue" ref to setting the return queue ref.
    //
    // Returns true if the cache accepted the Resource (in which case the set return ref should
    // remain set for the cache to remove). If false is returned, the caller should clear the
    // return queue ref (and possible dispose of the object).
    bool returnToCache() const;

    // Frees the object in the underlying 3D API *and* deletes the object itself.
    void internalDispose();

    // Resource tracks its different ref counts packed into a single atomic 64-bit value.
    // The bits are split into subfields:
    // commandBufferRefs:31 e.g. RefMask(kCB)
    // usageRefs:31              RefMask(kUsage)
    // returnQueueRef: 1         RefMask(kReturnQueue)
    // cacheRefs:1               RefMask(kCache)
    //
    // RefIncrement() and RefMask() help access specific ref type's values.
    static constexpr uint64_t RefIncrement(RefType refType) {
        switch (refType) {
            case RefType::kCommandBuffer: return (uint64_t) 1 << 33;
            case RefType::kUsage:         return (uint64_t) 1 << 2;
            case RefType::kCache:         return (uint64_t) 1 << 1;
            case RefType::kReturnQueue:   return (uint64_t) 1 << 0;
        }
        SkUNREACHABLE;
    }
    static inline constexpr uint64_t RefMask(RefType refType) {
        switch (refType) {
            case RefType::kCommandBuffer: return (((uint64_t)1 << 31) - 1) << 33;
            case RefType::kUsage:         return (((uint64_t)1 << 31) - 1) << 2;
            case RefType::kCache:         return 0b10;
            case RefType::kReturnQueue:   return 0b01;
        }
        SkUNREACHABLE;
    }
    static inline constexpr uint64_t PurgeableMask() {
        return RefMask(RefType::kUsage) | RefMask(RefType::kCommandBuffer);
    }

    template <RefType kType, bool MustHaveUsageRefs=true>
    void addRef() const {
        static_assert(kType != RefType::kReturnQueue, "return queue refs cannot be added directly");
        static constexpr uint64_t kRefIncrement = RefIncrement(kType);
        // No barrier required
        [[maybe_unused]] uint64_t origCnt =
                fRefs.fetch_add(kRefIncrement, std::memory_order_relaxed);
        // Require that there was an already held usage ref in order to add this new ref,
        // e.g. to add a command buffer ref, a usage ref must already be held; calling code can't
        // add usage refs if it wasn't explicitly handed out by the cache.
        SkASSERT(!MustHaveUsageRefs || (origCnt & RefMask(RefType::kUsage)) > 0);
        // And make sure that the specific type of ref did not overflow into another field
        SkASSERT((RefMask(kType) - (origCnt & RefMask(kType))) >= kRefIncrement);
    }

    template <RefType kType>
    uint64_t removeRef() const {
        static constexpr uint64_t kRefIncrement = RefIncrement(kType);

        uint64_t origRefs;
        if (kType == RefType::kCache || kType == RefType::kReturnQueue || !fReturnCache) {
            // Without a ResourceCache, or when it's a cache/return-queue unref, there is no
            // non-atomic work that has to happen so simply update the ref count. If the net ref
            // count reaches 0 we can safely delete the resource because no other thread will
            // increase the refs.
            origRefs = fRefs.fetch_sub(kRefIncrement, std::memory_order_acq_rel);
            SkASSERT((origRefs & RefMask(kType)) >= kRefIncrement); // had a ref to remove

            if (origRefs == kRefIncrement) {
                SkASSERT(!this->hasAnyRefs());
                Resource* mutableThis = const_cast<Resource*>(this);
                mutableThis->internalDispose();
            }
        } else {
            SkASSERT(kType == RefType::kCommandBuffer || kType == RefType::kUsage);
            // When removing a usage or CB ref and the resource is registered with the cache,
            // it may need to be returned to the cache. A resource can only be in the return queue
            // a single time and must remain alive until cache removes it from the queue. A CAS
            // loop is used to atomically decrement the ref and add the return queue ref.
            uint64_t nextRefs;
            bool needsReturn;
            do {
                origRefs = fRefs.load(std::memory_order_acquire);
                SkASSERT((origRefs & RefMask(kType)) >= kRefIncrement); // have a ref to remove

                // When unreffing a usage or command buffer ref, the Resource needs to return to
                // the queue when:
                //  - it's not already in the return queue (return queue ref is 0) AND
                //  - it's transitioning from non-reusable -> reusable OR non-purgeable -> purgeable
                //
                // Including RefMask(kReturnQueue) in the bitwise &'s before comparing to the
                // ref increment ensures that the return queue ref was 0 in origRefs.
                static constexpr uint64_t kPurgeableReturnMask = PurgeableMask() |
                                                                 RefMask(RefType::kReturnQueue);
                // fReusableRefMask should have added this bit added during construction.
                SkASSERT((fReusableRefMask & RefMask(RefType::kReturnQueue)) != 0);
                // This expression matches the above logic for returning because:
                //  - Both kPurgeableReturnMask and fReusableRefMask include the return queue bit,
                //    but kRefIncrement does not. The only way the comparisons can be true is if
                //    the return queue bit is unset.
                //  - When the resource is reusable only when purgeable, then both sides of the ||
                //    are identical because fReusableRefMask will equal kPurgeableReturnMask.
                //    And if the == returns true, we know origRefs was non-zero and nextRefs will
                //    be zero since it subtracts kRefIncrement.
                //  - When the resource is reusable when just the usage refs reach 0, the purgeable
                //    state transition works like before. But fReusableRefMask will mask out any
                //    non-zero bits in the command buffer subfield.
                //      - When kRefType==kUsage, the right-hand == will be true when origRefs had
                //        one usage ref left and nextRefs holds zero.
                //      - When kRefType==kCommandBuffer, the right-hand side of the || will always
                //        be false because kRefIncrement will hold bits outside of fReusableRefMask.
                //        This ensures that the non-reusable -> reusable transition occurs solely
                //        on removing a usage ref.
                SkASSERT((kRefIncrement & RefMask(RefType::kReturnQueue)) == 0);
                needsReturn = ((origRefs & kPurgeableReturnMask) == kRefIncrement) ||
                              ((origRefs & fReusableRefMask) == kRefIncrement);

                nextRefs = (origRefs - kRefIncrement) |
                           (needsReturn ? RefMask(RefType::kReturnQueue) : 0);
                // If origRefs already included a return queue ref, nextRefs hasn't changed that
                SkASSERT((origRefs & RefMask(RefType::kReturnQueue)) ==
                         (nextRefs & RefMask(RefType::kReturnQueue)) || needsReturn);
            } while (!fRefs.compare_exchange_weak(origRefs, nextRefs,
                                                  std::memory_order_release,
                                                  std::memory_order_relaxed));
            // NOTE: because RefMask(RefType::kReturnQueue) was included in the `needsReturn` check,
            // we know that it was unset in `origRefs`, and was added to `nextRefs`. The CAS ensures
            // that this was the thread that added the return queue ref if `needsReturn` is true
            // when the do-while loop exits.

            if (needsReturn && !this->returnToCache()) {
                // The cache rejected the resource, so we need to unset the "return queue" ref that
                // we added above, which may be the last ref keeping the object alive.
                SkASSERT(!fNextInReturnQueue);
                origRefs = this->removeRef<RefType::kReturnQueue>();
                // so do not access *this* after this point!
            }
            // else we weren't returning the resource yet, or the cache is maintaining the return
            // ref until the return queue has been drained.
        }

        return origRefs;
    }

    static constexpr size_t kInvalidGpuMemorySize = ~static_cast<size_t>(0);

    // See RefIncrement() for how the bits in this field are interpreted.
    mutable std::atomic<uint64_t> fRefs;

    // Depending on when the resource can be reused, there are two base values:
    // 1. RefMask(kUsage): reused while there is outstanding GPU work (CB ref count is ignored).
    // 2. RefMask(kUsage) | RefMask(kCB): cannot be reused until it is also purgeable.
    // To simplify logic in removeRef(), this value always includes RefMask(kReturnQueue).
    // See removeRef() for rationale.
    //
    // NOTE: Reusability is related but distinct from shareability. Shareability takes into account
    // external information about when and for how long the state of the resource must remain stable
    // We track when all resources become "reusable" again even if they were fully shareable because
    // that marks when the resource can also change its Shareable type.
    const uint64_t fReusableRefMask;

    // This is not ref'ed but internalDispose() will be called before the Gpu object is destroyed.
    // That call will set this to nullptr.
    const SharedContext* fSharedContext;

    const Ownership fOwnership;
    const UniqueID fUniqueID;

    // The resource key and return cache are both set at most once, during registerWithCache().
    /*const*/ GraphiteResourceKey  fKey;
    /*const*/ sk_sp<ResourceCache> fReturnCache;

    // Resources added to their return cache's queue are tracked in a lock-free thread-safe
    // singly-linked list whose head element is stored on the cache, and next elements are stored
    // inline in Resource. This can only be modified by the thread that set the return queue ref,
    // or by the thread that is removing said ref.
    //
    // A null value means the Resource is not in the return queue. A non-null value means it is in
    // the queue, although the ResourceCache assigns a special sentinel value for the tail address.
    Resource* fNextInReturnQueue = nullptr;

    // The remaining fields are mutable state that is only modified by the ResourceCache on the
    // cache's thread, guarded by `fReturnCache::fSingleOwner`.

    size_t fGpuMemorySize = kInvalidGpuMemorySize;

    // All resources created internally by Graphite that are held in the ResourceCache as shared or
    // available scratch resources are considered budgeted. Resources that back client-owned objects
    // (e.g. SkSurface or SkImage) and wrapper objects (e.g. BackendTexture) do not count against
    // cache limits and therefore should never be budgeted.
    Budgeted fBudgeted = Budgeted::kNo;
    // All resources start out as non-shareable (the strictest mode) and revert to non-shareable
    // when they are returned to the cache and have no more usage refs. An available resource can
    // be returned if its shareable type matches the request, or if it was non-shareable at which
    // point the resource is upgraded to the more permissive mode (until all shared usages are
    // dropped at which point it can be used for any purpose again).
    Shareable fShareable = Shareable::kNo;

    // This is only used by ProxyCache::purgeProxiesNotUsedSince which is called from
    // ResourceCache::purgeResourcesNotUsedSince. When kYes, this signals that the Resource
    // should've been purged based on its timestamp at some point regardless of what its
    // current timestamp may indicate (since the timestamp will be updated when the Resource
    // is returned to the ResourceCache).
    DeleteASAP fDeleteASAP = DeleteASAP::kNo;

    // Set to true when the resource is contained in its cache's `fResourceMap`, which allows it to
    // be returned from findAndRefResource().
    bool fAvailableForReuse = false;

    // An index into a heap when this resource is purgeable or an array when not. This is maintained
    // by the cache. Must be mutable to fit SkTDPQueue's access API.
    mutable int fCacheArrayIndex = -1;

    // This value reflects how recently this resource was accessed in the cache. This is maintained
    // by the cache. It defines a total order over resources, even if their fLastAccess times are
    // the same (i.e. returned at time points less than the system's granularity).
    uint32_t fLastUseToken;
    skgpu::StdSteadyClock::time_point fLastAccess;

    // String used to describe the current use of this Resource.
    std::string fLabel;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Resource_DEFINED
