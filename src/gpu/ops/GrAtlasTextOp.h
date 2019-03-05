/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasTextOp_DEFINED
#define GrAtlasTextOp_DEFINED

#include "ops/GrMeshDrawOp.h"
#include "text/GrTextBlob.h"
#include "text/GrDistanceFieldAdjustTable.h"

class GrRecordingContext;
class SkAtlasTextTarget;

class GrAtlasTextOp final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    ~GrAtlasTextOp() override {
        for (int i = 0; i < fGeoCount; i++) {
            fGeoData[i].fBlob->unref();
        }
    }

    static const int kVerticesPerGlyph = GrTextBlob::kVerticesPerGlyph;
    static const int kIndicesPerGlyph = 6;

    typedef GrTextBlob Blob;
    struct Geometry {
        SkMatrix    fViewMatrix;
        SkIRect     fClipRect;
        Blob*       fBlob;
        SkScalar    fX;
        SkScalar    fY;
        uint16_t    fRun;
        uint16_t    fSubRun;
        SkPMColor4f fColor;
    };

    static std::unique_ptr<GrAtlasTextOp> MakeBitmap(GrRecordingContext*,
                                                     GrPaint&&,
                                                     GrMaskFormat,
                                                     int glyphCount,
                                                     bool needsTransform);

    static std::unique_ptr<GrAtlasTextOp> MakeDistanceField(
            GrRecordingContext*,
            GrPaint&&,
            int glyphCount,
            const GrDistanceFieldAdjustTable*,
            bool useGammaCorrectDistanceTable,
            SkColor luminanceColor,
            const SkSurfaceProps&,
            bool isAntiAliased,
            bool useLCD);

    // To avoid even the initial copy of the struct, we have a getter for the first item which
    // is used to seed the op with its initial geometry.  After seeding, the client should call
    // init() so the op can initialize itself
    Geometry& geometry() { return fGeoData[0]; }

    /** Called after this->geometry() has been configured. */
    void init();

    const char* name() const override { return "AtlasTextOp"; }

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override;

#ifdef SK_DEBUG
    SkString dumpInfo() const override;
#endif

    FixedFunctionFlags fixedFunctionFlags() const override;

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrFSAAType) override;

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

    void finalizeForTextTarget(uint32_t color, const GrCaps&);
    void executeForTextTarget(SkAtlasTextTarget*);

private:
    friend class GrOpMemoryPool; // for ctor

    // The minimum number of Geometry we will try to allocate.
    static constexpr auto kMinGeometryAllocated = 12;

    GrAtlasTextOp(GrPaint&& paint)
            : INHERITED(ClassID())
            , fGeoDataAllocSize(kMinGeometryAllocated)
            , fProcessors(std::move(paint)) {}

    struct FlushInfo {
        sk_sp<const GrBuffer> fVertexBuffer;
        sk_sp<const GrBuffer> fIndexBuffer;
        sk_sp<GrGeometryProcessor> fGeometryProcessor;
        GrPipeline::FixedDynamicState* fFixedDynamicState;
        int fGlyphsToFlush;
        int fVertexOffset;
    };

    void onPrepareDraws(Target*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

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

    inline void flush(GrMeshDrawOp::Target* target, FlushInfo* flushInfo) const;

    const SkPMColor4f& color() const { SkASSERT(fGeoCount > 0); return fGeoData[0].fColor; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }
    int numGlyphs() const { return fNumGlyphs; }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override;

    sk_sp<GrGeometryProcessor> setupDfProcessor(const GrShaderCaps& caps,
                                                const sk_sp<GrTextureProxy>* proxies,
                                                unsigned int numActiveProxies) const;

    SkAutoSTMalloc<kMinGeometryAllocated, Geometry> fGeoData;
    int fGeoDataAllocSize;
    GrProcessorSet fProcessors;
    struct {
        uint32_t fUsesLocalCoords : 1;
        uint32_t fUseGammaCorrectDistanceTable : 1;
        uint32_t fNeedsGlyphTransform : 1;
    };
    int fGeoCount;
    int fNumGlyphs;
    MaskType fMaskType;
    // Distance field properties
    sk_sp<const GrDistanceFieldAdjustTable> fDistanceAdjustTable;
    SkColor fLuminanceColor;
    uint32_t fDFGPFlags = 0;

    typedef GrMeshDrawOp INHERITED;
};

#endif
