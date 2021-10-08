/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AtlasTextOp_DEFINED
#define AtlasTextOp_DEFINED

#include "src/gpu/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/text/GrTextBlob.h"

class GrRecordingContext;

namespace skgpu::v1 {

class AtlasTextOp final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    ~AtlasTextOp() override {
        for (const Geometry* g = fHead; g != nullptr;) {
            const Geometry* next = g->fNext;
            g->~Geometry();
            g = next;
        }
    }

    void* operator new(size_t s);
    void operator delete(void* b) noexcept;
    static void ClearCache();

    static const int kVerticesPerGlyph = GrAtlasSubRun::kVerticesPerGlyph;
    static const int kIndicesPerGlyph = 6;

    struct Geometry {
        Geometry(const GrAtlasSubRun& subRun,
                 const SkMatrix& drawMatrix,
                 SkPoint drawOrigin,
                 SkIRect clipRect,
                 sk_sp<GrTextBlob> blob,
                 GrAtlasSubRunOwner subRunOwner,
                 const SkPMColor4f& color)
            : fSubRun{subRun}
            , fBlob{std::move(blob)}
            , fSubRunDtor{std::move(subRunOwner)}
            , fDrawMatrix{drawMatrix}
            , fDrawOrigin{drawOrigin}
            , fClipRect{clipRect}
            , fColor{color} {
                SkASSERT(fBlob != nullptr || fSubRunDtor != nullptr);
                SkASSERT(SkToBool(fSubRunDtor) != SkToBool(fBlob));
        }

        static Geometry* MakeForBlob(const GrAtlasSubRun& subRun,
                                     const SkMatrix& drawMatrix,
                                     SkPoint drawOrigin,
                                     SkIRect clipRect,
                                     sk_sp<GrTextBlob> blob,
                                     const SkPMColor4f& color,
                                     SkArenaAlloc* alloc);

        void fillVertexData(void* dst, int offset, int count) const;

        const GrAtlasSubRun& fSubRun;

        // Either this Geometry holds a ref to the GrTextBlob in the case of a text blob based
        // SubRun (WithCaching case), or it holds a unique_ptr to a SubRun allocated on the
        // GrTextBlobAllocator in the NoCache case. It must hold one, and can't hold both.
        sk_sp<GrTextBlob> fBlob;  // mutable to make unref call in Op dtor.
        GrAtlasSubRunOwner fSubRunDtor;

        const SkMatrix fDrawMatrix;
        const SkPoint fDrawOrigin;

        // fClipRect is only used in the DirectMaskSubRun case to do geometric clipping.
        // TransformedMaskSubRun, and SDFTSubRun don't use this field, and expect an empty rect.
        const SkIRect fClipRect;

        // Color is updated after processor analysis if it was determined the shader resolves to
        // a constant color that we then evaluate on the CPU.
        // TODO: This can be made const once processor analysis is separated from op creation.
        SkPMColor4f fColor;
        Geometry* fNext{nullptr};
    };

    const char* name() const override { return "AtlasTextOp"; }

    void visitProxies(const GrVisitProxyFunc&) const override;

    FixedFunctionFlags fixedFunctionFlags() const override;

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;

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
    inline static constexpr int kMaskTypeCount = static_cast<int>(MaskType::kLast) + 1;

#if GR_TEST_UTILS && SK_GPU_V1
    static GrOp::Owner CreateOpTestingOnly(skgpu::v1::SurfaceDrawContext*,
                                           const SkPaint&,
                                           const SkFont&,
                                           const SkMatrixProvider&,
                                           const char* text,
                                           int x,
                                           int y);
#endif

private:
    friend class GrOp; // for ctor

    struct FlushInfo {
        sk_sp<const GrBuffer> fVertexBuffer;
        sk_sp<const GrBuffer> fIndexBuffer;
        GrGeometryProcessor*  fGeometryProcessor;
        const GrSurfaceProxy** fPrimProcProxies;
        int fGlyphsToFlush = 0;
        int fVertexOffset = 0;
        int fNumDraws = 0;
    };

    AtlasTextOp(MaskType maskType,
                bool needsTransform,
                int glyphCount,
                SkRect deviceRect,
                Geometry* geo,
                GrPaint&& paint);

    AtlasTextOp(MaskType maskType,
                bool needsTransform,
                int glyphCount,
                SkRect deviceRect,
                SkColor luminanceColor,
                bool useGammaCorrectDistanceTable,
                uint32_t DFGPFlags,
                Geometry* geo,
                GrPaint&& paint);

    GrProgramInfo* programInfo() override {
        // TODO [PI]: implement
        return nullptr;
    }

    void addGeometry(Geometry* geometry) {
        *fTail = geometry;
        // The geometry may have many entries. Find the end.
        do {
            fTail = &(*fTail)->fNext;
        } while (*fTail != nullptr);
    }

    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&&,
                             const GrDstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        // We cannot surface the AtlasTextOp's programInfo at record time. As currently
        // implemented, the GP is modified at flush time based on the number of pages in the
        // atlas.
    }

    void onPrePrepareDraws(GrRecordingContext*,
                           const GrSurfaceProxyView& writeView,
                           GrAppliedClip*,
                           const GrDstProxyView&,
                           GrXferBarrierFlags renderPassXferBarriers,
                           GrLoadOp colorLoadOp) override {
        // TODO [PI]: implement
    }

    void onPrepareDraws(GrMeshDrawTarget*) override;
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
            GrMeshDrawTarget* target, FlushInfo* flushInfo) const;

    MaskType maskType() const { return static_cast<MaskType>(fMaskType); }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override;

    GrGeometryProcessor* setupDfProcessor(SkArenaAlloc*,
                                          const GrShaderCaps&,
                                          const SkMatrix& localMatrix,
                                          const GrSurfaceProxyView* views,
                                          unsigned int numActiveViews) const;

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

    Geometry* fHead{nullptr};
    Geometry** fTail{&fHead};

    using INHERITED = GrMeshDrawOp;
};

} // namespace skgpu::v1

#endif // AtlasTextOp_DEFINED
