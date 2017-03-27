/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkPathEffect.h"
#include "SkPipeCanvas.h"
#include "SkPipeFormat.h"
#include "SkRSXform.h"
#include "SkRasterizer.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

template <typename T> void write_rrect(T* writer, const SkRRect& rrect) {
    char tmp[SkRRect::kSizeInMemory];
    rrect.writeToMemory(tmp);
    writer->write(tmp, SkRRect::kSizeInMemory);
}

template <typename T> void write_pad(T* writer, const void* buffer, size_t len) {
    writer->write(buffer, len & ~3);
    if (len & 3) {
        const char* src = (const char*)buffer + (len & ~3);
        len &= 3;
        uint32_t tmp = 0;
        memcpy(&tmp, src, len);
        writer->write(&tmp, 4);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static uint16_t compute_nondef(const SkPaint& paint, PaintUsage usage) {
    // kRespectsStroke_PaintUsage is only valid if other bits are also set
    SkASSERT(0 != (usage & ~kRespectsStroke_PaintUsage));

    const SkScalar kTextSize_Default    = 12;
    const SkScalar kTextScaleX_Default  = 1;
    const SkScalar kTextSkewX_Default   = 0;
    const SkScalar kStrokeWidth_Default = 0;
    const SkScalar kStrokeMiter_Default = 4;
    const SkColor  kColor_Default       = SK_ColorBLACK;

    unsigned bits = (paint.getColor() != kColor_Default) ? kColor_NonDef : 0;

    if (usage & kText_PaintUsage) {
        bits |= (paint.getTextSize() != kTextSize_Default       ? kTextSize_NonDef : 0);
        bits |= (paint.getTextScaleX() != kTextScaleX_Default   ? kTextScaleX_NonDef : 0);
        bits |= (paint.getTextSkewX() != kTextSkewX_Default     ? kTextSkewX_NonDef : 0);
        bits |= (paint.getTypeface()                            ? kTypeface_NonDef : 0);
    }

    // TODO: kImage_PaintUsage only needs the shader/maskfilter IF its colortype is kAlpha_8

    if (usage & (kVertices_PaintUsage | kDrawPaint_PaintUsage | kImage_PaintUsage |
                 kText_PaintUsage | kGeometry_PaintUsage | kTextBlob_PaintUsage)) {
        bits |= (paint.getShader()      ? kShader_NonDef : 0);
    }

    if (usage & (kText_PaintUsage | kGeometry_PaintUsage | kTextBlob_PaintUsage)) {
        bits |= (paint.getPathEffect()  ? kPathEffect_NonDef : 0);
        bits |= (paint.getRasterizer()  ? kRasterizer_NonDef : 0);

        if (paint.getStyle() != SkPaint::kFill_Style || (usage & kRespectsStroke_PaintUsage)) {
            bits |= (paint.getStrokeWidth() != kStrokeWidth_Default ? kStrokeWidth_NonDef : 0);
            bits |= (paint.getStrokeMiter() != kStrokeMiter_Default ? kStrokeMiter_NonDef : 0);
        }
    }

    if (usage &
        (kText_PaintUsage | kGeometry_PaintUsage | kImage_PaintUsage | kTextBlob_PaintUsage))
    {
        bits |= (paint.getMaskFilter()  ? kMaskFilter_NonDef : 0);
    }

    bits |= (paint.getColorFilter() ? kColorFilter_NonDef : 0);
    bits |= (paint.getImageFilter() ? kImageFilter_NonDef : 0);
    bits |= (paint.getDrawLooper()  ? kDrawLooper_NonDef : 0);

    return SkToU16(bits);
}

static uint32_t pack_paint_flags(unsigned flags, unsigned hint, unsigned align,
                                 unsigned filter, unsigned style, unsigned caps, unsigned joins,
                                 unsigned encoding) {
    SkASSERT(kFlags_BPF + kHint_BPF + kAlign_BPF + kFilter_BPF <= 32);

    ASSERT_FITS_IN(flags, kFlags_BPF);
    ASSERT_FITS_IN(filter, kFilter_BPF);
    ASSERT_FITS_IN(style, kStyle_BPF);
    ASSERT_FITS_IN(caps, kCaps_BPF);
    ASSERT_FITS_IN(joins, kJoins_BPF);
    ASSERT_FITS_IN(hint, kHint_BPF);
    ASSERT_FITS_IN(align, kAlign_BPF);
    ASSERT_FITS_IN(encoding, kEncoding_BPF);

    // left-align the fields of "known" size, and right-align the last (flatFlags) so it can easly
    // add more bits in the future.

    uint32_t packed = 0;
    int shift = 32;

    shift -= kFlags_BPF;    packed |= (flags << shift);
    shift -= kFilter_BPF;   packed |= (filter << shift);
    shift -= kStyle_BPF;    packed |= (style << shift);
    // these are only needed for stroking (geometry or text)
    shift -= kCaps_BPF;     packed |= (caps << shift);
    shift -= kJoins_BPF;    packed |= (joins << shift);
    // these are only needed for text
    shift -= kHint_BPF;     packed |= (hint << shift);
    shift -= kAlign_BPF;    packed |= (align << shift);
    shift -= kEncoding_BPF; packed |= (encoding << shift);

    return packed;
}

#define CHECK_WRITE_SCALAR(writer, nondef, paint, Field)    \
    do { if (nondef & (k##Field##_NonDef)) {                \
        writer.writeScalar(paint.get##Field());             \
    }} while (0)

#define CHECK_WRITE_FLATTENABLE(writer, nondef, paint, Field)   \
    do { if (nondef & (k##Field##_NonDef)) {                    \
        SkFlattenable* f = paint.get##Field();                  \
        SkASSERT(f != nullptr);                                 \
        writer.writeFlattenable(f);                             \
    } } while (0)

/*
 *  Header:
 *      paint flags     : 32
 *      non_def bits    : 16
 *      xfermode enum   : 8
 *      pad zeros       : 8
 */
static void write_paint(SkWriteBuffer& writer, const SkPaint& paint, unsigned usage) {
    uint32_t packedFlags = pack_paint_flags(paint.getFlags(), paint.getHinting(),
                                            paint.getTextAlign(), paint.getFilterQuality(),
                                            paint.getStyle(), paint.getStrokeCap(),
                                            paint.getStrokeJoin(), paint.getTextEncoding());
    writer.write32(packedFlags);

    unsigned nondef = compute_nondef(paint, (PaintUsage)usage);
    const uint8_t pad = 0;
    writer.write32((nondef << 16) | ((unsigned)paint.getBlendMode() << 8) | pad);

    CHECK_WRITE_SCALAR(writer, nondef, paint, TextSize);
    CHECK_WRITE_SCALAR(writer, nondef, paint, TextScaleX);
    CHECK_WRITE_SCALAR(writer, nondef, paint, TextSkewX);
    CHECK_WRITE_SCALAR(writer, nondef, paint, StrokeWidth);
    CHECK_WRITE_SCALAR(writer, nondef, paint, StrokeMiter);

    if (nondef & kColor_NonDef) {
        writer.write32(paint.getColor());
    }
    if (nondef & kTypeface_NonDef) {
        // TODO: explore idea of writing bits indicating "use the prev (or prev N) face"
        // e.g. 1-N bits is an index into a ring buffer of typefaces
        SkTypeface* tf = paint.getTypeface();
        SkASSERT(tf);
        writer.writeTypeface(tf);
    }

    CHECK_WRITE_FLATTENABLE(writer, nondef, paint, PathEffect);
    CHECK_WRITE_FLATTENABLE(writer, nondef, paint, Shader);
    CHECK_WRITE_FLATTENABLE(writer, nondef, paint, MaskFilter);
    CHECK_WRITE_FLATTENABLE(writer, nondef, paint, ColorFilter);
    CHECK_WRITE_FLATTENABLE(writer, nondef, paint, Rasterizer);
    CHECK_WRITE_FLATTENABLE(writer, nondef, paint, ImageFilter);
    CHECK_WRITE_FLATTENABLE(writer, nondef, paint, DrawLooper);
}

class SkPipeWriter : public SkBinaryWriteBuffer {
    enum {
        N = 1024/4,
    };
    uint32_t fStorage[N];
    SkWStream* fStream;

public:
    SkPipeWriter(SkWStream* stream, SkDeduper* deduper)
        : SkBinaryWriteBuffer(fStorage, sizeof(fStorage))
        , fStream(stream)
    {
        this->setDeduper(deduper);
    }

    SkPipeWriter(SkPipeCanvas* pc) : SkPipeWriter(pc->fStream, pc->fDeduper) {}

    ~SkPipeWriter() override {
        SkASSERT(SkIsAlign4(fStream->bytesWritten()));
        this->writeToStream(fStream);
    }

    void writePaint(const SkPaint& paint) override {
        write_paint(*this, paint, kUnknown_PaintUsage);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

SkPipeCanvas::SkPipeCanvas(const SkRect& cull, SkPipeDeduper* deduper, SkWStream* stream)
    : INHERITED(cull.roundOut())
    , fDeduper(deduper)
    , fStream(stream)
{}

SkPipeCanvas::~SkPipeCanvas() {}

void SkPipeCanvas::willSave() {
    fStream->write32(pack_verb(SkPipeVerb::kSave));
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkPipeCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    SkPipeWriter writer(this);
    uint32_t extra = rec.fSaveLayerFlags;

    // remap this wacky flag
    if (extra & (1 << 31)/*SkCanvas::kDontClipToLayer_PrivateSaveLayerFlag*/) {
        extra &= ~(1 << 31);
        extra |= kDontClipToLayer_SaveLayerMask;
    }

    if (rec.fBounds) {
        extra |= kHasBounds_SaveLayerMask;
    }
    if (rec.fPaint) {
        extra |= kHasPaint_SaveLayerMask;
    }
    if (rec.fBackdrop) {
        extra |= kHasBackdrop_SaveLayerMask;
    }

    writer.write32(pack_verb(SkPipeVerb::kSaveLayer, extra));
    if (rec.fBounds) {
        writer.writeRect(*rec.fBounds);
    }
    if (rec.fPaint) {
        write_paint(writer, *rec.fPaint, kSaveLayer_PaintUsage);
    }
    if (rec.fBackdrop) {
        writer.writeFlattenable(rec.fBackdrop);
    }
    return kNoLayer_SaveLayerStrategy;
}

void SkPipeCanvas::willRestore() {
    fStream->write32(pack_verb(SkPipeVerb::kRestore));
    this->INHERITED::willRestore();
}

template <typename T> void write_sparse_matrix(T* writer, const SkMatrix& matrix) {
    SkMatrix::TypeMask tm = matrix.getType();
    SkScalar tmp[9];
    if (tm & SkMatrix::kPerspective_Mask) {
        matrix.get9(tmp);
        writer->write(tmp, 9 * sizeof(SkScalar));
    } else if (tm & SkMatrix::kAffine_Mask) {
        tmp[0] = matrix[SkMatrix::kMScaleX];
        tmp[1] = matrix[SkMatrix::kMSkewX];
        tmp[2] = matrix[SkMatrix::kMTransX];
        tmp[3] = matrix[SkMatrix::kMScaleY];
        tmp[4] = matrix[SkMatrix::kMSkewY];
        tmp[5] = matrix[SkMatrix::kMTransY];
        writer->write(tmp, 6 * sizeof(SkScalar));
    } else if (tm & SkMatrix::kScale_Mask) {
        tmp[0] = matrix[SkMatrix::kMScaleX];
        tmp[1] = matrix[SkMatrix::kMTransX];
        tmp[2] = matrix[SkMatrix::kMScaleY];
        tmp[3] = matrix[SkMatrix::kMTransY];
        writer->write(tmp, 4 * sizeof(SkScalar));
    } else if (tm & SkMatrix::kTranslate_Mask) {
        tmp[0] = matrix[SkMatrix::kMTransX];
        tmp[1] = matrix[SkMatrix::kMTransY];
        writer->write(tmp, 2 * sizeof(SkScalar));
    }
    // else write nothing for Identity
}

static void do_concat(SkWStream* stream, const SkMatrix& matrix, bool isSetMatrix) {
    unsigned mtype = matrix.getType();
    SkASSERT(0 == (mtype & ~kTypeMask_ConcatMask));
    unsigned extra = mtype;
    if (isSetMatrix) {
        extra |= kSetMatrix_ConcatMask;
    }
    if (mtype || isSetMatrix) {
        stream->write32(pack_verb(SkPipeVerb::kConcat, extra));
        write_sparse_matrix(stream, matrix);
    }
}

void SkPipeCanvas::didConcat(const SkMatrix& matrix) {
    do_concat(fStream, matrix, false);
    this->INHERITED::didConcat(matrix);
}

void SkPipeCanvas::didSetMatrix(const SkMatrix& matrix) {
    do_concat(fStream, matrix, true);
    this->INHERITED::didSetMatrix(matrix);
}

void SkPipeCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    fStream->write32(pack_verb(SkPipeVerb::kClipRect, ((unsigned)op << 1) | edgeStyle));
    fStream->write(&rect, 4 * sizeof(SkScalar));

    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

void SkPipeCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    fStream->write32(pack_verb(SkPipeVerb::kClipRRect, ((unsigned)op << 1) | edgeStyle));
    write_rrect(fStream, rrect);

    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

void SkPipeCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kClipPath, ((unsigned)op << 1) | edgeStyle));
    writer.writePath(path);

    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void SkPipeCanvas::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kClipRegion, (unsigned)op << 1));
    writer.writeRegion(deviceRgn);

    this->INHERITED::onClipRegion(deviceRgn, op);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkPipeCanvas::onDrawArc(const SkRect& bounds, SkScalar startAngle, SkScalar sweepAngle,
                             bool useCenter, const SkPaint& paint) {
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawArc, (int)useCenter));
    writer.writeRect(bounds);
    writer.writeScalar(startAngle);
    writer.writeScalar(sweepAngle);
    write_paint(writer, paint, kGeometry_PaintUsage);
}

void SkPipeCanvas::onDrawAtlas(const SkImage* image, const SkRSXform xform[], const SkRect rect[],
                               const SkColor colors[], int count, SkBlendMode mode,
                               const SkRect* cull, const SkPaint* paint) {
    unsigned extra = (unsigned)mode;
    SkASSERT(0 == (extra & ~kMode_DrawAtlasMask));
    if (colors) {
        extra |= kHasColors_DrawAtlasMask;
    }
    if (cull) {
        extra |= kHasCull_DrawAtlasMask;
    }
    if (paint) {
        extra |= kHasPaint_DrawAtlasMask;
    }

    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawAtlas, extra));
    writer.writeImage(image);
    writer.write32(count);
    writer.write(xform, count * sizeof(SkRSXform));
    writer.write(rect, count * sizeof(SkRect));
    if (colors) {
        writer.write(colors, count * sizeof(SkColor));
    }
    if (cull) {
        writer.writeRect(*cull);
    }
    if (paint) {
        write_paint(writer, *paint, kImage_PaintUsage);
    }
}

void SkPipeCanvas::onDrawPaint(const SkPaint& paint) {
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawPaint));
    write_paint(writer, paint, kDrawPaint_PaintUsage);
}

void SkPipeCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                const SkPaint& paint) {
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawPoints, mode));
    writer.write32(SkToU32(count));
    writer.write(pts, count * sizeof(SkPoint));
    write_paint(writer, paint, kGeometry_PaintUsage | kRespectsStroke_PaintUsage);
}

void SkPipeCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawRect));
    writer.write(&rect, sizeof(SkRect));
    write_paint(writer, paint, kGeometry_PaintUsage);
}

void SkPipeCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawOval));
    writer.write(&rect, sizeof(SkRect));
    write_paint(writer, paint, kGeometry_PaintUsage);
}

void SkPipeCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawRRect));
    write_rrect(&writer, rrect);
    write_paint(writer, paint, kGeometry_PaintUsage);
}

void SkPipeCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawDRRect));
    write_rrect(&writer, outer);
    write_rrect(&writer, inner);
    write_paint(writer, paint, kGeometry_PaintUsage);
}

void SkPipeCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawPath));
    writer.writePath(path);
    write_paint(writer, paint, kGeometry_PaintUsage);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> make_from_bitmap(const SkBitmap& bitmap) {
    // If we just "make" an image, it will force a CPU copy (if its mutable), only to have
    // us then either find it in our cache, or compress and send it.
    //
    // Better could be to look it up in our cache first, and only create/compress it if we have to.
    //
    // But for now, just do the dumb thing...
    return SkImage::MakeFromBitmap(bitmap);
}

void SkPipeCanvas::onDrawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                                const SkPaint* paint) {
    sk_sp<SkImage> image = make_from_bitmap(bitmap);
    if (image) {
        this->onDrawImage(image.get(), x, y, paint);
    }
}

void SkPipeCanvas::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                                    const SkPaint* paint, SrcRectConstraint constraint) {
    sk_sp<SkImage> image = make_from_bitmap(bitmap);
    if (image) {
        this->onDrawImageRect(image.get(), src, dst, paint, constraint);
    }
}

void SkPipeCanvas::onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                    const SkRect& dst, const SkPaint* paint) {
    sk_sp<SkImage> image = make_from_bitmap(bitmap);
    if (image) {
        this->onDrawImageNine(image.get(), center, dst, paint);
    }
}

void SkPipeCanvas::onDrawBitmapLattice(const SkBitmap& bitmap, const Lattice& lattice,
                                       const SkRect& dst, const SkPaint* paint) {
    sk_sp<SkImage> image = make_from_bitmap(bitmap);
    if (image) {
        this->onDrawImageLattice(image.get(), lattice, dst, paint);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkPipeCanvas::onDrawImage(const SkImage* image, SkScalar left, SkScalar top,
                               const SkPaint* paint) {
    unsigned extra = 0;
    if (paint) {
        extra |= kHasPaint_DrawImageMask;
    }
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawImage, extra));
    writer.writeImage(image);
    writer.writeScalar(left);
    writer.writeScalar(top);
    if (paint) {
        write_paint(writer, *paint, kImage_PaintUsage);
    }
}

void SkPipeCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                   const SkPaint* paint, SrcRectConstraint constraint) {
    SkASSERT(0 == ((unsigned)constraint & ~1));
    unsigned extra = (unsigned)constraint;
    if (paint) {
        extra |= kHasPaint_DrawImageRectMask;
    }
    if (src) {
        extra |= kHasSrcRect_DrawImageRectMask;
    }

    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawImageRect, extra));
    writer.writeImage(image);
    if (src) {
        writer.write(src, sizeof(*src));
    }
    writer.write(&dst, sizeof(dst));
    if (paint) {
        write_paint(writer, *paint, kImage_PaintUsage);
    }
}

void SkPipeCanvas::onDrawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                                   const SkPaint* paint) {
    unsigned extra = 0;
    if (paint) {
        extra |= kHasPaint_DrawImageNineMask;
    }
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawImageNine, extra));
    writer.writeImage(image);
    writer.write(&center, sizeof(center));
    writer.write(&dst, sizeof(dst));
    if (paint) {
        write_paint(writer, *paint, kImage_PaintUsage);
    }
}

void SkPipeCanvas::onDrawImageLattice(const SkImage* image, const Lattice& lattice,
                                      const SkRect& dst, const SkPaint* paint) {
    unsigned extra = 0;
    if (paint) {
        extra |= kHasPaint_DrawImageLatticeMask;
    }
    if (lattice.fFlags) {
        extra |= kHasFlags_DrawImageLatticeMask;
    }
    if (lattice.fXCount >= kCount_DrawImageLatticeMask) {
        extra |= kCount_DrawImageLatticeMask << kXCount_DrawImageLatticeShift;
    } else {
        extra |= lattice.fXCount << kXCount_DrawImageLatticeShift;
    }
    if (lattice.fYCount >= kCount_DrawImageLatticeMask) {
        extra |= kCount_DrawImageLatticeMask << kYCount_DrawImageLatticeShift;
    } else {
        extra |= lattice.fYCount << kYCount_DrawImageLatticeShift;
    }

    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawImageLattice, extra));
    writer.writeImage(image);
    if (lattice.fXCount >= kCount_DrawImageLatticeMask) {
        writer.write32(lattice.fXCount);
    }
    if (lattice.fYCount >= kCount_DrawImageLatticeMask) {
        writer.write32(lattice.fYCount);
    }
    // Often these divs will be small (8 or 16 bits). Consider sniffing that and writing a flag
    // so we can store them smaller.
    writer.write(lattice.fXDivs, lattice.fXCount * sizeof(int32_t));
    writer.write(lattice.fYDivs, lattice.fYCount * sizeof(int32_t));
    if (lattice.fFlags) {
        int32_t count = (lattice.fXCount + 1) * (lattice.fYCount + 1);
        SkASSERT(count > 0);
        write_pad(&writer, lattice.fFlags, count);
    }
    SkASSERT(lattice.fBounds);
    writer.write(&lattice.fBounds, sizeof(*lattice.fBounds));
    writer.write(&dst, sizeof(dst));
    if (paint) {
        write_paint(writer, *paint, kImage_PaintUsage);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkPipeCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                              const SkPaint& paint) {
    SkASSERT(byteLength);

    bool compact = fits_in(byteLength, 24);

    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawText, compact ? (unsigned)byteLength : 0));
    if (!compact) {
        writer.write32(SkToU32(byteLength));
    }
    write_pad(&writer, text, byteLength);
    writer.writeScalar(x);
    writer.writeScalar(y);
    write_paint(writer, paint, kText_PaintUsage);
}

void SkPipeCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                 const SkPaint& paint) {
    SkASSERT(byteLength);

    bool compact = fits_in(byteLength, 24);

    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawPosText, compact ? (unsigned)byteLength : 0));
    if (!compact) {
        writer.write32(SkToU32(byteLength));
    }
    write_pad(&writer, text, byteLength);
    writer.writePointArray(pos, paint.countText(text, byteLength));
    write_paint(writer, paint, kText_PaintUsage);
}

void SkPipeCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                  SkScalar constY, const SkPaint& paint) {
    SkASSERT(byteLength);

    bool compact = fits_in(byteLength, 24);

    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawPosTextH, compact ? (unsigned)byteLength : 0));
    if (!compact) {
        writer.write32(SkToU32(byteLength));
    }
    write_pad(&writer, text, byteLength);
    writer.writeScalarArray(xpos, paint.countText(text, byteLength));
    writer.writeScalar(constY);
    write_paint(writer, paint, kText_PaintUsage);
}

void SkPipeCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                    const SkMatrix* matrix, const SkPaint& paint) {
    SkASSERT(byteLength > 0);

    unsigned extra = 0;
    if (byteLength <= kTextLength_DrawTextOnPathMask) {
        extra |= byteLength;
    } // else we will write the length after the packedverb
    SkMatrix::TypeMask tm = matrix ? matrix->getType() : SkMatrix::kIdentity_Mask;
    extra |= (unsigned)tm << kMatrixType_DrawTextOnPathShift;

    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawTextOnPath, extra));
    if (byteLength > kTextLength_DrawTextOnPathMask) {
        writer.write32(byteLength);
    }
    write_pad(&writer, text, byteLength);
    writer.writePath(path);
    if (matrix) {
        write_sparse_matrix(&writer, *matrix);
    }
    write_paint(writer, paint, kText_PaintUsage);
}

void SkPipeCanvas::onDrawTextRSXform(const void* text, size_t byteLength, const SkRSXform xform[],
                                     const SkRect* cull, const SkPaint& paint) {
    SkASSERT(byteLength);

    bool compact = fits_in(byteLength, 23);
    unsigned extra = compact ? (byteLength << 1) : 0;
    if (cull) {
        extra |= 1;
    }

    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawTextRSXform, extra));
    if (!compact) {
        writer.write32(SkToU32(byteLength));
    }
    write_pad(&writer, text, byteLength);

    int count = paint.countText(text, byteLength);
    writer.write32(count);  // maybe we can/should store this in extra as well?
    writer.write(xform, count * sizeof(SkRSXform));
    if (cull) {
        writer.writeRect(*cull);
    }
    write_paint(writer, paint, kText_PaintUsage);
}

void SkPipeCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                  const SkPaint &paint) {
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawTextBlob, 0));
    blob->flatten(writer);
    writer.writeScalar(x);
    writer.writeScalar(y);
    write_paint(writer, paint, kTextBlob_PaintUsage);
}

void SkPipeCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                 const SkPaint* paint) {
    unsigned extra = fDeduper->findOrDefinePicture(const_cast<SkPicture*>(picture));
    if (matrix) {
        extra |= kHasMatrix_DrawPictureExtra;
    }
    if (paint) {
        extra |= kHasPaint_DrawPictureExtra;
    }
    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawPicture, extra));
    if (matrix) {
        writer.writeMatrix(*matrix);
    }
    if (paint) {
        write_paint(writer, *paint, kSaveLayer_PaintUsage);
    }
}

void SkPipeCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    size_t size = region.writeToMemory(nullptr);
    unsigned extra = 0;
    if (fits_in(size, 24)) {
        extra = SkToUInt(size);
    }

    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawRegion, extra));
    if (0 == extra) {
        writer.write32(size);
    }
    SkAutoSMalloc<2048> storage(size);
    region.writeToMemory(storage.get());
    write_pad(&writer, storage.get(), size);
    write_paint(writer, paint, kGeometry_PaintUsage);
}

void SkPipeCanvas::onDrawVerticesObject(const SkVertices* vertices, SkBlendMode bmode,
                                        const SkPaint& paint) {
    unsigned extra = static_cast<unsigned>(bmode);

    SkPipeWriter writer(this);
    writer.write32(pack_verb(SkPipeVerb::kDrawVertices, extra));
    // TODO: dedup vertices?
    writer.writeDataAsByteArray(vertices->encode().get());
    write_paint(writer, paint, kVertices_PaintUsage);
}

void SkPipeCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                               const SkPoint texCoords[4], SkBlendMode bmode,
                               const SkPaint& paint) {
    SkPipeWriter writer(this);
    unsigned extra = 0;
    SkASSERT(0 == ((int)bmode & ~kModeEnum_DrawPatchExtraMask));
    extra = (unsigned)bmode;
    if (colors) {
        extra |= kHasColors_DrawPatchExtraMask;
    }
    if (texCoords) {
        extra |= kHasTexture_DrawPatchExtraMask;
    }
    writer.write32(pack_verb(SkPipeVerb::kDrawPatch, extra));
    writer.write(cubics, sizeof(SkPoint) * 12);
    if (colors) {
        writer.write(colors, sizeof(SkColor) * 4);
    }
    if (texCoords) {
        writer.write(texCoords, sizeof(SkPoint) * 4);
    }
    write_paint(writer, paint, kGeometry_PaintUsage);
}

void SkPipeCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* data) {
    const size_t len = strlen(key) + 1; // must write the trailing 0
    bool compact = fits_in(len, 23);
    uint32_t extra = compact ? (unsigned)len : 0;
    extra <<= 1;   // make room for has_data_sentinel
    if (data) {
        extra |= 1;
    }

    fStream->write32(pack_verb(SkPipeVerb::kDrawAnnotation, extra));
    fStream->write(&rect, sizeof(SkRect));
    if (!compact) {
        fStream->write32(SkToU32(len));
    }
    write_pad(fStream, key, len);
    if (data) {
        fStream->write32(SkToU32(data->size()));
        write_pad(fStream, data->data(), data->size());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class A8Serializer : public SkPixelSerializer {
protected:
    bool onUseEncodedData(const void* data, size_t len) {
        return true;
    }

    SkData* onEncode(const SkPixmap& pmap) {
        if (kAlpha_8_SkColorType == pmap.colorType()) {
            SkDynamicMemoryWStream stream;
            stream.write("skiaimgf", 8);
            stream.write32(pmap.width());
            stream.write32(pmap.height());
            stream.write16(pmap.colorType());
            stream.write16(pmap.alphaType());
            stream.write32(0);  // no colorspace for now
            for (int y = 0; y < pmap.height(); ++y) {
                stream.write(pmap.addr8(0, y), pmap.width());
            }
            return stream.detachAsData().release();
        }
        return nullptr;
    }
};

static sk_sp<SkData> default_image_serializer(SkImage* image) {
    A8Serializer serial;
    sk_sp<SkData> data(image->encode(&serial));
    if (!data) {
        data.reset(image->encode());
    }
    return data;
}

static bool show_deduper_traffic = false;

int SkPipeDeduper::findOrDefineImage(SkImage* image) {
    int index = fImages.find(image->uniqueID());
    SkASSERT(index >= 0);
    if (index) {
        if (show_deduper_traffic) {
            SkDebugf("  reuseImage(%d)\n", index - 1);
        }
        return index;
    }

    sk_sp<SkData> data = fIMSerializer ? fIMSerializer->serialize(image)
                                       : default_image_serializer(image);
    if (data) {
        index = fImages.add(image->uniqueID());
        SkASSERT(index > 0);
        SkASSERT(fits_in(index, 24));
        fStream->write32(pack_verb(SkPipeVerb::kDefineImage, index));

        uint32_t len = SkToU32(data->size());
        fStream->write32(SkAlign4(len));
        write_pad(fStream, data->data(), len);

        if (show_deduper_traffic) {
            int size = image->width() * image->height() << 2;
            SkDebugf("  defineImage(%d) %d -> %d\n", index - 1, size, len);
        }
        return index;
    }
    SkDebugf("+++ failed to encode image [%d %d]\n", image->width(), image->height());
    return 0;   // failed to encode
}

int SkPipeDeduper::findOrDefinePicture(SkPicture* picture) {
    int index = fPictures.find(picture->uniqueID());
    SkASSERT(index >= 0);
    if (index) {
        if (show_deduper_traffic) {
            SkDebugf("  reusePicture(%d)\n", index - 1);
        }
        return index;
    }

    size_t prevWritten = fStream->bytesWritten();
    unsigned extra = 0; // 0 means we're defining a new picture, non-zero means undef_index + 1
    fStream->write32(pack_verb(SkPipeVerb::kDefinePicture, extra));
    const SkRect cull = picture->cullRect();
    fStream->write(&cull, sizeof(cull));
    picture->playback(fPipeCanvas);
    // call fPictures.add *after* we're written the picture, so that any nested pictures will have
    // already been defined, and we get the "last" index value.
    index = fPictures.add(picture->uniqueID());
    ASSERT_FITS_IN(index, kObjectDefinitionBits);
    fStream->write32(pack_verb(SkPipeVerb::kEndPicture, index));

    if (show_deduper_traffic) {
        SkDebugf("  definePicture(%d) %d\n",
                 index - 1, SkToU32(fStream->bytesWritten() - prevWritten));
    }
    return index;
}

static sk_sp<SkData> encode(SkTypeface* tf) {
    SkDynamicMemoryWStream stream;
    tf->serialize(&stream);
    return sk_sp<SkData>(stream.detachAsData());
}

int SkPipeDeduper::findOrDefineTypeface(SkTypeface* typeface) {
    if (!typeface) {
        return 0;   // default
    }

    int index = fTypefaces.find(typeface->uniqueID());
    SkASSERT(index >= 0);
    if (index) {
        if (show_deduper_traffic) {
            SkDebugf("  reuseTypeface(%d)\n", index - 1);
        }
        return index;
    }

    sk_sp<SkData> data = fTFSerializer ? fTFSerializer->serialize(typeface) : encode(typeface);
    if (data) {
        index = fTypefaces.add(typeface->uniqueID());
        SkASSERT(index > 0);
        SkASSERT(fits_in(index, 24));
        fStream->write32(pack_verb(SkPipeVerb::kDefineTypeface, index));

        uint32_t len = SkToU32(data->size());
        fStream->write32(SkAlign4(len));
        write_pad(fStream, data->data(), len);

        if (show_deduper_traffic) {
            SkDebugf("  defineTypeface(%d) %d\n", index - 1, len);
        }
        return index;
    }
    SkDebugf("+++ failed to encode typeface %d\n", typeface->uniqueID());
    return 0;   // failed to encode
}

int SkPipeDeduper::findOrDefineFactory(SkFlattenable* flattenable) {
    if (!flattenable) {
        return 0;
    }

    int index = fFactories.find(flattenable->getFactory());
    SkASSERT(index >= 0);
    if (index) {
        if (show_deduper_traffic) {
            SkDebugf("  reuseFactory(%d)\n", index - 1);
        }
        return index;
    }

    index = fFactories.add(flattenable->getFactory());
    ASSERT_FITS_IN(index, kIndex_DefineFactoryExtraBits);
    const char* name = flattenable->getTypeName();
    size_t len = strlen(name);
    ASSERT_FITS_IN(len, kNameLength_DefineFactoryExtraBits);
    unsigned extra = (index << kNameLength_DefineFactoryExtraBits) | len;
    size_t prevWritten = fStream->bytesWritten();
    fStream->write32(pack_verb(SkPipeVerb::kDefineFactory, extra));
    write_pad(fStream, name, len + 1);
    if (false) {
        SkDebugf("  defineFactory(%d) %d %s\n",
             index - 1, SkToU32(fStream->bytesWritten() - prevWritten), name);
    }
    return index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkPipe.h"

class SkPipeSerializer::Impl {
public:
    SkPipeDeduper   fDeduper;
    std::unique_ptr<SkPipeCanvas> fCanvas;
};

SkPipeSerializer::SkPipeSerializer() : fImpl(new Impl) {}

SkPipeSerializer::~SkPipeSerializer() {
    if (fImpl->fCanvas) {
        this->endWrite();
    }
}

void SkPipeSerializer::setTypefaceSerializer(SkTypefaceSerializer* tfs) {
    fImpl->fDeduper.setTypefaceSerializer(tfs);
}

void SkPipeSerializer::setImageSerializer(SkImageSerializer* ims) {
    fImpl->fDeduper.setImageSerializer(ims);
}

void SkPipeSerializer::resetCache() {
    fImpl->fDeduper.resetCaches();
}

sk_sp<SkData> SkPipeSerializer::writeImage(SkImage* image) {
    SkDynamicMemoryWStream stream;
    this->writeImage(image, &stream);
    return stream.detachAsData();
}

sk_sp<SkData> SkPipeSerializer::writePicture(SkPicture* picture) {
    SkDynamicMemoryWStream stream;
    this->writePicture(picture, &stream);
    return stream.detachAsData();
}

void SkPipeSerializer::writePicture(SkPicture* picture, SkWStream* stream) {
    int index = fImpl->fDeduper.findPicture(picture);
    if (0 == index) {
        // Try to define the picture
        this->beginWrite(picture->cullRect(), stream);
        index = fImpl->fDeduper.findOrDefinePicture(picture);
        this->endWrite();
    }
    stream->write32(pack_verb(SkPipeVerb::kWritePicture, index));
}

void SkPipeSerializer::writeImage(SkImage* image, SkWStream* stream) {
    int index = fImpl->fDeduper.findImage(image);
    if (0 == index) {
        // Try to define the image
        fImpl->fDeduper.setStream(stream);
        index = fImpl->fDeduper.findOrDefineImage(image);
    }
    stream->write32(pack_verb(SkPipeVerb::kWriteImage, index));
}

SkCanvas* SkPipeSerializer::beginWrite(const SkRect& cull, SkWStream* stream) {
    SkASSERT(nullptr == fImpl->fCanvas);
    fImpl->fCanvas.reset(new SkPipeCanvas(cull, &fImpl->fDeduper, stream));
    fImpl->fDeduper.setStream(stream);
    fImpl->fDeduper.setCanvas(fImpl->fCanvas.get());
    return fImpl->fCanvas.get();
}

void SkPipeSerializer::endWrite() {
    fImpl->fCanvas->restoreToCount(1);
    fImpl->fCanvas.reset(nullptr);
    fImpl->fDeduper.setCanvas(nullptr);
}
