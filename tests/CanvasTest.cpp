/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkDocument.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/core/SkVertices.h"
#include "include/docs/SkPDFDocument.h"
#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkPaintImageFilter.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkNWayCanvas.h"
#include "include/utils/SkPaintFilterCanvas.h"
#include "src/core/SkClipOpPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/utils/SkCanvasStack.h"
#include "tests/Test.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
#include "include/core/SkColorSpace.h"
#include "include/private/SkColorData.h"
#endif

#include <memory>
#include <utility>

class SkReadBuffer;

DEF_TEST(canvas_clipbounds, reporter) {
    SkCanvas canvas(10, 10);
    SkIRect irect, irect2;
    SkRect rect, rect2;

    irect = canvas.getDeviceClipBounds();
    REPORTER_ASSERT(reporter, irect == SkIRect::MakeWH(10, 10));
    REPORTER_ASSERT(reporter, canvas.getDeviceClipBounds(&irect2));
    REPORTER_ASSERT(reporter, irect == irect2);

    // local bounds are always too big today -- can we trim them?
    rect = canvas.getLocalClipBounds();
    REPORTER_ASSERT(reporter, rect.contains(SkRect::MakeWH(10, 10)));
    REPORTER_ASSERT(reporter, canvas.getLocalClipBounds(&rect2));
    REPORTER_ASSERT(reporter, rect == rect2);

    canvas.clipRect(SkRect::MakeEmpty());

    irect = canvas.getDeviceClipBounds();
    REPORTER_ASSERT(reporter, irect == SkIRect::MakeEmpty());
    REPORTER_ASSERT(reporter, !canvas.getDeviceClipBounds(&irect2));
    REPORTER_ASSERT(reporter, irect == irect2);

    rect = canvas.getLocalClipBounds();
    REPORTER_ASSERT(reporter, rect == SkRect::MakeEmpty());
    REPORTER_ASSERT(reporter, !canvas.getLocalClipBounds(&rect2));
    REPORTER_ASSERT(reporter, rect == rect2);

    // Test for wacky sizes that we (historically) have guarded against
    {
        SkCanvas c(-10, -20);
        REPORTER_ASSERT(reporter, c.getBaseLayerSize() == SkISize::MakeEmpty());

        SkPictureRecorder().beginRecording({ 5, 5, 4, 4 });
    }
}

// Will call proc with multiple styles of canvas (recording, raster, pdf)
template <typename F> static void multi_canvas_driver(int w, int h, F proc) {
    proc(SkPictureRecorder().beginRecording(SkRect::MakeIWH(w, h)));

    SkNullWStream stream;
    if (auto doc = SkPDF::MakeDocument(&stream)) {
        proc(doc->beginPage(SkIntToScalar(w), SkIntToScalar(h)));
    }

    proc(SkSurface::MakeRasterN32Premul(w, h, nullptr)->getCanvas());
}

const SkIRect gBaseRestrictedR = { 0, 0, 10, 10 };

static void test_restriction(skiatest::Reporter* reporter, SkCanvas* canvas) {
    REPORTER_ASSERT(reporter, canvas->getDeviceClipBounds() == gBaseRestrictedR);

    const SkIRect restrictionR = { 2, 2, 8, 8 };
    canvas->androidFramework_setDeviceClipRestriction(restrictionR);
    REPORTER_ASSERT(reporter, canvas->getDeviceClipBounds() == restrictionR);

    const SkIRect clipR = { 4, 4, 6, 6 };
    canvas->clipRect(SkRect::Make(clipR), SkClipOp::kIntersect);
    REPORTER_ASSERT(reporter, canvas->getDeviceClipBounds() == clipR);

#ifdef SK_SUPPORT_DEPRECATED_CLIPOPS
    // now test that expanding clipops can't exceed the restriction
    const SkClipOp expanders[] = {
        SkClipOp::kUnion_deprecated,
        SkClipOp::kXOR_deprecated,
        SkClipOp::kReverseDifference_deprecated,
        SkClipOp::kReplace_deprecated,
    };

    const SkRect expandR = { 0, 0, 5, 9 };
    SkASSERT(!SkRect::Make(restrictionR).contains(expandR));

    for (SkClipOp op : expanders) {
        canvas->save();
        canvas->clipRect(expandR, op);
        REPORTER_ASSERT(reporter, gBaseRestrictedR.contains(canvas->getDeviceClipBounds()));
        canvas->restore();
    }
#endif
}

/**
 *  Clip restriction logic exists in the canvas itself, and in various kinds of devices.
 *
 *  This test explicitly tries to exercise that variety:
 *  - picture : empty device but exercises canvas itself
 *  - pdf : uses SkClipStack in its device (as does SVG and GPU)
 *  - raster : uses SkRasterClip in its device
 */
DEF_TEST(canvas_clip_restriction, reporter) {
    multi_canvas_driver(gBaseRestrictedR.width(), gBaseRestrictedR.height(),
                        [reporter](SkCanvas* canvas) { test_restriction(reporter, canvas); });
}

DEF_TEST(canvas_empty_clip, reporter) {
    multi_canvas_driver(50, 50, [reporter](SkCanvas* canvas) {
        canvas->save();
        canvas->clipRect({0, 0, 20, 40 });
        REPORTER_ASSERT(reporter, !canvas->isClipEmpty());
        canvas->clipRect({30, 0, 50, 40 });
        REPORTER_ASSERT(reporter, canvas->isClipEmpty());
    });
}

DEF_TEST(CanvasNewRasterTest, reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);
    const size_t minRowBytes = info.minRowBytes();
    const size_t size = info.computeByteSize(minRowBytes);
    SkAutoTMalloc<SkPMColor> storage(size);
    SkPMColor* baseAddr = storage.get();
    sk_bzero(baseAddr, size);

    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirect(info, baseAddr, minRowBytes);
    REPORTER_ASSERT(reporter, canvas);

    SkPixmap pmap;
    const SkPMColor* addr = canvas->peekPixels(&pmap) ? pmap.addr32() : nullptr;
    REPORTER_ASSERT(reporter, addr);
    REPORTER_ASSERT(reporter, info == pmap.info());
    REPORTER_ASSERT(reporter, minRowBytes == pmap.rowBytes());
    for (int y = 0; y < info.height(); ++y) {
        for (int x = 0; x < info.width(); ++x) {
            REPORTER_ASSERT(reporter, 0 == addr[x]);
        }
        addr = (const SkPMColor*)((const char*)addr + pmap.rowBytes());
    }

    // now try a deliberately bad info
    info = info.makeWH(-1, info.height());
    REPORTER_ASSERT(reporter, nullptr == SkCanvas::MakeRasterDirect(info, baseAddr, minRowBytes));

    // too big
    info = info.makeWH(1 << 30, 1 << 30);
    REPORTER_ASSERT(reporter, nullptr == SkCanvas::MakeRasterDirect(info, baseAddr, minRowBytes));

    // not a valid pixel type
    info = SkImageInfo::Make(10, 10, kUnknown_SkColorType, info.alphaType());
    REPORTER_ASSERT(reporter, nullptr == SkCanvas::MakeRasterDirect(info, baseAddr, minRowBytes));

    // We should not succeed with a zero-sized valid info
    info = SkImageInfo::MakeN32Premul(0, 0);
    canvas = SkCanvas::MakeRasterDirect(info, baseAddr, minRowBytes);
    REPORTER_ASSERT(reporter, nullptr == canvas);
}

static SkPath make_path_from_rect(SkRect r) {
    SkPath path;
    path.addRect(r);
    return path;
}

static SkRegion make_region_from_irect(SkIRect r) {
    SkRegion region;
    region.setRect(r);
    return region;
}

static SkBitmap make_n32_bitmap(int w, int h, SkColor c = SK_ColorWHITE) {
    SkBitmap bm;
    bm.allocN32Pixels(w, h);
    bm.eraseColor(c);
    return bm;
}

// Constants used by test steps
static constexpr SkRect kRect = {0, 0, 2, 1};
static constexpr SkColor kColor = 0x01020304;
static constexpr int kWidth = 2;
static constexpr int kHeight = 2;

using CanvasTest = void (*)(SkCanvas*, skiatest::Reporter*);

static CanvasTest kCanvasTests[] = {
    [](SkCanvas* c, skiatest::Reporter* r) {
        c->translate(SkIntToScalar(1), SkIntToScalar(2));
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        c->scale(SkIntToScalar(1), SkIntToScalar(2));
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        c->rotate(SkIntToScalar(1));
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        c->skew(SkIntToScalar(1), SkIntToScalar(2));
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        c->concat(SkMatrix::MakeScale(2, 3));
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        c->setMatrix(SkMatrix::MakeScale(2, 3));
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        c->clipRect(kRect);
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        c->clipPath(make_path_from_rect(SkRect{0, 0, 2, 1}));
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        c->clipRegion(make_region_from_irect(SkIRect{0, 0, 2, 1}), kReplace_SkClipOp);
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        c->clear(kColor);
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        int saveCount = c->getSaveCount();
        c->save();
        c->translate(SkIntToScalar(1), SkIntToScalar(2));
        c->clipRegion(make_region_from_irect(SkIRect{0, 0, 2, 1}));
        c->restore();
        REPORTER_ASSERT(r, c->getSaveCount() == saveCount);
        REPORTER_ASSERT(r, c->getTotalMatrix().isIdentity());
        //REPORTER_ASSERT(reporter, c->getTotalClip() != kTestRegion);
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        int saveCount = c->getSaveCount();
        c->saveLayer(nullptr, nullptr);
        c->restore();
        REPORTER_ASSERT(r, c->getSaveCount() == saveCount);
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        int saveCount = c->getSaveCount();
        c->saveLayer(&kRect, nullptr);
        c->restore();
        REPORTER_ASSERT(r, c->getSaveCount() == saveCount);
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        int saveCount = c->getSaveCount();
        SkPaint p;
        c->saveLayer(nullptr, &p);
        c->restore();
        REPORTER_ASSERT(r, c->getSaveCount() == saveCount);
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        // This test exercises a functionality in SkPicture that leads to the
        // recording of restore offset placeholders.  This test will trigger an
        // assertion at playback time if the placeholders are not properly
        // filled when the recording ends.
        c->clipRect(kRect);
        c->clipRegion(make_region_from_irect(SkIRect{0, 0, 2, 1}));
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        // exercise fix for http://code.google.com/p/skia/issues/detail?id=560
        // ('SkPathStroker::lineTo() fails for line with length SK_ScalarNearlyZero')
        SkPaint paint;
        paint.setStrokeWidth(SkIntToScalar(1));
        paint.setStyle(SkPaint::kStroke_Style);
        SkPath path;
        path.moveTo(SkPoint{ 0, 0 });
        path.lineTo(SkPoint{ 0, SK_ScalarNearlyZero });
        path.lineTo(SkPoint{ SkIntToScalar(1), 0 });
        path.lineTo(SkPoint{ SkIntToScalar(1), SK_ScalarNearlyZero/2 });
        // test nearly zero length path
        c->drawPath(path, paint);
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        SkPictureRecorder recorder;
        SkCanvas* testCanvas = recorder.beginRecording(
                SkIntToScalar(kWidth), SkIntToScalar(kHeight), nullptr, 0);
        testCanvas->scale(SkIntToScalar(2), SkIntToScalar(1));
        testCanvas->clipRect(kRect);
        testCanvas->drawRect(kRect, SkPaint());
        c->drawPicture(recorder.finishRecordingAsPicture());
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        int baseSaveCount = c->getSaveCount();
        int n = c->save();
        REPORTER_ASSERT(r, baseSaveCount == n);
        REPORTER_ASSERT(r, baseSaveCount + 1 == c->getSaveCount());
        c->save();
        c->save();
        REPORTER_ASSERT(r, baseSaveCount + 3 == c->getSaveCount());
        c->restoreToCount(baseSaveCount + 1);
        REPORTER_ASSERT(r, baseSaveCount + 1 == c->getSaveCount());

       // should this pin to 1, or be a no-op, or crash?
       c->restoreToCount(0);
       REPORTER_ASSERT(r, 1 == c->getSaveCount());
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
       // This test step challenges the TestDeferredCanvasStateConsistency
       // test cases because the opaque paint can trigger an optimization
       // that discards previously recorded commands. The challenge is to maintain
       // correct clip and matrix stack state.
       c->resetMatrix();
       c->rotate(SkIntToScalar(30));
       c->save();
       c->translate(SkIntToScalar(2), SkIntToScalar(1));
       c->save();
       c->scale(SkIntToScalar(3), SkIntToScalar(3));
       SkPaint paint;
       paint.setColor(0xFFFFFFFF);
       c->drawPaint(paint);
       c->restore();
       c->restore();
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
       // This test step challenges the TestDeferredCanvasStateConsistency
       // test case because the canvas flush on a deferred canvas will
       // reset the recording session. The challenge is to maintain correct
       // clip and matrix stack state on the playback canvas.
       c->resetMatrix();
       c->rotate(SkIntToScalar(30));
       c->save();
       c->translate(SkIntToScalar(2), SkIntToScalar(1));
       c->save();
       c->scale(SkIntToScalar(3), SkIntToScalar(3));
       c->drawRect(kRect, SkPaint());
       c->flush();
       c->restore();
       c->restore();
    },
    [](SkCanvas* c, skiatest::Reporter* r) {
        SkPoint pts[4];
        pts[0].set(0, 0);
        pts[1].set(SkIntToScalar(kWidth), 0);
        pts[2].set(SkIntToScalar(kWidth), SkIntToScalar(kHeight));
        pts[3].set(0, SkIntToScalar(kHeight));
        SkPaint paint;
        SkBitmap bitmap(make_n32_bitmap(kWidth, kHeight, 0x05060708));
        paint.setShader(bitmap.makeShader());
        c->drawVertices(
            SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode, 4, pts, pts, nullptr),
            SkBlendMode::kModulate, paint);
    }
};

DEF_TEST(Canvas_bitmap, reporter) {
    for (const CanvasTest& test : kCanvasTests) {
        SkBitmap referenceStore = make_n32_bitmap(kWidth, kHeight);
        SkCanvas referenceCanvas(referenceStore);
        test(&referenceCanvas, reporter);
    }
}

DEF_TEST(Canvas_pdf, reporter) {
    for (const CanvasTest& test : kCanvasTests) {
        SkNullWStream outStream;
        if (auto doc = SkPDF::MakeDocument(&outStream)) {
            SkCanvas* canvas = doc->beginPage(SkIntToScalar(kWidth),
                                              SkIntToScalar(kHeight));
            REPORTER_ASSERT(reporter, canvas);
            test(canvas, reporter);
        }
    }
}

DEF_TEST(Canvas_SaveState, reporter) {
    SkCanvas canvas(10, 10);
    REPORTER_ASSERT(reporter, 1 == canvas.getSaveCount());

    int n = canvas.save();
    REPORTER_ASSERT(reporter, 1 == n);
    REPORTER_ASSERT(reporter, 2 == canvas.getSaveCount());

    n = canvas.saveLayer(nullptr, nullptr);
    REPORTER_ASSERT(reporter, 2 == n);
    REPORTER_ASSERT(reporter, 3 == canvas.getSaveCount());

    canvas.restore();
    REPORTER_ASSERT(reporter, 2 == canvas.getSaveCount());
    canvas.restore();
    REPORTER_ASSERT(reporter, 1 == canvas.getSaveCount());
}

DEF_TEST(Canvas_ClipEmptyPath, reporter) {
    SkCanvas canvas(10, 10);
    canvas.save();
    SkPath path;
    canvas.clipPath(path);
    canvas.restore();
    canvas.save();
    path.moveTo(5, 5);
    canvas.clipPath(path);
    canvas.restore();
    canvas.save();
    path.moveTo(7, 7);
    canvas.clipPath(path);  // should not assert here
    canvas.restore();
}

namespace {

class MockFilterCanvas : public SkPaintFilterCanvas {
public:
    MockFilterCanvas(SkCanvas* canvas) : INHERITED(canvas) { }

protected:
    bool onFilter(SkPaint&) const override { return true; }

private:
    typedef SkPaintFilterCanvas INHERITED;
};

} // anonymous namespace

// SkPaintFilterCanvas should inherit the initial target canvas state.
DEF_TEST(PaintFilterCanvas_ConsistentState, reporter) {
    SkCanvas canvas(100, 100);
    canvas.clipRect(SkRect::MakeXYWH(12.7f, 12.7f, 75, 75));
    canvas.scale(0.5f, 0.75f);

    MockFilterCanvas filterCanvas(&canvas);
    REPORTER_ASSERT(reporter, canvas.getTotalMatrix() == filterCanvas.getTotalMatrix());
    REPORTER_ASSERT(reporter, canvas.getLocalClipBounds() == filterCanvas.getLocalClipBounds());

    filterCanvas.clipRect(SkRect::MakeXYWH(30.5f, 30.7f, 100, 100));
    filterCanvas.scale(0.75f, 0.5f);
    REPORTER_ASSERT(reporter, canvas.getTotalMatrix() == filterCanvas.getTotalMatrix());
    REPORTER_ASSERT(reporter, filterCanvas.getLocalClipBounds().contains(canvas.getLocalClipBounds()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

// Subclass that takes a bool*, which it updates in its construct (true) and destructor (false)
// to allow the caller to know how long the object is alive.
class LifeLineCanvas : public SkCanvas {
    bool*   fLifeLine;
public:
    LifeLineCanvas(int w, int h, bool* lifeline) : SkCanvas(w, h), fLifeLine(lifeline) {
        *fLifeLine = true;
    }
    ~LifeLineCanvas() {
        *fLifeLine = false;
    }
};

}

// Check that NWayCanvas does NOT try to manage the lifetime of its sub-canvases
DEF_TEST(NWayCanvas, r) {
    const int w = 10;
    const int h = 10;
    bool life[2];
    {
        LifeLineCanvas c0(w, h, &life[0]);
        REPORTER_ASSERT(r, life[0]);
    }
    REPORTER_ASSERT(r, !life[0]);


    std::unique_ptr<SkCanvas> c0 = std::unique_ptr<SkCanvas>(new LifeLineCanvas(w, h, &life[0]));
    std::unique_ptr<SkCanvas> c1 = std::unique_ptr<SkCanvas>(new LifeLineCanvas(w, h, &life[1]));
    REPORTER_ASSERT(r, life[0]);
    REPORTER_ASSERT(r, life[1]);

    {
        SkNWayCanvas nway(w, h);
        nway.addCanvas(c0.get());
        nway.addCanvas(c1.get());
        REPORTER_ASSERT(r, life[0]);
        REPORTER_ASSERT(r, life[1]);
    }
    // Now assert that the death of the nway has NOT also killed the sub-canvases
    REPORTER_ASSERT(r, life[0]);
    REPORTER_ASSERT(r, life[1]);
}

// Check that CanvasStack DOES manage the lifetime of its sub-canvases
DEF_TEST(CanvasStack, r) {
    const int w = 10;
    const int h = 10;
    bool life[2];
    std::unique_ptr<SkCanvas> c0 = std::unique_ptr<SkCanvas>(new LifeLineCanvas(w, h, &life[0]));
    std::unique_ptr<SkCanvas> c1 = std::unique_ptr<SkCanvas>(new LifeLineCanvas(w, h, &life[1]));
    REPORTER_ASSERT(r, life[0]);
    REPORTER_ASSERT(r, life[1]);

    {
        SkCanvasStack stack(w, h);
        stack.pushCanvas(std::move(c0), {0,0});
        stack.pushCanvas(std::move(c1), {0,0});
        REPORTER_ASSERT(r, life[0]);
        REPORTER_ASSERT(r, life[1]);
    }
    // Now assert that the death of the canvasstack has also killed the sub-canvases
    REPORTER_ASSERT(r, !life[0]);
    REPORTER_ASSERT(r, !life[1]);
}

static void test_cliptype(SkCanvas* canvas, skiatest::Reporter* r) {
    REPORTER_ASSERT(r, !canvas->isClipEmpty());
    REPORTER_ASSERT(r, canvas->isClipRect());

    canvas->save();
    canvas->clipRect({0, 0, 0, 0});
    REPORTER_ASSERT(r, canvas->isClipEmpty());
    REPORTER_ASSERT(r, !canvas->isClipRect());
    canvas->restore();

    canvas->save();
    canvas->clipRect({2, 2, 6, 6});
    REPORTER_ASSERT(r, !canvas->isClipEmpty());
    REPORTER_ASSERT(r, canvas->isClipRect());
    canvas->restore();

    canvas->save();
    canvas->clipRect({2, 2, 6, 6}, SkClipOp::kDifference);  // punch a hole in the clip
    REPORTER_ASSERT(r, !canvas->isClipEmpty());
    REPORTER_ASSERT(r, !canvas->isClipRect());
    canvas->restore();

    REPORTER_ASSERT(r, !canvas->isClipEmpty());
    REPORTER_ASSERT(r, canvas->isClipRect());
}

DEF_TEST(CanvasClipType, r) {
    // test rasterclip backend
    test_cliptype(SkSurface::MakeRasterN32Premul(10, 10)->getCanvas(), r);

    // test clipstack backend
    SkDynamicMemoryWStream stream;
    if (auto doc = SkPDF::MakeDocument(&stream)) {
        test_cliptype(doc->beginPage(100, 100), r);
    }
}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
DEF_TEST(Canvas_LegacyColorBehavior, r) {
    sk_sp<SkColorSpace> cs = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                   SkNamedGamut::kAdobeRGB);

    // Make a Adobe RGB bitmap.
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32(1, 1, kOpaque_SkAlphaType, cs));
    bitmap.eraseColor(0xFF000000);

    // Wrap it in a legacy canvas.  Test that the canvas behaves like a legacy canvas.
    SkCanvas canvas(bitmap, SkCanvas::ColorBehavior::kLegacy);
    REPORTER_ASSERT(r, !canvas.imageInfo().colorSpace());
    SkPaint p;
    p.setColor(SK_ColorRED);
    canvas.drawIRect(SkIRect::MakeWH(1, 1), p);
    REPORTER_ASSERT(r, SK_ColorRED == SkSwizzle_BGRA_to_PMColor(*bitmap.getAddr32(0, 0)));
}
#endif

namespace {

class ZeroBoundsImageFilter : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> Make() { return sk_sp<SkImageFilter>(new ZeroBoundsImageFilter); }

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage*, const Context&, SkIPoint*) const override {
        return nullptr;
    }
    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix&,
                               MapDirection, const SkIRect* inputRect) const override {
        return SkIRect::MakeEmpty();
    }

private:
    SK_FLATTENABLE_HOOKS(ZeroBoundsImageFilter)

    ZeroBoundsImageFilter() : INHERITED(nullptr, 0, nullptr) {}

    typedef SkImageFilter INHERITED;
};

sk_sp<SkFlattenable> ZeroBoundsImageFilter::CreateProc(SkReadBuffer& buffer) {
    SkDEBUGFAIL("Should never get here");
    return nullptr;
}

}  // anonymous namespace

DEF_TEST(Canvas_SaveLayerWithNullBoundsAndZeroBoundsImageFilter, r) {
    SkCanvas canvas(10, 10);
    SkPaint p;
    p.setImageFilter(ZeroBoundsImageFilter::Make());
    // This should not fail any assert.
    canvas.saveLayer(nullptr, &p);
    REPORTER_ASSERT(r, canvas.getDeviceClipBounds().isEmpty());
    canvas.restore();
}

// Test that we don't crash/assert when building a canvas with degenerate coordintes
// (esp. big ones, that might invoke tiling).
DEF_TEST(Canvas_degenerate_dimension, reporter) {
    // Need a paint that will sneak us past the quickReject in SkCanvas, so we can test the
    // raster code further downstream.
    SkPaint paint;
    paint.setImageFilter(SkPaintImageFilter::Make(SkPaint(), nullptr));
    REPORTER_ASSERT(reporter, !paint.canComputeFastBounds());

    const int big = 100 * 1024; // big enough to definitely trigger tiling
    const SkISize sizes[] {SkISize{0, big}, {big, 0}, {0, 0}};
    for (SkISize size : sizes) {
        SkBitmap bm;
        bm.setInfo(SkImageInfo::MakeN32Premul(size.width(), size.height()));
        SkCanvas canvas(bm);
        canvas.drawRect({0, 0, 100, 90*1024}, paint);
    }
}

DEF_TEST(Canvas_ClippedOutImageFilter, reporter) {
    SkCanvas canvas(100, 100);

    SkPaint p;
    p.setColor(SK_ColorGREEN);
    p.setImageFilter(SkBlurImageFilter::Make(3.0f, 3.0f, nullptr, nullptr));

    SkRect blurredRect = SkRect::MakeXYWH(60, 10, 30, 30);

    SkMatrix invM;
    invM.setRotate(-45);
    invM.mapRect(&blurredRect);

    const SkRect clipRect = SkRect::MakeXYWH(0, 50, 50, 50);

    canvas.clipRect(clipRect);

    canvas.rotate(45);
    const SkMatrix preCTM = canvas.getTotalMatrix();
    canvas.drawRect(blurredRect, p);
    const SkMatrix postCTM = canvas.getTotalMatrix();
    REPORTER_ASSERT(reporter, preCTM == postCTM);
}

