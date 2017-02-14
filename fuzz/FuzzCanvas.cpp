/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"

#include "SkCanvas.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkPathEffect.h"
#include "SkTypeface.h"
#include "SkRegion.h"

template <typename T, void (SkPaint::*S)(T)>
inline void set_any_value(Fuzz* fuzz, SkPaint* paint) {
    T value;
    fuzz->next(&value);
    (paint->*S)(value);
}

template <typename T, void (SkPaint::*S)(T)>
inline void set_any_enum_range(Fuzz* fuzz, SkPaint* paint, T rmin, T rmax) {
    int value;
    fuzz->nextRange(&value, SkToInt(rmin), SkToInt(rmax));
    (paint->*S)((T)value);
}

// be careful: `foo(make_bool(f), make_bool(f))` is undefined.
static bool make_bool(Fuzz* fuzz) {
    bool b;
    fuzz->next(&b);
    return b;
}

static void fuzz_matrix(Fuzz* fuzz, SkMatrix* matrix) {
    constexpr int kArrayLength = 9;
    SkScalar buffer[kArrayLength];
    fuzz->nextN(buffer, kArrayLength);
    matrix->set9(buffer);
}

static void fuzz_rrect(Fuzz* fuzz, SkRRect* rr) {
    SkRect r;
    SkVector radii[4];
    fuzz->next(&r);
    fuzz->nextN(radii, 4);
    rr->setRectRadii(r, radii);
}

static void fuzz_path(Fuzz* fuzz, SkPath* path) {
    uint8_t fillType;
    fuzz->nextRange(&fillType, 0, (uint8_t)SkPath::kInverseEvenOdd_FillType);
    path->setFillType((SkPath::FillType)fillType);
    uint8_t numOps;
    fuzz->nextRange(&numOps, 0, 60);
    for (uint8_t i = 0; i < numOps; ++i) {
        uint8_t op;
        fuzz->nextRange(&op, 0, 6);
        SkScalar a, b, c, d, e, f;
        switch (op) {
            case 0:
                fuzz->next(&a, &b);
                path->moveTo(a, b);
                break;
            case 1:
                fuzz->next(&a, &b);
                path->lineTo(a, b);
                break;
            case 2:
                fuzz->next(&a, &b, &c, &d);
                path->quadTo(a, b, c, d);
                break;
            case 3:
                fuzz->next(&a, &b, &c, &d, &e);
                path->conicTo(a, b, c, d, e);
                break;
            case 4:
                fuzz->next(&a, &b, &c, &d, &e, &f);
                path->cubicTo(a, b, c, d, e, f);
                break;
            case 5:
                fuzz->next(&a, &b, &c, &d, &e);
                path->arcTo(a, b, c, d, e);
                break;
            case 6:
                path->close();
                break;
            default:
                break;
        }
    }
}

static void fuzz_region(Fuzz* fuzz, SkRegion* region) {
    uint8_t N;
    fuzz->nextRange(&N, 0, 10);
    for (uint8_t i = 0; i < N; ++i) {
        int32_t x, y, w, h;
        uint8_t op;
        fuzz->next(&x, &y, &w, &h);
        fuzz->nextRange(&op, 0, (uint8_t)SkRegion::kLastOp);
        if (!region->op(x, y, w, h, (SkRegion::Op)op)) {
            return;
        }
    }
}

sk_sp<SkShader> MakeFuzzShader(Fuzz* fuzz) { return nullptr; /*TODO*/ }
sk_sp<SkPathEffect> MakeFuzzPathEffect(Fuzz* fuzz) { return nullptr; /*TODO*/ }
sk_sp<SkMaskFilter> MakeFuzzMaskFilter(Fuzz* fuzz) { return nullptr; /*TODO*/ }
sk_sp<SkTypeface> MakeFuzzTypeface(Fuzz* fuzz) { return nullptr; /*TODO*/ }
sk_sp<SkImageFilter> MakeFuzzImageFilter(Fuzz* fuzz) { return nullptr; /*TODO*/ }
sk_sp<SkImage> MakeFuzzImage(Fuzz* fuzz) { return nullptr; /*TODO*/ }

void FuzzPaint(Fuzz* fuzz, SkPaint* paint) {
    if (!fuzz || !paint) {
        return;
    }

    set_any_value<bool, &SkPaint::setAntiAlias>(fuzz, paint);
    set_any_value<bool, &SkPaint::setDither>(fuzz, paint);
    set_any_value<bool, &SkPaint::setLinearText>(fuzz, paint);
    set_any_value<bool, &SkPaint::setSubpixelText>(fuzz, paint);
    set_any_value<bool, &SkPaint::setLCDRenderText>(fuzz, paint);
    set_any_value<bool, &SkPaint::setEmbeddedBitmapText>(fuzz, paint);
    set_any_value<bool, &SkPaint::setAutohinted>(fuzz, paint);
    set_any_value<bool, &SkPaint::setVerticalText>(fuzz, paint);
    set_any_value<bool, &SkPaint::setUnderlineText>(fuzz, paint);
    set_any_value<bool, &SkPaint::setStrikeThruText>(fuzz, paint);
    set_any_value<bool, &SkPaint::setFakeBoldText>(fuzz, paint);
    set_any_value<bool, &SkPaint::setDevKernText>(fuzz, paint);
    set_any_value<SkColor, &SkPaint::setColor>(fuzz, paint);
    set_any_value<SkScalar, &SkPaint::setStrokeWidth>(fuzz, paint);
    set_any_value<SkScalar, &SkPaint::setStrokeMiter>(fuzz, paint);
    set_any_value<SkScalar, &SkPaint::setTextSize>(fuzz, paint);
    set_any_value<SkScalar, &SkPaint::setTextScaleX>(fuzz, paint);
    set_any_value<SkScalar, &SkPaint::setTextSkewX>(fuzz, paint);

    set_any_enum_range<SkBlendMode, &SkPaint::setBlendMode>(fuzz, paint, (SkBlendMode)0,
                                                            SkBlendMode::kLastMode);
    set_any_enum_range<SkPaint::Hinting, &SkPaint::setHinting>(fuzz, paint, SkPaint::kNo_Hinting,
                                                               SkPaint::kFull_Hinting);
    set_any_enum_range<SkFilterQuality, &SkPaint::setFilterQuality>(
            fuzz, paint, SkFilterQuality::kNone_SkFilterQuality,
            SkFilterQuality::kLast_SkFilterQuality);
    set_any_enum_range<SkPaint::Style, &SkPaint::setStyle>(fuzz, paint, SkPaint::kFill_Style,
                                                           SkPaint::kStrokeAndFill_Style);
    set_any_enum_range<SkPaint::Cap, &SkPaint::setStrokeCap>(fuzz, paint, SkPaint::kButt_Cap,
                                                             SkPaint::kLast_Cap);
    set_any_enum_range<SkPaint::Join, &SkPaint::setStrokeJoin>(fuzz, paint, SkPaint::kMiter_Join,
                                                               SkPaint::kLast_Join);
    set_any_enum_range<SkPaint::Align, &SkPaint::setTextAlign>(fuzz, paint, SkPaint::kLeft_Align,
                                                               SkPaint::kRight_Align);
    set_any_enum_range<SkPaint::TextEncoding, &SkPaint::setTextEncoding>(
            fuzz, paint, SkPaint::kUTF8_TextEncoding, SkPaint::kGlyphID_TextEncoding);

    paint->setShader(MakeFuzzShader(fuzz));
    paint->setPathEffect(MakeFuzzPathEffect(fuzz));
    paint->setMaskFilter(MakeFuzzMaskFilter(fuzz));
    paint->setTypeface(MakeFuzzTypeface(fuzz));
    paint->setImageFilter(MakeFuzzImageFilter(fuzz));
}

void FuzzCanvas(Fuzz* fuzz, SkCanvas* canvas) {
    if (!fuzz || !canvas) {
        return;
    }
    unsigned N;
    fuzz->nextRange(&N, 0, 2000);
    for (unsigned i = 0; i < N; ++i) {
        if (fuzz->exhausted()) {
            return;
        }
        SkPaint paint;
        FuzzPaint(fuzz, &paint);
        if (fuzz->exhausted()) {
            return;
        }
        unsigned drawCommand;
        fuzz->nextRange(&drawCommand, 0, 54);
        switch (drawCommand) {
            case 0:
                canvas->flush();
                break;
            case 1:
                canvas->save();
                break;
            case 2: {
                SkRect bounds;
                fuzz->next(&bounds);
                canvas->saveLayer(&bounds, &paint);
                break;
            }
            case 3: {
                SkRect bounds;
                fuzz->next(&bounds);
                canvas->saveLayer(&bounds, nullptr);
                break;
            }
            case 4:
                canvas->saveLayer(nullptr, &paint);
                break;
            case 5:
                canvas->saveLayer(nullptr, nullptr);
                break;
            case 6: {
                uint8_t alpha;
                fuzz->next(&alpha);
                canvas->saveLayerAlpha(nullptr, (U8CPU)alpha);
                break;
            }
            case 7: {
                SkRect bounds;
                uint8_t alpha;
                fuzz->next(&bounds, &alpha);
                canvas->saveLayerAlpha(&bounds, (U8CPU)alpha);
                break;
            }
            case 8: {
                SkCanvas::SaveLayerRec saveLayerRec;
                SkRect bounds;
                if (make_bool(fuzz)) {
                    fuzz->next(&bounds);
                    saveLayerRec.fBounds = &bounds;
                }
                if (make_bool(fuzz)) {
                    saveLayerRec.fPaint = &paint;
                }
                sk_sp<SkImageFilter> imageFilter;
                if (make_bool(fuzz)) {
                    imageFilter = MakeFuzzImageFilter(fuzz);
                    saveLayerRec.fBackdrop = imageFilter.get();
                }
                if (make_bool(fuzz)) {
                    saveLayerRec.fSaveLayerFlags |= SkCanvas::kIsOpaque_SaveLayerFlag;
                }
                if (make_bool(fuzz)) {
                    saveLayerRec.fSaveLayerFlags |= SkCanvas::kPreserveLCDText_SaveLayerFlag;
                }
                canvas->saveLayer(saveLayerRec);
                break;
            }
            case 9:
                canvas->restore();
                break;
            case 10: {
                int saveCount;
                fuzz->next(&saveCount);
                canvas->restoreToCount(saveCount);
                break;
            }
            case 11: {
                SkScalar x, y;
                fuzz->next(&x, &y);
                canvas->translate(x, y);
                break;
            }
            case 12: {
                SkScalar x, y;
                fuzz->next(&x, &y);
                canvas->scale(x, y);
                break;
            }
            case 13: {
                SkScalar v;
                fuzz->next(&v);
                canvas->rotate(v);
                break;
            }
            case 14: {
                SkScalar x, y, v;
                fuzz->next(&x, &y, &v);
                canvas->rotate(v, x, y);
                break;
            }
            case 15: {
                SkScalar x, y;
                fuzz->next(&x, &y);
                canvas->skew(x, y);
                break;
            }
            case 16: {
                SkMatrix mat;
                fuzz_matrix(fuzz, &mat);
                canvas->concat(mat);
                break;
            }
            case 17: {
                SkMatrix mat;
                fuzz_matrix(fuzz, &mat);
                canvas->setMatrix(mat);
                break;
            }
            case 18:
                canvas->resetMatrix();
                break;
            case 19: {
                SkRect r;
                int op;
                bool doAntiAlias;
                fuzz->next(&r, &doAntiAlias);
                fuzz->nextRange(&op, 0, 1);
                canvas->clipRect(r, (SkClipOp)op, doAntiAlias);
                break;
            }
            case 20: {
                SkRRect rr;
                int op;
                bool doAntiAlias;
                fuzz_rrect(fuzz, &rr);
                fuzz->next(&doAntiAlias);
                fuzz->nextRange(&op, 0, 1);
                canvas->clipRRect(rr, (SkClipOp)op, doAntiAlias);
                break;
            }
            case 21: {
                SkPath path;
                fuzz_path(fuzz, &path);
                int op;
                bool doAntiAlias;
                fuzz->next(&doAntiAlias);
                fuzz->nextRange(&op, 0, 1);
                canvas->clipPath(path, (SkClipOp)op, doAntiAlias);
                break;
            }
            case 22: {
                SkRegion region;
                fuzz_region(fuzz, &region);
                int op;
                fuzz->nextRange(&op, 0, 1);
                canvas->clipRegion(region, (SkClipOp)op);
                break;
            }
            case 23:
                canvas->drawPaint(paint);
                break;
            case 24: {
                uint8_t pointMode;
                fuzz->nextRange(&pointMode, 0, 3);
                size_t count;
                constexpr int kMaxCount = 30;
                fuzz->nextRange(&count, 0, kMaxCount);
                SkPoint pts[kMaxCount];
                fuzz->nextN(pts, count);
                canvas->drawPoints((SkCanvas::PointMode)pointMode, count, pts, paint);
                break;
            }
            case 25: {
                SkRect r;
                fuzz->next(&r);
                canvas->drawRect(r, paint);
                break;
            }
            case 26: {
                SkRegion region;
                fuzz_region(fuzz, &region);
                canvas->drawRegion(region, paint);
                break;
            }
            case 27: {
                SkRect r;
                fuzz->next(&r);
                canvas->drawOval(r, paint);
                break;
            }
            case 29: {
                SkRRect rr;
                fuzz_rrect(fuzz, &rr);
                canvas->drawRRect(rr, paint);
                break;
            }
            case 30: {
                SkRRect orr, irr;
                fuzz_rrect(fuzz, &orr);
                fuzz_rrect(fuzz, &irr);
                canvas->drawDRRect(orr, irr, paint);
                break;
            }
            case 31: {
                SkRect r;
                SkScalar start, sweep;
                bool useCenter;
                fuzz->next(&r, &start, &sweep, &useCenter);
                canvas->drawArc(r, start, sweep, useCenter, paint);
                break;
            }
            case 32: {
                SkPath path;
                fuzz_path(fuzz, &path);
                canvas->drawPath(path, paint);
                break;
            }
            case 33: {
                sk_sp<SkImage> img = MakeFuzzImage(fuzz);
                SkScalar left, top;
                bool usePaint;
                fuzz->next(&left, &top, &usePaint);
                canvas->drawImage(img.get(), left, top, usePaint ? &paint : nullptr);
                break;
            }
            case 34: {
                // canvas->drawImageRect(...);
                break;
            }
            case 35: {
                // canvas->drawImageRect(...);
                break;
            }
            case 36: {
                // canvas->drawImageRect(...);
                break;
            }
            case 37: {
                // canvas->drawImageNine(...);
                break;
            }
            case 38: {
                // canvas->drawBitmap(...);
                break;
            }
            case 39: {
                // canvas->drawBitmapRect(...);
                break;
            }
            case 40: {
                // canvas->drawBitmapRect(...);
                break;
            }
            case 41: {
                // canvas->drawBitmapRect(...);
                break;
            }
            case 42: {
                // canvas->drawBitmapNine(...);
                break;
            }
            case 43: {
                // canvas->drawBitmapLattice(...);
                break;
            }
            case 44: {
                // canvas->drawImageLattice(...);
                break;
            }
            case 45: {
                // canvas->drawText(...);
                break;
            }
            case 46: {
                // canvas->drawPosText(...);
                break;
            }
            case 47: {
                // canvas->drawPosTextH(...);
                break;
            }
            case 48: {
                // canvas->drawTextOnPathHV(...);
                break;
            }
            case 49: {
                // canvas->drawTextOnPath(...);
                break;
            }
            case 50: {
                // canvas->drawTextRSXform(...);
                break;
            }
            case 51: {
                // canvas->drawTextBlob(...);
                break;
            }
            case 52: {
                // canvas->drawPicture(...);
                break;
            }
            case 53: {
                // canvas->drawVertices(...);
                break;
            }
            case 54: {
                // canvas->drawVertices(...);
                break;
            }
            default:
                break;
        }
    }
}
