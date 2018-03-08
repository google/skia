/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecorder_DEFINED
#define SkRecorder_DEFINED

#include "SkBigPicture.h"
#include "SkMiniRecorder.h"
#include "SkNoDrawCanvas.h"
#include "SkRecord.h"
#include "SkRecords.h"
#include "SkTDArray.h"

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

class SkRecorder final : public SkNoDrawCanvas {
public:
    // Does not take ownership of the SkRecord.
    SkRecorder(SkRecord*, int width, int height, SkMiniRecorder* = nullptr);   // legacy version
    SkRecorder(SkRecord*, const SkRect& bounds, SkMiniRecorder* = nullptr);

    enum DrawPictureMode { Record_DrawPictureMode, Playback_DrawPictureMode };
    void reset(SkRecord*, const SkRect& bounds, DrawPictureMode, SkMiniRecorder* = nullptr);

    size_t approxBytesUsedBySubPictures() const { return fApproxBytesUsedBySubPictures; }

    SkDrawableList* getDrawableList() const { return fDrawableList.get(); }
    std::unique_ptr<SkDrawableList> detachDrawableList() { return std::move(fDrawableList); }

    // Make SkRecorder forget entirely about its SkRecord*; all calls to SkRecorder will fail.
    void forgetRecord();

    void onFlush() override;

    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    void willRestore() override {}
    void didRestore() override;

    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;
    void didTranslate(SkScalar, SkScalar) override;

#define X(func, ...) void func(__VA_ARGS__) override;
#include "SkCanvasDrawVirtuals.inc"

    void onClipRect(const SkRect& rect, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect& rrect, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath& path, SkClipOp, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp) override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    sk_sp<SkSurface> onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    void flushMiniRecorder();

private:
    template <typename T>
    T* copy(const T*);

    template <typename T>
    T* copy(const T[], size_t count);

    DrawPictureMode fDrawPictureMode;
    size_t fApproxBytesUsedBySubPictures;
    SkRecord* fRecord;
    std::unique_ptr<SkDrawableList> fDrawableList;

    SkMiniRecorder* fMiniRecorder;
};

#endif//SkRecorder_DEFINED
