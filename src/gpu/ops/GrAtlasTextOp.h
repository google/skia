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
        Blob* fBlob;
        SkScalar fX;
        SkScalar fY;
        int fRun;
        int fSubRun;
        GrColor fColor;
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

    void init() {
        const Geometry& geo = fGeoData[0];
        fColor = geo.fColor;
        SkRect bounds;
        geo.fBlob->computeSubRunBounds(&bounds, geo.fRun, geo.fSubRun, geo.fViewMatrix, geo.fX,
                                       geo.fY);
        // We don't have tight bounds on the glyph paths in device space. For the purposes of bounds
        // we treat this as a set of non-AA rects rendered with a texture.
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "AtlasTextOp"; }

    SkString dumpInfo() const override;

    FixedFunctionFlags fixedFunctionFlags() const override;

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override;

private:
    GrAtlasTextOp(GrPaint&& paint)
            : INHERITED(ClassID())
            , fColor(paint.getColor())
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

    void onPrepareDraws(Target* target) const override;

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

    GrColor color() const { return fColor; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }
    int numGlyphs() const { return fNumGlyphs; }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override;

    // TODO just use class params
    sk_sp<GrGeometryProcessor> setupDfProcessor(const SkMatrix& viewMatrix, SkColor luminanceColor,
                                                GrColor color, sk_sp<GrTextureProxy> proxy) const;


    // The minimum number of Geometry we will try to allocate.
    enum { kMinGeometryAllocated = 4 };

    enum MaskType {
        kGrayscaleCoverageMask_MaskType,
        kLCDCoverageMask_MaskType,
        kColorBitmapMask_MaskType,
        kAliasedDistanceField_MaskType,
        kGrayscaleDistanceField_MaskType,
        kLCDDistanceField_MaskType,
        kLCDBGRDistanceField_MaskType,
    };

    SkAutoSTMalloc<kMinGeometryAllocated, Geometry> fGeoData;
    GrColor fColor;
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

    friend class GrBlobRegenHelper;  // Needs to trigger flushes

    typedef GrMeshDrawOp INHERITED;
};

/*
 * A simple helper class to abstract the interface GrAtlasTextBlob needs to regenerate itself.
 * It'd be nicer if this was nested, but we need to forward declare it in GrAtlasTextBlob.h
 */
class GrBlobRegenHelper {
public:
    GrBlobRegenHelper(const GrAtlasTextOp* op, GrMeshDrawOp::Target* target,
                      GrAtlasTextOp::FlushInfo* flushInfo)
            : fOp(op), fTarget(target), fFlushInfo(flushInfo) {}

    void flush();

    void incGlyphCount(int glyphCount = 1) { fFlushInfo->fGlyphsToFlush += glyphCount; }

private:
    const GrAtlasTextOp* fOp;
    GrMeshDrawOp::Target* fTarget;
    GrAtlasTextOp::FlushInfo* fFlushInfo;
};

#endif
