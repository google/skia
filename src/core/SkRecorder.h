/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecorder_DEFINED
#define SkRecorder_DEFINED

#include "include/core/SkCanvasVirtualEnforcer.h"
#include "include/private/SkTDArray.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "src/core/SkBigPicture.h"
#include "src/core/SkMiniRecorder.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecords.h"

class SkBBHFactory;

class SkDrawableList : SkNoncopyable {
public:
    SkDrawableList() {}
    ~SkDrawableList();

    int count() const { return fArray.count(); }
    SkDrawable* const* begin() const { return fArray.begin(); }

    void append(SkDrawable* drawable);

    // Return a new or ref'd array of pictures that were snapped from our drawables.
    SkBigPicture::SnapshotArray* newDrawableSnapshot();

private:
    SkTDArray<SkDrawable*> fArray;
};

// SkRecorder provides an SkCanvas interface for recording into an SkRecord.

class SkRecorder final : public SkCanvasVirtualEnforcer<SkNoDrawCanvas> {
public:
    // Does not take ownership of the SkRecord.
    SkRecorder(SkRecord*, int width, int height, SkMiniRecorder* = nullptr);   // legacy version
    SkRecorder(SkRecord*, const SkRect& bounds, SkMiniRecorder* = nullptr);

    enum DrawPictureMode {
        Record_DrawPictureMode,
        Playback_DrawPictureMode,
        // Plays back top level drawPicture calls only, but records pictures within those.
        PlaybackTop_DrawPictureMode,
    };
    void reset(SkRecord*, const SkRect& bounds, DrawPictureMode, SkMiniRecorder* = nullptr);

    size_t approxBytesUsedBySubPictures() const { return fApproxBytesUsedBySubPictures; }

    SkDrawableList* getDrawableList() const { return fDrawableList.get(); }
    std::unique_ptr<SkDrawableList> detachDrawableList() { return std::move(fDrawableList); }

    // Make SkRecorder forget entirely about its SkRecord*; all calls to SkRecorder will fail.
    void forgetRecord();

    void onFlush() override;

    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    bool onDoSaveBehind(const SkRect*) override;
    void willRestore() override {}
    void didRestore() override;

    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;
    void didTranslate(SkScalar, SkScalar) override;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar x,
                        SkScalar y,
                        const SkPaint& paint) override;
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkBlendMode,
                     const SkPaint& paint) override;

    void onDrawPaint(const SkPaint&) override;
    void onDrawBehind(const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawBitmap(const SkBitmap&, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&, const SkRect* src, const SkRect& dst, const SkPaint*,
                          SrcRectConstraint) override;
    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*, SrcRectConstraint) override;
    void onDrawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                         const SkPaint*) override;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                          const SkPaint*) override;
    void onDrawImageLattice(const SkImage*, const Lattice& lattice, const SkRect& dst,
                            const SkPaint*) override;
    void onDrawBitmapLattice(const SkBitmap&, const Lattice& lattice, const SkRect& dst,
                             const SkPaint*) override;
    void onDrawVerticesObject(const SkVertices*, const SkVertices::Bone bones[], int boneCount,
                              SkBlendMode, const SkPaint&) override;
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                     int count, SkBlendMode, const SkRect* cull, const SkPaint*) override;
    void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override;

    void onClipRect(const SkRect& rect, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect& rrect, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath& path, SkClipOp, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp) override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    void onDrawAnnotation(const SkRect&, const char[], SkData*) override;

    void onDrawEdgeAAQuad(const SkRect&, const SkPoint[4], QuadAAFlags, const SkColor4f&,
                          SkBlendMode) override;
    void onDrawEdgeAAImageSet(const ImageSetEntry[], int count, const SkPoint[], const SkMatrix[],
                              const SkPaint*, SrcRectConstraint) override;

    sk_sp<SkSurface> onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    void flushMiniRecorder();

private:
    template <typename T>
    T* copy(const T*);

    template <typename T>
    T* copy(const T[], size_t count);

    template<typename T, typename... Args>
    void append(Args&&...);

    DrawPictureMode fDrawPictureMode;
    size_t fApproxBytesUsedBySubPictures;
    SkRecord* fRecord;
    std::unique_ptr<SkDrawableList> fDrawableList;

    SkMiniRecorder* fMiniRecorder;
};

#endif//SkRecorder_DEFINED
