/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceAllocator_DEFINED
#define GrResourceAllocator_DEFINED

#include "include/private/SkTHash.h"

#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrHashMapWithCache.h"
#include "src/gpu/GrSurface.h"
#include "src/gpu/GrSurfaceProxy.h"

#include "src/core/SkArenaAlloc.h"
#include "src/core/SkTMultiMap.h"

class GrResourceProvider;

// Print out explicit allocation information
#define GR_ALLOCATION_SPEW 0

// Print out information about interval creation
#define GR_TRACK_INTERVAL_CREATION 0

/*
 * The ResourceAllocator explicitly distributes GPU resources at flush time. It operates by
 * being given the usage intervals of the various proxies. It keeps these intervals in a singly
 * linked list sorted by increasing start index. (It also maintains a hash table from proxyID
 * to interval to find proxy reuse). When it comes time to allocate the resources it
 * traverses the sorted list and:
 *     removes intervals from the active list that have completed (returning their GrSurfaces
 *     to the free pool)

 *     allocates a new resource (preferably from the free pool) for the new interval
 *     adds the new interval to the active list (that is sorted by increasing end index)
 *
 * Note: the op indices (used in the usage intervals) come from the order of the ops in
 * their opsTasks after the opsTask DAG has been linearized.
 *
 *************************************************************************************************
 * How does instantiation failure handling work when explicitly allocating?
 *
 * In the gather usage intervals pass all the GrSurfaceProxies used in the flush should be
 * gathered (i.e., in GrOpsTask::gatherProxyIntervals).
 *
 * The allocator will churn through this list but could fail anywhere.
 *
 * Allocation failure handling occurs at two levels:
 *
 * 1) If the GrSurface backing an opsTask fails to allocate then the entire opsTask is dropped.
 *
 * 2) If an individual GrSurfaceProxy fails to allocate then any ops that use it are dropped
 * (via GrOpsTask::purgeOpsWithUninstantiatedProxies)
 *
 * The pass to determine which ops to drop is a bit laborious so we only check the opsTasks and
 * individual ops when something goes wrong in allocation (i.e., when the return code from
 * GrResourceAllocator::assign is bad)
 *
 * All together this means we should never attempt to draw an op which is missing some
 * required GrSurface.
 *
 * One wrinkle in this plan is that promise images are fulfilled during the gather interval pass.
 * If any of the promise images fail at this stage then the allocator is set into an error
 * state and all allocations are then scanned for failures during the main allocation pass.
 */
class GrResourceAllocator {
public:
    GrResourceAllocator(GrResourceProvider* resourceProvider SkDEBUGCODE(, int numOpsTasks))
            : fResourceProvider(resourceProvider) {}

    ~GrResourceAllocator();

    unsigned int curOp() const { return fNumOps; }
    void incOps() { fNumOps++; }

    /** Indicates whether a given call to addInterval represents an actual usage of the
     *  provided proxy. This is mainly here to accommodate deferred proxies attached to opsTasks.
     *  In that case we need to create an extra long interval for them (due to the upload) but
     *  don't want to count that usage/reference towards the proxy's recyclability.
     */
    enum class ActualUse : bool {
        kNo  = false,
        kYes = true
    };

    // Add a usage interval from 'start' to 'end' inclusive. This is usually used for renderTargets.
    // If an existing interval already exists it will be expanded to include the new range.
    void addInterval(GrSurfaceProxy*, unsigned int start, unsigned int end, ActualUse actualUse
                     SkDEBUGCODE(, bool isDirectDstRead = false));

    // Assign resources to all proxies. Returns whether the assignment was successful.
    bool assign();

#if GR_ALLOCATION_SPEW
    void dumpIntervals();
#endif

private:
    class Interval;
    class Register;

    // Remove dead intervals from the active list
    void expire(unsigned int curIndex);

    // These two methods wrap the interactions with the free pool
    void recycleRegister(Register* r);
    Register* findRegisterFor(const GrSurfaceProxy* proxy);

    struct FreePoolTraits {
        static const GrScratchKey& GetKey(const Register& r) {
            return r.scratchKey();
        }

        static uint32_t Hash(const GrScratchKey& key) { return key.hash(); }
        static void OnFree(Register* r) { }
    };
    typedef SkTMultiMap<Register, GrScratchKey, FreePoolTraits> FreePoolMultiMap;

    typedef SkTHashMap<uint32_t, Interval*, GrCheapHash> IntvlHash;

    // Right now this is just a wrapper around an actual SkSurface.
    // In the future this will be a placeholder for an SkSurface that will be
    // created later, after the user of the resource allocator commits to
    // a specific series of intervals.
    class Register {
    public:
        Register(sk_sp<GrSurface> s) : fSurface(std::move(s)) {
            SkASSERT(fSurface);
            SkDEBUGCODE(fUniqueID = CreateUniqueID();)
        }
        ~Register() = default;

        const GrScratchKey& scratchKey() const {
            return fSurface->resourcePriv().getScratchKey();
        }

        GrSurface* surface() const { return fSurface.get(); }
        sk_sp<GrSurface> refSurface() const { return fSurface; }

        SkDEBUGCODE(uint32_t uniqueID() const { return fUniqueID; })

    private:
        sk_sp<GrSurface> fSurface;

#ifdef SK_DEBUG
        uint32_t         fUniqueID;

        static uint32_t  CreateUniqueID();
#endif
    };

    class Interval {
    public:
        Interval(GrSurfaceProxy* proxy, unsigned int start, unsigned int end)
            : fProxy(proxy)
            , fStart(start)
            , fEnd(end) {
            SkASSERT(proxy);
            SkDEBUGCODE(fUniqueID = CreateUniqueID());
#if GR_TRACK_INTERVAL_CREATION
            SkString proxyStr = proxy->dump();
            SkDebugf("New intvl %d: %s [%d, %d]\n", fUniqueID, proxyStr.c_str(), start, end);
#endif
        }

        const GrSurfaceProxy* proxy() const { return fProxy; }
        GrSurfaceProxy* proxy() { return fProxy; }

        unsigned int start() const { return fStart; }
        unsigned int end() const { return fEnd; }

        void setNext(Interval* next) { fNext = next; }
        const Interval* next() const { return fNext; }
        Interval* next() { return fNext; }

        Register* getRegister() const { return fRegister; }
        void setRegister(Register* r) { fRegister = r; }

        bool isSurfaceRecyclable() const;

        void addUse() { fUses++; }
        int uses() const { return fUses; }

        void extendEnd(unsigned int newEnd) {
            if (newEnd > fEnd) {
                fEnd = newEnd;
#if GR_TRACK_INTERVAL_CREATION
                SkDebugf("intvl %d: extending from %d to %d\n", fUniqueID, fEnd, newEnd);
#endif
            }
        }

        SkDEBUGCODE(uint32_t uniqueID() const { return fUniqueID; })

    private:
        GrSurfaceProxy*  fProxy;
        unsigned int     fStart;
        unsigned int     fEnd;
        Interval*        fNext = nullptr;
        unsigned int     fUses = 0;
        Register*        fRegister = nullptr;

#ifdef SK_DEBUG
        uint32_t        fUniqueID;

        static uint32_t CreateUniqueID();
#endif
    };

    class IntervalList {
    public:
        IntervalList() = default;
        ~IntervalList() {
            // The only time we delete an IntervalList is in the GrResourceAllocator dtor.
            // Since the arena allocator will clean up for us we don't bother here.
        }

        bool empty() const {
            SkASSERT(SkToBool(fHead) == SkToBool(fTail));
            return !SkToBool(fHead);
        }
        const Interval* peekHead() const { return fHead; }
        Interval* peekHead() { return fHead; }
        Interval* popHead();
        void insertByIncreasingStart(Interval*);
        void insertByIncreasingEnd(Interval*);

    private:
        SkDEBUGCODE(void validate() const;)

        Interval* fHead = nullptr;
        Interval* fTail = nullptr;
    };

    // Compositing use cases can create > 80 intervals.
    static const int kInitialArenaSize = 128 * sizeof(Interval);

    GrResourceProvider*          fResourceProvider;
    FreePoolMultiMap             fFreePool;          // Recently created/used GrSurfaces
    IntvlHash                    fIntvlHash;         // All the intervals, hashed by proxyID

    IntervalList                 fIntvlList;         // All the intervals sorted by increasing start
    IntervalList                 fActiveIntvls;      // List of live intervals during assignment
                                                     // (sorted by increasing end)
    unsigned int                 fNumOps = 0;

    SkDEBUGCODE(bool             fAssigned = false;)

    SkSTArenaAlloc<kInitialArenaSize>   fInternalAllocator; // intervals & registers live here
    bool                                fFailedInstantiation = false;
};

#endif // GrResourceAllocator_DEFINED
