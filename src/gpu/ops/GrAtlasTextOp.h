/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasTextOp_DEFINED
#define GrAtlasTextOp_DEFINED

#include "src/gpu/GrTBlockList.h"
#include "src/gpu/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/text/GrTextBlob.h"

class GrRecordingContext;

class GrAtlasTextOp final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    ~GrAtlasTextOp() override {
        for (const auto& g : fGeometries.items()) {
            g.fBlob->unref();
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

        // Color is updated after processor analysis if it was determined the shader resolves to
        // a constant color that we then evaluate on the CPU.
        // TODO: This can be made const once processor analysis is separated from op creation.
        SkPMColor4f          fColor;
    };

    const char* name() const override { return "AtlasTextOp"; }

    void visitProxies(const VisitProxyFunc& func) const override;

    FixedFunctionFlags fixedFunctionFlags() const override;

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;

    enum class MaskType : uint32_t {
        kGrayscaleCoverage,
        kLCDCoverage,
        kColorBitmap,
        kAliasedDistanceField,
        kGrayscaleDistanceField,
        kLCDDistanceField,
        kLCDBGRDistanceField,

        kLast = kLCDBGRDistanceField
    };
    static constexpr int kMaskTypeCount = static_cast<int>(MaskType::kLast) + 1;

#if GR_TEST_UTILS
    static GrOp::Owner CreateOpTestingOnly(GrSurfaceDrawContext* rtc,
                                           const SkPaint& skPaint,
                                           const SkFont& font,
                                           const SkMatrixProvider& mtxProvider,
                                           const char* text,
                                           int x,
                                           int y);
#endif

private:
    friend class GrOp; // for ctor

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
                             const GrSurfaceProxyView& writeView,
                             GrAppliedClip&&,
                             const GrXferProcessor::DstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        // We cannot surface the GrAtlasTextOp's programInfo at record time. As currently
        // implemented, the GP is modified at flush time based on the number of pages in the
        // atlas.
    }

    void onPrePrepareDraws(GrRecordingContext*,
                           const GrSurfaceProxyView& writeView,
                           GrAppliedClip*,
                           const GrXferProcessor::DstProxyView&,
                           GrXferBarrierFlags renderPassXferBarriers,
                           GrLoadOp colorLoadOp) override {
        // TODO [PI]: implement
    }

    void onPrepareDraws(Target*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

#if GR_TEST_UTILS
    SkString onDumpInfo() const override;
#endif

    GrMaskFormat maskFormat() const {
        switch (this->maskType()) {
            case MaskType::kLCDCoverage:
                return kA565_GrMaskFormat;
            case MaskType::kColorBitmap:
                return kARGB_GrMaskFormat;
            case MaskType::kGrayscaleCoverage:
            case MaskType::kAliasedDistanceField:
            case MaskType::kGrayscaleDistanceField:
            case MaskType::kLCDDistanceField:
            case MaskType::kLCDBGRDistanceField:
                return kA8_GrMaskFormat;
        }
        // SkUNREACHABLE;
        return kA8_GrMaskFormat;
    }

    bool usesDistanceFields() const {
        return MaskType::kAliasedDistanceField == this->maskType() ||
               MaskType::kGrayscaleDistanceField == this->maskType() ||
               MaskType::kLCDDistanceField == this->maskType() ||
               MaskType::kLCDBGRDistanceField == this->maskType();
    }

    bool isLCD() const {
        return MaskType::kLCDCoverage == this->maskType() ||
               MaskType::kLCDDistanceField == this->maskType() ||
               MaskType::kLCDBGRDistanceField == this->maskType();
    }

    inline void createDrawForGeneratedGlyphs(
            GrMeshDrawOp::Target* target, FlushInfo* flushInfo) const;

    MaskType maskType() const { return static_cast<MaskType>(fMaskType); }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override;

    GrGeometryProcessor* setupDfProcessor(SkArenaAlloc*,
                                          const GrShaderCaps&,
                                          const SkMatrix& localMatrix,
                                          const GrSurfaceProxyView* views,
                                          unsigned int numActiveViews) const;

    // The minimum number of Geometry we will try to allocate as ops are merged together.
    // The atlas text op holds one Geometry inline. When combined with the linear growth policy,
    // the total number of geometries follows 6, 18, 36, 60, 90 (the deltas are 6*n).
    static constexpr auto kMinGeometryAllocated = 6;

    GrTBlockList<Geometry> fGeometries{kMinGeometryAllocated,
                                       GrBlockAllocator::GrowthPolicy::kLinear};

    GrProcessorSet fProcessors;
    int fNumGlyphs; // Sum of glyphs in each geometry's subrun

    // All combinable atlas ops have equal bit field values
    uint32_t fDFGPFlags                    : 9; // Distance field properties
    uint32_t fMaskType                     : 3; // MaskType
    uint32_t fUsesLocalCoords              : 1; // Filled in post processor analysis
    uint32_t fNeedsGlyphTransform          : 1;
    uint32_t fHasPerspective               : 1; // True if perspective affects draw
    uint32_t fUseGammaCorrectDistanceTable : 1;
    static_assert(kMaskTypeCount <= 8, "MaskType does not fit in 3 bits");
    static_assert(kInvalid_DistanceFieldEffectFlag <= (1 << 8),  "DFGP Flags do not fit in 9 bits");

    // Only used for distance fields; per-channel luminance for LCD, or gamma-corrected luminance
    // for single-channel distance fields.
    const SkColor fLuminanceColor{0};

    using INHERITED = GrMeshDrawOp;
};

#endif
