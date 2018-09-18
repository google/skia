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
#include "SkSurface.h"
#include "gm.h"

// GM to test combinations of stroking zero length paths with different caps and other settings
// Variables:
// * Antialiasing: On, Off
// * Caps: Butt, Round, Square
// * Stroke width: 0, 0.9, 1, 1.1, 15, 25
// * Path form: M, ML, MLZ, MZ
// * Path contours: 1 or 2
// * Path verbs: Line, Quad, Cubic, Conic
//
// Each test is drawn to a 50x20 offscreen surface, and expected to produce some number (0 - 2) of
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
        // We drew white-on-black. We can look for any non-zero value. Just check red.
        // And we care if either row is non-zero, so just add them to simplify everything.
        uint32_t v = SkGetPackedR32(*pmap.addr32(x, y)) + SkGetPackedR32(*pmap.addr32(x, y + 1));

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

static const SkPaint::Cap kCaps[] = {
    SkPaint::kButt_Cap,
    SkPaint::kRound_Cap,
    SkPaint::kSquare_Cap
};

static const SkScalar kWidths[] = { 0.0f, 0.9f, 1.0f, 1.1f, 15.0f, 25.0f };

// Full set of path structures for single contour case (each primitive with and without a close)
static const char* kAllVerbs[] = {
    nullptr,
    "z ",
    "l 0 0 ",
    "l 0 0 z ",
    "q 0 0 0 0 ",
    "q 0 0 0 0 z ",
    "c 0 0 0 0 0 0 ",
    "c 0 0 0 0 0 0 z ",
    "a 0 0 0 0 0 0 0 ",
    "a 0 0 0 0 0 0 0 z "
};

// Reduced set of path structures for double contour case, to keep total number of cases down
static const char* kSomeVerbs[] = {
    nullptr,
    "z ",
    "l 0 0 ",
    "l 0 0 z ",
    "q 0 0 0 0 ",
    "q 0 0 0 0 z ",
};

static const int kCellWidth = 50;
static const int kCellHeight = 20;
static const int kCellPad = 2;

static const int kNumRows = SK_ARRAY_COUNT(kCaps) * SK_ARRAY_COUNT(kWidths);
static const int kNumColumns = SK_ARRAY_COUNT(kAllVerbs);
static const int kTotalWidth = kNumColumns * (kCellWidth + kCellPad) + kCellPad;
static const int kTotalHeight = kNumRows * (kCellHeight + kCellPad) + kCellPad;

static const int kDblContourNumColums = SK_ARRAY_COUNT(kSomeVerbs) * SK_ARRAY_COUNT(kSomeVerbs);
static const int kDblContourTotalWidth = kDblContourNumColums * (kCellWidth + kCellPad) + kCellPad;

// 50% transparent versions of the colors used for positive/negative triage icons on gold.skia.org
static const SkColor kFailureRed = 0x7FE7298A;
static const SkColor kSuccessGreen = 0x7F1B9E77;

static void draw_zero_length_capped_paths(SkCanvas* canvas, bool aa) {
    canvas->translate(kCellPad, kCellPad);

    SkImageInfo info = canvas->imageInfo().makeWH(kCellWidth, kCellHeight);
    auto surface = canvas->makeSurface(info);
    if (!surface) {
        surface = SkSurface::MakeRasterN32Premul(kCellWidth, kCellHeight);
    }

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setAntiAlias(aa);
    paint.setStyle(SkPaint::kStroke_Style);

    int numFailedTests = 0;
    for (auto cap : kCaps) {
        for (auto width : kWidths) {
            paint.setStrokeCap(cap);
            paint.setStrokeWidth(width);
            canvas->save();

            for (auto verb : kAllVerbs) {
                SkString pathStr;
                pathStr.appendf("M %f %f ", (kCellWidth - 1) * 0.5f, (kCellHeight - 1) * 0.5f);
                if (verb) {
                    pathStr.append(verb);
                }

                SkPath path;
                SkParsePath::FromSVGString(pathStr.c_str(), &path);

                surface->getCanvas()->clear(SK_ColorTRANSPARENT);
                surface->getCanvas()->drawPath(path, paint);
                auto img = surface->makeImageSnapshot();

                // All cases should draw one cap, except for butt capped, and dangling moves
                // (without a verb or close), which shouldn't draw anything.
                int expectedCaps = ((SkPaint::kButt_Cap == cap) || !verb) ? 0 : 1;

                if (!draw_path_cell(canvas, img.get(), expectedCaps)) {
                    ++numFailedTests;
                }
                canvas->translate(kCellWidth + kCellPad, 0);
            }
            canvas->restore();
            canvas->translate(0, kCellHeight + kCellPad);
        }
    }

    canvas->drawColor(numFailedTests > 0 ? kFailureRed : kSuccessGreen);
}

DEF_SIMPLE_GM_BG(zero_length_paths_aa, canvas, kTotalWidth, kTotalHeight, SK_ColorBLACK) {
    draw_zero_length_capped_paths(canvas, true);
}

DEF_SIMPLE_GM_BG(zero_length_paths_bw, canvas, kTotalWidth, kTotalHeight, SK_ColorBLACK) {
    draw_zero_length_capped_paths(canvas, false);
}

static void draw_zero_length_capped_paths_dbl_contour(SkCanvas* canvas, bool aa) {
    canvas->translate(kCellPad, kCellPad);

    SkImageInfo info = canvas->imageInfo().makeWH(kCellWidth, kCellHeight);
    auto surface = canvas->makeSurface(info);
    if (!surface) {
        surface = SkSurface::MakeRasterN32Premul(kCellWidth, kCellHeight);
    }

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setAntiAlias(aa);
    paint.setStyle(SkPaint::kStroke_Style);

    int numFailedTests = 0;
    for (auto cap : kCaps) {
        for (auto width : kWidths) {
            paint.setStrokeCap(cap);
            paint.setStrokeWidth(width);
            canvas->save();

            for (auto firstVerb : kSomeVerbs) {
                for (auto secondVerb : kSomeVerbs) {
                    int expectedCaps = 0;

                    SkString pathStr;
                    pathStr.append("M 9.5 9.5 ");
                    if (firstVerb) {
                        pathStr.append(firstVerb);
                        ++expectedCaps;
                    }
                    pathStr.append("M 40.5 9.5 ");
                    if (secondVerb) {
                        pathStr.append(secondVerb);
                        ++expectedCaps;
                    }

                    SkPath path;
                    SkParsePath::FromSVGString(pathStr.c_str(), &path);

                    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
                    surface->getCanvas()->drawPath(path, paint);
                    auto img = surface->makeImageSnapshot();

                    if (SkPaint::kButt_Cap == cap) {
                        expectedCaps = 0;
                    }

                    if (!draw_path_cell(canvas, img.get(), expectedCaps)) {
                        ++numFailedTests;
                    }
                    canvas->translate(kCellWidth + kCellPad, 0);
                }
            }
            canvas->restore();
            canvas->translate(0, kCellHeight + kCellPad);
        }
    }

    canvas->drawColor(numFailedTests > 0 ? kFailureRed : kSuccessGreen);
}

DEF_SIMPLE_GM_BG(zero_length_paths_dbl_aa, canvas, kDblContourTotalWidth, kTotalHeight,
                 SK_ColorBLACK) {
    draw_zero_length_capped_paths_dbl_contour(canvas, true);
}

DEF_SIMPLE_GM_BG(zero_length_paths_dbl_bw, canvas, kDblContourTotalWidth, kTotalHeight,
                 SK_ColorBLACK) {
    draw_zero_length_capped_paths_dbl_contour(canvas, false);
}
