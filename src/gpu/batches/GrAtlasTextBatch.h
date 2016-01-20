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
        fBatch.fViewMatrix = geo.fBlob->fViewMatrix;

        // We don't yet position distance field text on the cpu, so we have to map the vertex bounds
        // into device space.
        // We handle vertex bounds differently for distance field text and bitmap text because
        // the vertex bounds of bitmap text are in device space.  If we are flushing multiple runs
        // from one blob then we are going to pay the price here of mapping the rect for each run.
        const Run& run = geo.fBlob->fRuns[geo.fRun];
        SkRect bounds = run.fSubRunInfo[geo.fSubRun].vertexBounds();
        if (run.fSubRunInfo[geo.fSubRun].drawAsDistanceFields()) {
            // Distance field text is positioned with the (X,Y) as part of the glyph position,
            // and currently the view matrix is applied on the GPU
            bounds.offset(geo.fBlob->fX - geo.fBlob->fInitialX,
                          geo.fBlob->fY - geo.fBlob->fInitialY);
            fBatch.fViewMatrix.mapRect(&bounds);
            this->setBounds(bounds);
        } else {
            // Bitmap text is fully positioned on the CPU
            SkMatrix boundsMatrix;
            bounds.offset(-geo.fBlob->fInitialX, -geo.fBlob->fInitialY);
            boundsMatrix.setConcat(fBatch.fViewMatrix, geo.fBlob->fInitialViewMatrixInverse);
            boundsMatrix.mapRect(&bounds);

            // Due to floating point numerical inaccuracies, we have to round out here
            SkRect roundedOutBounds;
            bounds.roundOut(&roundedOutBounds);
            roundedOutBounds.offset(geo.fBlob->fX, geo.fBlob->fY);
            this->setBounds(roundedOutBounds);
        }
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
    SkAutoTUnref<const GrDistanceFieldAdjustTable> fDistanceAdjustTable;
    SkColor fFilteredColor;

    typedef GrVertexBatch INHERITED;
};

#endif
