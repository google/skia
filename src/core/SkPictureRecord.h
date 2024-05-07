/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureRecord_DEFINED
#define SkPictureRecord_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkCanvasVirtualEnforcer.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkImage.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkVertices.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTo.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkPictureFlat.h"
#include "src/core/SkTHash.h"
#include "src/core/SkWriter32.h"

#include <cstddef>
#include <cstdint>

class SkBitmap;
class SkMatrix;
class SkPixmap;
class SkRRect;
class SkRegion;
class SkShader;
class SkSurface;
class SkSurfaceProps;
enum class SkBlendMode;
enum class SkClipOp;
struct SkDrawShadowRec;
struct SkIRect;
struct SkISize;
struct SkImageInfo;
struct SkPoint;
struct SkRSXform;
struct SkRect;


// These macros help with packing and unpacking a single byte value and
// a 3 byte value into/out of a uint32_t
#define MASK_24 0x00FFFFFF
#define UNPACK_8_24(combined, small, large) \
    small = (combined >> 24) & 0xFF;        \
    large = combined & MASK_24
#define PACK_8_24(small, large) ((small << 24) | large)


class SkPictureRecord : public SkCanvasVirtualEnforcer<SkCanvas> {
public:
    SkPictureRecord(const SkISize& dimensions, uint32_t recordFlags);

    SkPictureRecord(const SkIRect& dimensions, uint32_t recordFlags);

    const skia_private::TArray<sk_sp<const SkPicture>>& getPictures() const {
        return fPictures;
    }

    const skia_private::TArray<sk_sp<SkDrawable>>& getDrawables() const {
        return fDrawables;
    }

    const skia_private::TArray<sk_sp<const SkTextBlob>>& getTextBlobs() const {
        return fTextBlobs;
    }

    const skia_private::TArray<sk_sp<const sktext::gpu::Slug>>& getSlugs() const {
        return fSlugs;
    }

    const skia_private::TArray<sk_sp<const SkVertices>>& getVertices() const {
        return fVertices;
    }

    const skia_private::TArray<sk_sp<const SkImage>>& getImages() const {
        return fImages;
    }

    sk_sp<SkData> opData() const {
        this->validate(fWriter.bytesWritten(), 0);

        if (fWriter.bytesWritten() == 0) {
            return SkData::MakeEmpty();
        }
        return fWriter.snapshotAsData();
    }

    void setFlags(uint32_t recordFlags) {
        fRecordFlags = recordFlags;
    }

    const SkWriter32& writeStream() const {
        return fWriter;
    }

    void beginRecording();
    void endRecording();

protected:
    void addNoOp();

private:
    void handleOptimization(int opt);
    size_t recordRestoreOffsetPlaceholder();
    void fillRestoreOffsetPlaceholdersForCurrentStackLevel(uint32_t restoreOffset);

    SkTDArray<int32_t> fRestoreOffsetStack;

    SkTDArray<uint32_t> fCullOffsetStack;

    /*
     * Write the 'drawType' operation and chunk size to the skp. 'size'
     * can potentially be increased if the chunk size needs its own storage
     * location (i.e., it overflows 24 bits).
     * Returns the start offset of the chunk. This is the location at which
     * the opcode & size are stored.
     * TODO: since we are handing the size into here we could call reserve
     * and then return a pointer to the memory storage. This could decrease
     * allocation overhead but could lead to more wasted space (the tail
     * end of blocks could go unused). Possibly add a second addDraw that
     * operates in this manner.
     */
    size_t addDraw(DrawType drawType, size_t* size) {
        size_t offset = fWriter.bytesWritten();

        SkASSERT_RELEASE(this->predrawNotify());

        SkASSERT(0 != *size);
        SkASSERT(((uint8_t) drawType) == drawType);

        if (0 != (*size & ~MASK_24) || *size == MASK_24) {
            fWriter.writeInt(PACK_8_24(drawType, MASK_24));
            *size += 1;
            fWriter.writeInt(SkToU32(*size));
        } else {
            fWriter.writeInt(PACK_8_24(drawType, SkToU32(*size)));
        }

        return offset;
    }

    void addInt(int value) {
        fWriter.writeInt(value);
    }
    void addScalar(SkScalar scalar) {
        fWriter.writeScalar(scalar);
    }

    void addImage(const SkImage*);
    void addMatrix(const SkMatrix& matrix);
    void addPaint(const SkPaint& paint) { this->addPaintPtr(&paint); }
    void addPaintPtr(const SkPaint* paint);
    void addPatch(const SkPoint cubics[12]);
    void addPath(const SkPath& path);
    void addPicture(const SkPicture* picture);
    void addDrawable(SkDrawable* picture);
    void addPoint(const SkPoint& point);
    void addPoints(const SkPoint pts[], int count);
    void addRect(const SkRect& rect);
    void addRectPtr(const SkRect* rect);
    void addIRect(const SkIRect& rect);
    void addIRectPtr(const SkIRect* rect);
    void addRRect(const SkRRect&);
    void addRegion(const SkRegion& region);
    void addSampling(const SkSamplingOptions&);
    void addText(const void* text, size_t byteLength);
    void addTextBlob(const SkTextBlob* blob);
    void addSlug(const sktext::gpu::Slug* slug);
    void addVertices(const SkVertices*);

    int find(const SkBitmap& bitmap);

protected:
    void validate(size_t initialOffset, size_t size) const {
        SkASSERT(fWriter.bytesWritten() == initialOffset + size);
    }

    sk_sp<SkSurface> onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;
    bool onPeekPixels(SkPixmap*) override { return false; }

    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    bool onDoSaveBehind(const SkRect*) override;
    void willRestore() override;

    void didConcat44(const SkM44&) override;
    void didSetM44(const SkM44&) override;
    void didScale(SkScalar, SkScalar) override;
    void didTranslate(SkScalar, SkScalar) override;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;

    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint& paint) override;
    void onDrawSlug(const sktext::gpu::Slug* slug, const SkPaint& paint) override;
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkBlendMode, const SkPaint& paint) override;

    void onDrawPaint(const SkPaint&) override;
    void onDrawBehind(const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;

    void onDrawImage2(const SkImage*, SkScalar, SkScalar, const SkSamplingOptions&,
                      const SkPaint*) override;
    void onDrawImageRect2(const SkImage*, const SkRect&, const SkRect&, const SkSamplingOptions&,
                          const SkPaint*, SrcRectConstraint) override;
    void onDrawImageLattice2(const SkImage*, const Lattice&, const SkRect&, SkFilterMode,
                             const SkPaint*) override;
    void onDrawAtlas2(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int,
                     SkBlendMode, const SkSamplingOptions&, const SkRect*, const SkPaint*) override;

    void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override;
    void onDrawVerticesObject(const SkVertices*, SkBlendMode, const SkPaint&) override;

    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipShader(sk_sp<SkShader>, SkClipOp) override;
    void onClipRegion(const SkRegion&, SkClipOp) override;
    void onResetClip() override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawAnnotation(const SkRect&, const char[], SkData*) override;

    void onDrawEdgeAAQuad(const SkRect&, const SkPoint[4], QuadAAFlags, const SkColor4f&,
                          SkBlendMode) override;
    void onDrawEdgeAAImageSet2(const ImageSetEntry[], int count, const SkPoint[], const SkMatrix[],
                               const SkSamplingOptions&,const SkPaint*, SrcRectConstraint) override;

    int addPathToHeap(const SkPath& path);  // does not write to ops stream

    // These entry points allow the writing of matrices, clips, saves &
    // restores to be deferred (e.g., if the MC state is being collapsed and
    // only written out as needed).
    void recordConcat(const SkMatrix& matrix);
    void recordTranslate(const SkMatrix& matrix);
    void recordScale(const SkMatrix& matrix);
    size_t recordClipRect(const SkRect& rect, SkClipOp op, bool doAA);
    size_t recordClipRRect(const SkRRect& rrect, SkClipOp op, bool doAA);
    size_t recordClipPath(int pathID, SkClipOp op, bool doAA);
    size_t recordClipRegion(const SkRegion& region, SkClipOp op);
    void recordSave();
    void recordSaveLayer(const SaveLayerRec&);
    void recordRestore(bool fillInSkips = true);

private:
    skia_private::TArray<SkPaint>  fPaints;

    struct PathHash {
        uint32_t operator()(const SkPath& p) { return p.getGenerationID(); }
    };
    skia_private::THashMap<SkPath, int, PathHash> fPaths;

    SkWriter32 fWriter;

    skia_private::TArray<sk_sp<const SkImage>>    fImages;
    skia_private::TArray<sk_sp<const SkPicture>>  fPictures;
    skia_private::TArray<sk_sp<SkDrawable>>       fDrawables;
    skia_private::TArray<sk_sp<const SkTextBlob>> fTextBlobs;
    skia_private::TArray<sk_sp<const SkVertices>> fVertices;
    skia_private::TArray<sk_sp<const sktext::gpu::Slug>> fSlugs;

    uint32_t fRecordFlags;
    int      fInitialSaveCount;

    friend class SkPictureData;   // for SkPictureData's SkPictureRecord-based constructor
};

#endif
