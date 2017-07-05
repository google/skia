/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoPixmapStorage.h"
#include "SkColorPriv.h"
#include "SkImage.h"
#include "SkParsePath.h"
#include "SkPath.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "gm.h"

// GM to test combinations of stroking zero length paths with different caps and other settings
// Variables:
// * Antialiasing: On, Off
// * Caps: Butt, Round, Square
// * Stroke width: 0, 0.9, 1, 1.1, 15, 25
// * Path form: M, ML, MLZ, MZ
// * Path contours: 1 or 2 ** not yet
// * Path verbs: Line, Quad, Cubic, Conic
//
// Each test is drawn to a 60x20 offscreen surface, and expected to produce some number (0 - 2) of
// visible pieces of cap geometry. These are counted by scanning horizontally for peaks (blobs).

static bool draw_path_cell(SkCanvas* canvas, SkImage* img, int expectedCaps) {
    // Draw the image
    canvas->drawImage(img, 0, 0);

    int w = img->width(), h = img->height();

    // Read the pixels back
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    SkAutoPixmapStorage pmap;
    pmap.alloc(info);
    SkAssertResult(img->readPixels(pmap, 0, 0));

    // To account for rasterization differences, we scan the middle two rows [y, y+1] of the image
    SkASSERT(h % 2 == 0);
    int y = (h - 1) / 2;

    bool inBlob = false;
    int numBlobs = 0;
    for (int x = 0; x < w; ++x) {
        // We drew white-on-transparent-black. We can look for any non-zero value.
        // And we care if either row is non-zero, so just add them to simplify everything.
        uint32_t v = *pmap.addr32(x, y) + *pmap.addr32(x, y + 1);

        if (!inBlob && v) {
            ++numBlobs;
        }
        inBlob = SkToBool(v);
    }

    SkPaint outline;
    outline.setStyle(SkPaint::kStroke_Style);
    if (numBlobs == expectedCaps) {
        outline.setColor(0xFF007F00); // Green
    } else if (numBlobs > expectedCaps) {
        outline.setColor(0xFF7F7F00); // Yellow -- more geometry than expected
    } else {
        outline.setColor(0xFF7F0000); // Red -- missing some geometry
    }

    canvas->drawRect(SkRect::MakeWH(w, h), outline);
    return numBlobs == expectedCaps;
}

DEF_SIMPLE_GM_BG(zero_length_paths_aa, canvas, 800, 800, SK_ColorBLACK) {
    SkPaint::Cap kCaps[] = { SkPaint::kButt_Cap, SkPaint::kRound_Cap, SkPaint::kSquare_Cap };
    SkScalar kWidths[] = { 0.0f, 0.9f, 1.0f, 1.1f, 15.0f, 25.0f };
    const char* kVerbs[] = {
        nullptr,
        "l 0 0 ",
        "q 0 0 0 0 ",
        "c 0 0 0 0 0 0 ",
        "a 0 0 0 0 0 0 0 "
    };

    canvas->translate(10.5f, 10.5f);

    SkImageInfo info = canvas->imageInfo().makeWH(60, 20);
    auto surface = canvas->makeSurface(info);
    if (!surface) {
        surface = SkSurface::MakeRasterN32Premul(60, 20);
    }

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    for (auto cap : kCaps) {
        for (auto width : kWidths) {
            paint.setStrokeCap(cap);
            paint.setStrokeWidth(width);
            canvas->save();

            for (auto verb : kVerbs) {
                for (bool close : { false, true }) {
                    SkString pathStr("M 29.5 9.5 ");
                    if (verb) {
                        pathStr.append(verb);
                    }
                    if (close) {
                        pathStr.append("Z");
                    }

                    SkPath path;
                    SkParsePath::FromSVGString(pathStr.c_str(), &path);

                    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
                    surface->getCanvas()->drawPath(path, paint);
                    auto img = surface->makeImageSnapshot();

                    // All cases should draw one cap, except for butt capped, and dangling moves
                    // (without a verb or close), which shouldn't draw anything.
                    int expectedCaps = ((SkPaint::kButt_Cap == cap) || (!verb && !close)) ? 0 : 1;

                    draw_path_cell(canvas, img.get(), expectedCaps);
                    canvas->translate(70, 0);
                }
            }
            canvas->restore();
            canvas->translate(0, 30);
        }
    }
}

// Test how short paths are stroked with various caps
class StrokeZeroGM : public skiagm::GM {
    SkPath fPaths[8];
    SkPath fClipL, fClipR, fClipS;

protected:
    void onOnceBeforeDraw() override {
        fClipL.moveTo(0, 0);
        fClipL.lineTo(3, 0);
        fClipL.lineTo(2.5f, 1);
        fClipL.lineTo(3.5f, 2.5f);
        fClipL.lineTo(2.5f, 4);
        fClipL.lineTo(3, 5);
        fClipL.lineTo(0, 5);
        fClipL.close();

        fClipR.moveTo(34, 0);
        fClipR.lineTo(34, 5);
        fClipR.lineTo(31, 5);
        fClipR.lineTo(30.5, 4);
        fClipR.lineTo(31.5, 2.5);
        fClipR.lineTo(30.5, 1);
        fClipR.lineTo(31, 0);
        fClipR.close();

        fClipS.addRect(SkRect::MakeIWH(4, 5));

        fPaths[0].moveTo(30, 0);  // single line segment
        fPaths[0].rLineTo(30, 0);

        fPaths[1].moveTo(90, 0);  // single line segment with close (does not draw caps)
        fPaths[1].rLineTo(30, 0);
        fPaths[1].close();

        fPaths[2].moveTo(150, 0);  // zero-length line
        fPaths[2].rLineTo(0, 0);

        fPaths[3].moveTo(180, 0);  // zero-length line with close (expected not to draw)
        fPaths[3].rLineTo(0, 0);
        fPaths[3].close();

        fPaths[4].moveTo(210, 0);  // close only, no line
        fPaths[4].close();

        fPaths[5].moveTo(30, 90);  // all combos below should draw two caps
        fPaths[5].rLineTo(0, 0);
        fPaths[5].moveTo(60, 90);
        fPaths[5].rLineTo(0, 0);

        fPaths[6].moveTo(90, 90);
        fPaths[6].close();
        fPaths[6].moveTo(120, 90);
        fPaths[6].close();

        fPaths[7].moveTo(150, 90);
        fPaths[7].rLineTo(0, 0);
        fPaths[7].moveTo(180, 90);
        fPaths[7].close();
    }


    SkString onShortName() override {
        return SkString("path_stroke_with_zero_length");
    }

    SkISize onISize() override {
        return SkISize::Make(1120, 840);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint bkgrnd;
        bkgrnd.setColor(SK_ColorWHITE);
        canvas->drawRect(SkRect::MakeIWH(onISize().fWidth, onISize().fHeight), bkgrnd);

        // Move to pixel centers. The non-AA parts of this test are really poorly defined otherwise.
        canvas->translate(0.5f, 0.5f);

         auto drawPaths = [&](SkPaint& paint, int indexMask) {
            canvas->translate(0, 30.0f);
            int index = 0;
            for (const SkPath& path : fPaths) {
                if (indexMask & (1 << index)) {
                    canvas->drawPath(path, paint);
                }
                if (this->getMode() == skiagm::GM::kSample_Mode && paint.getStrokeWidth() < 2) {
                    drawFat(canvas, path, paint, index);
                }
                ++index;
            }
        };

        if (false) { // debugging variant that draws a single element
            SkScalar width = 0;
            bool antialias = true;

            SkPaint butt;
            butt.setAntiAlias(antialias);
            butt.setStyle(SkPaint::kStroke_Style);
            butt.setStrokeWidth(width);

            SkPaint round(butt);
            round.setStrokeCap(SkPaint::kRound_Cap);
            drawPaths(round, 1 << 7);
            return;
        }

        SkScalar widths[] = { 0, .999f, 1, 1.001f, 20 };
        bool aliases[] = { false, true };
        for (bool antialias : aliases) {
            canvas->save();
            for (SkScalar width : widths) {
                canvas->save();
                SkPaint butt;
                butt.setAntiAlias(antialias);
                butt.setStyle(SkPaint::kStroke_Style);
                butt.setStrokeWidth(width);
                drawPaths(butt, -1);

                SkPaint round(butt);
                round.setStrokeCap(SkPaint::kRound_Cap);
                drawPaths(round, -1);

                SkPaint square(butt);
                square.setStrokeCap(SkPaint::kSquare_Cap);
                drawPaths(square, -1);
                canvas->restore();
                canvas->translate(220, 0);
            }
            canvas->restore();
            canvas->translate(0, 210);
        }
    }

private:
    void drawFat(SkCanvas* canvas, const SkPath& path, const SkPaint& paint, int index) {
        const SkScalar scale = 10;
        SkRect bounds = path.getBounds();
        SkBitmap offscreen;
        offscreen.allocN32Pixels(SkScalarRoundToInt(bounds.width() + 4),
                SkScalarRoundToInt(bounds.height() + 4));
        offscreen.eraseColor(SK_ColorWHITE);
        SkScalar pathX = bounds.fLeft - 2;
        SkScalar pathY = bounds.fTop - 2;
        SkMatrix cMatrix = canvas->getTotalMatrix();
        if (!canvas->readPixels(offscreen, SkScalarRoundToInt(pathX + cMatrix.getTranslateX()),
                SkScalarRoundToInt(pathY + cMatrix.getTranslateY()))) {
            return;
        }

        canvas->save();
        SkMatrix clipM;
        clipM.reset();
        clipM.preScale(scale, scale);
        clipM.postTranslate(bounds.fLeft - 17, bounds.fTop - 24.5f + 420);
        SkPath clip;
        if (index < 2) {
            fClipL.transform(clipM, &clip);
        } else {
            fClipS.transform(clipM, &clip);
        }
        canvas->clipPath(clip, true);
        canvas->scale(scale, scale);
        canvas->drawBitmap(offscreen, (bounds.fLeft - 17) / scale,
                    (bounds.fTop - 20 + 420) / scale);
        canvas->restore();

        if (bounds.width() > 20) {
            canvas->save();
            clipM.reset();
            clipM.preScale(scale, scale);
            clipM.postTranslate(bounds.fLeft - 17 - 275, bounds.fTop - 24.5f + 420);
            SkPath clip;
            fClipR.transform(clipM, &clip);
            canvas->clipPath(clip, true);
            canvas->scale(10.f, 10.f);
            canvas->drawBitmap(offscreen, (bounds.fLeft - 17 - 275
                    + (index >= 5 ? 5 : 0)) / scale, (bounds.fTop - 20 + 420) / scale);
            canvas->restore();
        }
    }

};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new StrokeZeroGM(); )
