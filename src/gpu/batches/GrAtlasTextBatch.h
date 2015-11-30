/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasTextBatch_DEFINED
#define GrAtlasTextBatch_DEFINED

#include "batches/GrVertexBatch.h"

#include "GrAtlasTextContext.h"

class GrAtlasTextBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID
    static const size_t kLCDTextVASize = sizeof(SkPoint) + sizeof(SkIPoint16);

    // position + local coord
    static const size_t kColorTextVASize = sizeof(SkPoint) + sizeof(SkIPoint16);
    static const size_t kGrayTextVASize = sizeof(SkPoint) + sizeof(GrColor) + sizeof(SkIPoint16);
    static const int kVerticesPerGlyph = 4;
    static const int kIndicesPerGlyph = 6;

    typedef GrAtlasTextContext::DistanceAdjustTable DistanceAdjustTable;
    typedef GrAtlasTextBlob Blob;
    typedef Blob::Run Run;
    typedef Run::SubRunInfo TextInfo;
    struct Geometry {
        Blob* fBlob;
        int fRun;
        int fSubRun;
        GrColor fColor;
        SkScalar fTransX;
        SkScalar fTransY;
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

    static GrAtlasTextBatch* CreateDistanceField(int glyphCount, GrBatchFontCache* fontCache,
                                                 const DistanceAdjustTable* distanceAdjustTable,
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
        fBatch.fViewMatrix = geo.fBlob->fViewMatrix;

        // We don't yet position distance field text on the cpu, so we have to map the vertex bounds
        // into device space
        const Run& run = geo.fBlob->fRuns[geo.fRun];
        if (run.fSubRunInfo[geo.fSubRun].fDrawAsDistanceFields) {
            SkRect bounds = run.fVertexBounds;
            fBatch.fViewMatrix.mapRect(&bounds);
            this->setBounds(bounds);
        } else {
            this->setBounds(run.fVertexBounds);
        }
    }

    const char* name() const override { return "TextBatch"; }

    SkString dumpInfo() const override;

    static size_t GetVertexStride(GrMaskFormat maskFormat) {
        switch (maskFormat) {
            case kA8_GrMaskFormat:
                return kGrayTextVASize;
            case kARGB_GrMaskFormat:
                return kColorTextVASize;
            default:
                return kLCDTextVASize;
        }
    }

    static size_t GetVertexStrideDf(GrMaskFormat maskFormat, bool useLCDText) {
        SkASSERT(maskFormat == kA8_GrMaskFormat);
        if (useLCDText) {
            return kLCDTextVASize;
        } else {
            return kGrayTextVASize;
        }
    }

protected:
    void computePipelineOptimizations(GrInitInvariantOutput* color, 
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override;


private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override;

    struct FlushInfo {
        SkAutoTUnref<const GrVertexBuffer> fVertexBuffer;
        SkAutoTUnref<const GrIndexBuffer> fIndexBuffer;
        int fGlyphsToFlush;
        int fVertexOffset;
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

    template <bool regenTexCoords, bool regenPos, bool regenCol, bool regenGlyphs>
    inline void regenBlob(Target* target, FlushInfo* flushInfo, Blob* blob, Run* run,
                          TextInfo* info, SkGlyphCache** cache,
                          SkTypeface** typeface, GrFontScaler** scaler, const SkDescriptor** desc,
                          const GrGeometryProcessor* gp, int glyphCount, size_t vertexStride,
                          GrColor color, SkScalar transX, SkScalar transY) const;

    inline void flush(GrVertexBatch::Target* target, FlushInfo* flushInfo) const;

    GrColor color() const { return fBatch.fColor; }
    const SkMatrix& viewMatrix() const { return fBatch.fViewMatrix; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    int numGlyphs() const { return fBatch.fNumGlyphs; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override;

    // TODO just use class params
    // TODO trying to figure out why lcd is so whack
    GrGeometryProcessor* setupDfProcessor(const SkMatrix& viewMatrix, SkColor filteredColor,
                                          GrColor color, GrTexture* texture) const;

    struct BatchTracker {
        GrColor fColor;
        SkMatrix fViewMatrix;
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
    SkAutoTUnref<const DistanceAdjustTable> fDistanceAdjustTable;
    SkColor fFilteredColor;

    typedef GrVertexBatch INHERITED;
};

#endif
