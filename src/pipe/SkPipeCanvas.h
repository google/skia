/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPipeCanvas_DEFINED
#define SkPipeCanvas_DEFINED

#include "SkDeduper.h"
#include "SkImage.h"
#include "SkNoDrawCanvas.h"
#include "SkPipe.h"
#include "SkTypeface.h"
#include "SkWriteBuffer.h"

class SkPipeCanvas;
class SkPipeWriter;

template <typename T> class SkTIndexSet {
public:
    void reset() { fArray.reset(); }

    // returns the found index or 0
    int find(const T& key) const {
        const Rec* stop = fArray.end();
        for (const Rec* curr = fArray.begin(); curr < stop; ++curr) {
            if (key == curr->fKey) {
                return curr->fIndex;
            }
        }
        return 0;
    }

    // returns the new index
    int add(const T& key) {
        Rec* rec = fArray.append();
        rec->fKey = key;
        rec->fIndex = fNextIndex++;
        return rec->fIndex;
    }

private:
    struct Rec {
        T   fKey;
        int fIndex;
    };

    SkTDArray<Rec>  fArray;
    int fNextIndex = 1;
};

class SkPipeDeduper : public SkDeduper {
public:
    void resetCaches() {
        fImages.reset();
        fPictures.reset();
        fTypefaces.reset();
        fFactories.reset();
    }

    void setCanvas(SkPipeCanvas* canvas) { fPipeCanvas = canvas; }
    void setStream(SkWStream* stream) { fStream = stream; }
    void setTypefaceSerializer(SkTypefaceSerializer* tfs) { fTFSerializer = tfs; }
    void setImageSerializer(SkImageSerializer* ims) { fIMSerializer = ims; }

    // returns 0 if not found
    int findImage(SkImage* image) const { return fImages.find(image->uniqueID()); }
    int findPicture(SkPicture* picture) const { return fPictures.find(picture->uniqueID()); }

    int findOrDefineImage(SkImage*) override;
    int findOrDefinePicture(SkPicture*) override;
    int findOrDefineTypeface(SkTypeface*) override;
    int findOrDefineFactory(SkFlattenable*) override;

private:
    SkPipeCanvas*           fPipeCanvas = nullptr;
    SkWStream*              fStream = nullptr;

    SkTypefaceSerializer*   fTFSerializer = nullptr;
    SkImageSerializer*      fIMSerializer = nullptr;

    // All our keys (at the moment) are 32bit uniqueIDs
    SkTIndexSet<uint32_t>   fImages;
    SkTIndexSet<uint32_t>   fPictures;
    SkTIndexSet<uint32_t>   fTypefaces;
    SkTIndexSet<SkFlattenable::Factory> fFactories;
};


class SkPipeCanvas : public SkNoDrawCanvas {
public:
    SkPipeCanvas(const SkRect& cull, SkPipeDeduper*, SkWStream*);
    ~SkPipeCanvas() override;

protected:
    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    void willRestore() override;

    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;

    void onDrawArc(const SkRect&, SkScalar startAngle, SkScalar sweepAngle, bool useCenter,
                   const SkPaint&) override;
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                     int count, SkBlendMode, const SkRect* cull, const SkPaint*) override;
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                    const SkPaint&) override;
    void onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                       const SkPaint&) override;
    void onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                        SkScalar constY, const SkPaint&) override;
    void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath&, const SkMatrix*,
                          const SkPaint&) override;
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint&) override;
    void onDrawTextRSXform(const void* text, size_t byteLength, const SkRSXform xform[],
                           const SkRect* cull, const SkPaint& paint) override;
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4], const SkPoint texCoords[4],
                     SkBlendMode, const SkPaint&) override;

    void onDrawPaint(const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;

    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*, SrcRectConstraint) override;
    void onDrawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                         const SkPaint*) override;
    void onDrawImageLattice(const SkImage*, const Lattice& lattice, const SkRect& dst,
                            const SkPaint*) override;
    void onDrawVerticesObject(const SkVertices*, SkBlendMode, const SkPaint&) override;

    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion&, SkClipOp) override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;
    void onDrawAnnotation(const SkRect&, const char[], SkData*) override;

    // These we turn into images
    void onDrawBitmap(const SkBitmap&, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&, const SkRect* src, const SkRect& dst, const SkPaint*,
                          SrcRectConstraint) override;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                          const SkPaint*) override;
    void onDrawBitmapLattice(const SkBitmap&, const Lattice& lattice, const SkRect& dst,
                             const SkPaint*) override;
    
private:
    SkPipeDeduper*  fDeduper;
    SkWStream*      fStream;

    friend class SkPipeWriter;

    typedef SkNoDrawCanvas INHERITED;
};


#endif
