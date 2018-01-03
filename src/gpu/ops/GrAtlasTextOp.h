/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasTextOp_DEFINED
#define GrAtlasTextOp_DEFINED

#include "ops/GrMeshDrawOp.h"
#include "text/GrAtlasTextContext.h"
#include "text/GrDistanceFieldAdjustTable.h"

class SkAtlasTextTarget;

class GrAtlasTextOp final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    ~GrAtlasTextOp() override {
        for (int i = 0; i < fGeoCount; i++) {
            fGeoData[i].fBlob->unref();
        }
    }

    static const int kVerticesPerGlyph = GrAtlasTextBlob::kVerticesPerGlyph;
    static const int kIndicesPerGlyph = 6;

    typedef GrAtlasTextBlob Blob;
    struct Geometry {
        SkMatrix fViewMatrix;
        SkIRect  fClipRect;
        Blob*    fBlob;
        SkScalar fX;
        SkScalar fY;
        uint16_t fRun;
        uint16_t fSubRun;
        GrColor  fColor;
    };

    static std::unique_ptr<GrAtlasTextOp> MakeBitmap(GrPaint&& paint, GrMaskFormat maskFormat,
                                                     int glyphCount, GrAtlasGlyphCache* fontCache) {
        std::unique_ptr<GrAtlasTextOp> op(new GrAtlasTextOp(std::move(paint)));

        op->fFontCache = fontCache;
        switch (maskFormat) {
            case kA8_GrMaskFormat:
                op->fMaskType = kGrayscaleCoverageMask_MaskType;
                break;
            case kA565_GrMaskFormat:
                op->fMaskType = kLCDCoverageMask_MaskType;
                break;
            case kARGB_GrMaskFormat:
                op->fMaskType = kColorBitmapMask_MaskType;
                break;
        }
        op->fNumGlyphs = glyphCount;
        op->fGeoCount = 1;
        op->fLuminanceColor = 0;
        op->fFontCache = fontCache;
        return op;
    }

    static std::unique_ptr<GrAtlasTextOp> MakeDistanceField(
            GrPaint&& paint, int glyphCount, GrAtlasGlyphCache* fontCache,
            const GrDistanceFieldAdjustTable* distanceAdjustTable,
            bool useGammaCorrectDistanceTable, SkColor luminanceColor, bool isLCD, bool useBGR,
            bool isAntiAliased) {
        std::unique_ptr<GrAtlasTextOp> op(new GrAtlasTextOp(std::move(paint)));

        op->fFontCache = fontCache;
        op->fMaskType = !isAntiAliased ? kAliasedDistanceField_MaskType
                                       : isLCD ? (useBGR ? kLCDBGRDistanceField_MaskType
                                                         : kLCDDistanceField_MaskType)
                                               : kGrayscaleDistanceField_MaskType;
        op->fDistanceAdjustTable.reset(SkRef(distanceAdjustTable));
        op->fUseGammaCorrectDistanceTable = useGammaCorrectDistanceTable;
        op->fLuminanceColor = luminanceColor;
        op->fNumGlyphs = glyphCount;
        op->fGeoCount = 1;
        return op;
    }

    // To avoid even the initial copy of the struct, we have a getter for the first item which
    // is used to seed the op with its initial geometry.  After seeding, the client should call
    // init() so the op can initialize itself
    Geometry& geometry() { return fGeoData[0]; }

    /** Called after this->geometry() has been configured. */
    void init();

    const char* name() const override { return "AtlasTextOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fProcessors.visitProxies(func);

        const sk_sp<GrTextureProxy>* proxies = fFontCache->getProxies(this->maskFormat());
        for (int i = 0; i < kMaxTextures; ++i) {
            if (proxies[i]) {
                func(proxies[i].get());
            }
        }
    }

    SkString dumpInfo() const override;

    FixedFunctionFlags fixedFunctionFlags() const override;

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                GrPixelConfigIsClamped dstIsClamped) override;

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
    // The minimum number of Geometry we will try to allocate.
    static constexpr auto kMinGeometryAllocated = 12;

    GrAtlasTextOp(GrPaint&& paint)
            : INHERITED(ClassID())
            , fGeoDataAllocSize(kMinGeometryAllocated)
            , fSRGBFlags(GrPipeline::SRGBFlagsFromPaint(paint))
            , fProcessors(std::move(paint)) {}

    struct FlushInfo {
        sk_sp<const GrBuffer> fVertexBuffer;
        sk_sp<const GrBuffer> fIndexBuffer;
        sk_sp<GrGeometryProcessor> fGeometryProcessor;
        const GrPipeline* fPipeline;
        int fGlyphsToFlush;
        int fVertexOffset;
    };

    void onPrepareDraws(Target*) override;

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

    GrColor color() const { SkASSERT(fGeoCount > 0); return fGeoData[0].fColor; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }
    int numGlyphs() const { return fNumGlyphs; }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override;

    static constexpr auto kMaxTextures = 4;

    sk_sp<GrGeometryProcessor> setupDfProcessor() const;

    SkAutoSTMalloc<kMinGeometryAllocated, Geometry> fGeoData;
    int fGeoDataAllocSize;
    uint32_t fSRGBFlags;
    GrProcessorSet fProcessors;
    bool fUsesLocalCoords;
    bool fCanCombineOnTouchOrOverlap;
    int fGeoCount;
    int fNumGlyphs;
    MaskType fMaskType;
    GrAtlasGlyphCache* fFontCache;
    // Distance field properties
    sk_sp<const GrDistanceFieldAdjustTable> fDistanceAdjustTable;
    SkColor fLuminanceColor;
    bool fUseGammaCorrectDistanceTable;
    uint32_t fDFGPFlags = 0;

    typedef GrMeshDrawOp INHERITED;
};

#endif
