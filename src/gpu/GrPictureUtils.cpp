/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPictureUtils.h"

#include "SkPaintPriv.h"
#include "SkPatchUtils.h"
#include "SkRecord.h"
#include "SkRecords.h"

SkPicture::AccelData::Key GrAccelData::ComputeAccelDataKey() {
    static const SkPicture::AccelData::Key gGPUID = SkPicture::AccelData::GenerateDomain();

    return gGPUID;
}

// This looks silly, I know.  Why not just use SkRect::MakeLargest()?
// In practice, this is well large enough, and it has a few extra advantages:
// it fits in an SkIRect, and we can munge it a little in both SkRect and
// SKIRect space without worrying about overflow.
static const SkRect kUnbounded = { -2e9f, -2e9f, 2e9f, 2e9f };

namespace SkRecords {

// SkRecord visitor to gather saveLayer/restore information.
class CollectLayers : SkNoncopyable {
public:
    CollectLayers(const SkPicture* pict, GrAccelData* accelData)
        : fPictureID(pict->uniqueID())
        , fSaveLayersInStack(0)
        , fAccelData(accelData) {

        // Calculate bounds for all ops.  This won't go quite in order, so we'll need
        // to store the bounds separately then feed them in to the BBH later in order.
        fCTM = &SkMatrix::I();
        fCurrentClipBounds = kUnbounded;

        if (NULL == pict->fRecord.get()) {
            return;
        }

        fBounds.reset(pict->fRecord->count());

        for (fCurrentOp = 0; fCurrentOp < pict->fRecord->count(); ++fCurrentOp) {
            pict->fRecord->visit<void>(fCurrentOp, *this);
        }

        // If we have any lingering unpaired Saves, simulate restores to make
        // sure all ops in those Save blocks have their bounds calculated.
        while (!fSaveStack.isEmpty()) {
            this->popSaveBlock();
        }

        // Any control ops not part of any Save/Restore block draw everywhere.
        while (!fControlIndices.isEmpty()) {
            this->popControl(kUnbounded);
        }

        //--------- LAYER HOISTING
        while (!fSaveLayerStack.isEmpty()) {
            this->popSaveLayerInfo();
        }
        //--------- LAYER HOISTING
    }

    template <typename T> void operator()(const T& op) {
        this->updateCTM(op);
        this->updateClipBounds(op);
        this->trackBounds(op);
        //--------- LAYER HOISTING
        this->trackSaveLayers(op);
        //--------- LAYER HOISTING
    }

private:
    // In this file, SkRect are in local coordinates, Bounds are translated back to identity space.
    typedef SkRect Bounds;

    struct SaveBounds {
        int controlOps;        // Number of control ops in this Save block, including the Save.
        Bounds bounds;         // Bounds of everything in the block.
        const SkPaint* paint;  // Unowned.  If set, adjusts the bounds of all ops in this block.
    };

    //--------- LAYER HOISTING
    class SaveLayerInfo {
    public:
        SaveLayerInfo() { }
        SaveLayerInfo(int opIndex, bool isSaveLayer, const SkPaint* paint, const Bounds& clipBound)
            : fStartIndex(opIndex)
            , fIsSaveLayer(isSaveLayer)
            , fHasNestedSaveLayer(false)
            , fPaint(paint)
            , fClipBound(clipBound) {
        }

        int            fStartIndex;
        bool           fIsSaveLayer;
        bool           fHasNestedSaveLayer;
        const SkPaint* fPaint;
        Bounds         fClipBound;
    };
    //--------- LAYER HOISTING

    // Only Restore and SetMatrix change the CTM.
    template <typename T> void updateCTM(const T&) {}
    void updateCTM(const Restore& op)   { fCTM = &op.matrix; }
    void updateCTM(const SetMatrix& op) { fCTM = &op.matrix; }

    // Most ops don't change the clip.
    template <typename T> void updateClipBounds(const T&) {}

    // Clip{Path,RRect,Rect,Region} obviously change the clip.  They all know their bounds already.
    void updateClipBounds(const ClipPath& op)   { this->updateClipBoundsForClipOp(op.devBounds); }
    void updateClipBounds(const ClipRRect& op)  { this->updateClipBoundsForClipOp(op.devBounds); }
    void updateClipBounds(const ClipRect& op)   { this->updateClipBoundsForClipOp(op.devBounds); }
    void updateClipBounds(const ClipRegion& op) { this->updateClipBoundsForClipOp(op.devBounds); }

    // The bounds of clip ops need to be adjusted for the paints of saveLayers they're inside.
    void updateClipBoundsForClipOp(const SkIRect& devBounds) {
        Bounds clip = SkRect::Make(devBounds);
        // We don't call adjustAndMap() because as its last step it would intersect the adjusted
        // clip bounds with the previous clip, exactly what we can't do when the clip grows.
        fCurrentClipBounds = this->adjustForSaveLayerPaints(&clip) ? clip : kUnbounded;
    }

    // Restore holds the devBounds for the clip after the {save,saveLayer}/restore block completes.
    void updateClipBounds(const Restore& op) {
        // This is just like the clip ops above, but we need to skip the effects (if any) of our
        // paired saveLayer (if it is one); it has not yet been popped off the save stack.  Our
        // devBounds reflect the state of the world after the saveLayer/restore block is done,
        // so they are not affected by the saveLayer's paint.
        const int kSavesToIgnore = 1;
        Bounds clip = SkRect::Make(op.devBounds);
        fCurrentClipBounds =
            this->adjustForSaveLayerPaints(&clip, kSavesToIgnore) ? clip : kUnbounded;
    }

    // We also take advantage of SaveLayer bounds when present to further cut the clip down.
    void updateClipBounds(const SaveLayer& op)  {
        if (op.bounds) {
            // adjustAndMap() intersects these layer bounds with the previous clip for us.
            fCurrentClipBounds = this->adjustAndMap(*op.bounds, op.paint);
        }
    }

    // The bounds of these ops must be calculated when we hit the Restore
    // from the bounds of the ops in the same Save block.
    void trackBounds(const Save&)          { this->pushSaveBlock(NULL); }
    void trackBounds(const SaveLayer& op)  { this->pushSaveBlock(op.paint); }
    void trackBounds(const Restore&) { fBounds[fCurrentOp] = this->popSaveBlock(); }

    void trackBounds(const SetMatrix&)         { this->pushControl(); }
    void trackBounds(const ClipRect&)          { this->pushControl(); }
    void trackBounds(const ClipRRect&)         { this->pushControl(); }
    void trackBounds(const ClipPath&)          { this->pushControl(); }
    void trackBounds(const ClipRegion&)        { this->pushControl(); }
    void trackBounds(const PushCull&)          { this->pushControl(); }
    void trackBounds(const PopCull&)           { this->pushControl(); }
    void trackBounds(const BeginCommentGroup&) { this->pushControl(); }
    void trackBounds(const AddComment&)        { this->pushControl(); }
    void trackBounds(const EndCommentGroup&)   { this->pushControl(); }
    void trackBounds(const DrawData&)          { this->pushControl(); }

    // For all other ops, we can calculate and store the bounds directly now.
    template <typename T> void trackBounds(const T& op) {
        fBounds[fCurrentOp] = this->bounds(op);
        this->updateSaveBounds(fBounds[fCurrentOp]);
    }

    void pushSaveBlock(const SkPaint* paint) {
        // Starting a new Save block.  Push a new entry to represent that.
        SaveBounds sb = { 0, Bounds::MakeEmpty(), paint };
        fSaveStack.push(sb);
        this->pushControl();
    }

    static bool PaintMayAffectTransparentBlack(const SkPaint* paint) {
        if (paint) {
            // FIXME: this is very conservative
            if (paint->getImageFilter() || paint->getColorFilter()) {
                return true;
            }

            // Unusual Xfermodes require us to process a saved layer
            // even with operations outisde the clip.
            // For example, DstIn is used by masking layers.
            // https://code.google.com/p/skia/issues/detail?id=1291
            // https://crbug.com/401593
            SkXfermode* xfermode = paint->getXfermode();
            SkXfermode::Mode mode;
            // SrcOver is ok, and is also the common case with a NULL xfermode.
            // So we should make that the fast path and bypass the mode extraction
            // and test.
            if (xfermode && xfermode->asMode(&mode)) {
                switch (mode) {
                    // For each of the following transfer modes, if the source
                    // alpha is zero (our transparent black), the resulting
                    // blended alpha is not necessarily equal to the original
                    // destination alpha.
                    case SkXfermode::kClear_Mode:
                    case SkXfermode::kSrc_Mode:
                    case SkXfermode::kSrcIn_Mode:
                    case SkXfermode::kDstIn_Mode:
                    case SkXfermode::kSrcOut_Mode:
                    case SkXfermode::kDstATop_Mode:
                    case SkXfermode::kModulate_Mode:
                        return true;
                        break;
                    default:
                        break;
                }
            }
        }
        return false;
    }

    Bounds popSaveBlock() {
        // We're done the Save block.  Apply the block's bounds to all control ops inside it.
        SaveBounds sb;
        fSaveStack.pop(&sb);

        // If the paint affects transparent black, we can't trust any of our calculated bounds.
        const Bounds& bounds =
            PaintMayAffectTransparentBlack(sb.paint) ? fCurrentClipBounds : sb.bounds;

        while (sb.controlOps-- > 0) {
            this->popControl(bounds);
        }

        // This whole Save block may be part another Save block.
        this->updateSaveBounds(bounds);

        // If called from a real Restore (not a phony one for balance), it'll need the bounds.
        return bounds;
    }

    void pushControl() {
        fControlIndices.push(fCurrentOp);
        if (!fSaveStack.isEmpty()) {
            fSaveStack.top().controlOps++;
        }
    }

    void popControl(const Bounds& bounds) {
        fBounds[fControlIndices.top()] = bounds;
        fControlIndices.pop();
    }

    void updateSaveBounds(const Bounds& bounds) {
        // If we're in a Save block, expand its bounds to cover these bounds too.
        if (!fSaveStack.isEmpty()) {
            fSaveStack.top().bounds.join(bounds);
        }
    }

    // FIXME: this method could use better bounds
    Bounds bounds(const DrawText&) const { return fCurrentClipBounds; }

    Bounds bounds(const Clear&) const { return kUnbounded; }             // Ignores the clip.
    Bounds bounds(const DrawPaint&) const { return fCurrentClipBounds; }
    Bounds bounds(const NoOp&)  const { return Bounds::MakeEmpty(); }    // NoOps don't draw.

    Bounds bounds(const DrawSprite& op) const {
        const SkBitmap& bm = op.bitmap;

        return Bounds::Make(SkIRect::MakeXYWH(op.left, op.top, bm.width(), bm.height()));  // Ignores the matrix.
    }

    Bounds bounds(const DrawRect& op) const { return this->adjustAndMap(op.rect, &op.paint); }
    Bounds bounds(const DrawOval& op) const { return this->adjustAndMap(op.oval, &op.paint); }
    Bounds bounds(const DrawRRect& op) const {
        return this->adjustAndMap(op.rrect.rect(), &op.paint);
    }
    Bounds bounds(const DrawDRRect& op) const {
        return this->adjustAndMap(op.outer.rect(), &op.paint);
    }
    Bounds bounds(const DrawImage& op) const {
        const SkImage* image = op.image;
        SkRect rect = SkRect::MakeXYWH(op.left, op.top,
                                        SkIntToScalar(image->width()), SkIntToScalar(image->height()));

        return this->adjustAndMap(rect, op.paint);
    }
    Bounds bounds(const DrawImageRect& op) const {
        return this->adjustAndMap(op.dst, op.paint);
    }
    Bounds bounds(const DrawBitmapRectToRect& op) const {
        return this->adjustAndMap(op.dst, op.paint);
    }
    Bounds bounds(const DrawBitmapNine& op) const {
        return this->adjustAndMap(op.dst, op.paint);
    }
    Bounds bounds(const DrawBitmap& op) const {
        const SkBitmap& bm = op.bitmap;
        return this->adjustAndMap(SkRect::MakeXYWH(op.left, op.top, SkIntToScalar(bm.width()), SkIntToScalar(bm.height())),
                                    op.paint);
    }
    Bounds bounds(const DrawBitmapMatrix& op) const {
        const SkBitmap& bm = op.bitmap;
        SkRect dst = SkRect::Make(SkIRect::MakeWH(bm.width(), bm.height()));
        op.matrix.mapRect(&dst);
        return this->adjustAndMap(dst, op.paint);
    }

    Bounds bounds(const DrawPath& op) const {
        return op.path.isInverseFillType() ? fCurrentClipBounds
            : this->adjustAndMap(op.path.getBounds(), &op.paint);
    }
    Bounds bounds(const DrawPoints& op) const {
        SkRect dst;
        dst.set(op.pts, op.count);

        // Pad the bounding box a little to make sure hairline points' bounds aren't empty.
        SkScalar stroke = SkMaxScalar(op.paint.getStrokeWidth(), 0.01f);
        dst.outset(stroke / 2, stroke / 2);

        return this->adjustAndMap(dst, &op.paint);
    }
    Bounds bounds(const DrawPatch& op) const {
        SkRect dst;
        dst.set(op.cubics, SkPatchUtils::kNumCtrlPts);
        return this->adjustAndMap(dst, &op.paint);
    }
    Bounds bounds(const DrawVertices& op) const {
        SkRect dst;
        dst.set(op.vertices, op.vertexCount);
        return this->adjustAndMap(dst, &op.paint);
    }

    Bounds bounds(const DrawPicture& op) const {
        SkRect dst = op.picture->cullRect();
        if (op.matrix) {
            op.matrix->mapRect(&dst);
        }
        return this->adjustAndMap(dst, op.paint);
    }

    Bounds bounds(const DrawPosText& op) const {
        const int N = op.paint.countText(op.text, op.byteLength);
        if (N == 0) {
            return Bounds::MakeEmpty();
        }

        SkRect dst;
        dst.set(op.pos, N);
        AdjustTextForFontMetrics(&dst, op.paint);
        return this->adjustAndMap(dst, &op.paint);
    }
    Bounds bounds(const DrawPosTextH& op) const {
        const int N = op.paint.countText(op.text, op.byteLength);
        if (N == 0) {
            return Bounds::MakeEmpty();
        }

        SkScalar left = op.xpos[0], right = op.xpos[0];
        for (int i = 1; i < N; i++) {
            left = SkMinScalar(left, op.xpos[i]);
            right = SkMaxScalar(right, op.xpos[i]);
        }
        SkRect dst = { left, op.y, right, op.y };
        AdjustTextForFontMetrics(&dst, op.paint);
        return this->adjustAndMap(dst, &op.paint);
    }
    Bounds bounds(const DrawTextOnPath& op) const {
        SkRect dst = op.path.getBounds();

        // Pad all sides by the maximum padding in any direction we'd normally apply.
        SkRect pad = { 0, 0, 0, 0 };
        AdjustTextForFontMetrics(&pad, op.paint);

        // That maximum padding happens to always be the right pad today.
        SkASSERT(pad.fLeft == -pad.fRight);
        SkASSERT(pad.fTop == -pad.fBottom);
        SkASSERT(pad.fRight > pad.fBottom);
        dst.outset(pad.fRight, pad.fRight);

        return this->adjustAndMap(dst, &op.paint);
    }

    Bounds bounds(const DrawTextBlob& op) const {
        SkRect dst = op.blob->bounds();
        dst.offset(op.x, op.y);
        return this->adjustAndMap(dst, &op.paint);
    }

    static void AdjustTextForFontMetrics(SkRect* rect, const SkPaint& paint) {
#ifdef SK_DEBUG
        SkRect correct = *rect;
#endif
        // crbug.com/373785 ~~> xPad = 4x yPad
        // crbug.com/424824 ~~> bump yPad from 2x text size to 2.5x
        const SkScalar yPad = 2.5f * paint.getTextSize(),
            xPad = 4.0f * yPad;
        rect->outset(xPad, yPad);
#ifdef SK_DEBUG
        SkPaint::FontMetrics metrics;
        paint.getFontMetrics(&metrics);
        correct.fLeft += metrics.fXMin;
        correct.fTop += metrics.fTop;
        correct.fRight += metrics.fXMax;
        correct.fBottom += metrics.fBottom;
        // See skia:2862 for why we ignore small text sizes.
        SkASSERTF(paint.getTextSize() < 0.001f || rect->contains(correct),
                    "%f %f %f %f vs. %f %f %f %f\n",
                    -xPad, -yPad, +xPad, +yPad,
                    metrics.fXMin, metrics.fTop, metrics.fXMax, metrics.fBottom);
#endif
    }

    //--------- LAYER HOISTING
    template <typename T> void trackSaveLayers(const T& op) {
        /* most ops aren't involved in saveLayers */
    }
    void trackSaveLayers(const Save& s) { this->pushSaveLayerInfo(false, NULL); }
    void trackSaveLayers(const SaveLayer& sl) {
        this->pushSaveLayerInfo(true, sl.paint);
    }
    void trackSaveLayers(const Restore& r) { this->popSaveLayerInfo(); }
    void trackSaveLayers(const DrawPicture& dp) {
        // For sub-pictures, we wrap their layer information within the parent
        // picture's rendering hierarchy
        const GrAccelData* childData = GPUOptimize(dp.picture);

        for (int i = 0; i < childData->numSaveLayers(); ++i) {
            const GrAccelData::SaveLayerInfo& src = childData->saveLayerInfo(i);

            Bounds newClip(fCurrentClipBounds);

            if (!newClip.intersect(this->adjustAndMap(src.fBounds, dp.paint))) {
                continue;
            }

            this->updateStackForSaveLayer();

            GrAccelData::SaveLayerInfo& dst = fAccelData->addSaveLayerInfo();

            // If src.fPicture is NULL the layer is in dp.picture; otherwise
            // it belongs to a sub-picture.
            dst.fPicture = src.fPicture ? src.fPicture : static_cast<const SkPicture*>(dp.picture);
            dst.fPicture->ref();
            dst.fBounds = newClip;
            dst.fLocalMat = src.fLocalMat;
            dst.fPreMat = src.fPreMat;
            dst.fPreMat.postConcat(*fCTM);
            if (src.fPaint) {
                dst.fPaint = SkNEW_ARGS(SkPaint, (*src.fPaint));
            }
            dst.fSaveLayerOpID = src.fSaveLayerOpID;
            dst.fRestoreOpID = src.fRestoreOpID;
            dst.fHasNestedLayers = src.fHasNestedLayers;
            dst.fIsNested = fSaveLayersInStack > 0 || src.fIsNested;
        }
    }

    // Inform all the saveLayers already on the stack that they now have a
    // nested saveLayer inside them
    void updateStackForSaveLayer() {
        for (int index = fSaveLayerStack.count() - 1; index >= 0; --index) {
            if (fSaveLayerStack[index].fHasNestedSaveLayer) {
                break;
            }
            fSaveLayerStack[index].fHasNestedSaveLayer = true;
            if (fSaveLayerStack[index].fIsSaveLayer) {
                break;
            }
        }
    }

    void pushSaveLayerInfo(bool isSaveLayer, const SkPaint* paint) {
        if (isSaveLayer) {
            this->updateStackForSaveLayer();
            ++fSaveLayersInStack;
        }

        fSaveLayerStack.push(SaveLayerInfo(fCurrentOp, isSaveLayer, paint, fCurrentClipBounds));
    }

    void popSaveLayerInfo() {
        if (fSaveLayerStack.count() <= 0) {
            SkASSERT(false);
            return;
        }

        SaveLayerInfo sli;
        fSaveLayerStack.pop(&sli);

        if (!sli.fIsSaveLayer) {
            return;
        }

        --fSaveLayersInStack;

        GrAccelData::SaveLayerInfo& slInfo = fAccelData->addSaveLayerInfo();

        SkASSERT(NULL == slInfo.fPicture);  // This layer is in the top-most picture

        slInfo.fBounds = fBounds[sli.fStartIndex];
        slInfo.fBounds.intersect(sli.fClipBound);
        slInfo.fLocalMat = *fCTM;
        slInfo.fPreMat = SkMatrix::I();
        if (sli.fPaint) {
            slInfo.fPaint = SkNEW_ARGS(SkPaint, (*sli.fPaint));
        }
        slInfo.fSaveLayerOpID = sli.fStartIndex;
        slInfo.fRestoreOpID = fCurrentOp;
        slInfo.fHasNestedLayers = sli.fHasNestedSaveLayer;
        slInfo.fIsNested = fSaveLayersInStack > 0;
    }
    //--------- LAYER HOISTING

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

    bool adjustForSaveLayerPaints(SkRect* rect, int savesToIgnore = 0) const {
        for (int i = fSaveStack.count() - 1 - savesToIgnore; i >= 0; i--) {
            if (!AdjustForPaint(fSaveStack[i].paint, rect)) {
                return false;
            }
        }
        return true;
    }

    // Adjust rect for all paints that may affect its geometry, then map it to identity space.
    Bounds adjustAndMap(SkRect rect, const SkPaint* paint) const {
        // Inverted rectangles really confuse our BBHs.
        rect.sort();

        // Adjust the rect for its own paint.
        if (!AdjustForPaint(paint, &rect)) {
            // The paint could do anything to our bounds.  The only safe answer is the current clip.
            return fCurrentClipBounds;
        }

        // Adjust rect for all the paints from the SaveLayers we're inside.
        if (!this->adjustForSaveLayerPaints(&rect)) {
            // Same deal as above.
            return fCurrentClipBounds;
        }

        // Map the rect back to identity space.
        fCTM->mapRect(&rect);

        // Nothing can draw outside the current clip.
        // (Only bounded ops call into this method, so oddballs like Clear don't matter here.)
        rect.intersect(fCurrentClipBounds);
        return rect;
    }

    // Conservative identity-space bounds for each op in the SkRecord.
    SkAutoTMalloc<Bounds> fBounds;

    // We walk fCurrentOp through the SkRecord, as we go using updateCTM()
    // and updateClipBounds() to maintain the exact CTM (fCTM) and conservative
    // identity-space bounds of the current clip (fCurrentClipBounds).
    unsigned fCurrentOp;
    const SkMatrix* fCTM;
    Bounds fCurrentClipBounds;

    // Used to track the bounds of Save/Restore blocks and the control ops inside them.
    SkTDArray<SaveBounds> fSaveStack;
    SkTDArray<unsigned>   fControlIndices;

    //--------- LAYER HOISTING
    // Used to collect saveLayer information for layer hoisting
    uint32_t              fPictureID;
    int                   fSaveLayersInStack;
    SkTDArray<SaveLayerInfo> fSaveLayerStack;
    GrAccelData*          fAccelData;
    //--------- LAYER HOISTING
};

} // namespace SkRecords

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

    SkRecords::CollectLayers collector(pict, data);

    return data;
}
