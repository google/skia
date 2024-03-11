/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecorder_DEFINED
#define SkRecorder_DEFINED

#include "include/core/SkCanvasVirtualEnforcer.h"
#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTDArray.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "src/core/SkBigPicture.h"

#include <cstddef>
#include <memory>
#include <utility>

class SkBlender;
class SkData;
class SkDrawable;
class SkImage;
class SkMatrix;
class SkMesh;
class SkPaint;
class SkPath;
class SkPicture;
class SkRRect;
class SkRecord;
class SkRegion;
class SkShader;
class SkSurface;
class SkSurfaceProps;
class SkTextBlob;
class SkVertices;
enum class SkBlendMode;
enum class SkClipOp;
struct SkDrawShadowRec;
struct SkImageInfo;
struct SkPoint;
struct SkRSXform;
struct SkRect;

namespace sktext {
    class GlyphRunList;
    namespace gpu { class Slug; }
}

class SkDrawableList : SkNoncopyable {
public:
    SkDrawableList() {}
    ~SkDrawableList();

    int count() const { return fArray.size(); }
    SkDrawable* const* begin() const { return fArray.begin(); }
    SkDrawable* const* end() const { return fArray.end(); }

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
    SkRecorder(SkRecord*, int width, int height);   // TODO: remove
    SkRecorder(SkRecord*, const SkRect& bounds);

    void reset(SkRecord*, const SkRect& bounds);

    size_t approxBytesUsedBySubPictures() const { return fApproxBytesUsedBySubPictures; }

    SkDrawableList* getDrawableList() const { return fDrawableList.get(); }
    std::unique_ptr<SkDrawableList> detachDrawableList() { return std::move(fDrawableList); }

    // Make SkRecorder forget entirely about its SkRecord*; all calls to SkRecorder will fail.
    void forgetRecord();

    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    bool onDoSaveBehind(const SkRect*) override;
    void willRestore() override {}
    void didRestore() override;

    void didConcat44(const SkM44&) override;
    void didSetM44(const SkM44&) override;
    void didScale(SkScalar, SkScalar) override;
    void didTranslate(SkScalar, SkScalar) override;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar x,
                        SkScalar y,
                        const SkPaint& paint) override;
    void onDrawSlug(const sktext::gpu::Slug* slug, const SkPaint& paint) override;
    void onDrawGlyphRunList(
            const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) override;
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

    void onDrawImage2(const SkImage*, SkScalar, SkScalar, const SkSamplingOptions&,
                      const SkPaint*) override;
    void onDrawImageRect2(const SkImage*, const SkRect&, const SkRect&, const SkSamplingOptions&,
                          const SkPaint*, SrcRectConstraint) override;
    void onDrawImageLattice2(const SkImage*, const Lattice&, const SkRect&, SkFilterMode,
                             const SkPaint*) override;
    void onDrawAtlas2(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int,
                     SkBlendMode, const SkSamplingOptions&, const SkRect*, const SkPaint*) override;

    void onDrawVerticesObject(const SkVertices*, SkBlendMode, const SkPaint&) override;

    void onDrawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) override;

    void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override;

    void onClipRect(const SkRect& rect, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect& rrect, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath& path, SkClipOp, ClipEdgeStyle) override;
    void onClipShader(sk_sp<SkShader>, SkClipOp) override;
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp) override;
    void onResetClip() override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    void onDrawAnnotation(const SkRect&, const char[], SkData*) override;

    void onDrawEdgeAAQuad(const SkRect&, const SkPoint[4], QuadAAFlags, const SkColor4f&,
                          SkBlendMode) override;
    void onDrawEdgeAAImageSet2(const ImageSetEntry[], int count, const SkPoint[], const SkMatrix[],
                               const SkSamplingOptions&, const SkPaint*,
                               SrcRectConstraint) override;

    sk_sp<SkSurface> onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;

private:
    template <typename T>
    T* copy(const T*);

    template <typename T>
    T* copy(const T[], size_t count);

    template<typename T, typename... Args>
    void append(Args&&...);

    size_t fApproxBytesUsedBySubPictures;
    SkRecord* fRecord;
    std::unique_ptr<SkDrawableList> fDrawableList;
};

#endif//SkRecorder_DEFINED
