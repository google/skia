/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBatch_DEFINED
#define GrBatch_DEFINED

#include "../private/SkAtomics.h"
#include "GrGpuResource.h"
#include "GrNonAtomicRef.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "SkString.h"

#include <new>

class GrCaps;
class GrGpuCommandBuffer;
class GrBatchFlushState;
class GrRenderTarget;

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
 *
 * The bounds of the batch must contain all the vertices in device space *irrespective* of the clip.
 * The bounds are used in determining which clip elements must be applied and thus the bounds cannot
 * in turn depend upon the clip.
 */
#define GR_BATCH_SPEW 0
#if GR_BATCH_SPEW
    #define GrBATCH_INFO(...) SkDebugf(__VA_ARGS__)
    #define GrBATCH_SPEW(code) code
#else
    #define GrBATCH_SPEW(code)
    #define GrBATCH_INFO(...)
#endif

// A helper macro to generate a class static id
#define DEFINE_BATCH_CLASS_ID \
    static uint32_t ClassID() { \
        static uint32_t kClassID = GenBatchClassID(); \
        return kClassID; \
    }

class GrBatch : public GrNonAtomicRef<GrBatch> {
public:
    GrBatch(uint32_t classID);
    virtual ~GrBatch();

    virtual const char* name() const = 0;

    bool combineIfPossible(GrBatch* that, const GrCaps& caps) {
        if (this->classID() != that->classID()) {
            return false;
        }

        return this->onCombineIfPossible(that, caps);
    }

    const SkRect& bounds() const {
        SkASSERT(kUninitialized_BoundsFlag != fBoundsFlags);
        return fBounds;
    }

    bool hasAABloat() const {
        SkASSERT(fBoundsFlags != kUninitialized_BoundsFlag);
        return SkToBool(fBoundsFlags & kAABloat_BoundsFlag);
    }

    bool hasZeroArea() const {
        SkASSERT(fBoundsFlags != kUninitialized_BoundsFlag);
        return SkToBool(fBoundsFlags & kZeroArea_BoundsFlag);
    }

    void* operator new(size_t size);
    void operator delete(void* target);

    void* operator new(size_t size, void* placement) {
        return ::operator new(size, placement);
    }
    void operator delete(void* target, void* placement) {
        ::operator delete(target, placement);
    }

    /**
     * Helper for safely down-casting to a GrBatch subclass
     */
    template <typename T> const T& cast() const {
        SkASSERT(T::ClassID() == this->classID());
        return *static_cast<const T*>(this);
    }

    template <typename T> T* cast() {
        SkASSERT(T::ClassID() == this->classID());
        return static_cast<T*>(this);
    }

    uint32_t classID() const { SkASSERT(kIllegalBatchID != fClassID); return fClassID; }

    // We lazily initialize the uniqueID because currently the only user is GrAuditTrail
    uint32_t uniqueID() const {
        if (kIllegalBatchID == fUniqueID) {
            fUniqueID = GenBatchID();
        }
        return fUniqueID;
    }
    SkDEBUGCODE(bool isUsed() const { return fUsed; })

    /** Called prior to drawing. The batch should perform any resource creation necessary to
        to quickly issue its draw when draw is called. */
    void prepare(GrBatchFlushState* state) { this->onPrepare(state); }

    /** Issues the batches commands to GrGpu. */
    void draw(GrBatchFlushState* state, const SkRect& bounds) { this->onDraw(state, bounds); }

    /** Used to block batching across render target changes. Remove this once we store
        GrBatches for different RTs in different targets. */
    // TODO: this needs to be updated to return GrSurfaceProxy::UniqueID
    virtual GrGpuResource::UniqueID renderTargetUniqueID() const = 0;

    /** Used for spewing information about batches when debugging. */
    virtual SkString dumpInfo() const {
        SkString string;
        string.appendf("BatchBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                       fBounds.fLeft, fBounds.fTop, fBounds.fRight, fBounds.fBottom);
        return string;
    }

    /** Can remove this when multi-draw-buffer lands */
    virtual GrRenderTarget* renderTarget() const = 0;

protected:
    /**
     * Indicates that the batch will produce geometry that extends beyond its bounds for the
     * purpose of ensuring that the fragment shader runs on partially covered pixels for
     * non-MSAA antialiasing.
     */
    enum class HasAABloat {
        kYes,
        kNo
    };
    /**
     * Indicates that the geometry represented by the batch has zero area (i.e. it is hairline
     * or points).
     */
    enum class IsZeroArea {
        kYes,
        kNo
    };
    void setBounds(const SkRect& newBounds, HasAABloat aabloat, IsZeroArea zeroArea) {
        fBounds = newBounds;
        this->setBoundsFlags(aabloat, zeroArea);
    }
    void setTransformedBounds(const SkRect& srcBounds, const SkMatrix& m,
                              HasAABloat aabloat, IsZeroArea zeroArea) {
        m.mapRect(&fBounds, srcBounds);
        this->setBoundsFlags(aabloat, zeroArea);
    }

    void joinBounds(const GrBatch& that) {
        if (that.hasAABloat()) {
            fBoundsFlags |= kAABloat_BoundsFlag;
        }
        if (that.hasZeroArea()) {
            fBoundsFlags |= kZeroArea_BoundsFlag;
        }
        return fBounds.joinPossiblyEmptyRect(that.fBounds);
    }

    void replaceBounds(const GrBatch& that) {
        fBounds = that.fBounds;
        fBoundsFlags = that.fBoundsFlags;
    }

    static uint32_t GenBatchClassID() { return GenID(&gCurrBatchClassID); }

private:
    virtual bool onCombineIfPossible(GrBatch*, const GrCaps& caps) = 0;

    virtual void onPrepare(GrBatchFlushState*) = 0;
    virtual void onDraw(GrBatchFlushState*, const SkRect& bounds) = 0;

    static uint32_t GenID(int32_t* idCounter) {
        // The atomic inc returns the old value not the incremented value. So we add
        // 1 to the returned value.
        uint32_t id = static_cast<uint32_t>(sk_atomic_inc(idCounter)) + 1;
        if (!id) {
            SkFAIL("This should never wrap as it should only be called once for each GrBatch "
                   "subclass.");
        }
        return id;
    }

    void setBoundsFlags(HasAABloat aabloat, IsZeroArea zeroArea) {
        fBoundsFlags = 0;
        fBoundsFlags |= (HasAABloat::kYes == aabloat) ? kAABloat_BoundsFlag : 0;
        fBoundsFlags |= (IsZeroArea ::kYes == zeroArea) ? kZeroArea_BoundsFlag : 0;
    }

    enum {
        kIllegalBatchID = 0,
    };

    enum BoundsFlags {
        kAABloat_BoundsFlag                     = 0x1,
        kZeroArea_BoundsFlag                    = 0x2,
        SkDEBUGCODE(kUninitialized_BoundsFlag   = 0x4)
    };

    SkDEBUGCODE(bool                    fUsed;)
    const uint16_t                      fClassID;
    uint16_t                            fBoundsFlags;

    static uint32_t GenBatchID() { return GenID(&gCurrBatchUniqueID); }
    mutable uint32_t                    fUniqueID;
    SkRect                              fBounds;

    static int32_t                      gCurrBatchUniqueID;
    static int32_t                      gCurrBatchClassID;
};

#endif
