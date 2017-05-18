/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOp_DEFINED
#define GrOp_DEFINED

#include "../private/SkAtomics.h"
#include "GrGpuResource.h"
#include "GrNonAtomicRef.h"
#include "GrXferProcessor.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "SkString.h"

#include <new>

class GrCaps;
class GrGpuCommandBuffer;
class GrOpFlushState;
class GrRenderTargetOpList;

/**
 * GrOp is the base class for all Ganesh deferred GPU operations. To facilitate reordering and to
 * minimize draw calls, Ganesh does not generate geometry inline with draw calls. Instead, it
 * captures the arguments to the draw and then generates the geometry when flushing. This gives GrOp
 * subclasses complete freedom to decide how/when to combine in order to produce fewer draw calls
 * and minimize state changes.
 *
 * Ops of the same subclass may be merged using combineIfPossible. When two ops merge, one
 * takes on the union of the data and the other is left empty. The merged op becomes responsible
 * for drawing the data from both the original ops.
 *
 * If there are any possible optimizations which might require knowing more about the full state of
 * the draw, e.g. whether or not the GrOp is allowed to tweak alpha for coverage, then this
 * information will be communicated to the GrOp prior to geometry generation.
 *
 * The bounds of the op must contain all the vertices in device space *irrespective* of the clip.
 * The bounds are used in determining which clip elements must be applied and thus the bounds cannot
 * in turn depend upon the clip.
 */
#define GR_OP_SPEW 0
#if GR_OP_SPEW
    #define GrOP_SPEW(code) code
    #define GrOP_INFO(...) SkDebugf(__VA_ARGS__)
#else
    #define GrOP_SPEW(code)
    #define GrOP_INFO(...)
#endif

// A helper macro to generate a class static id
#define DEFINE_OP_CLASS_ID \
    static uint32_t ClassID() { \
        static uint32_t kClassID = GenOpClassID(); \
        return kClassID; \
    }

class GrOp : private SkNoncopyable {
public:
    GrOp(uint32_t classID);
    virtual ~GrOp();

    virtual const char* name() const = 0;

    bool combineIfPossible(GrOp* that, const GrCaps& caps) {
        if (this->classID() != that->classID()) {
            return false;
        }

        return this->onCombineIfPossible(that, caps);
    }

    const SkRect& bounds() const {
        SkASSERT(kUninitialized_BoundsFlag != fBoundsFlags);
        return fBounds;
    }

    void setClippedBounds(const SkRect& clippedBounds) {
        fBounds = clippedBounds;
        // The clipped bounds already incorporate any effect of the bounds flags.
        fBoundsFlags = 0;
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
     * Helper for safely down-casting to a GrOp subclass
     */
    template <typename T> const T& cast() const {
        SkASSERT(T::ClassID() == this->classID());
        return *static_cast<const T*>(this);
    }

    template <typename T> T* cast() {
        SkASSERT(T::ClassID() == this->classID());
        return static_cast<T*>(this);
    }

    uint32_t classID() const { SkASSERT(kIllegalOpID != fClassID); return fClassID; }

    // We lazily initialize the uniqueID because currently the only user is GrAuditTrail
    uint32_t uniqueID() const {
        if (kIllegalOpID == fUniqueID) {
            fUniqueID = GenOpID();
        }
        return fUniqueID;
    }

    /**
     * This is called to notify the op that it has been recorded into a GrOpList. Ops can use this
     * to begin preparations for the flush of the op list. Note that the op still may either be
     * combined into another op or have another op combined into it via combineIfPossible() after
     * this call is made.
     */
    virtual void wasRecorded(GrRenderTargetOpList*) {}

    /**
     * Called prior to executing. The op should perform any resource creation or data transfers
     * necessary before execute() is called.
     */
    void prepare(GrOpFlushState* state) { this->onPrepare(state); }

    /** Issues the op's commands to GrGpu. */
    void execute(GrOpFlushState* state) { this->onExecute(state); }

    /** Used for spewing information about ops when debugging. */
    virtual SkString dumpInfo() const {
        SkString string;
        string.appendf("OpBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                       fBounds.fLeft, fBounds.fTop, fBounds.fRight, fBounds.fBottom);
        return string;
    }

    virtual bool needsCommandBufferIsolation() const { return false; }

protected:
    /**
     * Indicates that the op will produce geometry that extends beyond its bounds for the
     * purpose of ensuring that the fragment shader runs on partially covered pixels for
     * non-MSAA antialiasing.
     */
    enum class HasAABloat {
        kYes,
        kNo
    };
    /**
     * Indicates that the geometry represented by the op has zero area (e.g. it is hairline or
     * points).
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

    void joinBounds(const GrOp& that) {
        if (that.hasAABloat()) {
            fBoundsFlags |= kAABloat_BoundsFlag;
        }
        if (that.hasZeroArea()) {
            fBoundsFlags |= kZeroArea_BoundsFlag;
        }
        return fBounds.joinPossiblyEmptyRect(that.fBounds);
    }

    void replaceBounds(const GrOp& that) {
        fBounds = that.fBounds;
        fBoundsFlags = that.fBoundsFlags;
    }

    static uint32_t GenOpClassID() { return GenID(&gCurrOpClassID); }

private:
    virtual bool onCombineIfPossible(GrOp*, const GrCaps& caps) = 0;

    virtual void onPrepare(GrOpFlushState*) = 0;
    virtual void onExecute(GrOpFlushState*) = 0;

    static uint32_t GenID(int32_t* idCounter) {
        // The atomic inc returns the old value not the incremented value. So we add
        // 1 to the returned value.
        uint32_t id = static_cast<uint32_t>(sk_atomic_inc(idCounter)) + 1;
        if (!id) {
            SkFAIL("This should never wrap as it should only be called once for each GrOp "
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
        kIllegalOpID = 0,
    };

    enum BoundsFlags {
        kAABloat_BoundsFlag                     = 0x1,
        kZeroArea_BoundsFlag                    = 0x2,
        SkDEBUGCODE(kUninitialized_BoundsFlag   = 0x4)
    };

    const uint16_t                      fClassID;
    uint16_t                            fBoundsFlags;

    static uint32_t GenOpID() { return GenID(&gCurrOpUniqueID); }
    mutable uint32_t                    fUniqueID;
    SkRect                              fBounds;

    static int32_t                      gCurrOpUniqueID;
    static int32_t                      gCurrOpClassID;
};

#endif
