/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDeduper.h"
#include "SkImageDeserializer.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPipe.h"
#include "SkPipeFormat.h"
#include "SkReadBuffer.h"
#include "SkRefSet.h"
#include "SkRSXform.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"
#include "SkVertices.h"

class SkPipeReader;

static bool do_playback(SkPipeReader& reader, SkCanvas* canvas, int* endPictureIndex = nullptr);

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkPipeInflator : public SkInflator {
public:
    SkPipeInflator(SkRefSet<SkImage>* images, SkRefSet<SkPicture>* pictures,
                   SkRefSet<SkTypeface>* typefaces, SkTDArray<SkFlattenable::Factory>* factories,
                   SkTypefaceDeserializer* tfd, SkImageDeserializer* imd)
        : fImages(images)
        , fPictures(pictures)
        , fTypefaces(typefaces)
        , fFactories(factories)
        , fTFDeserializer(tfd)
        , fIMDeserializer(imd)
    {}
    
    SkImage* getImage(int index) override {
        return index ? fImages->get(index - 1) : nullptr;
    }
    SkPicture* getPicture(int index) override {
        return index ? fPictures->get(index - 1) : nullptr;
    }
    SkTypeface* getTypeface(int index) override {
        return fTypefaces->get(index - 1);
    }
    SkFlattenable::Factory getFactory(int index) override {
        return index ? fFactories->getAt(index - 1) : nullptr;
    }

    bool setImage(int index, SkImage* img) {
        return fImages->set(index - 1, img);
    }
    bool setPicture(int index, SkPicture* pic) {
        return fPictures->set(index - 1, pic);
    }
    bool setTypeface(int index, SkTypeface* face) {
        return fTypefaces->set(index - 1, face);
    }
    bool setFactory(int index, SkFlattenable::Factory factory) {
        SkASSERT(index > 0);
        SkASSERT(factory);
        index -= 1;
        if ((unsigned)index < (unsigned)fFactories->count()) {
            (*fFactories)[index] = factory;
            return true;
        }
        if (fFactories->count() == index) {
            *fFactories->append() = factory;
            return true;
        }
        SkDebugf("setFactory: index [%d] out of range %d\n", index, fFactories->count());
        return false;
    }

    void setTypefaceDeserializer(SkTypefaceDeserializer* tfd) {
        fTFDeserializer = tfd;
    }
    
    void setImageDeserializer(SkImageDeserializer* imd) {
        fIMDeserializer = imd;
    }
    
    sk_sp<SkTypeface> makeTypeface(const void* data, size_t size);
    sk_sp<SkImage> makeImage(const sk_sp<SkData>&);

private:
    SkRefSet<SkImage>*                  fImages;
    SkRefSet<SkPicture>*                fPictures;
    SkRefSet<SkTypeface>*               fTypefaces;
    SkTDArray<SkFlattenable::Factory>*  fFactories;

    SkTypefaceDeserializer*             fTFDeserializer;
    SkImageDeserializer*                fIMDeserializer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> const T* skip(SkReadBuffer& reader, int count = 1) {
    return (const T*)reader.skip(count * sizeof(T));
}

static SkRRect read_rrect(SkReadBuffer& reader) {
    SkRRect rrect;
    rrect.readFromMemory(reader.skip(SkRRect::kSizeInMemory), SkRRect::kSizeInMemory);
    return rrect;
}

static SkMatrix read_sparse_matrix(SkReadBuffer& reader, SkMatrix::TypeMask tm) {
    SkMatrix matrix;
    matrix.reset();

    if (tm & SkMatrix::kPerspective_Mask) {
        matrix.set9(skip<SkScalar>(reader, 9));
    } else if (tm & SkMatrix::kAffine_Mask) {
        const SkScalar* tmp = skip<SkScalar>(reader, 6);
        matrix[SkMatrix::kMScaleX] = tmp[0];
        matrix[SkMatrix::kMSkewX]  = tmp[1];
        matrix[SkMatrix::kMTransX] = tmp[2];
        matrix[SkMatrix::kMScaleY] = tmp[3];
        matrix[SkMatrix::kMSkewY]  = tmp[4];
        matrix[SkMatrix::kMTransY] = tmp[5];
    } else if (tm & SkMatrix::kScale_Mask) {
        const SkScalar* tmp = skip<SkScalar>(reader, 4);
        matrix[SkMatrix::kMScaleX] = tmp[0];
        matrix[SkMatrix::kMTransX] = tmp[1];
        matrix[SkMatrix::kMScaleY] = tmp[2];
        matrix[SkMatrix::kMTransY] = tmp[3];
    } else if (tm & SkMatrix::kTranslate_Mask) {
        const SkScalar* tmp = skip<SkScalar>(reader, 2);
        matrix[SkMatrix::kMTransX] = tmp[0];
        matrix[SkMatrix::kMTransY] = tmp[1];
    }
    // else read nothing for Identity
    return matrix;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#define CHECK_SET_SCALAR(Field)                 \
    do { if (nondef & k##Field##_NonDef) {      \
        paint.set##Field(reader.readScalar());  \
    }} while (0)

#define CHECK_SET_FLATTENABLE(Field)            \
    do { if (nondef & k##Field##_NonDef) {      \
        paint.set##Field(reader.read##Field()); \
    }} while (0)

/*
 *  Header:
 *      paint flags     : 32
 *      non_def bits    : 16
 *      xfermode enum   : 8
 *      pad zeros       : 8
 */
static SkPaint read_paint(SkReadBuffer& reader) {
    SkPaint paint;

    uint32_t packedFlags = reader.read32();
    uint32_t extra = reader.read32();
    unsigned nondef = extra >> 16;
    paint.setBlendMode(SkBlendMode((extra >> 8) & 0xFF));
    SkASSERT((extra & 0xFF) == 0);  // zero pad byte

    packedFlags >>= 2;  // currently unused
    paint.setTextEncoding((SkPaint::TextEncoding)(packedFlags & 3));    packedFlags >>= 2;
    paint.setTextAlign((SkPaint::Align)(packedFlags & 3));              packedFlags >>= 2;
    paint.setHinting((SkPaint::Hinting)(packedFlags & 3));              packedFlags >>= 2;
    paint.setStrokeJoin((SkPaint::Join)(packedFlags & 3));              packedFlags >>= 2;
    paint.setStrokeCap((SkPaint::Cap)(packedFlags & 3));                packedFlags >>= 2;
    paint.setStyle((SkPaint::Style)(packedFlags & 3));                  packedFlags >>= 2;
    paint.setFilterQuality((SkFilterQuality)(packedFlags & 3));         packedFlags >>= 2;
    paint.setFlags(packedFlags);

    CHECK_SET_SCALAR(TextSize);
    CHECK_SET_SCALAR(TextScaleX);
    CHECK_SET_SCALAR(TextSkewX);
    CHECK_SET_SCALAR(StrokeWidth);
    CHECK_SET_SCALAR(StrokeMiter);

    if (nondef & kColor_NonDef) {
        paint.setColor(reader.read32());
    }

    CHECK_SET_FLATTENABLE(Typeface);
    CHECK_SET_FLATTENABLE(PathEffect);
    CHECK_SET_FLATTENABLE(Shader);
    CHECK_SET_FLATTENABLE(MaskFilter);
    CHECK_SET_FLATTENABLE(ColorFilter);
    CHECK_SET_FLATTENABLE(Rasterizer);
    CHECK_SET_FLATTENABLE(ImageFilter);
    CHECK_SET_FLATTENABLE(DrawLooper);

    return paint;
}

class SkPipeReader : public SkReadBuffer {
public:
    SkPipeReader(SkPipeDeserializer* sink, const void* data, size_t size)
    : SkReadBuffer(data, size)
    , fSink(sink)
    {}

    SkPipeDeserializer* fSink;

    SkFlattenable::Factory findFactory(const char name[]) {
        SkFlattenable::Factory factory;
        // Check if a custom Factory has been specified for this flattenable.
        if (!(factory = this->getCustomFactory(SkString(name)))) {
            // If there is no custom Factory, check for a default.
            factory = SkFlattenable::NameToFactory(name);
        }
        return factory;
    }
    
    void readPaint(SkPaint* paint) override {
        *paint = read_paint(*this);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*SkPipeHandler)(SkPipeReader&, uint32_t packedVerb, SkCanvas*);

static void save_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kSave == unpack_verb(packedVerb));
    canvas->save();
}

static void saveLayer_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kSaveLayer == unpack_verb(packedVerb));
    unsigned extra = unpack_verb_extra(packedVerb);
    const SkRect* bounds = (extra & kHasBounds_SaveLayerMask) ? skip<SkRect>(reader) : nullptr;
    SkPaint paintStorage, *paint = nullptr;
    if (extra & kHasPaint_SaveLayerMask) {
        paintStorage = read_paint(reader);
        paint = &paintStorage;
    }
    sk_sp<SkImageFilter> backdrop;
    if (extra & kHasBackdrop_SaveLayerMask) {
        backdrop = reader.readImageFilter();
    }
    sk_sp<SkImage> clipMask;
    if (extra & kHasClipMask_SaveLayerMask) {
        clipMask = reader.readImage();
    }
    SkMatrix clipMatrix;
    if (extra & kHasClipMatrix_SaveLayerMask) {
        reader.readMatrix(&clipMatrix);
    }
    SkCanvas::SaveLayerFlags flags = (SkCanvas::SaveLayerFlags)(extra & kFlags_SaveLayerMask);

    // unremap this wacky flag
    if (extra & kDontClipToLayer_SaveLayerMask) {
        flags |= (1 << 31);//SkCanvas::kDontClipToLayer_PrivateSaveLayerFlag;
    }

    canvas->saveLayer(SkCanvas::SaveLayerRec(bounds, paint, backdrop.get(), clipMask.get(),
                      (extra & kHasClipMatrix_SaveLayerMask) ? &clipMatrix : nullptr, flags));
}

static void restore_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kRestore == unpack_verb(packedVerb));
    canvas->restore();
}

static void concat_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kConcat == unpack_verb(packedVerb));
    SkMatrix::TypeMask tm = (SkMatrix::TypeMask)(packedVerb & kTypeMask_ConcatMask);
    const SkMatrix matrix = read_sparse_matrix(reader, tm);
    if (packedVerb & kSetMatrix_ConcatMask) {
        canvas->setMatrix(matrix);
    } else {
        canvas->concat(matrix);
    }
}

static void clipRect_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kClipRect == unpack_verb(packedVerb));
    SkClipOp op = (SkClipOp)(unpack_verb_extra(packedVerb) >> 1);
    bool isAA = unpack_verb_extra(packedVerb) & 1;
    canvas->clipRect(*skip<SkRect>(reader), op, isAA);
}

static void clipRRect_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kClipRRect == unpack_verb(packedVerb));
    SkClipOp op = (SkClipOp)(unpack_verb_extra(packedVerb) >> 1);
    bool isAA = unpack_verb_extra(packedVerb) & 1;
    canvas->clipRRect(read_rrect(reader), op, isAA);
}

static void clipPath_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kClipPath == unpack_verb(packedVerb));
    SkClipOp op = (SkClipOp)(unpack_verb_extra(packedVerb) >> 1);
    bool isAA = unpack_verb_extra(packedVerb) & 1;
    SkPath path;
    reader.readPath(&path);
    canvas->clipPath(path, op, isAA);
}

static void clipRegion_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kClipRegion == unpack_verb(packedVerb));
    SkClipOp op = (SkClipOp)(unpack_verb_extra(packedVerb) >> 1);
    SkRegion region;
    reader.readRegion(&region);
    canvas->clipRegion(region, op);
}

static void drawArc_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawArc == unpack_verb(packedVerb));
    const bool useCenter = (bool)(unpack_verb_extra(packedVerb) & 1);
    const SkScalar* scalars = skip<SkScalar>(reader, 6);    // bounds[0..3], start[4], sweep[5]
    const SkRect* bounds = (const SkRect*)scalars;
    canvas->drawArc(*bounds, scalars[4], scalars[5], useCenter, read_paint(reader));
}

static void drawAtlas_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawAtlas == unpack_verb(packedVerb));
    SkBlendMode mode = (SkBlendMode)(packedVerb & kMode_DrawAtlasMask);
    sk_sp<SkImage> image(reader.readImage());
    int count = reader.read32();
    const SkRSXform* xform = skip<SkRSXform>(reader, count);
    const SkRect* rect = skip<SkRect>(reader, count);
    const SkColor* color = nullptr;
    if (packedVerb & kHasColors_DrawAtlasMask) {
        color = skip<SkColor>(reader, count);
    }
    const SkRect* cull = nullptr;
    if (packedVerb & kHasCull_DrawAtlasMask) {
        cull = skip<SkRect>(reader);
    }
    SkPaint paintStorage, *paint = nullptr;
    if (packedVerb & kHasPaint_DrawAtlasMask) {
        paintStorage = read_paint(reader);
        paint = &paintStorage;
    }
    canvas->drawAtlas(image, xform, rect, color, count, mode, cull, paint);
}

static void drawDRRect_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawDRRect == unpack_verb(packedVerb));
    const SkRRect outer = read_rrect(reader);
    const SkRRect inner = read_rrect(reader);
    canvas->drawDRRect(outer, inner, read_paint(reader));
}

static void drawText_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawText == unpack_verb(packedVerb));
    uint32_t len = unpack_verb_extra(packedVerb);
    if (0 == len) {
        len = reader.read32();
    }
    const void* text = reader.skip(SkAlign4(len));
    SkScalar x = reader.readScalar();
    SkScalar y = reader.readScalar();
    canvas->drawText(text, len, x, y, read_paint(reader));
}

static void drawPosText_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawPosText == unpack_verb(packedVerb));
    uint32_t len = unpack_verb_extra(packedVerb);
    if (0 == len) {
        len = reader.read32();
    }
    const void* text = reader.skip(SkAlign4(len));
    int count = reader.read32();
    const SkPoint* pos = skip<SkPoint>(reader, count);
    SkPaint paint = read_paint(reader);
    SkASSERT(paint.countText(text, len) == count);
    canvas->drawPosText(text, len, pos, paint);
}

static void drawPosTextH_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawPosTextH == unpack_verb(packedVerb));
    uint32_t len = unpack_verb_extra(packedVerb);
    if (0 == len) {
        len = reader.read32();
    }
    const void* text = reader.skip(SkAlign4(len));
    int count = reader.read32();
    const SkScalar* xpos = skip<SkScalar>(reader, count);
    SkScalar constY = reader.readScalar();
    SkPaint paint = read_paint(reader);
    SkASSERT(paint.countText(text, len) == count);
    canvas->drawPosTextH(text, len, xpos, constY, paint);
}

static void drawTextOnPath_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawTextOnPath == unpack_verb(packedVerb));
    uint32_t byteLength = packedVerb & kTextLength_DrawTextOnPathMask;
    SkMatrix::TypeMask tm = (SkMatrix::TypeMask)
            ((packedVerb & kMatrixType_DrawTextOnPathMask) >> kMatrixType_DrawTextOnPathShift);

    if (0 == byteLength) {
        byteLength = reader.read32();
    }
    const void* text = reader.skip(SkAlign4(byteLength));
    SkPath path;
    reader.readPath(&path);
    const SkMatrix* matrix = nullptr;
    SkMatrix matrixStorage;
    if (tm != SkMatrix::kIdentity_Mask) {
        matrixStorage = read_sparse_matrix(reader, tm);
        matrix = &matrixStorage;
    }
    canvas->drawTextOnPath(text, byteLength, path, matrix, read_paint(reader));
}

static void drawTextBlob_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    sk_sp<SkTextBlob> tb = SkTextBlob::MakeFromBuffer(reader);
    SkScalar x = reader.readScalar();
    SkScalar y = reader.readScalar();
    canvas->drawTextBlob(tb, x, y, read_paint(reader));
}

static void drawTextRSXform_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawTextRSXform == unpack_verb(packedVerb));
    uint32_t len = unpack_verb_extra(packedVerb) >> 1;
    if (0 == len) {
        len = reader.read32();
    }
    const void* text = reader.skip(SkAlign4(len));
    int count = reader.read32();
    const SkRSXform* xform = skip<SkRSXform>(reader, count);
    const SkRect* cull = (packedVerb & 1) ? skip<SkRect>(reader) : nullptr;
    SkPaint paint = read_paint(reader);
    SkASSERT(paint.countText(text, len) == count);
    canvas->drawTextRSXform(text, len, xform, cull, paint);
}

static void drawPatch_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawPatch == unpack_verb(packedVerb));
    const SkColor* colors = nullptr;
    const SkPoint* tex = nullptr;
    const SkPoint* cubics = skip<SkPoint>(reader, 12);
    if (packedVerb & kHasColors_DrawPatchExtraMask) {
        colors = skip<SkColor>(reader, 4);
    }
    if (packedVerb & kHasTexture_DrawPatchExtraMask) {
        tex = skip<SkPoint>(reader, 4);
    }
    SkBlendMode mode = (SkBlendMode)(packedVerb & kModeEnum_DrawPatchExtraMask);
    canvas->drawPatch(cubics, colors, tex, mode, read_paint(reader));
}

static void drawPaint_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawPaint == unpack_verb(packedVerb));
    canvas->drawPaint(read_paint(reader));
}

static void drawRect_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawRect == unpack_verb(packedVerb));
    const SkRect* rect = skip<SkRect>(reader);
    canvas->drawRect(*rect, read_paint(reader));
}

static void drawRegion_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawRegion == unpack_verb(packedVerb));
    size_t size = unpack_verb_extra(packedVerb);
    if (0 == size) {
        size = reader.read32();
    }
    SkRegion region;
    region.readFromMemory(skip<char>(reader, SkAlign4(size)), size);
    canvas->drawRegion(region, read_paint(reader));
}

static void drawOval_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawOval == unpack_verb(packedVerb));
    const SkRect* rect = skip<SkRect>(reader);
    canvas->drawOval(*rect, read_paint(reader));
}

static void drawRRect_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawRRect == unpack_verb(packedVerb));
    SkRRect rrect = read_rrect(reader);
    canvas->drawRRect(rrect, read_paint(reader));
}

static void drawPath_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawPath == unpack_verb(packedVerb));
    SkPath path;
    reader.readPath(&path);
    canvas->drawPath(path, read_paint(reader));
}

static void drawPoints_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawPoints == unpack_verb(packedVerb));
    SkCanvas::PointMode mode = (SkCanvas::PointMode)unpack_verb_extra(packedVerb);
    int count = reader.read32();
    const SkPoint* points = skip<SkPoint>(reader, count);
    canvas->drawPoints(mode, count, points, read_paint(reader));
}

static void drawImage_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawImage == unpack_verb(packedVerb));
    sk_sp<SkImage> image(reader.readImage());
    SkScalar x = reader.readScalar();
    SkScalar y = reader.readScalar();
    SkPaint paintStorage, *paint = nullptr;
    if (packedVerb & kHasPaint_DrawImageMask) {
        paintStorage = read_paint(reader);
        paint = &paintStorage;
    }
    canvas->drawImage(image, x, y, paint);
}

static void drawImageRect_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawImageRect == unpack_verb(packedVerb));
    sk_sp<SkImage> image(reader.readImage());
    SkCanvas::SrcRectConstraint constraint =
            (SkCanvas::SrcRectConstraint)(packedVerb & kConstraint_DrawImageRectMask);
    const SkRect* src = (packedVerb & kHasSrcRect_DrawImageRectMask) ?
                        skip<SkRect>(reader) : nullptr;
    const SkRect* dst = skip<SkRect>(reader);
    SkPaint paintStorage, *paint = nullptr;
    if (packedVerb & kHasPaint_DrawImageRectMask) {
        paintStorage = read_paint(reader);
        paint = &paintStorage;
    }
    if (src) {
        canvas->drawImageRect(image, *src, *dst, paint, constraint);
    } else {
        canvas->drawImageRect(image, *dst, paint);
    }
}

static void drawImageNine_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawImageNine == unpack_verb(packedVerb));
    sk_sp<SkImage> image(reader.readImage());
    const SkIRect* center = skip<SkIRect>(reader);
    const SkRect* dst = skip<SkRect>(reader);
    SkPaint paintStorage, *paint = nullptr;
    if (packedVerb & kHasPaint_DrawImageNineMask) {
        paintStorage = read_paint(reader);
        paint = &paintStorage;
    }
    canvas->drawImageNine(image, *center, *dst, paint);
}

static void drawImageLattice_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawImageLattice == unpack_verb(packedVerb));
    sk_sp<SkImage> image(reader.readImage());

    SkCanvas::Lattice lattice;
    lattice.fXCount = (packedVerb >> kXCount_DrawImageLatticeShift) & kCount_DrawImageLatticeMask;
    if (lattice.fXCount == kCount_DrawImageLatticeMask) {
        lattice.fXCount = reader.read32();
    }
    lattice.fYCount = (packedVerb >> kXCount_DrawImageLatticeShift) & kCount_DrawImageLatticeMask;
    if (lattice.fYCount == kCount_DrawImageLatticeMask) {
        lattice.fYCount = reader.read32();
    }
    lattice.fXDivs = skip<int32_t>(reader, lattice.fXCount);
    lattice.fYDivs = skip<int32_t>(reader, lattice.fYCount);
    if (packedVerb & kHasFlags_DrawImageLatticeMask) {
        int32_t count = (lattice.fXCount + 1) * (lattice.fYCount + 1);
        SkASSERT(count > 0);
        lattice.fFlags = skip<SkCanvas::Lattice::Flags>(reader, SkAlign4(count));
    } else {
        lattice.fFlags = nullptr;
    }
    lattice.fBounds = skip<SkIRect>(reader);
    const SkRect* dst = skip<SkRect>(reader);

    SkPaint paintStorage, *paint = nullptr;
    if (packedVerb & kHasPaint_DrawImageLatticeMask) {
        paintStorage = read_paint(reader);
        paint = &paintStorage;
    }
    canvas->drawImageLattice(image.get(), lattice, *dst, paint);
}

static void drawVertices_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawVertices == unpack_verb(packedVerb));
    SkBlendMode bmode = (SkBlendMode)unpack_verb_extra(packedVerb);
    sk_sp<SkData> data = reader.readByteArrayAsData();
    canvas->drawVertices(SkVertices::Decode(data->data(), data->size()), bmode, read_paint(reader));
}

static void drawPicture_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawPicture == unpack_verb(packedVerb));
    unsigned extra = unpack_verb_extra(packedVerb);
    int index = extra & kIndex_ObjectDefinitionMask;
    SkPicture* pic = reader.getInflator()->getPicture(index);
    SkMatrix matrixStorage, *matrix = nullptr;
    SkPaint paintStorage, *paint = nullptr;
    if (extra & kHasMatrix_DrawPictureExtra) {
        reader.readMatrix(&matrixStorage);
        matrix = &matrixStorage;
    }
    if (extra & kHasPaint_DrawPictureExtra) {
        paintStorage = read_paint(reader);
        paint = &paintStorage;
    }
    canvas->drawPicture(pic, matrix, paint);
}

static void drawAnnotation_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDrawAnnotation == unpack_verb(packedVerb));
    const SkRect* rect = skip<SkRect>(reader);

    // len includes the key's trailing 0
    uint32_t len = unpack_verb_extra(packedVerb) >> 1;
    if (0 == len) {
        len = reader.read32();
    }
    const char* key = skip<char>(reader, len);
    sk_sp<SkData> data;
    if (packedVerb & 1) {
        uint32_t size = reader.read32();
        data = SkData::MakeWithCopy(reader.skip(SkAlign4(size)), size);
    }
    canvas->drawAnnotation(*rect, key, data);
}

#if 0
        stream.write("skiacodc", 8);
        stream.write32(pmap.width());
        stream.write32(pmap.height());
        stream.write16(pmap.colorType());
        stream.write16(pmap.alphaType());
        stream.write32(0);  // no colorspace for now
        for (int y = 0; y < pmap.height(); ++y) {
            stream.write(pmap.addr8(0, y), pmap.width());
        }
#endif

static sk_sp<SkImage> make_from_skiaimageformat(const void* encoded, size_t encodedSize) {
    if (encodedSize < 24) {
        return nullptr;
    }

    SkMemoryStream stream(encoded, encodedSize);
    char signature[8];
    stream.read(signature, 8);
    if (memcmp(signature, "skiaimgf", 8)) {
        return nullptr;
    }

    int width = stream.readU32();
    int height = stream.readU32();
    SkColorType ct = (SkColorType)stream.readU16();
    SkAlphaType at = (SkAlphaType)stream.readU16();
    SkASSERT(kAlpha_8_SkColorType == ct);

    SkDEBUGCODE(size_t colorSpaceSize =) stream.readU32();
    SkASSERT(0 == colorSpaceSize);

    SkImageInfo info = SkImageInfo::Make(width, height, ct, at);
    size_t size = width * height;
    sk_sp<SkData> pixels = SkData::MakeUninitialized(size);
    stream.read(pixels->writable_data(), size);
    SkASSERT(encodedSize == SkAlign4(stream.getPosition()));
    return SkImage::MakeRasterData(info, pixels, width);
}

sk_sp<SkImage> SkPipeInflator::makeImage(const sk_sp<SkData>& data) {
    if (fIMDeserializer) {
        return fIMDeserializer->makeFromData(data.get(), nullptr);
    }
    sk_sp<SkImage> image = make_from_skiaimageformat(data->data(), data->size());
    if (!image) {
        image = SkImage::MakeFromEncoded(data);
    }
    return image;
}


static void defineImage_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas*) {
    SkASSERT(SkPipeVerb::kDefineImage == unpack_verb(packedVerb));
    SkPipeInflator* inflator = (SkPipeInflator*)reader.getInflator();
    uint32_t extra = unpack_verb_extra(packedVerb);
    int index = extra & kIndex_ObjectDefinitionMask;

    if (extra & kUndef_ObjectDefinitionMask) {
        // zero-index means we are "forgetting" that cache entry
        inflator->setImage(index, nullptr);
    } else {
        // we are defining a new image
        sk_sp<SkData> data = reader.readByteArrayAsData();
        sk_sp<SkImage> image = inflator->makeImage(data);
        if (!image) {
            SkDebugf("-- failed to decode\n");
        }
        inflator->setImage(index, image.get());
    }
}

sk_sp<SkTypeface> SkPipeInflator::makeTypeface(const void* data, size_t size) {
    if (fTFDeserializer) {
        return fTFDeserializer->deserialize(data, size);
    }
    SkMemoryStream stream(data, size, false);
    return SkTypeface::MakeDeserialize(&stream);
}

static void defineTypeface_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDefineTypeface == unpack_verb(packedVerb));
    SkPipeInflator* inflator = (SkPipeInflator*)reader.getInflator();
    uint32_t extra = unpack_verb_extra(packedVerb);
    int index = extra & kIndex_ObjectDefinitionMask;

    if (extra & kUndef_ObjectDefinitionMask) {
        // zero-index means we are "forgetting" that cache entry
        inflator->setTypeface(index, nullptr);
    } else {
        // we are defining a new image
        sk_sp<SkData> data = reader.readByteArrayAsData();
        // TODO: seems like we could "peek" to see the array, and not need to copy it.
        sk_sp<SkTypeface> tf = inflator->makeTypeface(data->data(), data->size());
        inflator->setTypeface(index, tf.get());
    }
}

static void defineFactory_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDefineFactory == unpack_verb(packedVerb));
    SkPipeInflator* inflator = (SkPipeInflator*)reader.getInflator();
    uint32_t extra = unpack_verb_extra(packedVerb);
    int index = extra >> kNameLength_DefineFactoryExtraBits;
    size_t len = extra & kNameLength_DefineFactoryExtraMask;
    // +1 for the trailing null char
    const char* name = (const char*)reader.skip(SkAlign4(len + 1));
    SkFlattenable::Factory factory = reader.findFactory(name);
    if (factory) {
        inflator->setFactory(index, factory);
    }
}

static void definePicture_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    SkASSERT(SkPipeVerb::kDefinePicture == unpack_verb(packedVerb));
    int deleteIndex = unpack_verb_extra(packedVerb);

    SkPipeInflator* inflator = (SkPipeInflator*)reader.getInflator();

    if (deleteIndex) {
        inflator->setPicture(deleteIndex - 1, nullptr);
    } else {
        SkPictureRecorder recorder;
        int pictureIndex = -1;  // invalid
        const SkRect* cull = skip<SkRect>(reader);
        do_playback(reader, recorder.beginRecording(*cull), &pictureIndex);
        SkASSERT(pictureIndex > 0);
        sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
        inflator->setPicture(pictureIndex, picture.get());
    }
}

static void endPicture_handler(SkPipeReader& reader, uint32_t packedVerb, SkCanvas* canvas) {
    sk_throw();     // never call me
}

///////////////////////////////////////////////////////////////////////////////////////////////////

struct HandlerRec {
    SkPipeHandler   fProc;
    const char*     fName;
};

#define HANDLER(name)   { name##_handler, #name }
const HandlerRec gPipeHandlers[] = {
    HANDLER(save),
    HANDLER(saveLayer),
    HANDLER(restore),
    HANDLER(concat),
    
    HANDLER(clipRect),
    HANDLER(clipRRect),
    HANDLER(clipPath),
    HANDLER(clipRegion),
    
    HANDLER(drawArc),
    HANDLER(drawAtlas),
    HANDLER(drawDRRect),
    HANDLER(drawText),
    HANDLER(drawPosText),
    HANDLER(drawPosTextH),
    HANDLER(drawRegion),
    HANDLER(drawTextOnPath),
    HANDLER(drawTextBlob),
    HANDLER(drawTextRSXform),
    HANDLER(drawPatch),
    HANDLER(drawPaint),
    HANDLER(drawPoints),
    HANDLER(drawRect),
    HANDLER(drawPath),
    HANDLER(drawOval),
    HANDLER(drawRRect),
    
    HANDLER(drawImage),
    HANDLER(drawImageRect),
    HANDLER(drawImageNine),
    HANDLER(drawImageLattice),
    
    HANDLER(drawVertices),
    
    HANDLER(drawPicture),
    HANDLER(drawAnnotation),

    HANDLER(defineImage),
    HANDLER(defineTypeface),
    HANDLER(defineFactory),
    HANDLER(definePicture),
    HANDLER(endPicture),        // handled special -- should never be called
};
#undef HANDLER

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkPipeDeserializer::Impl {
public:
    SkRefSet<SkImage>                   fImages;
    SkRefSet<SkPicture>                 fPictures;
    SkRefSet<SkTypeface>                fTypefaces;
    SkTDArray<SkFlattenable::Factory>   fFactories;

    SkTypefaceDeserializer*             fTFDeserializer = nullptr;
    SkImageDeserializer*                fIMDeserializer = nullptr;
};

SkPipeDeserializer::SkPipeDeserializer() : fImpl(new Impl) {}
SkPipeDeserializer::~SkPipeDeserializer() {}

void SkPipeDeserializer::setTypefaceDeserializer(SkTypefaceDeserializer* tfd) {
    fImpl->fTFDeserializer = tfd;
}

void SkPipeDeserializer::setImageDeserializer(SkImageDeserializer* imd) {
    fImpl->fIMDeserializer = imd;
}

sk_sp<SkImage> SkPipeDeserializer::readImage(const void* data, size_t size) {
    if (size < sizeof(uint32_t)) {
        SkDebugf("-------- data length too short for readImage %d\n", size);
        return nullptr;
    }

    const uint32_t* ptr = (const uint32_t*)data;
    uint32_t packedVerb = *ptr++;
    size -= 4;

    if (SkPipeVerb::kDefineImage == unpack_verb(packedVerb)) {
        SkPipeInflator inflator(&fImpl->fImages, &fImpl->fPictures,
                                &fImpl->fTypefaces, &fImpl->fFactories,
                                fImpl->fTFDeserializer, fImpl->fIMDeserializer);
        SkPipeReader reader(this, ptr, size);
        reader.setInflator(&inflator);
        defineImage_handler(reader, packedVerb, nullptr);
        packedVerb = reader.read32();  // read the next verb
    }
    if (SkPipeVerb::kWriteImage != unpack_verb(packedVerb)) {
        SkDebugf("-------- unexpected verb for readImage %d\n", unpack_verb(packedVerb));
        return nullptr;
    }
    int index = unpack_verb_extra(packedVerb);
    if (0 == index) {
        return nullptr; // writer failed
    }
    return sk_ref_sp(fImpl->fImages.get(index - 1));
}

sk_sp<SkPicture> SkPipeDeserializer::readPicture(const void* data, size_t size) {
    if (size < sizeof(uint32_t)) {
        SkDebugf("-------- data length too short for readPicture %d\n", size);
        return nullptr;
    }

    const uint32_t* ptr = (const uint32_t*)data;
    uint32_t packedVerb = *ptr++;
    size -= 4;

    if (SkPipeVerb::kDefinePicture == unpack_verb(packedVerb)) {
        SkPipeInflator inflator(&fImpl->fImages, &fImpl->fPictures,
                                &fImpl->fTypefaces, &fImpl->fFactories,
                                fImpl->fTFDeserializer, fImpl->fIMDeserializer);
        SkPipeReader reader(this, ptr, size);
        reader.setInflator(&inflator);
        definePicture_handler(reader, packedVerb, nullptr);
        packedVerb = reader.read32();  // read the next verb
    }
    if (SkPipeVerb::kWritePicture != unpack_verb(packedVerb)) {
        SkDebugf("-------- unexpected verb for readPicture %d\n", unpack_verb(packedVerb));
        return nullptr;
    }
    int index = unpack_verb_extra(packedVerb);
    if (0 == index) {
        return nullptr; // writer failed
    }
    return sk_ref_sp(fImpl->fPictures.get(index - 1));
}

static bool do_playback(SkPipeReader& reader, SkCanvas* canvas, int* endPictureIndex) {
    int indent = 0;

    const bool showEachVerb = false;
    int counter = 0;
    while (!reader.eof()) {
        uint32_t prevOffset = reader.offset();
        uint32_t packedVerb = reader.read32();
        SkPipeVerb verb = unpack_verb(packedVerb);
        if ((unsigned)verb >= SK_ARRAY_COUNT(gPipeHandlers)) {
            SkDebugf("------- bad verb %d\n", verb);
            return false;
        }
        if (SkPipeVerb::kRestore == verb) {
            indent -= 1;
            SkASSERT(indent >= 0);
        }

        if (SkPipeVerb::kEndPicture == verb) {
            if (endPictureIndex) {
                *endPictureIndex = unpack_verb_extra(packedVerb);
            }
            return true;
        }
        HandlerRec rec = gPipeHandlers[(unsigned)verb];
        rec.fProc(reader, packedVerb, canvas);
        if (showEachVerb) {
            for (int i = 0; i < indent; ++i) {
                SkDebugf("    ");
            }
            SkDebugf("%d [%d] %s %d\n", prevOffset, counter++, rec.fName, reader.offset() - prevOffset);
        }
        if (!reader.isValid()) {
            SkDebugf("-------- bad reader\n");
            return false;
        }

        switch (verb) {
            case SkPipeVerb::kSave:
            case SkPipeVerb::kSaveLayer:
                indent += 1;
                break;
            default:
                break;
        }
    }
    return true;
}

bool SkPipeDeserializer::playback(const void* data, size_t size, SkCanvas* canvas) {
    SkPipeInflator inflator(&fImpl->fImages, &fImpl->fPictures,
                            &fImpl->fTypefaces, &fImpl->fFactories,
                            fImpl->fTFDeserializer, fImpl->fIMDeserializer);
    SkPipeReader reader(this, data, size);
    reader.setInflator(&inflator);
    return do_playback(reader, canvas);
}

