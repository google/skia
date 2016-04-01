/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasTextBatch_DEFINED
#define GrAtlasTextBatch_DEFINED

#include "batches/GrVertexBatch.h"

#include "text/GrAtlasTextContext.h"
#include "text/GrDistanceFieldAdjustTable.h"

class GrAtlasTextBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

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

    static GrAtlasTextBatch* CreateBitmap(GrMaskFormat maskFormat, int glyphCount,
                                          GrBatchFontCache* fontCache) {
        GrAtlasTextBatch* batch = new GrAtlasTextBatch;

        batch->fFontCache = fontCache;
        switch (maskFormat) {
            case kA8_GrMaskFormat:
                batch->fMaskType = kGrayscaleCoverageMask_MaskType;
                break;
            case kA565_GrMaskFormat:
                batch->fMaskType = kLCDCoverageMask_MaskType;
                break;
            case kARGB_GrMaskFormat:
                batch->fMaskType = kColorBitmapMask_MaskType;
                break;
        }
        batch->fBatch.fNumGlyphs = glyphCount;
        batch->fGeoCount = 1;
        batch->fFilteredColor = 0;
        batch->fFontCache = fontCache;
        batch->fUseBGR = false;
        return batch;
    }

    static GrAtlasTextBatch* CreateDistanceField(
                                              int glyphCount, GrBatchFontCache* fontCache,
                                              const GrDistanceFieldAdjustTable* distanceAdjustTable,
                                              SkColor filteredColor, bool isLCD,
                                              bool useBGR) {
        GrAtlasTextBatch* batch = new GrAtlasTextBatch;

        batch->fFontCache = fontCache;
        batch->fMaskType = isLCD ? kLCDDistanceField_MaskType : kGrayscaleDistanceField_MaskType;
        batch->fDistanceAdjustTable.reset(SkRef(distanceAdjustTable));
        batch->fFilteredColor = filteredColor;
        batch->fUseBGR = useBGR;
        batch->fBatch.fNumGlyphs = glyphCount;
        batch->fGeoCount = 1;
        return batch;
    }

    // to avoid even the initial copy of the struct, we have a getter for the first item which
    // is used to seed the batch with its initial geometry.  After seeding, the client should call
    // init() so the Batch can initialize itself
    Geometry& geometry() { return fGeoData[0]; }

    void init() {
        const Geometry& geo = fGeoData[0];
        fBatch.fColor = geo.fColor;

        geo.fBlob->computeSubRunBounds(&fBounds, geo.fRun, geo.fSubRun, geo.fViewMatrix, geo.fX,
                                       geo.fY);
    }

    const char* name() const override { return "TextBatch"; }

    SkString dumpInfo() const override;

protected:
    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override;


private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override;

    struct FlushInfo {
        SkAutoTUnref<const GrBuffer>            fVertexBuffer;
        SkAutoTUnref<const GrBuffer>            fIndexBuffer;
        SkAutoTUnref<const GrGeometryProcessor> fGeometryProcessor;
        int                                     fGlyphsToFlush;
        int                                     fVertexOffset;
    };

    void onPrepareDraws(Target* target) const override;

    GrAtlasTextBatch() : INHERITED(ClassID()) {} // initialized in factory functions.

    ~GrAtlasTextBatch() {
        for (int i = 0; i < fGeoCount; i++) {
            fGeoData[i].fBlob->unref();
        }
    }

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
        return kA8_GrMaskFormat; // suppress warning
    }

    bool usesDistanceFields() const {
        return kGrayscaleDistanceField_MaskType == fMaskType ||
               kLCDDistanceField_MaskType == fMaskType;
    }

    bool isLCD() const {
        return kLCDCoverageMask_MaskType == fMaskType ||
               kLCDDistanceField_MaskType == fMaskType;
    }

    inline void flush(GrVertexBatch::Target* target, FlushInfo* flushInfo) const;

    GrColor color() const { return fBatch.fColor; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    int numGlyphs() const { return fBatch.fNumGlyphs; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override;

    // TODO just use class params
    // TODO trying to figure out why lcd is so whack
    GrGeometryProcessor* setupDfProcessor(const SkMatrix& viewMatrix, SkColor filteredColor,
                                          GrColor color, GrTexture* texture) const;

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        int fNumGlyphs;
    };

    BatchTracker fBatch;
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
    bool fUseBGR; // fold this into the enum?

    GrBatchFontCache* fFontCache;

    // Distance field properties
    SkAutoTUnref<const GrDistanceFieldAdjustTable> fDistanceAdjustTable;
    SkColor fFilteredColor;

    friend class GrBlobRegenHelper; // Needs to trigger flushes

    typedef GrVertexBatch INHERITED;
};

/*
 * A simple helper class to abstract the interface GrAtlasTextBlob needs to regenerate itself.
 * It'd be nicer if this was nested, but we need to forward declare it in GrAtlasTextBlob.h
 */
class GrBlobRegenHelper {
public:
    GrBlobRegenHelper(const GrAtlasTextBatch* batch,
                      GrVertexBatch::Target* target,
                      GrAtlasTextBatch::FlushInfo* flushInfo)
        : fBatch(batch)
        , fTarget(target)
        , fFlushInfo(flushInfo) {}

    void flush();

    void incGlyphCount(int glyphCount = 1) {
        fFlushInfo->fGlyphsToFlush += glyphCount;
    }

private:
    const GrAtlasTextBatch* fBatch;
    GrVertexBatch::Target* fTarget;
    GrAtlasTextBatch::FlushInfo* fFlushInfo;
};

#endif
