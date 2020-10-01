/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasTextOp_DEFINED
#define GrAtlasTextOp_DEFINED

#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/text/GrTextBlob.h"

class GrRecordingContext;

class GrAtlasTextOp final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    ~GrAtlasTextOp() override {
        for (int i = 0; i < fGeoCount; i++) {
            fGeoData[i].fBlob->unref();
        }
    }

    static const int kVerticesPerGlyph = GrAtlasSubRun::kVerticesPerGlyph;
    static const int kIndicesPerGlyph = 6;

    struct Geometry {
        void fillVertexData(void* dst, int offset, int count) const;

        const GrAtlasSubRun& fSubRun;
        const SkMatrix       fDrawMatrix;
        const SkPoint        fDrawOrigin;
        const SkIRect        fClipRect;
        GrTextBlob* const    fBlob;  // mutable to make unref call in Op dtor.

        // Strangely, the color is mutated as part of the onPrepare process.
        SkPMColor4f          fColor;
    };

    const char* name() const override { return "AtlasTextOp"; }

    void visitProxies(const VisitProxyFunc& func) const override;

    FixedFunctionFlags fixedFunctionFlags() const override;

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;

    enum MaskType {
        kGrayscaleCoverageMask_MaskType,
        kLCDCoverageMask_MaskType,
        kColorBitmapMask_MaskType,
        kAliasedDistanceField_MaskType,
        kGrayscaleDistanceField_MaskType,
        kLCDDistanceField_MaskType,
        kLCDBGRDistanceField_MaskType,
    };

    MaskType maskType() const { return fMaskType; }

#if GR_TEST_UTILS
    static std::unique_ptr<GrDrawOp> CreateOpTestingOnly(GrRenderTargetContext* rtc,
                                                         const SkPaint& skPaint,
                                                         const SkFont& font,
                                                         const SkMatrixProvider& mtxProvider,
                                                         const char* text,
                                                         int x,
                                                         int y);
#endif

private:
    friend class GrOpMemoryPool; // for ctor

    // The minimum number of Geometry we will try to allocate.
    static constexpr auto kMinGeometryAllocated = 12;

    GrAtlasTextOp(MaskType maskType,
                  bool needsTransform,
                  int glyphCount,
                  SkRect deviceRect,
                  const Geometry& geo,
                  GrPaint&& paint);

    GrAtlasTextOp(MaskType maskType,
                  bool needsTransform,
                  int glyphCount,
                  SkRect deviceRect,
                  SkColor luminanceColor,
                  bool useGammaCorrectDistanceTable,
                  uint32_t DFGPFlags,
                  const Geometry& geo,
                  GrPaint&& paint);

    struct FlushInfo {
        sk_sp<const GrBuffer> fVertexBuffer;
        sk_sp<const GrBuffer> fIndexBuffer;
        GrGeometryProcessor*  fGeometryProcessor;
        const GrSurfaceProxy** fPrimProcProxies;
        int fGlyphsToFlush = 0;
        int fVertexOffset = 0;
        int fNumDraws = 0;
    };

    GrProgramInfo* programInfo() override {
        // TODO [PI]: implement
        return nullptr;
    }

    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView* writeView,
                             GrAppliedClip&&,
                             const GrXferProcessor::DstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers) override {
        // We cannot surface the GrAtlasTextOp's programInfo at record time. As currently
        // implemented, the GP is modified at flush time based on the number of pages in the
        // atlas.
    }

    void onPrePrepareDraws(GrRecordingContext*,
                           const GrSurfaceProxyView* writeView,
                           GrAppliedClip*,
                           const GrXferProcessor::DstProxyView&,
                           GrXferBarrierFlags renderPassXferBarriers) override {
        // TODO [PI]: implement
    }

    void onPrepareDraws(Target*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

#if GR_TEST_UTILS
    SkString onDumpInfo() const override;
#endif

    GrMaskFormat maskFormat() const {
        switch (fMaskType) {
            case kLCDCoverageMask_MaskType:
                return kA565_GrMaskFormat;
            case kColorBitmapMask_MaskType:
                return kARGB_GrMaskFormat;
            case kGrayscaleCoverageMask_MaskType:
            case kAliasedDistanceField_MaskType:
            case kGrayscaleDistanceField_MaskType:
            case kLCDDistanceField_MaskType:
            case kLCDBGRDistanceField_MaskType:
                return kA8_GrMaskFormat;
        }
        return kA8_GrMaskFormat;  // suppress warning
    }

    bool usesDistanceFields() const {
        return kAliasedDistanceField_MaskType == fMaskType ||
               kGrayscaleDistanceField_MaskType == fMaskType ||
               kLCDDistanceField_MaskType == fMaskType ||
               kLCDBGRDistanceField_MaskType == fMaskType;
    }

    bool isLCD() const {
        return kLCDCoverageMask_MaskType == fMaskType ||
               kLCDDistanceField_MaskType == fMaskType ||
               kLCDBGRDistanceField_MaskType == fMaskType;
    }

    inline void createDrawForGeneratedGlyphs(
            GrMeshDrawOp::Target* target, FlushInfo* flushInfo) const;

    const SkPMColor4f& color() const { SkASSERT(fGeoCount > 0); return fGeoData[0].fColor; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }
    int numGlyphs() const { return fNumGlyphs; }

    CombineResult onCombineIfPossible(GrOp* t, GrRecordingContext::Arenas*,
                                      const GrCaps& caps) override;

    GrGeometryProcessor* setupDfProcessor(SkArenaAlloc*,
                                          const GrShaderCaps&,
                                          const GrSurfaceProxyView* views,
                                          unsigned int numActiveViews) const;

    const MaskType fMaskType;
    const bool fNeedsGlyphTransform;
    const SkColor fLuminanceColor{0};
    const bool fUseGammaCorrectDistanceTable{false};
    // Distance field properties
    const uint32_t fDFGPFlags;
    SkAutoSTMalloc<kMinGeometryAllocated, Geometry> fGeoData;
    int fGeoDataAllocSize;
    GrProcessorSet fProcessors;
    bool fUsesLocalCoords;
    int fGeoCount;
    int fNumGlyphs;

    using INHERITED = GrMeshDrawOp;
};

#endif
