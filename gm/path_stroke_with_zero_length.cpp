/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkAutoPixmapStorage.h"

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

static void draw_path_cell(SkCanvas* canvas,
                           SkSurface* surface,
                           int expectedCaps) {
    static const SkColor kFailureRed = 0x7F7F0000;
    static const SkColor kFailureYellow = 0x7F7F7F00;
    static const SkColor kSuccessGreen = 0x7F007f00;

    int w = surface->width(), h = surface->height();

    // Read the pixels back
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    SkAutoPixmapStorage pmap;
    pmap.alloc(info);
    if (!surface->readPixels(pmap, 0, 0)) {
        return;
    }

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

    SkPaint result;

    if (numBlobs == expectedCaps) {
        result.setColor(kSuccessGreen); // Green
    } else if (numBlobs > expectedCaps) {
        result.setColor(kFailureYellow); // Yellow -- more geometry than expected
    } else {
        result.setColor(kFailureRed); // Red -- missing some geometry
    }

    auto img = surface->makeImageSnapshot();
    canvas->drawImage(img, 0, 0);
    canvas->drawRect(SkRect::MakeWH(w, h), result);
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

static const int kNumRows = std::size(kCaps) * std::size(kWidths);
static const int kNumColumns = std::size(kAllVerbs);
static const int kTotalWidth = kNumColumns * (kCellWidth + kCellPad) + kCellPad;
static const int kTotalHeight = kNumRows * (kCellHeight + kCellPad) + kCellPad;

static const int kDblContourNumColums = std::size(kSomeVerbs) * std::size(kSomeVerbs);
static const int kDblContourTotalWidth = kDblContourNumColums * (kCellWidth + kCellPad) + kCellPad;

static skiagm::DrawResult draw_zero_length_capped_paths(SkCanvas* canvas, bool aa,
                                                        SkString* errorMsg) {
    canvas->translate(kCellPad, kCellPad);

    SkImageInfo info = canvas->imageInfo().makeWH(kCellWidth, kCellHeight);
    auto surface = canvas->makeSurface(info);
    if (!surface) {
        surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(kCellWidth, kCellHeight));
    }

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setAntiAlias(aa);
    paint.setStyle(SkPaint::kStroke_Style);

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

                // All cases should draw one cap, except for butt capped, and dangling moves
                // (without a verb or close), which shouldn't draw anything.
                int expectedCaps = ((SkPaint::kButt_Cap == cap) || !verb) ? 0 : 1;

                draw_path_cell(canvas, surface.get(), expectedCaps);
                canvas->translate(kCellWidth + kCellPad, 0);
            }
            canvas->restore();
            canvas->translate(0, kCellHeight + kCellPad);
        }
    }

    return skiagm::DrawResult::kOk;
}

DEF_SIMPLE_GM_BG_CAN_FAIL(zero_length_paths_aa, canvas, errorMsg,
                          kTotalWidth, kTotalHeight, SK_ColorBLACK) {
    return draw_zero_length_capped_paths(canvas, true, errorMsg);
}

DEF_SIMPLE_GM_BG_CAN_FAIL(zero_length_paths_bw, canvas, errorMsg,
                          kTotalWidth, kTotalHeight, SK_ColorBLACK) {
    return draw_zero_length_capped_paths(canvas, false, errorMsg);
}

static skiagm::DrawResult draw_zero_length_capped_paths_dbl_contour(SkCanvas* canvas, bool aa,
                                                                    SkString* errorMsg) {
    auto rContext = canvas->recordingContext();
    auto dContext = GrAsDirectContext(rContext);

    if (!dContext && rContext) {
        *errorMsg = "Not supported in DDL mode";
        return skiagm::DrawResult::kSkip;
    }
    canvas->translate(kCellPad, kCellPad);

    SkImageInfo info = canvas->imageInfo().makeWH(kCellWidth, kCellHeight);
    auto surface = canvas->makeSurface(info);
    if (!surface) {
        surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(kCellWidth, kCellHeight));
    }

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setAntiAlias(aa);
    paint.setStyle(SkPaint::kStroke_Style);

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

                    if (SkPaint::kButt_Cap == cap) {
                        expectedCaps = 0;
                    }

                    draw_path_cell(canvas, surface.get(), expectedCaps);
                    canvas->translate(kCellWidth + kCellPad, 0);
                }
            }
            canvas->restore();
            canvas->translate(0, kCellHeight + kCellPad);
        }
    }

    return skiagm::DrawResult::kOk;
}

DEF_SIMPLE_GM_BG_CAN_FAIL(zero_length_paths_dbl_aa, canvas, errorMsg,
                          kDblContourTotalWidth, kTotalHeight, SK_ColorBLACK) {
    return draw_zero_length_capped_paths_dbl_contour(canvas, true, errorMsg);
}

DEF_SIMPLE_GM_BG_CAN_FAIL(zero_length_paths_dbl_bw, canvas, errorMsg,
                          kDblContourTotalWidth, kTotalHeight, SK_ColorBLACK) {
    return draw_zero_length_capped_paths_dbl_contour(canvas, false, errorMsg);
}
