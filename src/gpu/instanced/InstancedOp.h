/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gr_instanced_InstancedOp_DEFINED
#define gr_instanced_InstancedOp_DEFINED


#include "../private/GrInstancedPipelineInfo.h"
#include "GrMemoryPool.h"
#include "ops/GrDrawOp.h"
#include "instanced/InstancedRenderingTypes.h"

#include "SkTInternalLList.h"

namespace gr_instanced {

class InstancedRendering;
class OpAllocator;

class InstancedOp : public GrDrawOp {
public:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(InstancedOp);

    ~InstancedOp() override;
    const char* name() const override { return "InstancedOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.printf(
                "AA: %d, ShapeTypes: 0x%02x, IShapeTypes: 0x%02x, Persp %d, "
                "NonSquare: %d, PLoad: %0.2f, Tracked: %d, NumDraws: %d, "
                "GeomChanges: %d\n",
                (unsigned)fInfo.fAAType,
                fInfo.fShapeTypes,
                fInfo.fInnerShapeTypes,
                fInfo.fHasPerspective,
                fInfo.fNonSquare,
                fPixelLoad,
                fIsTracked,
                fNumDraws,
                fNumChangesInGeometry);
        string.append(INHERITED::dumpInfo());
        return string;
    }

    struct Draw {
        Instance     fInstance;
        IndexRange   fGeometry;
        Draw*        fNext;
    };

    Draw& getSingleDraw() const { SkASSERT(fHeadDraw && !fHeadDraw->fNext); return *fHeadDraw; }
    Instance& getSingleInstance() const { return this->getSingleDraw().fInstance; }

    void appendRRectParams(const SkRRect&);
    void appendParamsTexel(const SkScalar* vals, int count);
    void appendParamsTexel(SkScalar x, SkScalar y, SkScalar z, SkScalar w);
    void appendParamsTexel(SkScalar x, SkScalar y, SkScalar z);
    FixedFunctionFlags fixedFunctionFlags() const override {
        return GrAATypeIsHW(fInfo.aaType()) ? FixedFunctionFlags::kUsesHWAA
                                            : FixedFunctionFlags::kNone;
    }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override;

    // Registers the op with the InstancedRendering list of tracked ops.
    void wasRecorded(GrRenderTargetOpList*) override;

protected:
    InstancedOp(uint32_t classID, GrPaint&&, OpAllocator*);

    bool fIsTracked : 1;
    bool fRequiresBarrierOnOverlap : 1;
    bool fAllowsSRGBInputs : 1;
    bool fDisableSRGBOutputConversion : 1;
    int fNumDraws;
    int fNumChangesInGeometry;
    Draw* fHeadDraw;
    Draw* fTailDraw;
    OpAllocator* fAllocator;
    InstancedRendering* fInstancedRendering;
    OpInfo fInfo;
    SkScalar fPixelLoad;
    GrProcessorSet fProcessors;
    SkSTArray<5, ParamsTexel, true> fParams;

private:
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override;
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState*) override;

    typedef GrDrawOp INHERITED;

    friend class InstancedRendering;
    friend class OpAllocator;
};

class OpAllocator : public SkNoncopyable {
public:
    virtual ~OpAllocator();

    /**
     * These methods make a new record internally for an instanced draw, and return an op that is
     * effectively just an index to that record. The returned op is not self-contained, but
     * rather relies on this class to handle the rendering. The client must call beginFlush() on
     * this class before attempting to flush ops returned by it. It is invalid to record new
     * draws between beginFlush() and endFlush().
     */
    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordRect(const SkRect&, const SkMatrix&,
                                                               GrPaint&&, GrAA,
                                                               const GrInstancedPipelineInfo&);

    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordRect(const SkRect&, const SkMatrix&,
                                                               GrPaint&&, const SkRect& localRect,
                                                               GrAA,
                                                               const GrInstancedPipelineInfo&);

    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordRect(const SkRect&, const SkMatrix&,
                                                               GrPaint&&,
                                                               const SkMatrix& localMatrix, GrAA,
                                                               const GrInstancedPipelineInfo&);

    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordOval(const SkRect&, const SkMatrix&,
                                                               GrPaint&&, GrAA,
                                                               const GrInstancedPipelineInfo&);

    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordRRect(const SkRRect&, const SkMatrix&,
                                                                GrPaint&&, GrAA,
                                                                const GrInstancedPipelineInfo&);

    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordDRRect(const SkRRect& outer,
                                                                 const SkRRect& inner,
                                                                 const SkMatrix&, GrPaint&&, GrAA,
                                                                 const GrInstancedPipelineInfo&);

    InstancedOp::Draw* allocateDraw() { return fDrawPool.allocate(); }
    void releaseDraw(InstancedOp::Draw* draw) { fDrawPool.release(draw); }

protected:
    OpAllocator(const GrCaps*);

private:
    bool selectAntialiasMode(const SkMatrix& viewMatrix, GrAA aa, const GrInstancedPipelineInfo&,
                             GrAAType*);
    virtual std::unique_ptr<InstancedOp> makeOp(GrPaint&&) = 0;

    std::unique_ptr<InstancedOp> SK_WARN_UNUSED_RESULT recordShape(
                                                          ShapeType, const SkRect& bounds,
                                                          const SkMatrix& viewMatrix, GrPaint&&,
                                                          const SkRect& localRect, GrAA aa,
                                                          const GrInstancedPipelineInfo&);

    GrObjectMemoryPool<InstancedOp::Draw> fDrawPool;
    sk_sp<const GrCaps>                   fCaps;
};

}

#endif
