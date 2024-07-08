/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceAllocator_DEFINED
#define GrResourceAllocator_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkTHash.h"
#include "src/core/SkTMultiMap.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrHashMapWithCache.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"

#include <cstdint>

class GrDirectContext;
class GrResourceProvider;

// Print out explicit allocation information
#define GR_ALLOCATION_SPEW 0

// Print out information about interval creation
#define GR_TRACK_INTERVAL_CREATION 0

/*
 * The ResourceAllocator explicitly distributes GPU resources at flush time. It operates by
 * being given the usage intervals of the various proxies. It keeps these intervals in a singly
 * linked list sorted by increasing start index. (It also maintains a hash table from proxyID
 * to interval to find proxy reuse). The ResourceAllocator uses Registers (in the sense of register
 * allocation) to represent a future surface that will be used for each proxy during
 * `planAssignment`, and then assigns actual surfaces during `assign`.
 *
 * Note: the op indices (used in the usage intervals) come from the order of the ops in
 * their opsTasks after the opsTask DAG has been linearized.
 *
 * The planAssignment method traverses the sorted list and:
 *     moves intervals from the active list that have completed (returning their registers
 *     to the free pool) into the finished list (sorted by increasing start)
 *
 *     allocates a new register (preferably from the free pool) for the new interval
 *     adds the new interval to the active list (that is sorted by increasing end index)
 *
 * After assignment planning, the user can choose to call `makeBudgetHeadroom` which:
 *     computes how much VRAM would be needed for new resources for all extant Registers
 *
 *     asks the resource cache to purge enough resources to get that much free space
 *
 *     if it's not possible, do nothing and return false. The user may opt to reset
 *     the allocator and start over with a different DAG.
 *
 * If the user wants to commit to the current assignment plan, they call `assign` which:
 *     instantiates lazy proxies
 *
 *     instantantiates new surfaces for all registers that need them
 *
 *     assigns the surface for each register to all the proxies that will use it
 *
 *************************************************************************************************
 * How does instantiation failure handling work when explicitly allocating?
 *
 * In the gather usage intervals pass all the GrSurfaceProxies used in the flush should be
 * gathered (i.e., in OpsTask::gatherProxyIntervals).
 *
 * During addInterval, read-only lazy proxies are instantiated. If that fails, the resource
 * allocator will note the failure and ignore pretty much anything else until `reset`.
 *
 * During planAssignment, fully-lazy proxies are instantiated so that we can know their size for
 * budgeting purposes. If this fails, return false.
 *
 * During assign, partially-lazy proxies are instantiated and new surfaces are created for all other
 * proxies. If any of these fails, return false.
 *
 * The drawing manager will drop the flush if any proxies fail to instantiate.
 */
class GrResourceAllocator {
public:
    GrResourceAllocator(GrDirectContext* dContext)
            : fDContext(dContext) {}

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

    /** Indicates whether we allow a gpu texture assigned to a register to be recycled or not. This
     *  comes up when dealing with with Vulkan Secondary CommandBuffers since offscreens sampled
     *  into the scb will all be drawn before being sampled in the scb. This is because the scb
     *  will get submitted in a later command buffer. Thus offscreens cannot share an allocation or
     *  later reuses will overwrite earlier ones.
     */
    enum class AllowRecycling : bool {
        kNo  = false,
        kYes = true
    };

    // Add a usage interval from 'start' to 'end' inclusive. This is usually used for renderTargets.
    // If an existing interval already exists it will be expanded to include the new range.
    void addInterval(GrSurfaceProxy*, unsigned int start, unsigned int end, ActualUse actualUse,
                     AllowRecycling SkDEBUGCODE(, bool isDirectDstRead = false));

    bool failedInstantiation() const { return fFailedInstantiation; }

    // Generate an internal plan for resource allocation. After this you can optionally call
    // `makeBudgetHeadroom` to check whether that plan would go over our memory budget.
    // Fully-lazy proxies are also instantiated at this point so that their size can
    // be known accurately. Returns false if any lazy proxy failed to instantiate, true otherwise.
    bool planAssignment();

    // Figure out how much VRAM headroom this plan requires. If there's enough purgeable resources,
    // purge them and return true. Otherwise return false.
    bool makeBudgetHeadroom();

    // Clear all internal state in preparation for a new set of intervals.
    void reset();

    // Instantiate and assign resources to all proxies.
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
    Register* findOrCreateRegisterFor(GrSurfaceProxy* proxy);

    struct FreePoolTraits {
        static const skgpu::ScratchKey& GetKey(const Register& r) {
            return r.scratchKey();
        }

        static uint32_t Hash(const skgpu::ScratchKey& key) { return key.hash(); }
        static void OnFree(Register* r) { }
    };
    typedef SkTMultiMap<Register, skgpu::ScratchKey, FreePoolTraits> FreePoolMultiMap;

    typedef skia_private::THashMap<uint32_t, Interval*, GrCheapHash> IntvlHash;

    struct UniqueKeyHash {
        uint32_t operator()(const skgpu::UniqueKey& key) const { return key.hash(); }
    };
    typedef skia_private::THashMap<skgpu::UniqueKey, Register*, UniqueKeyHash>
            UniqueKeyRegisterHash;

    // Each proxy – with some exceptions – is assigned a register. After all assignments are made,
    // another pass is performed to instantiate and assign actual surfaces to the proxies. Right
    // now these are performed in one call, but in the future they will be separable and the user
    // will be able to query re: memory cost before committing to surface creation.
    class Register {
    public:
        // It's OK to pass an invalid scratch key iff the proxy has a unique key.
        Register(GrSurfaceProxy* originatingProxy, skgpu::ScratchKey, GrResourceProvider*);

        const skgpu::ScratchKey& scratchKey() const { return fScratchKey; }
        const skgpu::UniqueKey& uniqueKey() const { return fOriginatingProxy->getUniqueKey(); }

        bool accountedForInBudget() const { return fAccountedForInBudget; }
        void setAccountedForInBudget() { fAccountedForInBudget = true; }

        GrSurface* existingSurface() const { return fExistingSurface.get(); }

        // Can this register be used by other proxies after this one?
        bool isRecyclable(const GrCaps&, GrSurfaceProxy* proxy, int knownUseCount,
                          AllowRecycling) const;

        // Resolve the register allocation to an actual GrSurface. 'fOriginatingProxy'
        // is used to cache the allocation when a given register is used by multiple
        // proxies.
        bool instantiateSurface(GrSurfaceProxy*, GrResourceProvider*);

        SkDEBUGCODE(uint32_t uniqueID() const { return fUniqueID; })

    private:
        GrSurfaceProxy*   fOriginatingProxy;
        skgpu::ScratchKey fScratchKey; // free pool wants a reference to this.
        sk_sp<GrSurface>  fExistingSurface; // queried from resource cache. may be null.
        bool              fAccountedForInBudget = false;

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

        void disallowRecycling() {
            fAllowRecycling = AllowRecycling::kNo;
        }
        AllowRecycling allowRecycling() const { return fAllowRecycling; }

        SkDEBUGCODE(uint32_t uniqueID() const { return fUniqueID; })

    private:
        GrSurfaceProxy*  fProxy;
        unsigned int     fStart;
        unsigned int     fEnd;
        Interval*        fNext = nullptr;
        unsigned int     fUses = 0;
        Register*        fRegister = nullptr;
        AllowRecycling   fAllowRecycling = AllowRecycling::kYes;

#ifdef SK_DEBUG
        uint32_t        fUniqueID;

        static uint32_t CreateUniqueID();
#endif
    };

    class IntervalList {
    public:
        IntervalList() = default;
        // N.B. No need for a destructor – the arena allocator will clean up for us.

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

    GrDirectContext*             fDContext;
    FreePoolMultiMap             fFreePool;          // Recently created/used GrSurfaces
    IntvlHash                    fIntvlHash;         // All the intervals, hashed by proxyID

    IntervalList                 fIntvlList;         // All the intervals sorted by increasing start
    IntervalList                 fActiveIntvls;      // List of live intervals during assignment
                                                     // (sorted by increasing end)
    IntervalList                 fFinishedIntvls;    // All the completed intervals
                                                     // (sorted by increasing start)
    UniqueKeyRegisterHash        fUniqueKeyRegisters;
    unsigned int                 fNumOps = 0;

    SkDEBUGCODE(bool             fPlanned = false;)
    SkDEBUGCODE(bool             fAssigned = false;)

    SkSTArenaAllocWithReset<kInitialArenaSize>   fInternalAllocator; // intervals & registers
    bool                                         fFailedInstantiation = false;
};

#endif // GrResourceAllocator_DEFINED
