/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBatch_DEFINED
#define GrBatch_DEFINED

#include <new>
// TODO remove this header when we move entirely to batch
#include "GrDrawTarget.h"
#include "GrGeometryProcessor.h"
#include "SkRefCnt.h"
#include "SkThread.h"
#include "SkTypes.h"

class GrBatchTarget;
class GrGpu;
class GrIndexBufferAllocPool;
class GrPipeline;
class GrVertexBufferAllocPool;

struct GrInitInvariantOutput;

/*
 * GrBatch is the base class for all Ganesh deferred geometry generators.  To facilitate
 * reorderable batching, Ganesh does not generate geometry inline with draw calls.  Instead, it
 * captures the arguments to the draw and then generates the geometry on demand.  This gives GrBatch
 * subclasses complete freedom to decide how / what they can batch.
 *
 * Batches are created when GrContext processes a draw call. Batches of the same  subclass may be
 * merged using combineIfPossible. When two batches merge, one takes on the union of the data
 * and the other is left empty. The merged batch becomes responsible for drawing the data from both
 * the original batches.
 *
 * If there are any possible optimizations which might require knowing more about the full state of
 * the draw, ie whether or not the GrBatch is allowed to tweak alpha for coverage, then this
 * information will be communicated to the GrBatch prior to geometry generation.
 */

class GrBatch : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrBatch)
    GrBatch() : fClassID(kIllegalBatchClassID), fNumberOfDraws(0) { SkDEBUGCODE(fUsed = false;) }
    virtual ~GrBatch() {}

    virtual const char* name() const = 0;
    virtual void getInvariantOutputColor(GrInitInvariantOutput* out) const = 0;
    virtual void getInvariantOutputCoverage(GrInitInvariantOutput* out) const = 0;

    /*
     * initBatchTracker is a hook for the some additional overrides / optimization possibilities
     * from the GrXferProcessor.
     */
    virtual void initBatchTracker(const GrPipelineInfo& init) = 0;

    bool combineIfPossible(GrBatch* that) {
        if (this->classID() != that->classID()) {
            return false;
        }

        return onCombineIfPossible(that);
    }

    virtual bool onCombineIfPossible(GrBatch*) = 0;

    virtual void generateGeometry(GrBatchTarget*, const GrPipeline*) = 0;

    // TODO this goes away when batches are everywhere
    void setNumberOfDraws(int numberOfDraws) { fNumberOfDraws = numberOfDraws; }
    int numberOfDraws() const { return fNumberOfDraws; }

    void* operator new(size_t size);
    void operator delete(void* target);

    void* operator new(size_t size, void* placement) {
        return ::operator new(size, placement);
    }
    void operator delete(void* target, void* placement) {
        ::operator delete(target, placement);
    }

    /**
      * Helper for down-casting to a GrBatch subclass
      */
    template <typename T> const T& cast() const { return *static_cast<const T*>(this); }
    template <typename T> T* cast() { return static_cast<T*>(this); }

    uint32_t classID() const { SkASSERT(kIllegalBatchClassID != fClassID); return fClassID; }

    // TODO no GrPrimitiveProcessors yet read fragment position
    bool willReadFragmentPosition() const { return false; }

    SkDEBUGCODE(bool isUsed() const { return fUsed; })

protected:
    template <typename PROC_SUBCLASS> void initClassID() {
         static uint32_t kClassID = GenClassID();
         fClassID = kClassID;
    }

    uint32_t fClassID;

private:
    static uint32_t GenClassID() {
        // fCurrProcessorClassID has been initialized to kIllegalProcessorClassID. The
        // atomic inc returns the old value not the incremented value. So we add
        // 1 to the returned value.
        uint32_t id = static_cast<uint32_t>(sk_atomic_inc(&gCurrBatchClassID)) + 1;
        if (!id) {
            SkFAIL("This should never wrap as it should only be called once for each GrBatch "
                   "subclass.");
        }
        return id;
    }

    enum {
        kIllegalBatchClassID = 0,
    };
    static int32_t gCurrBatchClassID;

    SkDEBUGCODE(bool fUsed;)

    int fNumberOfDraws;

    typedef SkRefCnt INHERITED;
};

#endif
