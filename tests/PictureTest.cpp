/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkDashPathEffect.h"
#include "SkData.h"
#include "SkDecodingImageGenerator.h"
#include "SkError.h"
#if SK_SUPPORT_GPU
#include "SkGpuDevice.h"
#endif
#include "SkImageEncoder.h"
#include "SkImageGenerator.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPictureUtils.h"
#include "SkRRect.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkStream.h"

#if SK_SUPPORT_GPU
#include "SkSurface.h"
#include "GrContextFactory.h"
#include "GrPictureUtils.h"
#endif
#include "Test.h"

#include "SkLumaColorFilter.h"
#include "SkColorFilterImageFilter.h"

static const int gColorScale = 30;
static const int gColorOffset = 60;

static void make_bm(SkBitmap* bm, int w, int h, SkColor color, bool immutable) {
    bm->allocN32Pixels(w, h);
    bm->eraseColor(color);
    if (immutable) {
        bm->setImmutable();
    }
}

static void make_checkerboard(SkBitmap* bm, int w, int h, bool immutable) {
    SkASSERT(w % 2 == 0);
    SkASSERT(h % 2 == 0);
    bm->allocPixels(SkImageInfo::Make(w, h, kAlpha_8_SkColorType,
                                      kPremul_SkAlphaType));
    SkAutoLockPixels lock(*bm);
    for (int y = 0; y < h; y += 2) {
        uint8_t* s = bm->getAddr8(0, y);
        for (int x = 0; x < w; x += 2) {
            *s++ = 0xFF;
            *s++ = 0x00;
        }
        s = bm->getAddr8(0, y + 1);
        for (int x = 0; x < w; x += 2) {
            *s++ = 0x00;
            *s++ = 0xFF;
        }
    }
    if (immutable) {
        bm->setImmutable();
    }
}

static void init_paint(SkPaint* paint, const SkBitmap &bm) {
    SkShader* shader = SkShader::CreateBitmapShader(bm,
                                                    SkShader::kClamp_TileMode,
                                                    SkShader::kClamp_TileMode);
    paint->setShader(shader)->unref();
}

typedef void (*DrawBitmapProc)(SkCanvas*, const SkBitmap&,
                               const SkBitmap&, const SkPoint&,
                               SkTDArray<SkPixelRef*>* usedPixRefs);

static void drawpaint_proc(SkCanvas* canvas, const SkBitmap& bm,
                           const SkBitmap& altBM, const SkPoint& pos,
                           SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    canvas->drawPaint(paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawpoints_proc(SkCanvas* canvas, const SkBitmap& bm,
                            const SkBitmap& altBM, const SkPoint& pos,
                            SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    // draw a rect
    SkPoint points[5] = {
        { pos.fX, pos.fY },
        { pos.fX + bm.width() - 1, pos.fY },
        { pos.fX + bm.width() - 1, pos.fY + bm.height() - 1 },
        { pos.fX, pos.fY + bm.height() - 1 },
        { pos.fX, pos.fY },
    };

    canvas->drawPoints(SkCanvas::kPolygon_PointMode, 5, points, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawrect_proc(SkCanvas* canvas, const SkBitmap& bm,
                          const SkBitmap& altBM, const SkPoint& pos,
                          SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkRect r = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
    r.offset(pos.fX, pos.fY);

    canvas->drawRect(r, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawoval_proc(SkCanvas* canvas, const SkBitmap& bm,
                          const SkBitmap& altBM, const SkPoint& pos,
                          SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkRect r = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
    r.offset(pos.fX, pos.fY);

    canvas->drawOval(r, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawrrect_proc(SkCanvas* canvas, const SkBitmap& bm,
                           const SkBitmap& altBM, const SkPoint& pos,
                           SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkRect r = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
    r.offset(pos.fX, pos.fY);

    SkRRect rr;
    rr.setRectXY(r, SkIntToScalar(bm.width())/4, SkIntToScalar(bm.height())/4);
    canvas->drawRRect(rr, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawpath_proc(SkCanvas* canvas, const SkBitmap& bm,
                          const SkBitmap& altBM, const SkPoint& pos,
                          SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkPath path;
    path.lineTo(bm.width()/2.0f, SkIntToScalar(bm.height()));
    path.lineTo(SkIntToScalar(bm.width()), 0);
    path.close();
    path.offset(pos.fX, pos.fY);

    canvas->drawPath(path, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawbitmap_proc(SkCanvas* canvas, const SkBitmap& bm,
                            const SkBitmap& altBM, const SkPoint& pos,
                            SkTDArray<SkPixelRef*>* usedPixRefs) {
    canvas->drawBitmap(bm, pos.fX, pos.fY, NULL);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawbitmap_withshader_proc(SkCanvas* canvas, const SkBitmap& bm,
                                       const SkBitmap& altBM, const SkPoint& pos,
                                       SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    // The bitmap in the paint is ignored unless we're drawing an A8 bitmap
    canvas->drawBitmap(altBM, pos.fX, pos.fY, &paint);
    *usedPixRefs->append() = bm.pixelRef();
    *usedPixRefs->append() = altBM.pixelRef();
}

static void drawsprite_proc(SkCanvas* canvas, const SkBitmap& bm,
                            const SkBitmap& altBM, const SkPoint& pos,
                            SkTDArray<SkPixelRef*>* usedPixRefs) {
    const SkMatrix& ctm = canvas->getTotalMatrix();

    SkPoint p(pos);
    ctm.mapPoints(&p, 1);

    canvas->drawSprite(bm, (int)p.fX, (int)p.fY, NULL);
    *usedPixRefs->append() = bm.pixelRef();
}

#if 0
// Although specifiable, this case doesn't seem to make sense (i.e., the
// bitmap in the shader is never used).
static void drawsprite_withshader_proc(SkCanvas* canvas, const SkBitmap& bm,
                                       const SkBitmap& altBM, const SkPoint& pos,
                                       SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    const SkMatrix& ctm = canvas->getTotalMatrix();

    SkPoint p(pos);
    ctm.mapPoints(&p, 1);

    canvas->drawSprite(altBM, (int)p.fX, (int)p.fY, &paint);
    *usedPixRefs->append() = bm.pixelRef();
    *usedPixRefs->append() = altBM.pixelRef();
}
#endif

static void drawbitmaprect_proc(SkCanvas* canvas, const SkBitmap& bm,
                                const SkBitmap& altBM, const SkPoint& pos,
                                SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkRect r = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };

    r.offset(pos.fX, pos.fY);
    canvas->drawBitmapRectToRect(bm, NULL, r, NULL);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawbitmaprect_withshader_proc(SkCanvas* canvas,
                                           const SkBitmap& bm,
                                           const SkBitmap& altBM,
                                           const SkPoint& pos,
                                           SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkRect r = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
    r.offset(pos.fX, pos.fY);

    // The bitmap in the paint is ignored unless we're drawing an A8 bitmap
    canvas->drawBitmapRectToRect(altBM, NULL, r, &paint);
    *usedPixRefs->append() = bm.pixelRef();
    *usedPixRefs->append() = altBM.pixelRef();
}

static void drawtext_proc(SkCanvas* canvas, const SkBitmap& bm,
                          const SkBitmap& altBM, const SkPoint& pos,
                          SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);
    paint.setTextSize(SkIntToScalar(1.5*bm.width()));

    canvas->drawText("0", 1, pos.fX, pos.fY+bm.width(), paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawpostext_proc(SkCanvas* canvas, const SkBitmap& bm,
                             const SkBitmap& altBM, const SkPoint& pos,
                             SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);
    paint.setTextSize(SkIntToScalar(1.5*bm.width()));

    SkPoint point = { pos.fX, pos.fY + bm.height() };
    canvas->drawPosText("O", 1, &point, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawtextonpath_proc(SkCanvas* canvas, const SkBitmap& bm,
                                const SkBitmap& altBM, const SkPoint& pos,
                                SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;

    init_paint(&paint, bm);
    paint.setTextSize(SkIntToScalar(1.5*bm.width()));

    SkPath path;
    path.lineTo(SkIntToScalar(bm.width()), 0);
    path.offset(pos.fX, pos.fY+bm.height());

    canvas->drawTextOnPath("O", 1, path, NULL, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawverts_proc(SkCanvas* canvas, const SkBitmap& bm,
                           const SkBitmap& altBM, const SkPoint& pos,
                           SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkPoint verts[4] = {
        { pos.fX, pos.fY },
        { pos.fX + bm.width(), pos.fY },
        { pos.fX + bm.width(), pos.fY + bm.height() },
        { pos.fX, pos.fY + bm.height() }
    };
    SkPoint texs[4] = { { 0, 0 },
                        { SkIntToScalar(bm.width()), 0 },
                        { SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) },
                        { 0, SkIntToScalar(bm.height()) } };
    uint16_t indices[6] = { 0, 1, 2, 0, 2, 3 };

    canvas->drawVertices(SkCanvas::kTriangles_VertexMode, 4, verts, texs, NULL, NULL,
                         indices, 6, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

// Return a picture with the bitmaps drawn at the specified positions.
static SkPicture* record_bitmaps(const SkBitmap bm[],
                                 const SkPoint pos[],
                                 SkTDArray<SkPixelRef*> analytic[],
                                 int count,
                                 DrawBitmapProc proc) {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(1000, 1000);
    for (int i = 0; i < count; ++i) {
        analytic[i].rewind();
        canvas->save();
        SkRect clipRect = SkRect::MakeXYWH(pos[i].fX, pos[i].fY,
                                           SkIntToScalar(bm[i].width()),
                                           SkIntToScalar(bm[i].height()));
        canvas->clipRect(clipRect, SkRegion::kIntersect_Op);
        proc(canvas, bm[i], bm[count+i], pos[i], &analytic[i]);
        canvas->restore();
    }
    return recorder.endRecording();
}

static void rand_rect(SkRect* rect, SkRandom& rand, SkScalar W, SkScalar H) {
    rect->fLeft   = rand.nextRangeScalar(-W, 2*W);
    rect->fTop    = rand.nextRangeScalar(-H, 2*H);
    rect->fRight  = rect->fLeft + rand.nextRangeScalar(0, W);
    rect->fBottom = rect->fTop + rand.nextRangeScalar(0, H);

    // we integralize rect to make our tests more predictable, since Gather is
    // a little sloppy.
    SkIRect ir;
    rect->round(&ir);
    rect->set(ir);
}

static void draw(SkPicture* pic, int width, int height, SkBitmap* result) {
    make_bm(result, width, height, SK_ColorBLACK, false);

    SkCanvas canvas(*result);
    canvas.drawPicture(pic);
}

template <typename T> int find_index(const T* array, T elem, int count) {
    for (int i = 0; i < count; ++i) {
        if (array[i] == elem) {
            return i;
        }
    }
    return -1;
}

// Return true if 'ref' is found in array[]
static bool find(SkPixelRef const * const * array, SkPixelRef const * ref, int count) {
    return find_index<const SkPixelRef*>(array, ref, count) >= 0;
}

// Look at each pixel that is inside 'subset', and if its color appears in
// colors[], find the corresponding value in refs[] and append that ref into
// array, skipping duplicates of the same value.
// Note that gathering pixelRefs from rendered colors suffers from the problem
// that multiple simultaneous textures (e.g., A8 for alpha and 8888 for color)
// isn't easy to reconstruct.
static void gather_from_image(const SkBitmap& bm, SkPixelRef* const refs[],
                              int count, SkTDArray<SkPixelRef*>* array,
                              const SkRect& subset) {
    SkIRect ir;
    subset.roundOut(&ir);

    if (!ir.intersect(0, 0, bm.width()-1, bm.height()-1)) {
        return;
    }

    // Since we only want to return unique values in array, when we scan we just
    // set a bit for each index'd color found. In practice we only have a few
    // distinct colors, so we just use an int's bits as our array. Hence the
    // assert that count <= number-of-bits-in-our-int.
    SkASSERT((unsigned)count <= 32);
    uint32_t bitarray = 0;

    SkAutoLockPixels alp(bm);

    for (int y = ir.fTop; y < ir.fBottom; ++y) {
        for (int x = ir.fLeft; x < ir.fRight; ++x) {
            SkPMColor pmc = *bm.getAddr32(x, y);
            // the only good case where the color is not found would be if
            // the color is transparent, meaning no bitmap was drawn in that
            // pixel.
            if (pmc) {
                uint32_t index = SkGetPackedR32(pmc);
                SkASSERT(SkGetPackedG32(pmc) == index);
                SkASSERT(SkGetPackedB32(pmc) == index);
                if (0 == index) {
                    continue;           // background color
                }
                SkASSERT(0 == (index - gColorOffset) % gColorScale);
                index = (index - gColorOffset) / gColorScale;
                SkASSERT(static_cast<int>(index) < count);
                bitarray |= 1 << index;
            }
        }
    }

    for (int i = 0; i < count; ++i) {
        if (bitarray & (1 << i)) {
            *array->append() = refs[i];
        }
    }
}

static void gather_from_analytic(const SkPoint pos[], SkScalar w, SkScalar h,
                                 const SkTDArray<SkPixelRef*> analytic[],
                                 int count,
                                 SkTDArray<SkPixelRef*>* result,
                                 const SkRect& subset) {
    for (int i = 0; i < count; ++i) {
        SkRect rect = SkRect::MakeXYWH(pos[i].fX, pos[i].fY, w, h);

        if (SkRect::Intersects(subset, rect)) {
            result->append(analytic[i].count(), analytic[i].begin());
        }
    }
}


static const struct {
    const DrawBitmapProc proc;
    const char* const desc;
} gProcs[] = {
    {drawpaint_proc, "drawpaint"},
    {drawpoints_proc, "drawpoints"},
    {drawrect_proc, "drawrect"},
    {drawoval_proc, "drawoval"},
    {drawrrect_proc, "drawrrect"},
    {drawpath_proc, "drawpath"},
    {drawbitmap_proc, "drawbitmap"},
    {drawbitmap_withshader_proc, "drawbitmap_withshader"},
    {drawsprite_proc, "drawsprite"},
#if 0
    {drawsprite_withshader_proc, "drawsprite_withshader"},
#endif
    {drawbitmaprect_proc, "drawbitmaprect"},
    {drawbitmaprect_withshader_proc, "drawbitmaprect_withshader"},
    {drawtext_proc, "drawtext"},
    {drawpostext_proc, "drawpostext"},
    {drawtextonpath_proc, "drawtextonpath"},
    {drawverts_proc, "drawverts"},
};

static void create_textures(SkBitmap* bm, SkPixelRef** refs, int num, int w, int h) {
    // Our convention is that the color components contain an encoding of
    // the index of their corresponding bitmap/pixelref. (0,0,0,0) is
    // reserved for the background
    for (int i = 0; i < num; ++i) {
        make_bm(&bm[i], w, h,
                SkColorSetARGB(0xFF,
                               gColorScale*i+gColorOffset,
                               gColorScale*i+gColorOffset,
                               gColorScale*i+gColorOffset),
                true);
        refs[i] = bm[i].pixelRef();
    }

    // The A8 alternate bitmaps are all BW checkerboards
    for (int i = 0; i < num; ++i) {
        make_checkerboard(&bm[num+i], w, h, true);
        refs[num+i] = bm[num+i].pixelRef();
    }
}

static void test_gatherpixelrefs(skiatest::Reporter* reporter) {
    const int IW = 32;
    const int IH = IW;
    const SkScalar W = SkIntToScalar(IW);
    const SkScalar H = W;

    static const int N = 4;
    SkBitmap bm[2*N];
    SkPixelRef* refs[2*N];
    SkTDArray<SkPixelRef*> analytic[N];

    const SkPoint pos[N] = {
        { 0, 0 }, { W, 0 }, { 0, H }, { W, H }
    };

    create_textures(bm, refs, N, IW, IH);

    SkRandom rand;
    for (size_t k = 0; k < SK_ARRAY_COUNT(gProcs); ++k) {
        SkAutoTUnref<SkPicture> pic(
            record_bitmaps(bm, pos, analytic, N, gProcs[k].proc));

        REPORTER_ASSERT(reporter, pic->willPlayBackBitmaps() || N == 0);
        // quick check for a small piece of each quadrant, which should just
        // contain 1 or 2 bitmaps.
        for (size_t  i = 0; i < SK_ARRAY_COUNT(pos); ++i) {
            SkRect r;
            r.set(2, 2, W - 2, H - 2);
            r.offset(pos[i].fX, pos[i].fY);
            SkAutoDataUnref data(SkPictureUtils::GatherPixelRefs(pic, r));
            if (!data) {
                ERRORF(reporter, "SkPictureUtils::GatherPixelRefs returned "
                       "NULL for %s.", gProcs[k].desc);
                continue;
            }
            SkPixelRef** gatheredRefs = (SkPixelRef**)data->data();
            int count = static_cast<int>(data->size() / sizeof(SkPixelRef*));
            REPORTER_ASSERT(reporter, 1 == count || 2 == count);
            if (1 == count) {
                REPORTER_ASSERT(reporter, gatheredRefs[0] == refs[i]);
            } else if (2 == count) {
                REPORTER_ASSERT(reporter,
                    (gatheredRefs[0] == refs[i] && gatheredRefs[1] == refs[i+N]) ||
                    (gatheredRefs[1] == refs[i] && gatheredRefs[0] == refs[i+N]));
            }
        }

        SkBitmap image;
        draw(pic, 2*IW, 2*IH, &image);

        // Test a bunch of random (mostly) rects, and compare the gather results
        // with a deduced list of refs by looking at the colors drawn.
        for (int j = 0; j < 100; ++j) {
            SkRect r;
            rand_rect(&r, rand, 2*W, 2*H);

            SkTDArray<SkPixelRef*> fromImage;
            gather_from_image(image, refs, N, &fromImage, r);

            SkTDArray<SkPixelRef*> fromAnalytic;
            gather_from_analytic(pos, W, H, analytic, N, &fromAnalytic, r);

            SkData* data = SkPictureUtils::GatherPixelRefs(pic, r);
            size_t dataSize = data ? data->size() : 0;
            int gatherCount = static_cast<int>(dataSize / sizeof(SkPixelRef*));
            SkASSERT(gatherCount * sizeof(SkPixelRef*) == dataSize);
            SkPixelRef** gatherRefs = data ? (SkPixelRef**)(data->data()) : NULL;
            SkAutoDataUnref adu(data);

            // Everything that we saw drawn should appear in the analytic list
            // but the analytic list may contain some pixelRefs that were not
            // seen in the image (e.g., A8 textures used as masks)
            for (int i = 0; i < fromImage.count(); ++i) {
                if (-1 == fromAnalytic.find(fromImage[i])) {
                    ERRORF(reporter, "PixelRef missing %d %s",
                           i, gProcs[k].desc);
                }
            }

            /*
             *  GatherPixelRefs is conservative, so it can return more bitmaps
             *  than are strictly required. Thus our check here is only that
             *  Gather didn't miss any that we actually needed. Even that isn't
             *  a strict requirement on Gather, which is meant to be quick and
             *  only mostly-correct, but at the moment this test should work.
             */
            for (int i = 0; i < fromAnalytic.count(); ++i) {
                bool found = find(gatherRefs, fromAnalytic[i], gatherCount);
                if (!found) {
                    ERRORF(reporter, "PixelRef missing %d %s",
                           i, gProcs[k].desc);
                }
#if 0
                // enable this block of code to debug failures, as it will rerun
                // the case that failed.
                if (!found) {
                    SkData* data = SkPictureUtils::GatherPixelRefs(pic, r);
                    size_t dataSize = data ? data->size() : 0;
                }
#endif
            }
        }
    }
}

static void test_gatherpixelrefsandrects(skiatest::Reporter* reporter) {
    const int IW = 32;
    const int IH = IW;
    const SkScalar W = SkIntToScalar(IW);
    const SkScalar H = W;

    static const int N = 4;
    SkBitmap bm[2*N];
    SkPixelRef* refs[2*N];
    SkTDArray<SkPixelRef*> analytic[N];

    const SkPoint pos[N] = {
        { 0, 0 }, { W, 0 }, { 0, H }, { W, H }
    };

    create_textures(bm, refs, N, IW, IH);

    SkRandom rand;
    for (size_t k = 0; k < SK_ARRAY_COUNT(gProcs); ++k) {
        SkAutoTUnref<SkPicture> pic(
            record_bitmaps(bm, pos, analytic, N, gProcs[k].proc));

        REPORTER_ASSERT(reporter, pic->willPlayBackBitmaps() || N == 0);

        SkAutoTUnref<SkPictureUtils::SkPixelRefContainer> prCont(
                                new SkPictureUtils::SkPixelRefsAndRectsList);

        SkPictureUtils::GatherPixelRefsAndRects(pic, prCont);

        // quick check for a small piece of each quadrant, which should just
        // contain 1 or 2 bitmaps.
        for (size_t  i = 0; i < SK_ARRAY_COUNT(pos); ++i) {
            SkRect r;
            r.set(2, 2, W - 2, H - 2);
            r.offset(pos[i].fX, pos[i].fY);

            SkTDArray<SkPixelRef*> gatheredRefs;
            prCont->query(r, &gatheredRefs);

            int count = gatheredRefs.count();
            REPORTER_ASSERT(reporter, 1 == count || 2 == count);
            if (1 == count) {
                REPORTER_ASSERT(reporter, gatheredRefs[0] == refs[i]);
            } else if (2 == count) {
                REPORTER_ASSERT(reporter,
                    (gatheredRefs[0] == refs[i] && gatheredRefs[1] == refs[i+N]) ||
                    (gatheredRefs[1] == refs[i] && gatheredRefs[0] == refs[i+N]));
            }
        }

        SkBitmap image;
        draw(pic, 2*IW, 2*IH, &image);

        // Test a bunch of random (mostly) rects, and compare the gather results
        // with the analytic results and the pixel refs seen in a rendering.
        for (int j = 0; j < 100; ++j) {
            SkRect r;
            rand_rect(&r, rand, 2*W, 2*H);

            SkTDArray<SkPixelRef*> fromImage;
            gather_from_image(image, refs, N, &fromImage, r);

            SkTDArray<SkPixelRef*> fromAnalytic;
            gather_from_analytic(pos, W, H, analytic, N, &fromAnalytic, r);

            SkTDArray<SkPixelRef*> gatheredRefs;
            prCont->query(r, &gatheredRefs);

            // Everything that we saw drawn should appear in the analytic list
            // but the analytic list may contain some pixelRefs that were not
            // seen in the image (e.g., A8 textures used as masks)
            for (int i = 0; i < fromImage.count(); ++i) {
                REPORTER_ASSERT(reporter, -1 != fromAnalytic.find(fromImage[i]));
            }

            // Everything in the analytic list should appear in the gathered
            // list.
            for (int i = 0; i < fromAnalytic.count(); ++i) {
                REPORTER_ASSERT(reporter, -1 != gatheredRefs.find(fromAnalytic[i]));
            }
        }
    }
}

#ifdef SK_DEBUG
// Ensure that deleting SkPicturePlayback does not assert. Asserts only fire in debug mode, so only
// run in debug mode.
static void test_deleting_empty_playback() {
    SkPictureRecorder recorder;
    // Creates an SkPictureRecord
    recorder.beginRecording(0, 0);
    // Turns that into an SkPicturePlayback
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());
    // Deletes the old SkPicturePlayback, and creates a new SkPictureRecord
    recorder.beginRecording(0, 0);
}

// Ensure that serializing an empty picture does not assert. Likewise only runs in debug mode.
static void test_serializing_empty_picture() {
    SkPictureRecorder recorder;
    recorder.beginRecording(0, 0);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());
    SkDynamicMemoryWStream stream;
    picture->serialize(&stream);
}
#endif

static void rand_op(SkCanvas* canvas, SkRandom& rand) {
    SkPaint paint;
    SkRect rect = SkRect::MakeWH(50, 50);

    SkScalar unit = rand.nextUScalar1();
    if (unit <= 0.3) {
//        SkDebugf("save\n");
        canvas->save();
    } else if (unit <= 0.6) {
//        SkDebugf("restore\n");
        canvas->restore();
    } else if (unit <= 0.9) {
//        SkDebugf("clip\n");
        canvas->clipRect(rect);
    } else {
//        SkDebugf("draw\n");
        canvas->drawPaint(paint);
    }
}

#if SK_SUPPORT_GPU
static void test_gpu_veto(skiatest::Reporter* reporter) {

    SkPictureRecorder recorder;

    SkCanvas* canvas = recorder.beginRecording(100, 100);
    {
        SkPath path;
        path.moveTo(0, 0);
        path.lineTo(50, 50);

        SkScalar intervals[] = { 1.0f, 1.0f };
        SkAutoTUnref<SkDashPathEffect> dash(SkDashPathEffect::Create(intervals, 2, 0));

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setPathEffect(dash);

        canvas->drawPath(path, paint);
    }
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());
    // path effects currently render an SkPicture undesireable for GPU rendering

    const char *reason = NULL;
    REPORTER_ASSERT(reporter, !picture->suitableForGpuRasterization(NULL, &reason));
    REPORTER_ASSERT(reporter, NULL != reason);

    canvas = recorder.beginRecording(100, 100);
    {
        SkPath path;

        path.moveTo(0, 0);
        path.lineTo(0, 50);
        path.lineTo(25, 25);
        path.lineTo(50, 50);
        path.lineTo(50, 0);
        path.close();
        REPORTER_ASSERT(reporter, !path.isConvex());

        SkPaint paint;
        paint.setAntiAlias(true);
        for (int i = 0; i < 50; ++i) {
            canvas->drawPath(path, paint);
        }
    }
    picture.reset(recorder.endRecording());
    // A lot of AA concave paths currently render an SkPicture undesireable for GPU rendering
    REPORTER_ASSERT(reporter, !picture->suitableForGpuRasterization(NULL));

    canvas = recorder.beginRecording(100, 100);
    {
        SkPath path;

        path.moveTo(0, 0);
        path.lineTo(0, 50);
        path.lineTo(25, 25);
        path.lineTo(50, 50);
        path.lineTo(50, 0);
        path.close();
        REPORTER_ASSERT(reporter, !path.isConvex());

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(0);
        for (int i = 0; i < 50; ++i) {
            canvas->drawPath(path, paint);
        }
    }
    picture.reset(recorder.endRecording());
    // hairline stroked AA concave paths are fine for GPU rendering
    REPORTER_ASSERT(reporter, picture->suitableForGpuRasterization(NULL));
}

static void test_gpu_picture_optimization(skiatest::Reporter* reporter,
                                          GrContextFactory* factory) {

    GrContext* context = factory->get(GrContextFactory::kNative_GLContextType);

    static const int kWidth = 100;
    static const int kHeight = 100;

    SkAutoTUnref<SkPicture> pict;

    // create a picture with the structure:
    // 1)
    //      SaveLayer
    //      Restore
    // 2)
    //      SaveLayer
    //          Translate
    //          SaveLayer w/ bound
    //          Restore
    //      Restore
    // 3)
    //      SaveLayer w/ copyable paint
    //      Restore
    // 4)
    //      SaveLayer w/ non-copyable paint
    //      Restore
    {
        SkPictureRecorder recorder;

        SkCanvas* c = recorder.beginRecording(kWidth, kHeight);
        // 1)
        c->saveLayer(NULL, NULL);
        c->restore();

        // 2)
        c->saveLayer(NULL, NULL);
            c->translate(kWidth/2, kHeight/2);
            SkRect r = SkRect::MakeXYWH(0, 0, kWidth/2, kHeight/2);
            c->saveLayer(&r, NULL);
            c->restore();
        c->restore();

        // 3)
        {
            SkPaint p;
            p.setColor(SK_ColorRED);
            c->saveLayer(NULL, &p);
            c->restore();
        }
        // 4)
        // TODO: this case will need to be removed once the paint's are immutable
        {
            SkPaint p;
            SkAutoTUnref<SkColorFilter> cf(SkLumaColorFilter::Create());
            p.setImageFilter(SkColorFilterImageFilter::Create(cf.get()))->unref();
            c->saveLayer(NULL, &p);
            c->restore();
        }

        pict.reset(recorder.endRecording());
    }

    // Now test out the SaveLayer extraction
    {
        SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);

        SkAutoTUnref<SkSurface> surface(SkSurface::NewScratchRenderTarget(context, info));

        SkCanvas* canvas = surface->getCanvas();

        canvas->EXPERIMENTAL_optimize(pict);

        SkPicture::AccelData::Key key = GPUAccelData::ComputeAccelDataKey();

        const SkPicture::AccelData* data = pict->EXPERIMENTAL_getAccelData(key);
        REPORTER_ASSERT(reporter, NULL != data);

        const GPUAccelData *gpuData = static_cast<const GPUAccelData*>(data);
        REPORTER_ASSERT(reporter, 5 == gpuData->numSaveLayers());

        const GPUAccelData::SaveLayerInfo& info0 = gpuData->saveLayerInfo(0);
        // The parent/child layer appear in reverse order
        const GPUAccelData::SaveLayerInfo& info1 = gpuData->saveLayerInfo(2);
        const GPUAccelData::SaveLayerInfo& info2 = gpuData->saveLayerInfo(1);
        const GPUAccelData::SaveLayerInfo& info3 = gpuData->saveLayerInfo(3);
//        const GPUAccelData::SaveLayerInfo& info4 = gpuData->saveLayerInfo(4);

        REPORTER_ASSERT(reporter, info0.fValid);
        REPORTER_ASSERT(reporter, kWidth == info0.fSize.fWidth && kHeight == info0.fSize.fHeight);
        REPORTER_ASSERT(reporter, info0.fCTM.isIdentity());
        REPORTER_ASSERT(reporter, 0 == info0.fOffset.fX && 0 == info0.fOffset.fY);
        REPORTER_ASSERT(reporter, NULL != info0.fPaint);
        REPORTER_ASSERT(reporter, !info0.fIsNested && !info0.fHasNestedLayers);

        REPORTER_ASSERT(reporter, info1.fValid);
        REPORTER_ASSERT(reporter, kWidth == info1.fSize.fWidth && kHeight == info1.fSize.fHeight);
        REPORTER_ASSERT(reporter, info1.fCTM.isIdentity());
        REPORTER_ASSERT(reporter, 0 == info1.fOffset.fX && 0 == info1.fOffset.fY);
        REPORTER_ASSERT(reporter, NULL != info1.fPaint);
        REPORTER_ASSERT(reporter, !info1.fIsNested && info1.fHasNestedLayers); // has a nested SL

        REPORTER_ASSERT(reporter, info2.fValid);
        REPORTER_ASSERT(reporter, kWidth/2 == info2.fSize.fWidth &&
                                  kHeight/2 == info2.fSize.fHeight); // bound reduces size
        REPORTER_ASSERT(reporter, info2.fCTM.isIdentity());         // translated
        REPORTER_ASSERT(reporter, kWidth/2 == info2.fOffset.fX &&
                                  kHeight/2 == info2.fOffset.fY);
        REPORTER_ASSERT(reporter, NULL != info1.fPaint);
        REPORTER_ASSERT(reporter, info2.fIsNested && !info2.fHasNestedLayers); // is nested

        REPORTER_ASSERT(reporter, info3.fValid);
        REPORTER_ASSERT(reporter, kWidth == info3.fSize.fWidth && kHeight == info3.fSize.fHeight);
        REPORTER_ASSERT(reporter, info3.fCTM.isIdentity());
        REPORTER_ASSERT(reporter, 0 == info3.fOffset.fX && 0 == info3.fOffset.fY);
        REPORTER_ASSERT(reporter, NULL != info3.fPaint);
        REPORTER_ASSERT(reporter, !info3.fIsNested && !info3.fHasNestedLayers);

#if 0 // needs more though for GrGatherCanvas
        REPORTER_ASSERT(reporter, !info4.fValid);                 // paint is/was uncopyable
        REPORTER_ASSERT(reporter, kWidth == info4.fSize.fWidth && kHeight == info4.fSize.fHeight);
        REPORTER_ASSERT(reporter, 0 == info4.fOffset.fX && 0 == info4.fOffset.fY);
        REPORTER_ASSERT(reporter, info4.fCTM.isIdentity());
        REPORTER_ASSERT(reporter, NULL == info4.fPaint);     // paint is/was uncopyable
        REPORTER_ASSERT(reporter, !info4.fIsNested && !info4.fHasNestedLayers);
#endif
    }
}

#endif

static void set_canvas_to_save_count_4(SkCanvas* canvas) {
    canvas->restoreToCount(1);
    canvas->save();
    canvas->save();
    canvas->save();
}

/**
 * A canvas that records the number of saves, saveLayers and restores.
 */
class SaveCountingCanvas : public SkCanvas {
public:
    SaveCountingCanvas(int width, int height)
        : INHERITED(width, height)
        , fSaveCount(0)
        , fSaveLayerCount(0)
        , fRestoreCount(0){
    }

    virtual SaveLayerStrategy willSaveLayer(const SkRect* bounds, const SkPaint* paint,
                                            SaveFlags flags) SK_OVERRIDE {
        ++fSaveLayerCount;
        return this->INHERITED::willSaveLayer(bounds, paint, flags);
    }

    virtual void willSave(SaveFlags flags) SK_OVERRIDE {
        ++fSaveCount;
        this->INHERITED::willSave(flags);
    }

    virtual void willRestore() SK_OVERRIDE {
        ++fRestoreCount;
        this->INHERITED::willRestore();
    }

    unsigned int getSaveCount() const { return fSaveCount; }
    unsigned int getSaveLayerCount() const { return fSaveLayerCount; }
    unsigned int getRestoreCount() const { return fRestoreCount; }

private:
    unsigned int fSaveCount;
    unsigned int fSaveLayerCount;
    unsigned int fRestoreCount;

    typedef SkCanvas INHERITED;
};

void check_save_state(skiatest::Reporter* reporter, SkPicture* picture,
                      unsigned int numSaves, unsigned int numSaveLayers,
                      unsigned int numRestores) {
    SaveCountingCanvas canvas(picture->width(), picture->height());

    picture->draw(&canvas);

    REPORTER_ASSERT(reporter, numSaves == canvas.getSaveCount());
    REPORTER_ASSERT(reporter, numSaveLayers == canvas.getSaveLayerCount());
    REPORTER_ASSERT(reporter, numRestores == canvas.getRestoreCount());
}

// This class exists so SkPicture can friend it and give it access to
// the 'partialReplay' method.
class SkPictureRecorderReplayTester {
public:
    static SkPicture* Copy(SkPictureRecorder* recorder) {
        SkPictureRecorder recorder2;

        SkCanvas* canvas = recorder2.beginRecording(10, 10);

        recorder->partialReplay(canvas);

        return recorder2.endRecording();
    }
};

static void create_imbalance(SkCanvas* canvas) {
    SkRect clipRect = SkRect::MakeWH(2, 2);
    SkRect drawRect = SkRect::MakeWH(10, 10);
    canvas->save();
        canvas->clipRect(clipRect, SkRegion::kReplace_Op);
        canvas->translate(1.0f, 1.0f);
        SkPaint p;
        p.setColor(SK_ColorGREEN);
        canvas->drawRect(drawRect, p);
    // no restore
}

// This tests that replaying a potentially unbalanced picture into a canvas
// doesn't affect the canvas' save count or matrix/clip state.
static void check_balance(skiatest::Reporter* reporter, SkPicture* picture) {
    SkBitmap bm;
    bm.allocN32Pixels(4, 3);
    SkCanvas canvas(bm);

    int beforeSaveCount = canvas.getSaveCount();

    SkMatrix beforeMatrix = canvas.getTotalMatrix();

    SkRect beforeClip;

    canvas.getClipBounds(&beforeClip);

    canvas.drawPicture(picture);

    REPORTER_ASSERT(reporter, beforeSaveCount == canvas.getSaveCount());
    REPORTER_ASSERT(reporter, beforeMatrix == canvas.getTotalMatrix());

    SkRect afterClip;

    canvas.getClipBounds(&afterClip);

    REPORTER_ASSERT(reporter, afterClip == beforeClip);
}

// Test out SkPictureRecorder::partialReplay
DEF_TEST(PictureRecorder_replay, reporter) {
    // check save/saveLayer state
    {
        SkPictureRecorder recorder;

        SkCanvas* canvas = recorder.beginRecording(10, 10);

        canvas->saveLayer(NULL, NULL);

        SkAutoTUnref<SkPicture> copy(SkPictureRecorderReplayTester::Copy(&recorder));

        // The extra save and restore comes from the Copy process.
        check_save_state(reporter, copy, 2, 1, 3);

        canvas->saveLayer(NULL, NULL);

        SkAutoTUnref<SkPicture> final(recorder.endRecording());

        check_save_state(reporter, final, 1, 2, 3);

        // The copy shouldn't pick up any operations added after it was made
        check_save_state(reporter, copy, 2, 1, 3);
    }

    // (partially) check leakage of draw ops
    {
        SkPictureRecorder recorder;

        SkCanvas* canvas = recorder.beginRecording(10, 10);

        SkRect r = SkRect::MakeWH(5, 5);
        SkPaint p;

        canvas->drawRect(r, p);

        SkAutoTUnref<SkPicture> copy(SkPictureRecorderReplayTester::Copy(&recorder));

        REPORTER_ASSERT(reporter, !copy->willPlayBackBitmaps());

        SkBitmap bm;
        make_bm(&bm, 10, 10, SK_ColorRED, true);

        r.offset(5.0f, 5.0f);
        canvas->drawBitmapRectToRect(bm, NULL, r);

        SkAutoTUnref<SkPicture> final(recorder.endRecording());
        REPORTER_ASSERT(reporter, final->willPlayBackBitmaps());

        REPORTER_ASSERT(reporter, copy->uniqueID() != final->uniqueID());

        // The snapshot shouldn't pick up any operations added after it was made
        REPORTER_ASSERT(reporter, !copy->willPlayBackBitmaps());
    }

    // Recreate the Android partialReplay test case
    {
        SkPictureRecorder recorder;

        SkCanvas* canvas = recorder.beginRecording(4, 3, NULL, 0);
        create_imbalance(canvas);

        int expectedSaveCount = canvas->getSaveCount();

        SkAutoTUnref<SkPicture> copy(SkPictureRecorderReplayTester::Copy(&recorder));
        check_balance(reporter, copy);

        REPORTER_ASSERT(reporter, expectedSaveCount = canvas->getSaveCount());

        // End the recording of source to test the picture finalization
        // process isn't complicated by the partialReplay step
        SkAutoTUnref<SkPicture> final(recorder.endRecording());
    }
}

static void test_unbalanced_save_restores(skiatest::Reporter* reporter) {
    SkCanvas testCanvas(100, 100);
    set_canvas_to_save_count_4(&testCanvas);

    REPORTER_ASSERT(reporter, 4 == testCanvas.getSaveCount());

    SkPaint paint;
    SkRect rect = SkRect::MakeLTRB(-10000000, -10000000, 10000000, 10000000);

    SkPictureRecorder recorder;

    {
        // Create picture with 2 unbalanced saves
        SkCanvas* canvas = recorder.beginRecording(100, 100);
        canvas->save();
        canvas->translate(10, 10);
        canvas->drawRect(rect, paint);
        canvas->save();
        canvas->translate(10, 10);
        canvas->drawRect(rect, paint);
        SkAutoTUnref<SkPicture> extraSavePicture(recorder.endRecording());

        testCanvas.drawPicture(extraSavePicture);
        REPORTER_ASSERT(reporter, 4 == testCanvas.getSaveCount());
    }

    set_canvas_to_save_count_4(&testCanvas);

    {
        // Create picture with 2 unbalanced restores
        SkCanvas* canvas = recorder.beginRecording(100, 100);
        canvas->save();
        canvas->translate(10, 10);
        canvas->drawRect(rect, paint);
        canvas->save();
        canvas->translate(10, 10);
        canvas->drawRect(rect, paint);
        canvas->restore();
        canvas->restore();
        canvas->restore();
        canvas->restore();
        SkAutoTUnref<SkPicture> extraRestorePicture(recorder.endRecording());

        testCanvas.drawPicture(extraRestorePicture);
        REPORTER_ASSERT(reporter, 4 == testCanvas.getSaveCount());
    }

    set_canvas_to_save_count_4(&testCanvas);

    {
        SkCanvas* canvas = recorder.beginRecording(100, 100);
        canvas->translate(10, 10);
        canvas->drawRect(rect, paint);
        SkAutoTUnref<SkPicture> noSavePicture(recorder.endRecording());

        testCanvas.drawPicture(noSavePicture);
        REPORTER_ASSERT(reporter, 4 == testCanvas.getSaveCount());
        REPORTER_ASSERT(reporter, testCanvas.getTotalMatrix().isIdentity());
    }
}

static void test_peephole() {
    SkRandom rand;

    SkPictureRecorder recorder;

    for (int j = 0; j < 100; j++) {
        SkRandom rand2(rand); // remember the seed

        SkCanvas* canvas = recorder.beginRecording(100, 100);

        for (int i = 0; i < 1000; ++i) {
            rand_op(canvas, rand);
        }
        SkAutoTUnref<SkPicture> picture(recorder.endRecording());

        rand = rand2;
    }

    {
        SkCanvas* canvas = recorder.beginRecording(100, 100);
        SkRect rect = SkRect::MakeWH(50, 50);

        for (int i = 0; i < 100; ++i) {
            canvas->save();
        }
        while (canvas->getSaveCount() > 1) {
            canvas->clipRect(rect);
            canvas->restore();
        }
        SkAutoTUnref<SkPicture> picture(recorder.endRecording());
    }
}

#ifndef SK_DEBUG
// Only test this is in release mode. We deliberately crash in debug mode, since a valid caller
// should never do this.
static void test_bad_bitmap() {
    // This bitmap has a width and height but no pixels. As a result, attempting to record it will
    // fail.
    SkBitmap bm;
    bm.setInfo(SkImageInfo::MakeN32Premul(100, 100));
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(100, 100);
    recordingCanvas->drawBitmap(bm, 0, 0);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());

    SkCanvas canvas;
    canvas.drawPicture(picture);
}
#endif

static SkData* encode_bitmap_to_data(size_t*, const SkBitmap& bm) {
    return SkImageEncoder::EncodeData(bm, SkImageEncoder::kPNG_Type, 100);
}

static SkData* serialized_picture_from_bitmap(const SkBitmap& bitmap) {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(bitmap.width(), bitmap.height());
    canvas->drawBitmap(bitmap, 0, 0);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());

    SkDynamicMemoryWStream wStream;
    picture->serialize(&wStream, &encode_bitmap_to_data);
    return wStream.copyToData();
}

struct ErrorContext {
    int fErrors;
    skiatest::Reporter* fReporter;
};

static void assert_one_parse_error_cb(SkError error, void* context) {
    ErrorContext* errorContext = static_cast<ErrorContext*>(context);
    errorContext->fErrors++;
    // This test only expects one error, and that is a kParseError. If there are others,
    // there is some unknown problem.
    REPORTER_ASSERT_MESSAGE(errorContext->fReporter, 1 == errorContext->fErrors,
                            "This threw more errors than expected.");
    REPORTER_ASSERT_MESSAGE(errorContext->fReporter, kParseError_SkError == error,
                            SkGetLastErrorString());
}

static void test_bitmap_with_encoded_data(skiatest::Reporter* reporter) {
    // Create a bitmap that will be encoded.
    SkBitmap original;
    make_bm(&original, 100, 100, SK_ColorBLUE, true);
    SkDynamicMemoryWStream wStream;
    if (!SkImageEncoder::EncodeStream(&wStream, original, SkImageEncoder::kPNG_Type, 100)) {
        return;
    }
    SkAutoDataUnref data(wStream.copyToData());

    SkBitmap bm;
    bool installSuccess = SkInstallDiscardablePixelRef(
         SkDecodingImageGenerator::Create(data, SkDecodingImageGenerator::Options()), &bm);
    REPORTER_ASSERT(reporter, installSuccess);

    // Write both bitmaps to pictures, and ensure that the resulting data streams are the same.
    // Flattening original will follow the old path of performing an encode, while flattening bm
    // will use the already encoded data.
    SkAutoDataUnref picture1(serialized_picture_from_bitmap(original));
    SkAutoDataUnref picture2(serialized_picture_from_bitmap(bm));
    REPORTER_ASSERT(reporter, picture1->equals(picture2));
    // Now test that a parse error was generated when trying to create a new SkPicture without
    // providing a function to decode the bitmap.
    ErrorContext context;
    context.fErrors = 0;
    context.fReporter = reporter;
    SkSetErrorCallback(assert_one_parse_error_cb, &context);
    SkMemoryStream pictureStream(picture1);
    SkClearLastError();
    SkAutoUnref pictureFromStream(SkPicture::CreateFromStream(&pictureStream, NULL));
    REPORTER_ASSERT(reporter, pictureFromStream.get() != NULL);
    SkClearLastError();
    SkSetErrorCallback(NULL, NULL);
}

static void test_clone_empty(skiatest::Reporter* reporter) {
    // This is a regression test for crbug.com/172062
    // Before the fix, we used to crash accessing a null pointer when we
    // had a picture with no paints. This test passes by not crashing.
    {
        SkPictureRecorder recorder;
        recorder.beginRecording(1, 1);
        SkAutoTUnref<SkPicture> picture(recorder.endRecording());
        SkAutoTUnref<SkPicture> destPicture(picture->clone());
        REPORTER_ASSERT(reporter, NULL != destPicture);
    }
}

static void test_draw_empty(skiatest::Reporter* reporter) {
    SkBitmap result;
    make_bm(&result, 2, 2, SK_ColorBLACK, false);

    SkCanvas canvas(result);

    {
        // stock SkPicture
        SkPictureRecorder recorder;
        recorder.beginRecording(1, 1);
        SkAutoTUnref<SkPicture> picture(recorder.endRecording());

        canvas.drawPicture(picture);
    }

    {
        // tile grid
        SkTileGridFactory::TileGridInfo gridInfo;
        gridInfo.fMargin.setEmpty();
        gridInfo.fOffset.setZero();
        gridInfo.fTileInterval.set(1, 1);

        SkTileGridFactory factory(gridInfo);
        SkPictureRecorder recorder;
        recorder.beginRecording(1, 1, &factory);
        SkAutoTUnref<SkPicture> picture(recorder.endRecording());

        canvas.drawPicture(picture);
    }

    {
        // RTree
        SkRTreeFactory factory;
        SkPictureRecorder recorder;
        recorder.beginRecording(1, 1, &factory);
        SkAutoTUnref<SkPicture> picture(recorder.endRecording());

        canvas.drawPicture(picture);
    }

    {
        // quad tree
        SkQuadTreeFactory factory;
        SkPictureRecorder recorder;
        recorder.beginRecording(1, 1, &factory);
        SkAutoTUnref<SkPicture> picture(recorder.endRecording());

        canvas.drawPicture(picture);
    }
}

static void test_clip_bound_opt(skiatest::Reporter* reporter) {
    // Test for crbug.com/229011
    SkRect rect1 = SkRect::MakeXYWH(SkIntToScalar(4), SkIntToScalar(4),
                                    SkIntToScalar(2), SkIntToScalar(2));
    SkRect rect2 = SkRect::MakeXYWH(SkIntToScalar(7), SkIntToScalar(7),
                                    SkIntToScalar(1), SkIntToScalar(1));
    SkRect rect3 = SkRect::MakeXYWH(SkIntToScalar(6), SkIntToScalar(6),
                                    SkIntToScalar(1), SkIntToScalar(1));

    SkPath invPath;
    invPath.addOval(rect1);
    invPath.setFillType(SkPath::kInverseEvenOdd_FillType);
    SkPath path;
    path.addOval(rect2);
    SkPath path2;
    path2.addOval(rect3);
    SkIRect clipBounds;
    SkPictureRecorder recorder;
    // Minimalist test set for 100% code coverage of
    // SkPictureRecord::updateClipConservativelyUsingBounds
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(invPath, SkRegion::kIntersect_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(path, SkRegion::kIntersect_Op);
        canvas->clipPath(invPath, SkRegion::kIntersect_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(path, SkRegion::kIntersect_Op);
        canvas->clipPath(invPath, SkRegion::kUnion_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(path, SkRegion::kDifference_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(path, SkRegion::kReverseDifference_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        // True clip is actually empty in this case, but the best
        // determination we can make using only bounds as input is that the
        // clip is included in the bounds of 'path'.
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(path, SkRegion::kIntersect_Op);
        canvas->clipPath(path2, SkRegion::kXOR_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 6 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 6 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
}

/**
 * A canvas that records the number of clip commands.
 */
class ClipCountingCanvas : public SkCanvas {
public:
    ClipCountingCanvas(int width, int height)
        : INHERITED(width, height)
        , fClipCount(0){
    }

    virtual void onClipRect(const SkRect& r,
                            SkRegion::Op op,
                            ClipEdgeStyle edgeStyle) SK_OVERRIDE {
        fClipCount += 1;
        this->INHERITED::onClipRect(r, op, edgeStyle);
    }

    virtual void onClipRRect(const SkRRect& rrect,
                             SkRegion::Op op,
                             ClipEdgeStyle edgeStyle)SK_OVERRIDE {
        fClipCount += 1;
        this->INHERITED::onClipRRect(rrect, op, edgeStyle);
    }

    virtual void onClipPath(const SkPath& path,
                            SkRegion::Op op,
                            ClipEdgeStyle edgeStyle) SK_OVERRIDE {
        fClipCount += 1;
        this->INHERITED::onClipPath(path, op, edgeStyle);
    }

    virtual void onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) SK_OVERRIDE {
        fClipCount += 1;
        this->INHERITED::onClipRegion(deviceRgn, op);
    }

    unsigned getClipCount() const { return fClipCount; }

private:
    unsigned fClipCount;

    typedef SkCanvas INHERITED;
};

static void test_clip_expansion(skiatest::Reporter* reporter) {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(10, 10);

    canvas->clipRect(SkRect::MakeEmpty(), SkRegion::kReplace_Op);
    // The following expanding clip should not be skipped.
    canvas->clipRect(SkRect::MakeXYWH(4, 4, 3, 3), SkRegion::kUnion_Op);
    // Draw something so the optimizer doesn't just fold the world.
    SkPaint p;
    p.setColor(SK_ColorBLUE);
    canvas->drawPaint(p);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());

    ClipCountingCanvas testCanvas(10, 10);
    picture->draw(&testCanvas);

    // Both clips should be present on playback.
    REPORTER_ASSERT(reporter, testCanvas.getClipCount() == 2);
}

static void test_hierarchical(skiatest::Reporter* reporter) {
    SkBitmap bm;
    make_bm(&bm, 10, 10, SK_ColorRED, true);

    SkPictureRecorder recorder;

    recorder.beginRecording(10, 10);
    SkAutoTUnref<SkPicture> childPlain(recorder.endRecording());
    REPORTER_ASSERT(reporter, !childPlain->willPlayBackBitmaps()); // 0

    recorder.beginRecording(10, 10)->drawBitmap(bm, 0, 0);
    SkAutoTUnref<SkPicture> childWithBitmap(recorder.endRecording());
    REPORTER_ASSERT(reporter, childWithBitmap->willPlayBackBitmaps()); // 1

    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->drawPicture(childPlain);
        SkAutoTUnref<SkPicture> parentPP(recorder.endRecording());
        REPORTER_ASSERT(reporter, !parentPP->willPlayBackBitmaps()); // 0
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->drawPicture(childWithBitmap);
        SkAutoTUnref<SkPicture> parentPWB(recorder.endRecording());
        REPORTER_ASSERT(reporter, parentPWB->willPlayBackBitmaps()); // 1
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->drawBitmap(bm, 0, 0);
        canvas->drawPicture(childPlain);
        SkAutoTUnref<SkPicture> parentWBP(recorder.endRecording());
        REPORTER_ASSERT(reporter, parentWBP->willPlayBackBitmaps()); // 1
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->drawBitmap(bm, 0, 0);
        canvas->drawPicture(childWithBitmap);
        SkAutoTUnref<SkPicture> parentWBWB(recorder.endRecording());
        REPORTER_ASSERT(reporter, parentWBWB->willPlayBackBitmaps()); // 2
    }
}

static void test_gen_id(skiatest::Reporter* reporter) {

    SkPicture empty;

    // Empty pictures should still have a valid ID
    REPORTER_ASSERT(reporter, empty.uniqueID() != SK_InvalidGenID);

    SkPictureRecorder recorder;

    SkCanvas* canvas = recorder.beginRecording(1, 1);
    canvas->drawARGB(255, 255, 255, 255);
    SkAutoTUnref<SkPicture> hasData(recorder.endRecording());
    // picture should have a non-zero id after recording
    REPORTER_ASSERT(reporter, hasData->uniqueID() != SK_InvalidGenID);

    // both pictures should have different ids
    REPORTER_ASSERT(reporter, hasData->uniqueID() != empty.uniqueID());

    // test out copy constructor
    SkPicture copyWithData(*hasData);
    REPORTER_ASSERT(reporter, hasData->uniqueID() == copyWithData.uniqueID());

    SkPicture emptyCopy(empty);
    REPORTER_ASSERT(reporter, empty.uniqueID() != emptyCopy.uniqueID());

    // test out swap
    {
        SkPicture swapWithData;
        uint32_t beforeID1 = swapWithData.uniqueID();
        uint32_t beforeID2 = copyWithData.uniqueID();
        swapWithData.swap(copyWithData);
        REPORTER_ASSERT(reporter, copyWithData.uniqueID() == beforeID1);
        REPORTER_ASSERT(reporter, swapWithData.uniqueID() == beforeID2);
    }

    // test out clone
    {
        SkAutoTUnref<SkPicture> cloneWithData(hasData->clone());
        REPORTER_ASSERT(reporter, hasData->uniqueID() == cloneWithData->uniqueID());

        SkAutoTUnref<SkPicture> emptyClone(empty.clone());
        REPORTER_ASSERT(reporter, empty.uniqueID() != emptyClone->uniqueID());
    }
}

DEF_TEST(Picture, reporter) {
#ifdef SK_DEBUG
    test_deleting_empty_playback();
    test_serializing_empty_picture();
#else
    test_bad_bitmap();
#endif
    test_unbalanced_save_restores(reporter);
    test_peephole();
#if SK_SUPPORT_GPU
    test_gpu_veto(reporter);
#endif
    test_gatherpixelrefs(reporter);
    test_gatherpixelrefsandrects(reporter);
    test_bitmap_with_encoded_data(reporter);
    test_clone_empty(reporter);
    test_draw_empty(reporter);
    test_clip_bound_opt(reporter);
    test_clip_expansion(reporter);
    test_hierarchical(reporter);
    test_gen_id(reporter);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST(GPUPicture, reporter, factory) {
    test_gpu_picture_optimization(reporter, factory);
}
#endif

static void draw_bitmaps(const SkBitmap bitmap, SkCanvas* canvas) {
    const SkPaint paint;
    const SkRect rect = { 5.0f, 5.0f, 8.0f, 8.0f };
    const SkIRect irect =  { 2, 2, 3, 3 };

    // Don't care what these record, as long as they're legal.
    canvas->drawBitmap(bitmap, 0.0f, 0.0f, &paint);
    canvas->drawBitmapRectToRect(bitmap, &rect, rect, &paint, SkCanvas::kNone_DrawBitmapRectFlag);
    canvas->drawBitmapMatrix(bitmap, SkMatrix::I(), &paint);
    canvas->drawBitmapNine(bitmap, irect, rect, &paint);
    canvas->drawSprite(bitmap, 1, 1);
}

static void test_draw_bitmaps(SkCanvas* canvas) {
    SkBitmap empty;
    draw_bitmaps(empty, canvas);
    empty.setInfo(SkImageInfo::MakeN32Premul(10, 10));
    draw_bitmaps(empty, canvas);
}

DEF_TEST(Picture_EmptyBitmap, r) {
    SkPictureRecorder recorder;
    test_draw_bitmaps(recorder.beginRecording(10, 10));
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());
}

DEF_TEST(Canvas_EmptyBitmap, r) {
    SkBitmap dst;
    dst.allocN32Pixels(10, 10);
    SkCanvas canvas(dst);

    test_draw_bitmaps(&canvas);
}
