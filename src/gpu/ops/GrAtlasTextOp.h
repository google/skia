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

class GrAtlasTextOp final : public GrLegacyMeshDrawOp {
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

    static std::unique_ptr<GrAtlasTextOp> MakeBitmap(GrMaskFormat maskFormat, int glyphCount,
                                                     GrAtlasGlyphCache* fontCache) {
        std::unique_ptr<GrAtlasTextOp> op(new GrAtlasTextOp);

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
        op->fFilteredColor = 0;
        op->fFontCache = fontCache;
        op->fUseBGR = false;
        return op;
    }

    static std::unique_ptr<GrAtlasTextOp> MakeDistanceField(
            int glyphCount, GrAtlasGlyphCache* fontCache,
            const GrDistanceFieldAdjustTable* distanceAdjustTable,
            bool useGammaCorrectDistanceTable, SkColor filteredColor, bool isLCD, bool useBGR) {
        std::unique_ptr<GrAtlasTextOp> op(new GrAtlasTextOp);

        op->fFontCache = fontCache;
        op->fMaskType = isLCD ? kLCDDistanceField_MaskType : kGrayscaleDistanceField_MaskType;
        op->fDistanceAdjustTable.reset(SkRef(distanceAdjustTable));
        op->fUseGammaCorrectDistanceTable = useGammaCorrectDistanceTable;
        op->fFilteredColor = filteredColor;
        op->fUseBGR = useBGR;
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

private:
    void getProcessorAnalysisInputs(GrProcessorAnalysisColor*,
                                    GrProcessorAnalysisCoverage*) const override;
    void applyPipelineOptimizations(const PipelineOptimizations&) override;

    struct FlushInfo {
        sk_sp<const GrBuffer> fVertexBuffer;
        sk_sp<const GrBuffer> fIndexBuffer;
        sk_sp<GrGeometryProcessor> fGeometryProcessor;
        int fGlyphsToFlush;
        int fVertexOffset;
    };

    void onPrepareDraws(Target* target) const override;

    GrAtlasTextOp() : INHERITED(ClassID()) {}  // initialized in factory functions.

    GrMaskFormat maskFormat() const {
        switch (fMaskType) {
            case kLCDCoverageMask_MaskType:
                return kA565_GrMaskFormat;
            case kColorBitmapMask_MaskType:
                return kARGB_GrMaskFormat;
            case kGrayscaleCoverageMask_MaskType:
            case kGrayscaleDistanceField_MaskType:
            case kLCDDistanceField_MaskType:
                return kA8_GrMaskFormat;
        }
        return kA8_GrMaskFormat;  // suppress warning
    }

    bool usesDistanceFields() const {
        return kGrayscaleDistanceField_MaskType == fMaskType ||
               kLCDDistanceField_MaskType == fMaskType;
    }

    bool isLCD() const {
        return kLCDCoverageMask_MaskType == fMaskType || kLCDDistanceField_MaskType == fMaskType;
    }

    inline void flush(GrLegacyMeshDrawOp::Target* target, FlushInfo* flushInfo) const;

    GrColor color() const { return fColor; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }
    int numGlyphs() const { return fNumGlyphs; }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override;

    // TODO just use class params
    // TODO trying to figure out why lcd is so whack
    sk_sp<GrGeometryProcessor> setupDfProcessor(GrResourceProvider*,
                                                const SkMatrix& viewMatrix, SkColor filteredColor,
                                                GrColor color, sk_sp<GrTextureProxy> proxy) const;

    GrColor fColor;
    bool fUsesLocalCoords;
    int fNumGlyphs;

    // The minimum number of Geometry we will try to allocate.
    enum { kMinGeometryAllocated = 4 };
    SkAutoSTMalloc<kMinGeometryAllocated, Geometry> fGeoData;
    int fGeoCount;

    enum MaskType {
        kGrayscaleCoverageMask_MaskType,
        kLCDCoverageMask_MaskType,
        kColorBitmapMask_MaskType,
        kGrayscaleDistanceField_MaskType,
        kLCDDistanceField_MaskType,
    } fMaskType;
    bool fUseBGR;  // fold this into the enum?

    GrAtlasGlyphCache* fFontCache;

    // Distance field properties
    sk_sp<const GrDistanceFieldAdjustTable> fDistanceAdjustTable;
    SkColor fFilteredColor;
    bool fUseGammaCorrectDistanceTable;

    friend class GrBlobRegenHelper;  // Needs to trigger flushes

    typedef GrLegacyMeshDrawOp INHERITED;
};

/*
 * A simple helper class to abstract the interface GrAtlasTextBlob needs to regenerate itself.
 * It'd be nicer if this was nested, but we need to forward declare it in GrAtlasTextBlob.h
 */
class GrBlobRegenHelper {
public:
    GrBlobRegenHelper(const GrAtlasTextOp* op, GrLegacyMeshDrawOp::Target* target,
                      GrAtlasTextOp::FlushInfo* flushInfo)
            : fOp(op), fTarget(target), fFlushInfo(flushInfo) {}

    void flush();

    void incGlyphCount(int glyphCount = 1) { fFlushInfo->fGlyphsToFlush += glyphCount; }

private:
    const GrAtlasTextOp* fOp;
    GrLegacyMeshDrawOp::Target* fTarget;
    GrAtlasTextOp::FlushInfo* fFlushInfo;
};

#endif
