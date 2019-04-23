/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceAllocator_DEFINED
#define GrResourceAllocator_DEFINED

#include "include/gpu/GrSurface.h"
#include "include/private/GrSurfaceProxy.h"
#include "src/gpu/GrGpuResourcePriv.h"

#include "include/private/SkArenaAlloc.h"
#include "src/core/SkTDynamicHash.h"
#include "src/core/SkTMultiMap.h"

class GrDeinstantiateProxyTracker;
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
 * their opLists after the opList DAG has been linearized.
 */
class GrResourceAllocator {
public:
    GrResourceAllocator(GrResourceProvider* resourceProvider,
                        GrDeinstantiateProxyTracker* tracker
                        SkDEBUGCODE(, int numOpLists))
            : fResourceProvider(resourceProvider)
            , fDeinstantiateTracker(tracker)
            SkDEBUGCODE(, fNumOpLists(numOpLists)) {
    }

    ~GrResourceAllocator();

    unsigned int curOp() const { return fNumOps; }
    void incOps() { fNumOps++; }

    /** Indicates whether a given call to addInterval represents an actual usage of the
     *  provided proxy. This is mainly here to accomodate deferred proxies attached to opLists.
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

    enum class AssignError {
        kNoError,
        kFailedProxyInstantiation
    };

    // Returns true when the opLists from 'startIndex' to 'stopIndex' should be executed;
    // false when nothing remains to be executed.
    // If any proxy fails to instantiate, the AssignError will be set to kFailedProxyInstantiation.
    // If this happens, the caller should remove all ops which reference an uninstantiated proxy.
    // This is used to execute a portion of the queued opLists in order to reduce the total
    // amount of GPU resources required.
    bool assign(int* startIndex, int* stopIndex, AssignError* outError);

    void determineRecyclability();
    void markEndOfOpList(int opListIndex);

#if GR_ALLOCATION_SPEW
    void dumpIntervals();
#endif

private:
    class Interval;

    // Remove dead intervals from the active list
    void expire(unsigned int curIndex);

    bool onOpListBoundary() const;
    void forceIntermediateFlush(int* stopIndex);

    // These two methods wrap the interactions with the free pool
    void recycleSurface(sk_sp<GrSurface> surface);
    sk_sp<GrSurface> findSurfaceFor(const GrSurfaceProxy* proxy, bool needsStencil);

    struct FreePoolTraits {
        static const GrScratchKey& GetKey(const GrSurface& s) {
            return s.resourcePriv().getScratchKey();
        }

        static uint32_t Hash(const GrScratchKey& key) { return key.hash(); }
        static void OnFree(GrSurface* s) { s->unref(); }
    };
    typedef SkTMultiMap<GrSurface, GrScratchKey, FreePoolTraits> FreePoolMultiMap;

    typedef SkTDynamicHash<Interval, unsigned int> IntvlHash;

    class Interval {
    public:
        Interval(GrSurfaceProxy* proxy, unsigned int start, unsigned int end)
            : fProxy(proxy)
            , fProxyID(proxy->uniqueID().asUInt())
            , fStart(start)
            , fEnd(end)
            , fNext(nullptr) {
            SkASSERT(proxy);
#if GR_TRACK_INTERVAL_CREATION
            fUniqueID = CreateUniqueID();
            SkDebugf("New intvl %d: proxyID: %d [ %d, %d ]\n",
                     fUniqueID, proxy->uniqueID().asUInt(), start, end);
#endif
        }

        // Used when recycling an interval
        void resetTo(GrSurfaceProxy* proxy, unsigned int start, unsigned int end) {
            SkASSERT(proxy);
            SkASSERT(!fProxy && !fNext);

            fUses = 0;
            fProxy = proxy;
            fProxyID = proxy->uniqueID().asUInt();
            fStart = start;
            fEnd = end;
            fNext = nullptr;
#if GR_TRACK_INTERVAL_CREATION
            fUniqueID = CreateUniqueID();
            SkDebugf("New intvl %d: proxyID: %d [ %d, %d ]\n",
                     fUniqueID, proxy->uniqueID().asUInt(), start, end);
#endif
        }

        ~Interval() {
            SkASSERT(!fAssignedSurface);
        }

        const GrSurfaceProxy* proxy() const { return fProxy; }
        GrSurfaceProxy* proxy() { return fProxy; }

        unsigned int start() const { return fStart; }
        unsigned int end() const { return fEnd; }

        void setNext(Interval* next) { fNext = next; }
        const Interval* next() const { return fNext; }
        Interval* next() { return fNext; }

        void markAsRecyclable() { fIsRecyclable = true;}
        bool isRecyclable() const { return fIsRecyclable; }

        void addUse() { fUses++; }
        int uses() { return fUses; }

        void extendEnd(unsigned int newEnd) {
            if (newEnd > fEnd) {
                fEnd = newEnd;
#if GR_TRACK_INTERVAL_CREATION
                SkDebugf("intvl %d: extending from %d to %d\n", fUniqueID, fEnd, newEnd);
#endif
            }
        }

        void assign(sk_sp<GrSurface>);
        bool wasAssignedSurface() const { return fAssignedSurface != nullptr; }
        sk_sp<GrSurface> detachSurface() { return std::move(fAssignedSurface); }

        // for SkTDynamicHash
        static const uint32_t& GetKey(const Interval& intvl) {
            return intvl.fProxyID;
        }
        static uint32_t Hash(const uint32_t& key) { return key; }

    private:
        sk_sp<GrSurface> fAssignedSurface;
        GrSurfaceProxy*  fProxy;
        uint32_t         fProxyID; // This is here b.c. DynamicHash requires a ref to the key
        unsigned int     fStart;
        unsigned int     fEnd;
        Interval*        fNext;
        unsigned int     fUses = 0;
        bool             fIsRecyclable = false;

#if GR_TRACK_INTERVAL_CREATION
        uint32_t        fUniqueID;

        uint32_t CreateUniqueID();
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
        Interval* detachAll();

    private:
        SkDEBUGCODE(void validate() const;)

        Interval* fHead = nullptr;
        Interval* fTail = nullptr;
    };

    // Compositing use cases can create > 80 intervals.
    static const int kInitialArenaSize = 128 * sizeof(Interval);

    GrResourceProvider*          fResourceProvider;
    GrDeinstantiateProxyTracker* fDeinstantiateTracker;
    FreePoolMultiMap             fFreePool;          // Recently created/used GrSurfaces
    IntvlHash                    fIntvlHash;         // All the intervals, hashed by proxyID

    IntervalList                 fIntvlList;         // All the intervals sorted by increasing start
    IntervalList                 fActiveIntvls;      // List of live intervals during assignment
                                                     // (sorted by increasing end)
    unsigned int                 fNumOps = 0;
    SkTArray<unsigned int>       fEndOfOpListOpIndices;
    int                          fCurOpListIndex = 0;
    SkDEBUGCODE(const int        fNumOpLists = -1;)

    SkDEBUGCODE(bool             fAssigned = false;)

    char                         fStorage[kInitialArenaSize];
    SkArenaAlloc                 fIntervalAllocator{fStorage, kInitialArenaSize, kInitialArenaSize};
    Interval*                    fFreeIntervalList = nullptr;
};

#endif // GrResourceAllocator_DEFINED
