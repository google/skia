/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOp_DEFINED
#define GrOp_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrGpuResource.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/GrXferProcessor.h"
#include <atomic>
#include <new>

class GrAppliedClip;
class GrCaps;
class GrDstProxyView;
class GrOpFlushState;
class GrOpsRenderPass;
class GrPaint;

/**
 * GrOp is the base class for all Ganesh deferred GPU operations. To facilitate reordering and to
 * minimize draw calls, Ganesh does not generate geometry inline with draw calls. Instead, it
 * captures the arguments to the draw and then generates the geometry when flushing. This gives GrOp
 * subclasses complete freedom to decide how/when to combine in order to produce fewer draw calls
 * and minimize state changes.
 *
 * Ops of the same subclass may be merged or chained using combineIfPossible. When two ops merge,
 * one takes on the union of the data and the other is left empty. The merged op becomes responsible
 * for drawing the data from both the original ops. When ops are chained each op maintains its own
 * data but they are linked in a list and the head op becomes responsible for executing the work for
 * the chain.
 *
 * It is required that chainability is transitive. Moreover, if op A is able to merge with B then
 * it must be the case that any op that can chain with A will either merge or chain with any op
 * that can chain to B.
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

// Print out op information at flush time
#define GR_FLUSH_TIME_OP_SPEW 0

// A helper macro to generate a class static id
#define DEFINE_OP_CLASS_ID \
    static uint32_t ClassID() { \
        static uint32_t kClassID = GenOpClassID(); \
        return kClassID; \
    }

class GrOp : private SkNoncopyable {
public:
    using Owner = std::unique_ptr<GrOp>;

    template<typename Op, typename... Args>
    static Owner Make(GrRecordingContext* context, Args&&... args) {
        return Owner{new Op(std::forward<Args>(args)...)};
    }

    template<typename Op, typename... Args>
    static Owner MakeWithProcessorSet(
            GrRecordingContext* context, const SkPMColor4f& color,
            GrPaint&& paint, Args&&... args);

    template<typename Op, typename... Args>
    static Owner MakeWithExtraMemory(
            GrRecordingContext* context, size_t extraSize, Args&&... args) {
        void* bytes = ::operator new(sizeof(Op) + extraSize);
        return Owner{new (bytes) Op(std::forward<Args>(args)...)};
    }

    virtual ~GrOp() = default;

    virtual const char* name() const = 0;

    virtual void visitProxies(const GrVisitProxyFunc&) const {
        // This default implementation assumes the op has no proxies
    }

    enum class CombineResult {
        /**
         * The op that combineIfPossible was called on now represents its own work plus that of
         * the passed op. The passed op should be destroyed without being flushed. Currently it
         * is not legal to merge an op passed to combineIfPossible() the passed op is already in a
         * chain (though the op on which combineIfPossible() was called may be).
         */
        kMerged,
        /**
         * The caller *may* (but is not required) to chain these ops together. If they are chained
         * then prepare() and execute() will be called on the head op but not the other ops in the
         * chain. The head op will prepare and execute on behalf of all the ops in the chain.
         */
        kMayChain,
        /**
         * The ops cannot be combined.
         */
        kCannotCombine
    };

    // The arenas are the same as what was available when the op was created.
    CombineResult combineIfPossible(GrOp* that, SkArenaAlloc* alloc, const GrCaps& caps);

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

    void operator delete(void* p) { ::operator delete(p); }

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
     * This can optionally be called before 'prepare' (but after sorting). Each op that overrides
     * onPrePrepare must be prepared to handle both cases (when onPrePrepare has been called
     * ahead of time and when it has not been called).
     */
    void prePrepare(GrRecordingContext* context, const GrSurfaceProxyView& dstView,
                    GrAppliedClip* clip, const GrDstProxyView& dstProxyView,
                    GrXferBarrierFlags renderPassXferBarriers, GrLoadOp colorLoadOp) {
        TRACE_EVENT0("skia.gpu", name());
        this->onPrePrepare(context, dstView, clip, dstProxyView, renderPassXferBarriers,
                           colorLoadOp);
    }

    /**
     * Called prior to executing. The op should perform any resource creation or data transfers
     * necessary before execute() is called.
     */
    void prepare(GrOpFlushState* state) {
        TRACE_EVENT0("skia.gpu", name());
        this->onPrepare(state);
    }

    /** Issues the op's commands to GrGpu. */
    void execute(GrOpFlushState* state, const SkRect& chainBounds) {
        TRACE_EVENT0("skia.gpu", name());
        this->onExecute(state, chainBounds);
    }

    /** Used for spewing information about ops when debugging. */
#if GR_TEST_UTILS
    virtual SkString dumpInfo() const final {
        return SkStringPrintf("%s\nOpBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]",
                              this->onDumpInfo().c_str(), fBounds.fLeft, fBounds.fTop,
                              fBounds.fRight, fBounds.fBottom);
    }
#endif

    /**
     * A helper for iterating over an op chain in a range for loop that also downcasts to a GrOp
     * subclass. E.g.:
     *     for (MyOpSubClass& op : ChainRange<MyOpSubClass>(this)) {
     *         // ...
     *     }
     */
    template <typename OpSubclass = GrOp> class ChainRange {
    private:
        class Iter {
        public:
            explicit Iter(const OpSubclass* head) : fCurr(head) {}
            inline Iter& operator++() {
                return *this = Iter(static_cast<const OpSubclass*>(fCurr->nextInChain()));
            }
            const OpSubclass& operator*() const { return *fCurr; }
            bool operator!=(const Iter& that) const { return fCurr != that.fCurr; }

        private:
            const OpSubclass* fCurr;
        };
        const OpSubclass* fHead;

    public:
        explicit ChainRange(const OpSubclass* head) : fHead(head) {}
        Iter begin() { return Iter(fHead); }
        Iter end() { return Iter(nullptr); }
    };

    /**
     * Concatenates two op chains. This op must be a tail and the passed op must be a head. The ops
     * must be of the same subclass.
     */
    void chainConcat(GrOp::Owner);
    /** Returns true if this is the head of a chain (including a length 1 chain). */
    bool isChainHead() const { return !fPrevInChain; }
    /** Returns true if this is the tail of a chain (including a length 1 chain). */
    bool isChainTail() const { return !fNextInChain; }
    /** The next op in the chain. */
    GrOp* nextInChain() const { return fNextInChain.get(); }
    /** The previous op in the chain. */
    GrOp* prevInChain() const { return fPrevInChain; }
    /**
     * Cuts the chain after this op. The returned op is the op that was previously next in the
     * chain or null if this was already a tail.
     */
    GrOp::Owner cutChain();
    SkDEBUGCODE(void validateChain(GrOp* expectedTail = nullptr) const);

#ifdef SK_DEBUG
    virtual void validate() const {}
#endif

protected:
    GrOp(uint32_t classID);

    /**
     * Indicates that the op will produce geometry that extends beyond its bounds for the
     * purpose of ensuring that the fragment shader runs on partially covered pixels for
     * non-MSAA antialiasing.
     */
    enum class HasAABloat : bool {
        kNo = false,
        kYes = true
    };
    /**
     * Indicates that the geometry being drawn in a hairline stroke. A point that is drawn in device
     * space is also considered a hairline.
     */
    enum class IsHairline : bool {
        kNo = false,
        kYes = true
    };

    void setBounds(const SkRect& newBounds, HasAABloat aabloat, IsHairline zeroArea) {
        fBounds = newBounds;
        this->setBoundsFlags(aabloat, zeroArea);
    }
    void setTransformedBounds(const SkRect& srcBounds, const SkMatrix& m,
                              HasAABloat aabloat, IsHairline zeroArea) {
        m.mapRect(&fBounds, srcBounds);
        this->setBoundsFlags(aabloat, zeroArea);
    }
    void makeFullScreen(GrSurfaceProxy* proxy) {
        this->setBounds(proxy->getBoundsRect(), HasAABloat::kNo, IsHairline::kNo);
    }

    static uint32_t GenOpClassID() { return GenID(&gCurrOpClassID); }

private:
    void joinBounds(const GrOp& that) {
        if (that.hasAABloat()) {
            fBoundsFlags |= kAABloat_BoundsFlag;
        }
        if (that.hasZeroArea()) {
            fBoundsFlags |= kZeroArea_BoundsFlag;
        }
        return fBounds.joinPossiblyEmptyRect(that.fBounds);
    }

    virtual CombineResult onCombineIfPossible(GrOp*, SkArenaAlloc*, const GrCaps&) {
        return CombineResult::kCannotCombine;
    }

    // TODO: the parameters to onPrePrepare mirror GrOpFlushState::OpArgs - fuse the two?
    virtual void onPrePrepare(GrRecordingContext*,
                              const GrSurfaceProxyView& writeView,
                              GrAppliedClip*,
                              const GrDstProxyView&,
                              GrXferBarrierFlags renderPassXferBarriers,
                              GrLoadOp colorLoadOp) = 0;
    virtual void onPrepare(GrOpFlushState*) = 0;
    // If this op is chained then chainBounds is the union of the bounds of all ops in the chain.
    // Otherwise, this op's bounds.
    virtual void onExecute(GrOpFlushState*, const SkRect& chainBounds) = 0;
#if GR_TEST_UTILS
    virtual SkString onDumpInfo() const { return SkString(); }
#endif

    static uint32_t GenID(std::atomic<uint32_t>* idCounter) {
        uint32_t id = idCounter->fetch_add(1, std::memory_order_relaxed);
        if (id == 0) {
            SK_ABORT("This should never wrap as it should only be called once for each GrOp "
                     "subclass.");
        }
        return id;
    }

    void setBoundsFlags(HasAABloat aabloat, IsHairline zeroArea) {
        fBoundsFlags = 0;
        fBoundsFlags |= (HasAABloat::kYes == aabloat) ? kAABloat_BoundsFlag : 0;
        fBoundsFlags |= (IsHairline ::kYes == zeroArea) ? kZeroArea_BoundsFlag : 0;
    }

    enum {
        kIllegalOpID = 0,
    };

    enum BoundsFlags {
        kAABloat_BoundsFlag                     = 0x1,
        kZeroArea_BoundsFlag                    = 0x2,
        SkDEBUGCODE(kUninitialized_BoundsFlag   = 0x4)
    };

    Owner                               fNextInChain{nullptr};
    GrOp*                               fPrevInChain = nullptr;
    const uint16_t                      fClassID;
    uint16_t                            fBoundsFlags;

    static uint32_t GenOpID() { return GenID(&gCurrOpUniqueID); }
    mutable uint32_t                    fUniqueID = SK_InvalidUniqueID;
    SkRect                              fBounds;

    static std::atomic<uint32_t> gCurrOpUniqueID;
    static std::atomic<uint32_t> gCurrOpClassID;
};

#endif
