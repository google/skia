/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPictureUtils.h"

#include "SkPaintPriv.h"
#include "SkRecord.h"
#include "SkRecords.h"

SkPicture::AccelData::Key GrAccelData::ComputeAccelDataKey() {
    static const SkPicture::AccelData::Key gGPUID = SkPicture::AccelData::GenerateDomain();

    return gGPUID;
}

// SkRecord visitor to gather saveLayer/restore information.
class CollectLayers {
public:
    CollectLayers(const SkPicture* pict, GrAccelData* accelData)
        : fPictureID(pict->uniqueID())
        , fCTM(&SkMatrix::I())
        , fSaveLayersInStack(0)
        , fAccelData(accelData) {

        pict->cullRect().roundOut(&fCurrentClipBounds);

        if (NULL == pict->fRecord.get()) {
            return;
        }

        for (fCurrentOp = 0; fCurrentOp < pict->fRecord->count(); ++fCurrentOp) {
            pict->fRecord->visit<void>(fCurrentOp, *this);
        }

        while (!fSaveStack.isEmpty()) {
            this->popSaveBlock();
        }
    }

    template <typename T> void operator()(const T& op) {
        this->updateCTM(op);
        this->updateClipBounds(op);
        this->trackSaveLayers(op);
    }

private:

    class SaveInfo {
    public:
        SaveInfo() { }
        SaveInfo(int opIndex, bool isSaveLayer, const SkPaint* paint, const SkIRect& bounds) 
            : fStartIndex(opIndex)
            , fIsSaveLayer(isSaveLayer)
            , fHasNestedSaveLayer(false)
            , fPaint(paint)
            , fBounds(bounds) {

        }

        int            fStartIndex;
        bool           fIsSaveLayer;
        bool           fHasNestedSaveLayer;
        const SkPaint* fPaint;
        SkIRect        fBounds;
    };

    uint32_t            fPictureID;
    unsigned int        fCurrentOp;
    const SkMatrix*     fCTM;
    SkIRect             fCurrentClipBounds;
    int                 fSaveLayersInStack;
    SkTDArray<SaveInfo> fSaveStack;
    GrAccelData*        fAccelData;

    template <typename T> void updateCTM(const T&) { /* most ops don't change the CTM */ }
    void updateCTM(const SkRecords::Restore& op)   { fCTM = &op.matrix; }
    void updateCTM(const SkRecords::SetMatrix& op) { fCTM = &op.matrix; }

    template <typename T> void updateClipBounds(const T&) { /* most ops don't change the clip */ }
    // Each of these devBounds fields is the state of the device bounds after the op.
    // So Restore's devBounds are those bounds saved by its paired Save or SaveLayer.
    void updateClipBounds(const SkRecords::Restore& op)    { fCurrentClipBounds = op.devBounds; }
    void updateClipBounds(const SkRecords::ClipPath& op)   { fCurrentClipBounds = op.devBounds; }
    void updateClipBounds(const SkRecords::ClipRRect& op)  { fCurrentClipBounds = op.devBounds; }
    void updateClipBounds(const SkRecords::ClipRect& op)   { fCurrentClipBounds = op.devBounds; }
    void updateClipBounds(const SkRecords::ClipRegion& op) { fCurrentClipBounds = op.devBounds; }
    void updateClipBounds(const SkRecords::SaveLayer& op)  {
        if (op.bounds) {
            fCurrentClipBounds.intersect(this->adjustAndMap(*op.bounds, op.paint));
        }
    }

    template <typename T> void trackSaveLayers(const T& op) { 
        /* most ops aren't involved in saveLayers */ 
    }
    void trackSaveLayers(const SkRecords::Save& s) { this->pushSaveBlock(); }
    void trackSaveLayers(const SkRecords::SaveLayer& sl) { this->pushSaveLayerBlock(sl.paint); }
    void trackSaveLayers(const SkRecords::Restore& r) { this->popSaveBlock(); }
    void trackSaveLayers(const SkRecords::DrawPicture& dp) {
        // For sub-pictures, we wrap their layer information within the parent
        // picture's rendering hierarchy
        const GrAccelData* childData = GPUOptimize(dp.picture);

        for (int i = 0; i < childData->numSaveLayers(); ++i) {
            const GrAccelData::SaveLayerInfo& src = childData->saveLayerInfo(i);

            this->updateStackForSaveLayer();

            // TODO: need to store an SkRect in GrAccelData::SaveLayerInfo?
            SkRect srcRect = SkRect::MakeXYWH(SkIntToScalar(src.fOffset.fX),
                                              SkIntToScalar(src.fOffset.fY),
                                              SkIntToScalar(src.fSize.width()),
                                              SkIntToScalar(src.fSize.height()));
            SkIRect newClip(fCurrentClipBounds);
            newClip.intersect(this->adjustAndMap(srcRect, dp.paint));

            GrAccelData::SaveLayerInfo& dst = fAccelData->addSaveLayerInfo();

            dst.fValid = true;
            // If src.fPicture is NULL the layer is in dp.picture; otherwise
            // it belongs to a sub-picture.
            dst.fPicture = src.fPicture ? src.fPicture : static_cast<const SkPicture*>(dp.picture);
            dst.fPicture->ref();
            dst.fSize = SkISize::Make(newClip.width(), newClip.height());
            dst.fOffset = SkIPoint::Make(newClip.fLeft, newClip.fTop);
            dst.fOriginXform = *fCTM;
            dst.fOriginXform.postConcat(src.fOriginXform);
            if (src.fPaint) {
                dst.fPaint = SkNEW_ARGS(SkPaint, (*src.fPaint));
            }
            dst.fSaveLayerOpID = src.fSaveLayerOpID;
            dst.fRestoreOpID = src.fRestoreOpID;
            dst.fHasNestedLayers = src.fHasNestedLayers;
            dst.fIsNested = fSaveLayersInStack > 0 || src.fIsNested;
        }
    }

    void pushSaveBlock() {
        fSaveStack.push(SaveInfo(fCurrentOp, false, NULL, SkIRect::MakeEmpty()));
    }

    // Inform all the saveLayers already on the stack that they now have a
    // nested saveLayer inside them
    void updateStackForSaveLayer() {
        for (int index = fSaveStack.count() - 1; index >= 0; --index) {
            if (fSaveStack[index].fHasNestedSaveLayer) {
                break;
            }
            fSaveStack[index].fHasNestedSaveLayer = true;
            if (fSaveStack[index].fIsSaveLayer) {
                break;
            }
        }
    }

    void pushSaveLayerBlock(const SkPaint* paint) {
        this->updateStackForSaveLayer();

        fSaveStack.push(SaveInfo(fCurrentOp, true, paint, fCurrentClipBounds));
        ++fSaveLayersInStack;
    }

    void popSaveBlock() {
        if (fSaveStack.count() <= 0) {
            SkASSERT(false);
            return;
        }

        SaveInfo si;
        fSaveStack.pop(&si);

        if (!si.fIsSaveLayer) {
            return;
        }

        --fSaveLayersInStack;

        GrAccelData::SaveLayerInfo& slInfo = fAccelData->addSaveLayerInfo();

        slInfo.fValid = true;
        SkASSERT(NULL == slInfo.fPicture);  // This layer is in the top-most picture
        slInfo.fSize = SkISize::Make(si.fBounds.width(), si.fBounds.height());
        slInfo.fOffset = SkIPoint::Make(si.fBounds.fLeft, si.fBounds.fTop);
        slInfo.fOriginXform = *fCTM;
        if (si.fPaint) {
            slInfo.fPaint = SkNEW_ARGS(SkPaint, (*si.fPaint));
        }
        slInfo.fSaveLayerOpID = si.fStartIndex;
        slInfo.fRestoreOpID = fCurrentOp;
        slInfo.fHasNestedLayers = si.fHasNestedSaveLayer;
        slInfo.fIsNested = fSaveLayersInStack > 0;
    }

    // Returns true if rect was meaningfully adjusted for the effects of paint,
    // false if the paint could affect the rect in unknown ways.
    static bool AdjustForPaint(const SkPaint* paint, SkRect* rect) {
        if (paint) {
            if (paint->canComputeFastBounds()) {
                *rect = paint->computeFastBounds(*rect, rect);
                return true;
            }
            return false;
        }
        return true;
    }

    // Adjust rect for all paints that may affect its geometry, then map it to device space.
    SkIRect adjustAndMap(SkRect rect, const SkPaint* paint) const {
        // Inverted rectangles really confuse our BBHs.
        rect.sort();

        // Adjust the rect for its own paint.
        if (!AdjustForPaint(paint, &rect)) {
            // The paint could do anything to our bounds.  The only safe answer is the current clip.
            return fCurrentClipBounds;
        }

        // Adjust rect for all the paints from the SaveLayers we're inside.
        for (int i = fSaveStack.count() - 1; i >= 0; i--) {
            if (!AdjustForPaint(fSaveStack[i].fPaint, &rect)) {
                // Same deal as above.
                return fCurrentClipBounds;
            }
        }

        // Map the rect back to device space.
        fCTM->mapRect(&rect);
        SkIRect devRect;
        rect.roundOut(&devRect);

        // Nothing can draw outside the current clip.
        // (Only bounded ops call into this method, so oddballs like Clear don't matter here.)
        devRect.intersect(fCurrentClipBounds);
        return devRect;
    }
};


// GPUOptimize is only intended to be called within the context of SkGpuDevice's
// EXPERIMENTAL_optimize method.
const GrAccelData* GPUOptimize(const SkPicture* pict) {
    if (NULL == pict || pict->cullRect().isEmpty()) {
        return NULL;
    }

    SkPicture::AccelData::Key key = GrAccelData::ComputeAccelDataKey();

    const GrAccelData* existing = 
                            static_cast<const GrAccelData*>(pict->EXPERIMENTAL_getAccelData(key));
    if (existing) {
        return existing;
    }

    SkAutoTUnref<GrAccelData> data(SkNEW_ARGS(GrAccelData, (key)));

    pict->EXPERIMENTAL_addAccelData(data);

    CollectLayers collector(pict, data);

    return data;
}
