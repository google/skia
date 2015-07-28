/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkColorFilter.h"
#include "SkMultiPictureDraw.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"

static const SkScalar kRoot3Over2 = 0.86602545f;  // sin(60)
static const SkScalar kRoot3      = 1.73205081f;

static const int kHexSide = 30;
static const int kNumHexX = 6;
static const int kNumHexY = 6;
static const int kPicWidth = kNumHexX * kHexSide;
static const int kPicHeight = SkScalarCeilToInt((kNumHexY - 0.5f) * 2 * kHexSide * kRoot3Over2);
static const SkScalar kInset = 20.0f;
static const int kNumPictures = 4;

static const int kTriSide = 40;

// Create a hexagon centered at (originX, originY)
static SkPath make_hex_path(SkScalar originX, SkScalar originY) {
    SkPath hex;
    hex.moveTo(originX-kHexSide, originY);
    hex.rLineTo(SkScalarHalf(kHexSide), kRoot3Over2 * kHexSide);
    hex.rLineTo(SkIntToScalar(kHexSide), 0);
    hex.rLineTo(SkScalarHalf(kHexSide), -kHexSide * kRoot3Over2);
    hex.rLineTo(-SkScalarHalf(kHexSide), -kHexSide * kRoot3Over2);
    hex.rLineTo(-SkIntToScalar(kHexSide), 0);
    hex.close();
    return hex;
}

// Make a picture that is a tiling of the plane with stroked hexagons where
// each hexagon is in its own layer. The layers are to exercise Ganesh's
// layer hoisting.
static const SkPicture* make_hex_plane_picture(SkColor fillColor) {

    // Create a hexagon with its center at the origin
    SkPath hex = make_hex_path(0, 0);

    SkPaint fill;
    fill.setStyle(SkPaint::kFill_Style);
    fill.setColor(fillColor);

    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(3);

    SkPictureRecorder recorder;
    SkRTreeFactory bbhFactory;

    SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kPicWidth),
                                               SkIntToScalar(kPicHeight),
                                               &bbhFactory,
                                               SkPictureRecorder::kComputeSaveLayerInfo_RecordFlag);

    SkScalar xPos, yPos = 0;

    for (int y = 0; y < kNumHexY; ++y) {
        xPos = 0;

        for (int x = 0; x < kNumHexX; ++x) {
            canvas->saveLayer(NULL, NULL);
            canvas->translate(xPos, yPos + ((x % 2) ? kRoot3Over2 * kHexSide : 0));
            canvas->drawPath(hex, fill);
            canvas->drawPath(hex, stroke);
            canvas->restore();

            xPos += 1.5f * kHexSide;
        }

        yPos += 2 * kHexSide * kRoot3Over2;
    }

    return recorder.endRecording();
}

// Create a picture that consists of a single large layer that is tiled
// with hexagons.
// This is intended to exercise the layer hoisting code's clip handling (in
// tile mode).
static const SkPicture* make_single_layer_hex_plane_picture() {

    // Create a hexagon with its center at the origin
    SkPath hex = make_hex_path(0, 0);

    SkPaint whiteFill;
    whiteFill.setStyle(SkPaint::kFill_Style);
    whiteFill.setColor(SK_ColorWHITE);

    SkPaint greyFill;
    greyFill.setStyle(SkPaint::kFill_Style);
    greyFill.setColor(sk_tool_utils::color_to_565(SK_ColorLTGRAY));

    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(3);

    SkPictureRecorder recorder;
    SkRTreeFactory bbhFactory;

    static const SkScalar kBig = 10000.0f;
    SkCanvas* canvas = recorder.beginRecording(kBig, kBig, &bbhFactory,
                                               SkPictureRecorder::kComputeSaveLayerInfo_RecordFlag);

    canvas->saveLayer(NULL, NULL);

    SkScalar xPos = 0.0f, yPos = 0.0f;

    for (int y = 0; yPos < kBig; ++y) {
        xPos = 0;

        for (int x = 0; xPos < kBig; ++x) {
            canvas->save();
            canvas->translate(xPos, yPos + ((x % 2) ? kRoot3Over2 * kHexSide : 0));
            // The color of the filled hex is swapped to yield a different
            // pattern in each tile. This allows an error in layer hoisting (e.g.,
            // the clip isn't blocking cache reuse) to cause a visual discrepancy.
            canvas->drawPath(hex, ((x+y) % 3) ? whiteFill : greyFill);
            canvas->drawPath(hex, stroke);
            canvas->restore();

            xPos += 1.5f * kHexSide;
        }

        yPos += 2 * kHexSide * kRoot3Over2;
    }

    canvas->restore();

    return recorder.endRecording();
}

// Make an equilateral triangle path with its top corner at (originX, originY)
static SkPath make_tri_path(SkScalar originX, SkScalar originY) {
    SkPath tri;
    tri.moveTo(originX, originY);
    tri.rLineTo(SkScalarHalf(kTriSide), 1.5f * kTriSide / kRoot3);
    tri.rLineTo(-kTriSide, 0);
    tri.close();
    return tri;
}

static const SkPicture* make_tri_picture() {
    SkPath tri = make_tri_path(SkScalarHalf(kTriSide), 0);

    SkPaint fill;
    fill.setStyle(SkPaint::kFill_Style);
    fill.setColor(sk_tool_utils::color_to_565(SK_ColorLTGRAY));

    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(3);

    SkPictureRecorder recorder;
    SkRTreeFactory bbhFactory;

    SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kPicWidth),
                                               SkIntToScalar(kPicHeight),
                                               &bbhFactory,
                                               SkPictureRecorder::kComputeSaveLayerInfo_RecordFlag);
    SkRect r = tri.getBounds();
    r.outset(2.0f, 2.0f);       // outset for stroke
    canvas->clipRect(r);
    // The saveLayer/restore block is to exercise layer hoisting
    canvas->saveLayer(NULL, NULL);
        canvas->drawPath(tri, fill);
        canvas->drawPath(tri, stroke);
    canvas->restore();

    return recorder.endRecording();
}

static const SkPicture* make_sub_picture(const SkPicture* tri) {
    SkPictureRecorder recorder;
    SkRTreeFactory bbhFactory;

    SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kPicWidth),
                                               SkIntToScalar(kPicHeight),
                                               &bbhFactory,
                                               SkPictureRecorder::kComputeSaveLayerInfo_RecordFlag);

    canvas->scale(1.0f/2.0f, 1.0f/2.0f);

    canvas->save();
    canvas->translate(SkScalarHalf(kTriSide), 0);
    canvas->drawPicture(tri);
    canvas->restore();

    canvas->save();
    canvas->translate(SkIntToScalar(kTriSide), 1.5f * kTriSide / kRoot3);
    canvas->drawPicture(tri);
    canvas->restore();

    canvas->save();
    canvas->translate(0, 1.5f * kTriSide / kRoot3);
    canvas->drawPicture(tri);
    canvas->restore();

    return recorder.endRecording();
}

// Create a Sierpinkski-like picture that starts with a top row with a picture
// that just contains a triangle. Subsequent rows take the prior row's picture,
// shrinks it and replicates it 3 times then draws and appropriate number of
// copies of it.
static const SkPicture* make_sierpinski_picture() {
    SkAutoTUnref<const SkPicture> pic(make_tri_picture());

    SkPictureRecorder recorder;
    SkRTreeFactory bbhFactory;

    SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kPicWidth),
                                               SkIntToScalar(kPicHeight),
                                               &bbhFactory,
                                               SkPictureRecorder::kComputeSaveLayerInfo_RecordFlag);

    static const int kNumLevels = 4;
    for (int i = 0; i < kNumLevels; ++i) {
        canvas->save();
            canvas->translate(kPicWidth/2 - (i+1) * (kTriSide/2.0f), 0.0f);
            for (int j = 0; j < i+1; ++j) {
                canvas->drawPicture(pic);
                canvas->translate(SkIntToScalar(kTriSide), 0);
            }
        canvas->restore();

        pic.reset(make_sub_picture(pic));

        canvas->translate(0, 1.5f * kTriSide / kRoot3);
    }

    return recorder.endRecording();
}

static SkSurface* create_compat_surface(SkCanvas* canvas, int width, int height) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);

    SkSurface* surface = canvas->newSurface(info);
    if (NULL == surface) {
        // picture canvas returns NULL so fall back to raster
        surface = SkSurface::NewRaster(info);
    }

    return surface;
}

// This class stores the information required to compose all the result
// fragments potentially generated by the MultiPictureDraw object
class ComposeStep {
public:
    ComposeStep() : fSurf(NULL), fX(0.0f), fY(0.0f), fPaint(NULL) { }
    ~ComposeStep() { SkSafeUnref(fSurf);  SkDELETE(fPaint); }

    SkSurface* fSurf;
    SkScalar   fX;
    SkScalar   fY;
    SkPaint*   fPaint;
};

typedef void (*PFContentMtd)(SkCanvas* canvas, const SkPicture* pictures[kNumPictures]);

// Just a single picture with no clip
static void no_clip(SkCanvas* canvas, const SkPicture* pictures[kNumPictures]) {
    canvas->drawPicture(pictures[0]);
}

// Two pictures with a rect clip on the second one
static void rect_clip(SkCanvas* canvas, const SkPicture* pictures[kNumPictures]) {
    canvas->drawPicture(pictures[0]);

    SkRect rect = pictures[0]->cullRect();
    rect.inset(kInset, kInset);

    canvas->clipRect(rect);

    canvas->drawPicture(pictures[1]);
}

// Two pictures with a round rect clip on the second one
static void rrect_clip(SkCanvas* canvas, const SkPicture* pictures[kNumPictures]) {
    canvas->drawPicture(pictures[0]);

    SkRect rect = pictures[0]->cullRect();
    rect.inset(kInset, kInset);

    SkRRect rrect;
    rrect.setRectXY(rect, kInset, kInset);

    canvas->clipRRect(rrect);

    canvas->drawPicture(pictures[1]);
}

// Two pictures with a clip path on the second one
static void path_clip(SkCanvas* canvas, const SkPicture* pictures[kNumPictures]) {
    canvas->drawPicture(pictures[0]);

    // Create a hexagon centered on the middle of the hex grid
    SkPath hex = make_hex_path((kNumHexX / 2.0f) * kHexSide, kNumHexY * kHexSide * kRoot3Over2);

    canvas->clipPath(hex);

    canvas->drawPicture(pictures[1]);
}

// Two pictures with an inverse clip path on the second one
static void invpath_clip(SkCanvas* canvas, const SkPicture* pictures[kNumPictures]) {
    canvas->drawPicture(pictures[0]);

    // Create a hexagon centered on the middle of the hex grid
    SkPath hex = make_hex_path((kNumHexX / 2.0f) * kHexSide, kNumHexY * kHexSide * kRoot3Over2);
    hex.setFillType(SkPath::kInverseEvenOdd_FillType);

    canvas->clipPath(hex);

    canvas->drawPicture(pictures[1]);
}

// Reuse a single base (triangular) picture a _lot_ (rotated, scaled and translated).
static void sierpinski(SkCanvas* canvas, const SkPicture* pictures[kNumPictures]) {
    canvas->save();
        canvas->drawPicture(pictures[2]);

        canvas->rotate(180.0f);
        canvas->translate(-SkIntToScalar(kPicWidth), -SkIntToScalar(kPicHeight));
        canvas->drawPicture(pictures[2]);
    canvas->restore();
}

static void big_layer(SkCanvas* canvas, const SkPicture* pictures[kNumPictures]) {
    canvas->drawPicture(pictures[3]);
}

static const PFContentMtd gContentMthds[] = {
    no_clip,
    rect_clip,
    rrect_clip,
    path_clip,
    invpath_clip,
    sierpinski,
    big_layer,
};

static void create_content(SkMultiPictureDraw* mpd, PFContentMtd pfGen,
                           const SkPicture* pictures[kNumPictures],
                           SkCanvas* dest, const SkMatrix& xform) {
    SkAutoTUnref<SkPicture> composite;

    {
        SkPictureRecorder recorder;
        SkRTreeFactory bbhFactory;

        SkCanvas* pictureCanvas = recorder.beginRecording(SkIntToScalar(kPicWidth),
                                                          SkIntToScalar(kPicHeight),
                                                          &bbhFactory,
                                                          SkPictureRecorder::kComputeSaveLayerInfo_RecordFlag);

        (*pfGen)(pictureCanvas, pictures);

        composite.reset(recorder.endRecording());
    }

    mpd->add(dest, composite, &xform);
}

typedef void(*PFLayoutMtd)(SkCanvas* finalCanvas, SkMultiPictureDraw* mpd,
                           PFContentMtd pfGen, const SkPicture* pictures[kNumPictures],
                           SkTArray<ComposeStep>* composeSteps);

// Draw the content into a single canvas
static void simple(SkCanvas* finalCanvas, SkMultiPictureDraw* mpd,
                   PFContentMtd pfGen,
                   const SkPicture* pictures[kNumPictures],
                   SkTArray<ComposeStep> *composeSteps) {

    ComposeStep& step = composeSteps->push_back();

    step.fSurf = create_compat_surface(finalCanvas, kPicWidth, kPicHeight);

    SkCanvas* subCanvas = step.fSurf->getCanvas();

    create_content(mpd, pfGen, pictures, subCanvas, SkMatrix::I());
}

// Draw the content into multiple canvases/tiles
static void tiled(SkCanvas* finalCanvas, SkMultiPictureDraw* mpd,
                  PFContentMtd pfGen,
                  const SkPicture* pictures[kNumPictures],
                  SkTArray<ComposeStep> *composeSteps) {
    static const int kNumTilesX = 2;
    static const int kNumTilesY = 2;
    static const int kTileWidth = kPicWidth / kNumTilesX;
    static const int kTileHeight = kPicHeight / kNumTilesY;

    SkASSERT(kPicWidth == kNumTilesX * kTileWidth);
    SkASSERT(kPicHeight == kNumTilesY * kTileHeight);

    static const SkColor colors[kNumTilesX][kNumTilesY] = {
        { SK_ColorCYAN,   SK_ColorMAGENTA },
        { SK_ColorYELLOW, SK_ColorGREEN   }
    };

    for (int y = 0; y < kNumTilesY; ++y) {
        for (int x = 0; x < kNumTilesX; ++x) {
            ComposeStep& step = composeSteps->push_back();

            step.fX = SkIntToScalar(x*kTileWidth);
            step.fY = SkIntToScalar(y*kTileHeight);
            step.fPaint = SkNEW(SkPaint);
            step.fPaint->setColorFilter(
                SkColorFilter::CreateModeFilter(colors[x][y], SkXfermode::kModulate_Mode))->unref();

            step.fSurf = create_compat_surface(finalCanvas, kTileWidth, kTileHeight);

            SkCanvas* subCanvas = step.fSurf->getCanvas();

            const SkMatrix trans = SkMatrix::MakeTrans(-SkIntToScalar(x*kTileWidth), 
                                                       -SkIntToScalar(y*kTileHeight));

            create_content(mpd, pfGen, pictures, subCanvas, trans);
        }
    }
}

static const PFLayoutMtd gLayoutMthds[] = { simple, tiled };

namespace skiagm {
    /**
     * This GM exercises the SkMultiPictureDraw object. It tests the
     * cross product of:
     *      tiled vs. all-at-once rendering (e.g., into many or just 1 canvas)
     *      different clips (e.g., none, rect, rrect)
     *      single vs. multiple pictures (e.g., normal vs. picture-pile-style content)
     */
    class MultiPictureDraw : public GM {
    public:
        enum Content {
            kNoClipSingle_Content,
            kRectClipMulti_Content,
            kRRectClipMulti_Content,
            kPathClipMulti_Content,
            kInvPathClipMulti_Content,
            kSierpinski_Content,
            kBigLayer_Content,

            kLast_Content = kBigLayer_Content
        };

        static const int kContentCnt = kLast_Content + 1;

        enum Layout {
            kSimple_Layout,
            kTiled_Layout,

            kLast_Layout = kTiled_Layout
        };

        static const int kLayoutCnt = kLast_Layout + 1;

        MultiPictureDraw(Content content, Layout layout) : fContent(content), fLayout(layout) {
            SkASSERT(SK_ARRAY_COUNT(gLayoutMthds) == kLayoutCnt);
            SkASSERT(SK_ARRAY_COUNT(gContentMthds) == kContentCnt);

            for (int i = 0; i < kNumPictures; ++i) {
                fPictures[i] = NULL;
            }
        }

        virtual ~MultiPictureDraw() {
            for (int i = 0; i < kNumPictures; ++i) {
                SkSafeUnref(fPictures[i]);
            }
        }

    protected:
        Content          fContent;
        Layout           fLayout;
        const SkPicture* fPictures[kNumPictures];

        void onOnceBeforeDraw() override {
            fPictures[0] = make_hex_plane_picture(SK_ColorWHITE);
            fPictures[1] = make_hex_plane_picture(sk_tool_utils::color_to_565(SK_ColorGRAY));
            fPictures[2] = make_sierpinski_picture();
            fPictures[3] = make_single_layer_hex_plane_picture();
        }

        void onDraw(SkCanvas* canvas) override {
            SkMultiPictureDraw mpd;
            SkTArray<ComposeStep> composeSteps;

            // Fill up the MultiPictureDraw
            (*gLayoutMthds[fLayout])(canvas, &mpd,
                                     gContentMthds[fContent],
                                     fPictures, &composeSteps);

            mpd.draw();

            // Compose all the drawn canvases into the final canvas
            for (int i = 0; i < composeSteps.count(); ++i) {
                const ComposeStep& step = composeSteps[i];

                SkAutoTUnref<SkImage> image(step.fSurf->newImageSnapshot());

                canvas->drawImage(image, step.fX, step.fY, step.fPaint);
            }
        }

        SkISize onISize() override { return SkISize::Make(kPicWidth, kPicHeight); }

        SkString onShortName() override {
            static const char* gContentNames[] = {
                "noclip", "rectclip", "rrectclip", "pathclip", 
                "invpathclip", "sierpinski", "biglayer"
            };
            static const char* gLayoutNames[] = { "simple", "tiled" };

            SkASSERT(SK_ARRAY_COUNT(gLayoutNames) == kLayoutCnt);
            SkASSERT(SK_ARRAY_COUNT(gContentNames) == kContentCnt);

            SkString name("multipicturedraw_");

            name.append(gContentNames[fContent]);
            name.append("_");
            name.append(gLayoutNames[fLayout]);
            return name;
        }

        bool runAsBench() const override { return true; }

    private:
        typedef GM INHERITED;
    };

    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kNoClipSingle_Content,
                                                MultiPictureDraw::kSimple_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kRectClipMulti_Content,
                                                MultiPictureDraw::kSimple_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kRRectClipMulti_Content,
                                                MultiPictureDraw::kSimple_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kPathClipMulti_Content,
                                                MultiPictureDraw::kSimple_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kInvPathClipMulti_Content,
                                                MultiPictureDraw::kSimple_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kSierpinski_Content,
                                                MultiPictureDraw::kSimple_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kBigLayer_Content,
                                                MultiPictureDraw::kSimple_Layout));)

    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kNoClipSingle_Content,
                                                MultiPictureDraw::kTiled_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kRectClipMulti_Content,
                                                MultiPictureDraw::kTiled_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kRRectClipMulti_Content,
                                                MultiPictureDraw::kTiled_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kPathClipMulti_Content,
                                                MultiPictureDraw::kTiled_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kInvPathClipMulti_Content,
                                                MultiPictureDraw::kTiled_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kSierpinski_Content,
                                                MultiPictureDraw::kTiled_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kBigLayer_Content,
                                                MultiPictureDraw::kTiled_Layout));)
}
