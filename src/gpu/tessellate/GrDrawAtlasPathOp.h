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
                      bool transposedInAtlas, const SkMatrix& viewMatrix, GrPaint&& paint)
            : GrDrawOp(ClassID())
            , fEnableHWAA(numRenderTargetSamples > 1)
            , fAtlasProxy(std::move(atlasProxy))
            , fInstanceList(devIBounds, locationInAtlas, transposedInAtlas, paint.getColor4f(),
                            viewMatrix)
            , fProcessors(std::move(paint)) {
        this->setBounds(SkRect::Make(devIBounds), HasAABloat::kYes, IsHairline::kNo);
    }

    const char* name() const override { return "GrDrawAtlasPathOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override {
        return (fEnableHWAA) ? FixedFunctionFlags::kUsesHWAA : FixedFunctionFlags::kNone;
    }
    void visitProxies(const VisitProxyFunc& fn) const override {
        fn(fAtlasProxy.get(), GrMipmapped::kNo);
        fProcessors.visitProxies(fn);
    }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;
    CombineResult onCombineIfPossible(GrOp*, SkArenaAlloc*, const GrCaps&) override;
    void onPrepare(GrOpFlushState*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

private:
    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) override;

    struct Instance {
        constexpr static size_t Stride(bool usesLocalCoords) {
            size_t stride = sizeof(Instance);
            if (!usesLocalCoords) {
                stride -= sizeof(Instance::fViewMatrixIfUsingLocalCoords);
            }
            return stride;
        }
        Instance(const SkIRect& devIBounds, const SkIPoint16& locationInAtlas,
                 bool transposedInAtlas, const SkPMColor4f& color, const SkMatrix& m)
                : fDevXYWH{devIBounds.left(), devIBounds.top(), devIBounds.width(),
                           // We use negative height to indicate that the path is transposed.
                           (transposedInAtlas) ? -devIBounds.height() : devIBounds.height()}
                , fAtlasXY{locationInAtlas.x(), locationInAtlas.y()}
                , fColor(color)
                , fViewMatrixIfUsingLocalCoords{m.getScaleX(), m.getSkewY(),
                                                m.getSkewX(), m.getScaleY(),
                                                m.getTranslateX(), m.getTranslateY()} {
        }
        std::array<int, 4> fDevXYWH;
        std::array<int, 2> fAtlasXY;
        SkPMColor4f fColor;
        float fViewMatrixIfUsingLocalCoords[6];
    };

    struct InstanceList {
        InstanceList(const SkIRect& devIBounds, const SkIPoint16& locationInAtlas,
                     bool transposedInAtlas, const SkPMColor4f& color, const SkMatrix& viewMatrix)
                : fInstance(devIBounds, locationInAtlas, transposedInAtlas, color, viewMatrix) {
        }
        InstanceList* fNext = nullptr;
        Instance fInstance;
    };

    const bool fEnableHWAA;
    const sk_sp<GrTextureProxy> fAtlasProxy;
    bool fUsesLocalCoords = false;

    InstanceList fInstanceList;
    InstanceList** fInstanceTail = &fInstanceList.fNext;
    int fInstanceCount = 1;

    sk_sp<const GrBuffer> fInstanceBuffer;
    int fBaseInstance;

    GrProcessorSet fProcessors;
};

#endif
