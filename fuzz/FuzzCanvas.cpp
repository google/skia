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

template <typename T, void (SkPaint::*S)(T)>
inline void set_any_value(Fuzz* fuzz, SkPaint* paint) {
    bool skip{};
    fuzz->next(&skip);
    if (!skip) {
        T value{};
        fuzz->next(&value);
        (paint->*S)(value);
    }
}

template <typename T, void (SkPaint::*S)(T)>
inline void set_any_enum_range(Fuzz* fuzz, SkPaint* paint, T rmin, T rmax) {
    bool skip{};
    fuzz->next(&skip);
    if (!skip) {
        int value{};
        fuzz->nextRange(&value, SkToInt(rmin), SkToInt(rmax));
        (paint->*S)((T)value);
    }
}

sk_sp<SkShader> MakeFuzzShader(Fuzz* fuzz) { return nullptr; /*TODO*/ }
sk_sp<SkPathEffect> MakeFuzzPathEffect(Fuzz* fuzz) { return nullptr; /*TODO*/ }
sk_sp<SkMaskFilter> MakeFuzzMaskFilter(Fuzz* fuzz) { return nullptr; /*TODO*/ }
sk_sp<SkTypeface> MakeFuzzTypeface(Fuzz* fuzz) { return nullptr; /*TODO*/ }
sk_sp<SkImageFilter> MakeFuzzImageFilter(Fuzz* fuzz) { return nullptr; /*TODO*/ }

void FuzzPaint(Fuzz* fuzz, SkPaint* paint) {
    if (!fuzz || !paint) { return; }

    set_any_value<bool, &SkPaint::setAntiAlias         >(fuzz, paint);
    set_any_value<bool, &SkPaint::setDither            >(fuzz, paint);
    set_any_value<bool, &SkPaint::setLinearText        >(fuzz, paint);
    set_any_value<bool, &SkPaint::setSubpixelText      >(fuzz, paint);
    set_any_value<bool, &SkPaint::setLCDRenderText     >(fuzz, paint);
    set_any_value<bool, &SkPaint::setEmbeddedBitmapText>(fuzz, paint);
    set_any_value<bool, &SkPaint::setAutohinted        >(fuzz, paint);
    set_any_value<bool, &SkPaint::setVerticalText      >(fuzz, paint);
    set_any_value<bool, &SkPaint::setUnderlineText     >(fuzz, paint);
    set_any_value<bool, &SkPaint::setStrikeThruText    >(fuzz, paint);
    set_any_value<bool, &SkPaint::setFakeBoldText      >(fuzz, paint);
    set_any_value<bool, &SkPaint::setDevKernText       >(fuzz, paint);
    set_any_value<SkColor,  &SkPaint::setColor         >(fuzz, paint);
    set_any_value<SkScalar, &SkPaint::setStrokeWidth   >(fuzz, paint);
    set_any_value<SkScalar, &SkPaint::setStrokeMiter   >(fuzz, paint);
    set_any_value<SkScalar, &SkPaint::setTextSize      >(fuzz, paint);
    set_any_value<SkScalar, &SkPaint::setTextScaleX    >(fuzz, paint);
    set_any_value<SkScalar, &SkPaint::setTextSkewX     >(fuzz, paint);

    set_any_enum_range<SkPaint::Hinting, &SkPaint::setHinting>(
            fuzz, paint, SkPaint::kNo_Hinting, SkPaint::kFull_Hinting);
    set_any_enum_range<SkFilterQuality, &SkPaint::setFilterQuality>(
            fuzz, paint, SkFilterQuality::kNone_SkFilterQuality,
            SkFilterQuality::kLast_SkFilterQuality);
    set_any_enum_range<SkPaint::Style, &SkPaint::setStyle>(
            fuzz, paint, SkPaint::kFill_Style, SkPaint::kStrokeAndFill_Style);
    set_any_enum_range<SkPaint::Cap, &SkPaint::setStrokeCap>(
            fuzz, paint, SkPaint::kButt_Cap, SkPaint::kLast_Cap);
    set_any_enum_range<SkPaint::Join, &SkPaint::setStrokeJoin>(
            fuzz, paint, SkPaint::kMiter_Join, SkPaint::kLast_Join);
    set_any_enum_range<SkPaint::Align, &SkPaint::setTextAlign>(
            fuzz, paint, SkPaint::kLeft_Align, SkPaint::kRight_Align);
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
    unsigned N{};
    fuzz->nextRange(&N, 0, 2000);
    for (unsigned i = 0; i < N; ++i) {
        SkPaint paint;
        FuzzPaint(fuzz, &paint);
        unsigned drawCommand{};
        fuzz->nextRange(&drawCommand, 0, 50);
        switch (drawCommand) {
            case 0:
                canvas->flush();
                break;
            case 1:
                canvas->save();
                break;
            case 2:
                SkRect bounds;
                fuzz->next(&bounds.fLeft);
                fuzz->next(&bounds.fTop);
                fuzz->next(&bounds.fRight);
                fuzz->next(&bounds.fBottom);
                canvas->saveLayer(&bounds, &paint);
                break;
            case 3: {
                SkRect bounds;
                fuzz->next(&bounds.fLeft);
                fuzz->next(&bounds.fTop);
                fuzz->next(&bounds.fRight);
                fuzz->next(&bounds.fBottom);
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
                canvas->saveLayerAlpha(nullptr, (U8CPU) alpha);
            }
            case 7: {
                SkRect bounds;
                fuzz->next(&bounds.fLeft);
                fuzz->next(&bounds.fTop);
                fuzz->next(&bounds.fRight);
                fuzz->next(&bounds.fBottom);
                uint8_t alpha;                
                fuzz->next(&alpha);
                canvas->saveLayerAlpha(&bounds, (U8CPU) alpha);
                break;
            }
            // case 8: {
            //     SkCanvas::SaveLayerRec saveLayerRec;
            //     //...
            // }
            default:
                break;
        }
    }
}
