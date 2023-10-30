/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/core/SkFontPriv.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

static sk_sp<SkDrawable> make_drawable(const SkPath& path) {
    const auto bounds = path.computeTightBounds();

    SkPictureRecorder recorder;
    auto* canvas = recorder.beginRecording(bounds);

    SkPaint paint;
    paint.setColor(0xff008000);
    paint.setAntiAlias(true);

    canvas->drawPath(path, paint);

    return recorder.finishRecordingAsDrawable();
}

static sk_sp<SkTypeface> make_tf() {
    SkCustomTypefaceBuilder builder;
    SkFont font = ToolUtils::DefaultFont();
    SkASSERT(font.getTypeface());
    const float upem = font.getTypeface()->getUnitsPerEm();

    // request a big size, to improve precision at the fontscaler level
    font.setSize(upem);
    font.setHinting(SkFontHinting::kNone);

    // so we can scale our paths back down to 1-point
    const SkMatrix scale = SkMatrix::Scale(1.0f/upem, 1.0f/upem);

    {
        SkFontMetrics metrics;
        font.getMetrics(&metrics);
        builder.setMetrics(metrics, 1.0f/upem);
    }
    builder.setFontStyle(font.getTypeface()->fontStyle());

    // Steal the first 128 chars from the default font
    for (SkGlyphID index = 0; index <= 127; ++index) {
        SkGlyphID glyph = font.unicharToGlyph(index);

        SkScalar width;
        font.getWidths(&glyph, 1, &width);
        SkPath path;
        font.getPath(glyph, &path);
        path.transform(scale);

        // we use the charcode to be our glyph index, since we have no cmap table
        if (index % 2) {
            builder.setGlyph(index, width/upem, make_drawable(path), path.computeTightBounds());
        } else {
            builder.setGlyph(index, width/upem, path);
        }
    }

    return builder.detach();
}

#include "include/core/SkTextBlob.h"

static sk_sp<SkTypeface> round_trip(sk_sp<SkTypeface> tf) {
    auto data = tf->serialize();
    SkMemoryStream stream(data->data(), data->size());
    sk_sp<SkTypeface> face = SkTypeface::MakeDeserialize(&stream, nullptr);
    SkASSERT(face);
    return face;
}

class UserFontGM : public skiagm::GM {
    sk_sp<SkTypeface> fTF;

public:
    UserFontGM() {}

    void onOnceBeforeDraw() override {
        fTF = make_tf();
        // test serialization
        fTF = round_trip(fTF);
    }

    static sk_sp<SkTextBlob> make_blob(sk_sp<SkTypeface> tf, float size, float* spacing) {
        SkFont font(tf);
        font.setSize(size);
        font.setEdging(SkFont::Edging::kAntiAlias);
        *spacing = font.getMetrics(nullptr);
        return SkTextBlob::MakeFromString("Typeface", font);
    }

    bool runAsBench() const override { return true; }

    SkString getName() const override { return SkString("user_typeface"); }

    SkISize getISize() override { return {810, 452}; }

    void onDraw(SkCanvas* canvas) override {
        auto waterfall = [&](sk_sp<SkTypeface> tf, bool defaultFace) {
            SkPaint paint;
            paint.setAntiAlias(true);

            float spacing;
            float x = 20,
                  y = 16;
            for (float size = 9; size <= 100; size *= 1.25f) {
                auto blob = make_blob(tf, size, &spacing);

                // shared baseline
                if (defaultFace) {
                    paint.setColor(0xFFDDDDDD);
                    canvas->drawRect({0, y, 810, y+1}, paint);
                }

                paint.setColor(0xFFCCCCCC);
                paint.setStyle(SkPaint::kStroke_Style);
                canvas->drawRect(blob->bounds().makeOffset(x, y), paint);

                paint.setStyle(SkPaint::kFill_Style);
                paint.setColor(SK_ColorBLACK);
                canvas->drawTextBlob(blob, x, y, paint);

                y += SkScalarRoundToInt(spacing * 1.25f + 2);
            }
        };

        waterfall(ToolUtils::DefaultTypeface(), true);
        canvas->translate(400, 0);
        waterfall(fTF, false);
    }
};
DEF_GM(return new UserFontGM;)
