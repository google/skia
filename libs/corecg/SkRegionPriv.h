#ifndef SkRegionPriv_DEFINED
#define SkRegionPriv_DEFINED

#include "SkRegion.h"
#include "SkThread.h"

#define kRunTypeSentinel	0x7FFF
#define assert_sentinel(value, isSentinel)	SkASSERT(((value) == kRunTypeSentinel) == isSentinel)

//SkDEBUGCODE(extern int32_t gRgnAllocCounter;)

struct SkRegion::RunHead {
    int32_t fRefCnt;
    int32_t fRunCount;
    
    static RunHead* Alloc(int count)
    {
        //SkDEBUGCODE(sk_atomic_inc(&gRgnAllocCounter);)
        //SkDEBUGF(("************** gRgnAllocCounter::alloc %d\n", gRgnAllocCounter));

        SkASSERT(count >= SkRegion::kRectRegionRuns);

        RunHead* head = (RunHead*)sk_malloc_throw(sizeof(RunHead) + count * sizeof(RunType));
        head->fRefCnt = 1;
        head->fRunCount = count;
        return head;
    }
    
    bool isComplex() const
    {
        return this != SkRegion_gEmptyRunHeadPtr && this != SkRegion_gRectRunHeadPtr;
    }

    SkRegion::RunType* runs()
    {
        SkASSERT(this->isComplex());
        return (SkRegion::RunType*)(this + 1);
    }
    const SkRegion::RunType* runs() const
    {
        SkASSERT(this->isComplex());
        return (const SkRegion::RunType*)(this + 1);
    }
    
    RunHead* ensureWritable()
    {
        SkASSERT(this->isComplex());
        
        RunHead* writable = this;
        if (fRefCnt > 1)
        {
            // We need to alloc & copy the current region before we call
            // sk_atomic_dec because it could be freed in the meantime,
            // otherwise.            
            writable = Alloc(fRunCount);
            memcpy(writable->runs(), this->runs(), fRunCount * sizeof(RunType));

            // fRefCount might have changed since we last checked.
            // If we own the last reference at this point, we need to
            // free the memory.
            if (sk_atomic_dec(&fRefCnt) == 1)
            {
                sk_free(this);
            }
        }
        return writable;
    }
};

#endif
