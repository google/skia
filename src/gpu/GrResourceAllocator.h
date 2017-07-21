/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceAllocator_DEFINED
#define GrResourceAllocator_DEFINED

#include "GrSurfaceProxy.h"
#include "SkTDynamicHash.h"

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

    unsigned int curOp() const { return fNumOps; }
    void incOps() { fNumOps++; }
    unsigned int numOps() const { return fNumOps; }

    // Add a usage interval from start to end inclusive. This is usually used for renderTargets.
    // If an existing interval already exists it will be expanded to include the new range.
    void addInterval(GrSurfaceProxy*, unsigned int start, unsigned int end);

    // Add an interval that spans just the current op. Usually this is for texture uses.
    // If an existing interval already exists it will be expanded to include the new operation.
    void addInterval(GrSurfaceProxy* proxy) {
        this->addInterval(proxy, fNumOps, fNumOps);
    }

    void assign();

private:
    class Interval;

    // Remove dead intervals from the active list
    void expire(unsigned int curIndex);

    // These two methods wrap the interactions with the free pool
    void freeUpSurface(GrSurface* surface);
    sk_sp<GrSurface> findSurfaceFor(GrSurfaceProxy* proxy);

    struct UniqueHashTraits {
        static const GrUniqueKey& GetKey(const GrSurface& s) { return s.getUniqueKey(); }

        static uint32_t Hash(const GrUniqueKey& key) { return key.hash(); }
    };
    typedef SkTDynamicHash<GrSurface, GrUniqueKey, UniqueHashTraits> UniqueHash;
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

        // for SkTDynamicHash
        static const uint32_t& GetKey(const Interval& intvl) {
            return intvl.fProxyID;
        }
        static uint32_t Hash(const uint32_t& key) { return key; }

        GrSurfaceProxy* fProxy;
        uint32_t        fProxyID; // This is here b.c. DynamicHash requires a ref to the key 
        unsigned int    fStart;
        unsigned int    fEnd;
        Interval*       fNext;
    };

    class IntervalList {
    public:
        IntervalList() = default;
        ~IntervalList() {
            while (fHead) {
                Interval* temp = fHead;
                fHead = temp->fNext;
                delete temp;
            }
        }

        bool empty() const { return !SkToBool(fHead); }
        const Interval* peekHead() const { return fHead; }
        Interval* popHead();
        void insertByIncreasingStart(Interval*);
        void insertByIncreasingEnd(Interval*);

    private:
        Interval* fHead = nullptr;
    };

    GrResourceProvider* fResourceProvider;
    UniqueHash          fFreePool;          // Recently created/used GrSurfaces
    IntvlHash           fIntvlHash;         // All the intervals, hashed by proxyID

    IntervalList        fIntvlList;         // All the intervals sorted by increasing start
    IntervalList        fActiveIntvls;      // List of live intervals during assignment
                                            // (sorted by increasing end)
    unsigned int        fNumOps = 0;
    SkDEBUGCODE(bool    fAssigned = false;)
};

#endif // GrResourceAllocator_DEFINED
