/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkPath.h"

namespace skiagm {

class ThinAAGM : public GM {
public:
    ThinAAGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("thinaa");
    }

    SkISize onISize() override {
        return SkISize::Make(240, 320);
    }

    void onDraw(SkCanvas* canvas) override {

        canvas->translate(10.f, 0.f);

        static TileRenderer kRenderers[] = { DrawRect, DrawHairline, DrawLinePath, DrawCurvedPath };
        static SkScalar kAngles[] = { 0.0f, 45.0f, 90.0f };

        for (size_t i = 0; i < SK_ARRAY_COUNT(kRenderers); ++i) {
            canvas->translate(10.f, 0.0f);
            for (size_t j = 0; j < SK_ARRAY_COUNT(kAngles); ++j) {
                DrawGrid(canvas, kAngles[j], kRenderers[i]);
                canvas->translate(kGridOffsetX, 0.0f);
            }
        }

        // FIXME handle supersampling at 2, 4, and 8x
    }

private:
    static constexpr SkScalar kTileWidth = 10.f;
    static constexpr SkScalar kTileHeight = 10.f;
    static constexpr SkScalar kGridOffsetX = 80.f; // 6 sizes @ 10px + 20px gap
    static constexpr SkScalar kGridOffsetY = 110.f; // 9 shifts @ 10px + 20px gap

    typedef void (*TileRenderer)(SkCanvas*, SkScalar, SkScalar, SkScalar);

    static SkPaint MakePaint(SkPaint::Style style) {
        SkPaint paint;
        paint.setColor4f({0.f, 0.f, 0.f, 1.f}, nullptr);
        paint.setAntiAlias(true);
        paint.setStyle(style);
        return paint;
    }

    static void DrawRect(SkCanvas* canvas, SkScalar width, SkScalar shift, SkScalar rotate) {
        canvas->rotate(rotate, kTileWidth / 2.f, kTileHeight / 2.f);
        canvas->translate(shift, shift);
        canvas->drawRect(SkRect::MakeLTRB(kTileWidth / 2.f - width / 2.f, 1.f,
                                          kTileWidth / 2.f + width / 2.f, kTileHeight - 1.f),
                         MakePaint(SkPaint::kFill_Style));
    }

    static void DrawHairline(SkCanvas* canvas, SkScalar width, SkScalar shift, SkScalar rotate) {
        SkPaint paint = MakePaint(SkPaint::kStroke_Style);
        paint.setStrokeWidth(0.f); // Ignores width

        SkPath path;
        path.moveTo(kTileWidth / 2.f, 1.f);
        path.lineTo(kTileWidth / 2.f, kTileHeight - 1.f);

        canvas->rotate(rotate, kTileWidth / 2.f, kTileHeight / 2.f);
        canvas->translate(shift, shift);
        canvas->drawPath(path, paint);
    }

    static void DrawLinePath(SkCanvas* canvas, SkScalar width, SkScalar shift, SkScalar rotate) {
        SkPaint paint = MakePaint(SkPaint::kStroke_Style);
        paint.setStrokeWidth(width);
        paint.setStrokeCap(SkPaint::kRound_Cap); // Don't let it get converted into a filled rect

        SkPath path;
        path.moveTo(kTileWidth / 2.f, 1.f);
        path.lineTo(kTileWidth / 2.f, kTileHeight - 1.f);

        canvas->rotate(rotate, kTileWidth / 2.f, kTileHeight / 2.f);
        canvas->translate(shift, shift);
        canvas->drawPath(path, paint);
    }

    static void DrawCurvedPath(SkCanvas* canvas, SkScalar width, SkScalar shift, SkScalar rotate) {
        SkPaint paint = MakePaint(SkPaint::kStroke_Style);
        paint.setStrokeWidth(width);
        paint.setStrokeCap(SkPaint::kRound_Cap);

        SkPath path;
        path.moveTo(kTileWidth / 2.f, 1.f);
        path.quadTo(kTileWidth, kTileHeight / 2.f, kTileWidth / 2.f, kTileHeight - 1.f);
        // path.arcTo(kTileWidth / 2.f, 1.f, kTileWidth / 2.f, kTileHeight - 1.f, 0.f);

        canvas->rotate(rotate, kTileWidth / 2.f, kTileHeight / 2.f);
        canvas->translate(shift, shift);
        canvas->drawPath(path, paint);
    }

    static void DrawShiftedColumns(SkCanvas* canvas, SkScalar angle, SkScalar width,
                                   TileRenderer renderer) {
        constexpr SkScalar kShifts[] = { 0.f, 0.125f, 0.25f, 0.375f, 0.5f,
                                         0.625f, 0.75f, 0.875f, 1.f };
        for (size_t i = 0; i < SK_ARRAY_COUNT(kShifts); ++i) {
            canvas->save();
            canvas->translate(0.0f, i * kTileHeight);
            renderer(canvas, width, kShifts[i], angle);
            canvas->restore();
        }
    }

    static void DrawGrid(SkCanvas* canvas, SkScalar angle, TileRenderer renderer) {
        constexpr SkScalar kWidths[] = { 0.125f, 0.25f, 0.5f, 0.75f, 1.0f, 2.0f };
        for (size_t i = 0; i < SK_ARRAY_COUNT(kWidths); ++i) {
            canvas->save();
            canvas->translate(i * kTileWidth, 0.0f);
            DrawShiftedColumns(canvas, angle, kWidths[i], renderer);
            canvas->restore();
        }
    }

    static void DrawVertRects(SkCanvas* canvas, const SkPaint& p) {
        constexpr SkRect vertRects[] = {
            { 1,  1,    5.0f, 21 }, // 4 pix wide
            { 8,  1,   10.0f, 21 }, // 2 pix wide
            { 13, 1,   14.0f, 21 }, // 1 pix wide
            { 17, 1,   17.5f, 21 }, // 1/2 pix wide
            { 21, 1,  21.25f, 21 }, // 1/4 pix wide
            { 25, 1, 25.125f, 21 }, // 1/8 pix wide
            { 29, 1,   29.0f, 21 }  // 0 pix wide
        };

        for (size_t j = 0; j < SK_ARRAY_COUNT(vertRects); ++j) {
            canvas->drawRect(vertRects[j], p);
        }
    }

    static void DrawHorizRects(SkCanvas* canvas, const SkPaint& p) {
        constexpr SkRect horizRects[] = {
            { 1, 1,  21,    5.0f }, // 4 pix high
            { 1, 8,  21,   10.0f }, // 2 pix high
            { 1, 13, 21,   14.0f }, // 1 pix high
            { 1, 17, 21,   17.5f }, // 1/2 pix high
            { 1, 21, 21,  21.25f }, // 1/4 pix high
            { 1, 25, 21, 25.125f }, // 1/8 pix high
            { 1, 29, 21,   29.0f }  // 0 pix high
        };

        for (size_t j = 0; j < SK_ARRAY_COUNT(horizRects); ++j) {
            canvas->drawRect(horizRects[j], p);
        }
    }

    static void DrawSquares(SkCanvas* canvas, const SkPaint& p) {
        constexpr SkRect squares[] = {
            { 1,  1,     5.0f,    5.0f }, // 4 pix
            { 8,  8,    10.0f,   10.0f }, // 2 pix
            { 13, 13,   14.0f,   14.0f }, // 1 pix
            { 17, 17,   17.5f,   17.5f }, // 1/2 pix
            { 21, 21,  21.25f,  21.25f }, // 1/4 pix
            { 25, 25, 25.125f, 25.125f }, // 1/8 pix
            { 29, 29,   29.0f,   29.0f }  // 0 pix
        };

        for (size_t j = 0; j < SK_ARRAY_COUNT(squares); ++j) {
            canvas->drawRect(squares[j], p);
        }
    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ThinAAGM; )

}
