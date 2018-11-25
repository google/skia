/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceAllocator_DEFINED
#define GrResourceAllocator_DEFINED

#include "GrGpuResourcePriv.h"
#include "GrSurface.h"
#include "GrSurfaceProxy.h"

#include "SkArenaAlloc.h"
#include "SkTDynamicHash.h"
#include "SkTMultiMap.h"

class GrResourceProvider;

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
    GrResourceAllocator(GrResourceProvider* resourceProvider)
            : fResourceProvider(resourceProvider) {
    }

    ~GrResourceAllocator();

    unsigned int curOp() const { return fNumOps; }
    void incOps() { fNumOps++; }
    unsigned int numOps() const { return fNumOps; }

    // Add a usage interval from 'start' to 'end' inclusive. This is usually used for renderTargets.
    // If an existing interval already exists it will be expanded to include the new range.
    void addInterval(GrSurfaceProxy*, unsigned int start, unsigned int end
                     SkDEBUGCODE(, bool isDirectDstRead = false));

    // Add an interval that spans just the current op. Usually this is for texture uses.
    // If an existing interval already exists it will be expanded to include the new operation.
    void addInterval(GrSurfaceProxy* proxy
                     SkDEBUGCODE(, bool isDirectDstRead = false)) {
        this->addInterval(proxy, fNumOps, fNumOps SkDEBUGCODE(, isDirectDstRead));
    }

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

    void markEndOfOpList(int opListIndex);

private:
    class Interval;

    // Remove dead intervals from the active list
    void expire(unsigned int curIndex);

    // These two methods wrap the interactions with the free pool
    void freeUpSurface(sk_sp<GrSurface> surface);
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
        }

        void resetTo(GrSurfaceProxy* proxy, unsigned int start, unsigned int end) {
            SkASSERT(proxy);

            fProxy = proxy;
            fProxyID = proxy->uniqueID().asUInt();
            fStart = start;
            fEnd = end;
            fNext = nullptr;
        }

        ~Interval() {
            SkASSERT(!fAssignedSurface);
        }

        const GrSurfaceProxy* proxy() const { return fProxy; }
        GrSurfaceProxy* proxy() { return fProxy; }
        unsigned int start() const { return fStart; }
        unsigned int end() const { return fEnd; }
        const Interval* next() const { return fNext; }
        Interval* next() { return fNext; }

        void setNext(Interval* next) { fNext = next; }

        void extendEnd(unsigned int newEnd) {
            if (newEnd > fEnd) {
                fEnd = newEnd;
            }
        }

        void assign(sk_sp<GrSurface>);
        bool wasAssignedSurface() const { return fAssignedSurface; }
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
    };

    class IntervalList {
    public:
        IntervalList() = default;
        ~IntervalList() {
            // The only time we delete an IntervalList is in the GrResourceAllocator dtor.
            // Since the arena allocator will clean up for us we don't bother here.
        }

        bool empty() const { return !SkToBool(fHead); }
        const Interval* peekHead() const { return fHead; }
        Interval* popHead();
        void insertByIncreasingStart(Interval*);
        void insertByIncreasingEnd(Interval*);

    private:
        Interval* fHead = nullptr;
    };

    // Gathered statistics indicate that 99% of flushes will be covered by <= 12 Intervals
    static const int kInitialArenaSize = 12 * sizeof(Interval);

    GrResourceProvider*    fResourceProvider;
    FreePoolMultiMap       fFreePool;          // Recently created/used GrSurfaces
    IntvlHash              fIntvlHash;         // All the intervals, hashed by proxyID

    IntervalList           fIntvlList;         // All the intervals sorted by increasing start
    IntervalList           fActiveIntvls;      // List of live intervals during assignment
                                               // (sorted by increasing end)
    unsigned int           fNumOps = 1;        // op # 0 is reserved for uploads at the start
                                               // of a flush
    SkTArray<unsigned int> fEndOfOpListOpIndices;
    int                    fCurOpListIndex = 0;

    SkDEBUGCODE(bool       fAssigned = false;)

    char                   fStorage[kInitialArenaSize];
    SkArenaAlloc           fIntervalAllocator { fStorage, kInitialArenaSize, 0 };
    Interval*              fFreeIntervalList = nullptr;
};

#endif // GrResourceAllocator_DEFINED
