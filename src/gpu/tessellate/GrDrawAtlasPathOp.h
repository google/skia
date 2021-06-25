/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawAtlasPathOp_DEFINED
#define GrDrawAtlasPathOp_DEFINED

#include "src/core/SkIPoint16.h"
#include "src/gpu/ops/GrDrawOp.h"

class GrDrawAtlasPathOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    GrDrawAtlasPathOp(int numRenderTargetSamples, sk_sp<GrTextureProxy> atlasProxy,
                      const SkIRect& devIBounds, const SkIPoint16& locationInAtlas,
                      bool transposedInAtlas, const SkMatrix& viewMatrix,
                      GrPaint&& paint, const SkRect& drawBounds, bool isInverseFill)
            : GrDrawOp(ClassID())
            , fEnableHWAA(numRenderTargetSamples > 1)
            , fIsInverseFill(isInverseFill)
            , fAtlasProxy(std::move(atlasProxy))
            , fHeadInstance(devIBounds, locationInAtlas, transposedInAtlas, paint.getColor4f(),
                            drawBounds, viewMatrix)
            , fProcessors(std::move(paint)) {
        this->setBounds(drawBounds, HasAABloat::kYes, IsHairline::kNo);
    }

    const char* name() const override { return "GrDrawAtlasPathOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override {
        return (fEnableHWAA) ? FixedFunctionFlags::kUsesHWAA : FixedFunctionFlags::kNone;
    }
    void visitProxies(const GrVisitProxyFunc& func) const override {
        func(fAtlasProxy.get(), GrMipmapped::kNo);
        fProcessors.visitProxies(func);
    }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;
    CombineResult onCombineIfPossible(GrOp*, SkArenaAlloc*, const GrCaps&) override;
    void onPrepare(GrOpFlushState*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

private:
    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip*,
                      const GrDstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) override;

    struct Instance {
        Instance(const SkIRect& devIBounds, const SkIPoint16& locationInAtlas,
                 bool transposedInAtlas, const SkPMColor4f& color, const SkRect& drawBounds,
                 const SkMatrix& m)
                : fDevXYWH{devIBounds.left(), devIBounds.top(), devIBounds.width(),
                           // We use negative height to indicate that the path is transposed.
                           (transposedInAtlas) ? -devIBounds.height() : devIBounds.height()}
                , fAtlasXY{locationInAtlas.x(), locationInAtlas.y()}
                , fColor(color)
                , fDrawBoundsIfInverseFilled(drawBounds)
                , fViewMatrixIfUsingLocalCoords{m.getScaleX(), m.getSkewY(),
                                                m.getSkewX(), m.getScaleY(),
                                                m.getTranslateX(), m.getTranslateY()} {
        }
        std::array<int, 4> fDevXYWH;
        std::array<int, 2> fAtlasXY;
        SkPMColor4f fColor;
        SkRect fDrawBoundsIfInverseFilled;
        std::array<float, 6> fViewMatrixIfUsingLocalCoords;
        Instance* fNext = nullptr;
    };

    const bool fEnableHWAA;
    const bool fIsInverseFill;
    const sk_sp<GrTextureProxy> fAtlasProxy;
    bool fUsesLocalCoords = false;

    Instance fHeadInstance;
    Instance** fTailInstance = &fHeadInstance.fNext;
    int fInstanceCount = 1;

    GrProgramInfo* fProgram = nullptr;

    sk_sp<const GrBuffer> fInstanceBuffer;
    int fBaseInstance;

    GrProcessorSet fProcessors;
};

#endif
