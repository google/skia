/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBatch_DEFINED
#define GrBatch_DEFINED

#include <new>
#include "GrBatchTarget.h"
#include "GrGeometryProcessor.h"
#include "GrNonAtomicRef.h"
#include "GrVertices.h"
#include "SkAtomics.h"
#include "SkTypes.h"

class GrGpu;
class GrPipeline;

struct GrInitInvariantOutput;

/**
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
#define GR_BATCH_SPEW 0
#if GR_BATCH_SPEW
    #define GrBATCH_INFO(...) SkDebugf(__VA_ARGS__)
    #define GrBATCH_SPEW(code) code
#else
    #define GrBATCH_SPEW(code)
    #define GrBATCH_INFO(...)
#endif

class GrBatch : public GrNonAtomicRef {
public:
    GrBatch();
    ~GrBatch() override;

    virtual const char* name() const = 0;

    bool combineIfPossible(GrBatch* that, const GrCaps& caps) {
        if (this->classID() != that->classID()) {
            return false;
        }

        return this->onCombineIfPossible(that, caps);
    }

    const SkRect& bounds() const { return fBounds; }

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

    uint32_t classID() const { SkASSERT(kIllegalBatchID != fClassID); return fClassID; }

#if GR_BATCH_SPEW
    uint32_t uniqueID() const { return fUniqueID; }
#endif
    SkDEBUGCODE(bool isUsed() const { return fUsed; })

protected:
    template <typename PROC_SUBCLASS> void initClassID() {
         static uint32_t kClassID = GenID(&gCurrBatchClassID);
         fClassID = kClassID;
    }

    // NOTE, compute some bounds, even if extremely conservative.  Do *NOT* setLargest on the bounds
    // rect because we outset it for dst copy textures
    void setBounds(const SkRect& newBounds) { fBounds = newBounds; }

    void joinBounds(const SkRect& otherBounds) {
        return fBounds.joinPossiblyEmptyRect(otherBounds);
    }

    SkRect                              fBounds;

private:
    virtual bool onCombineIfPossible(GrBatch*, const GrCaps& caps) = 0;

    static uint32_t GenID(int32_t* idCounter) {
        // fCurrProcessorClassID has been initialized to kIllegalProcessorClassID. The
        // atomic inc returns the old value not the incremented value. So we add
        // 1 to the returned value.
        uint32_t id = static_cast<uint32_t>(sk_atomic_inc(idCounter)) + 1;
        if (!id) {
            SkFAIL("This should never wrap as it should only be called once for each GrBatch "
                   "subclass.");
        }
        return id;
    }

    enum {
        kIllegalBatchID = 0,
    };

    uint32_t                            fClassID;
    SkDEBUGCODE(bool                    fUsed;)
#if GR_BATCH_SPEW
    uint32_t                            fUniqueID;
    static int32_t gCurrBatchUniqueID;
#endif
    static int32_t gCurrBatchClassID;
    typedef GrNonAtomicRef INHERITED;
};

/**
 * Base class for GrBatches that draw. These batches have a GrPipeline installed by GrDrawTarget.
 */
class GrDrawBatch : public GrBatch {
public:
    GrDrawBatch();
    ~GrDrawBatch() override;

    virtual void getInvariantOutputColor(GrInitInvariantOutput* out) const = 0;
    virtual void getInvariantOutputCoverage(GrInitInvariantOutput* out) const = 0;

    const GrPipeline* pipeline() const {
        SkASSERT(fPipelineInstalled);
        return reinterpret_cast<const GrPipeline*>(fPipelineStorage.get());
    }

    bool installPipeline(const GrPipeline::CreateArgs&);

    // TODO no GrPrimitiveProcessors yet read fragment position
    bool willReadFragmentPosition() const { return false; }

private:
    /**
     * initBatchTracker is a hook for the some additional overrides / optimization possibilities
     * from the GrXferProcessor.
     */
    virtual void initBatchTracker(const GrPipelineOptimizations&) = 0;

    SkAlignedSTStorage<1, GrPipeline>   fPipelineStorage;
    bool                                fPipelineInstalled;
    typedef GrBatch INHERITED;
};

/**
 * Base class for vertex-based GrBatches.
 */
class GrVertexBatch : public GrDrawBatch {
public:
    GrVertexBatch();

    virtual void generateGeometry(GrBatchTarget*) = 0;

    // TODO this goes away when batches are everywhere
    void setNumberOfDraws(int numberOfDraws) { fNumberOfDraws = numberOfDraws; }
    int numberOfDraws() const { return fNumberOfDraws; }

protected:
    /** Helper for rendering instances using an instanced index index buffer. This class creates the
        space for the vertices and flushes the draws to the batch target. */
   class InstancedHelper {
   public:
        InstancedHelper() {}
        /** Returns the allocated storage for the vertices. The caller should populate the before
            vertices before calling issueDraws(). */
        void* init(GrBatchTarget* batchTarget, GrPrimitiveType, size_t vertexStride,
                   const GrIndexBuffer*, int verticesPerInstance, int indicesPerInstance,
                   int instancesToDraw);

        /** Call after init() to issue draws to the batch target.*/
        void issueDraw(GrBatchTarget* batchTarget) {
            SkASSERT(fVertices.instanceCount());
            batchTarget->draw(fVertices);
        }
    private:
        GrVertices  fVertices;
    };

    static const int kVerticesPerQuad = 4;
    static const int kIndicesPerQuad = 6;

    /** A specialization of InstanceHelper for quad rendering. */
    class QuadHelper : private InstancedHelper {
    public:
        QuadHelper() : INHERITED() {}
        /** Finds the cached quad index buffer and reserves vertex space. Returns NULL on failure
            and on sucess a pointer to the vertex data that the caller should populate before
            calling issueDraws(). */
        void* init(GrBatchTarget* batchTarget, size_t vertexStride, int quadsToDraw);

        using InstancedHelper::issueDraw;

    private:
        typedef InstancedHelper INHERITED;
    };

private:
    int                                 fNumberOfDraws;
    typedef GrDrawBatch INHERITED;
};

#endif
