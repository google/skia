/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*  Description:
 *      This test defines a series of elementatry test steps that perform
 *      a single or a small group of canvas API calls. Each test step is
 *      used in several test cases that verify that different types of SkCanvas
 *      flavors and derivatives pass it and yield consistent behavior. The
 *      test cases analyse results that are queryable through the API. They do
 *      not look at rendering results.
 *
 *  Adding test stepss:
 *      The general pattern for creating a new test step is to write a test
 *      function of the form:
 *
 *          static void MyTestStepFunction(SkCanvas* canvas,
 *                                         const TestData& d,
 *                                         skiatest::Reporter* reporter,
 *                                         CanvasTestStep* testStep)
 *          {
 *              canvas->someCanvasAPImethod();
 *              (...)
 *              REPORTER_ASSERT(reporter, (...), \
 *                  testStep->assertMessage());
 *          }
 *
 *      The definition of the test step function should be followed by an
 *      invocation of the TEST_STEP macro, which generates a class and
 *      instance for the test step:
 *
 *          TEST_STEP(MyTestStep, MyTestStepFunction)
 *
 *      There are also short hand macros for defining simple test steps
 *      in a single line of code.  A simple test step is a one that is made
 *      of a single canvas API call.
 *
 *          SIMPLE_TEST_STEP(MytestStep, someCanvasAPIMethod());
 *
 *      There is another macro called SIMPLE_TEST_STEP_WITH_ASSERT that
 *      works the same way as SIMPLE_TEST_STEP, and additionally verifies
 *      that the invoked method returns a non-zero value.
 */

#include "SkBitmap.h"
#include "SkBlendMode.h"
#include "SkCanvas.h"
#include "SkCanvasStack.h"
#include "SkClipOp.h"
#include "SkClipOpPriv.h"
#include "SkColor.h"
#include "SkImageFilter.h"
#include "SkImageInfo.h"
#include "SkMalloc.h"
#include "SkMatrix.h"
#include "SkNWayCanvas.h"
#include "SkPDFDocument.h"
#include "SkPaint.h"
#include "SkPaintFilterCanvas.h"
#include "SkPath.h"
#include "SkPictureRecorder.h"
#include "SkPixmap.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkRegion.h"
#include "SkScalar.h"
#include "SkShader.h"
#include "SkSize.h"
#include "SkSpecialImage.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkSurface.h"
#include "SkTDArray.h"
#include "SkTemplates.h"
#include "SkTypes.h"
#include "SkVertices.h"
#include "Test.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
#include "SkColorData.h"
#include "SkColorSpace.h"
#endif

#include <memory>
#include <utility>

class SkReadBuffer;
template <typename T> class SkTCopyOnFirstWrite;

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

static const int kWidth = 2, kHeight = 2;

static void createBitmap(SkBitmap* bm, SkColor color) {
    bm->allocN32Pixels(kWidth, kHeight);
    bm->eraseColor(color);
}

///////////////////////////////////////////////////////////////////////////////
// Constants used by test steps
const SkPoint kTestPoints[] = {
    {SkIntToScalar(0), SkIntToScalar(0)},
    {SkIntToScalar(2), SkIntToScalar(1)},
    {SkIntToScalar(0), SkIntToScalar(2)}
};
const SkPoint kTestPoints2[] = {
    { SkIntToScalar(0), SkIntToScalar(1) },
    { SkIntToScalar(1), SkIntToScalar(1) },
    { SkIntToScalar(2), SkIntToScalar(1) },
    { SkIntToScalar(3), SkIntToScalar(1) },
    { SkIntToScalar(4), SkIntToScalar(1) },
    { SkIntToScalar(5), SkIntToScalar(1) },
    { SkIntToScalar(6), SkIntToScalar(1) },
    { SkIntToScalar(7), SkIntToScalar(1) },
    { SkIntToScalar(8), SkIntToScalar(1) },
    { SkIntToScalar(9), SkIntToScalar(1) },
    { SkIntToScalar(10), SkIntToScalar(1) }
};

struct TestData {
public:
    TestData()
    : fRect(SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                                 SkIntToScalar(2), SkIntToScalar(1)))
    , fMatrix(TestMatrix())
    , fPath(TestPath())
    , fNearlyZeroLengthPath(TestNearlyZeroLengthPath())
    , fIRect(SkIRect::MakeXYWH(0, 0, 2, 1))
    , fRegion(TestRegion())
    , fColor(0x01020304)
    , fPoints(kTestPoints)
    , fPointCount(3)
    , fWidth(2)
    , fHeight(2)
    , fText("Hello World")
    , fPoints2(kTestPoints2)
    , fBitmap(TestBitmap())
    { }

    SkRect fRect;
    SkMatrix fMatrix;
    SkPath fPath;
    SkPath fNearlyZeroLengthPath;
    SkIRect fIRect;
    SkRegion fRegion;
    SkColor fColor;
    SkPaint fPaint;
    const SkPoint* fPoints;
    size_t fPointCount;
    int fWidth;
    int fHeight;
    SkString fText;
    const SkPoint* fPoints2;
    SkBitmap fBitmap;

private:
    static SkMatrix TestMatrix() {
        SkMatrix matrix;
        matrix.reset();
        matrix.setScale(SkIntToScalar(2), SkIntToScalar(3));

        return matrix;
    }
    static SkPath TestPath() {
        SkPath path;
        path.addRect(SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                                      SkIntToScalar(2), SkIntToScalar(1)));
        return path;
    }
    static SkPath TestNearlyZeroLengthPath() {
        SkPath path;
        SkPoint pt1 = { 0, 0 };
        SkPoint pt2 = { 0, SK_ScalarNearlyZero };
        SkPoint pt3 = { SkIntToScalar(1), 0 };
        SkPoint pt4 = { SkIntToScalar(1), SK_ScalarNearlyZero/2 };
        path.moveTo(pt1);
        path.lineTo(pt2);
        path.lineTo(pt3);
        path.lineTo(pt4);
        return path;
    }
    static SkRegion TestRegion() {
        SkRegion region;
        SkIRect rect = SkIRect::MakeXYWH(0, 0, 2, 1);
        region.setRect(rect);
        return region;
    }
    static SkBitmap TestBitmap() {
        SkBitmap bitmap;
        createBitmap(&bitmap, 0x05060708);
        return bitmap;
    }
};

// Format strings that describe the test context.  The %s token is where
// the name of the test step is inserted.  The context is required for
// disambiguating the error in the case of failures that are reported in
// functions that are called multiple times in different contexts (test
// cases and test steps).
static const char* const kDefaultAssertMessageFormat = "%s";
static const char* const kCanvasDrawAssertMessageFormat =
    "Drawing test step %s with SkCanvas";
static const char* const kPdfAssertMessageFormat =
    "PDF sanity check failed %s";

class CanvasTestStep;
static SkTDArray<CanvasTestStep*>& testStepArray() {
    static SkTDArray<CanvasTestStep*> theTests;
    return theTests;
}

class CanvasTestStep {
public:
    CanvasTestStep(bool fEnablePdfTesting = true) {
        *testStepArray().append() = this;
        fAssertMessageFormat = kDefaultAssertMessageFormat;
        this->fEnablePdfTesting = fEnablePdfTesting;
    }
    virtual ~CanvasTestStep() { }

    virtual void draw(SkCanvas*, const TestData&, skiatest::Reporter*) = 0;
    virtual const char* name() const = 0;

    const char* assertMessage() {
        fAssertMessage.printf(fAssertMessageFormat, name());
        return fAssertMessage.c_str();
    }

    void setAssertMessageFormat(const char* format) {
        fAssertMessageFormat = format;
    }

    bool enablePdfTesting() { return fEnablePdfTesting; }

private:
    SkString fAssertMessage;
    const char* fAssertMessageFormat;
    bool fEnablePdfTesting;
};

///////////////////////////////////////////////////////////////////////////////
// Macros for defining test steps

#define TEST_STEP(NAME, FUNCTION)                                       \
class NAME##_TestStep : public CanvasTestStep{                          \
public:                                                                 \
    virtual void draw(SkCanvas* canvas, const TestData& d,       \
        skiatest::Reporter* reporter) {                                 \
        FUNCTION (canvas, d, reporter, this);                    \
    }                                                                   \
    virtual const char* name() const {return #NAME ;}                   \
};                                                                      \
static NAME##_TestStep NAME##_TestStepInstance;

#define TEST_STEP_NO_PDF(NAME, FUNCTION)                                \
class NAME##_TestStep : public CanvasTestStep{                          \
public:                                                                 \
    NAME##_TestStep() : CanvasTestStep(false) {}                        \
    virtual void draw(SkCanvas* canvas, const TestData& d,       \
        skiatest::Reporter* reporter) {                                 \
        FUNCTION (canvas, d, reporter, this);                    \
    }                                                                   \
    virtual const char* name() const {return #NAME ;}                   \
};                                                                      \
static NAME##_TestStep NAME##_TestStepInstance;

#define SIMPLE_TEST_STEP(NAME, CALL)                                    \
static void NAME##TestStep(SkCanvas* canvas, const TestData& d,  \
    skiatest::Reporter*, CanvasTestStep*) {                             \
    canvas-> CALL ;                                                     \
}                                                                       \
TEST_STEP(NAME, NAME##TestStep )

#define SIMPLE_TEST_STEP_WITH_ASSERT(NAME, CALL)                                         \
    static void NAME##TestStep(SkCanvas* canvas, const TestData& d, skiatest::Reporter*, \
                               CanvasTestStep* testStep) {                               \
        REPORTER_ASSERT(reporter, canvas->CALL, testStep->assertMessage());              \
    }                                                                                    \
    TEST_STEP(NAME, NAME##TestStep)

///////////////////////////////////////////////////////////////////////////////
// Basic test steps for most virtual methods in SkCanvas that draw or affect
// the state of the canvas.

SIMPLE_TEST_STEP(Translate, translate(SkIntToScalar(1), SkIntToScalar(2)));
SIMPLE_TEST_STEP(Scale, scale(SkIntToScalar(1), SkIntToScalar(2)));
SIMPLE_TEST_STEP(Rotate, rotate(SkIntToScalar(1)));
SIMPLE_TEST_STEP(Skew, skew(SkIntToScalar(1), SkIntToScalar(2)));
SIMPLE_TEST_STEP(Concat, concat(d.fMatrix));
SIMPLE_TEST_STEP(SetMatrix, setMatrix(d.fMatrix));
SIMPLE_TEST_STEP(ClipRect, clipRect(d.fRect));
SIMPLE_TEST_STEP(ClipPath, clipPath(d.fPath));
SIMPLE_TEST_STEP(ClipRegion, clipRegion(d.fRegion, kReplace_SkClipOp));
SIMPLE_TEST_STEP(Clear, clear(d.fColor));

///////////////////////////////////////////////////////////////////////////////
// Complex test steps

static void SaveMatrixClipStep(SkCanvas* canvas, const TestData& d,
                               skiatest::Reporter* reporter, CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->save();
    canvas->translate(SkIntToScalar(1), SkIntToScalar(2));
    canvas->clipRegion(d.fRegion);
    canvas->restore();
    REPORTER_ASSERT(reporter, canvas->getSaveCount() == saveCount, testStep->assertMessage());
    REPORTER_ASSERT(reporter, canvas->getTotalMatrix().isIdentity(), testStep->assertMessage());
    //    REPORTER_ASSERT(reporter, canvas->getTotalClip() != kTestRegion,
    //                    testStep->assertMessage());
}
TEST_STEP(SaveMatrixClip, SaveMatrixClipStep);

static void SaveLayerStep(SkCanvas* canvas, const TestData& d,
                          skiatest::Reporter* reporter, CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->saveLayer(nullptr, nullptr);
    canvas->restore();
    REPORTER_ASSERT(reporter, canvas->getSaveCount() == saveCount, testStep->assertMessage());
}
TEST_STEP(SaveLayer, SaveLayerStep);

static void BoundedSaveLayerStep(SkCanvas* canvas, const TestData& d,
                                 skiatest::Reporter* reporter, CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->saveLayer(&d.fRect, nullptr);
    canvas->restore();
    REPORTER_ASSERT(reporter, canvas->getSaveCount() == saveCount, testStep->assertMessage());
}
TEST_STEP(BoundedSaveLayer, BoundedSaveLayerStep);

static void PaintSaveLayerStep(SkCanvas* canvas, const TestData& d,
                               skiatest::Reporter* reporter, CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->saveLayer(nullptr, &d.fPaint);
    canvas->restore();
    REPORTER_ASSERT(reporter, canvas->getSaveCount() == saveCount, testStep->assertMessage());
}
TEST_STEP(PaintSaveLayer, PaintSaveLayerStep);

static void TwoClipOpsStep(SkCanvas* canvas, const TestData& d,
                           skiatest::Reporter*, CanvasTestStep*) {
    // This test exercises a functionality in SkPicture that leads to the
    // recording of restore offset placeholders.  This test will trigger an
    // assertion at playback time if the placeholders are not properly
    // filled when the recording ends.
    canvas->clipRect(d.fRect);
    canvas->clipRegion(d.fRegion);
}
TEST_STEP(TwoClipOps, TwoClipOpsStep);

// exercise fix for http://code.google.com/p/skia/issues/detail?id=560
// ('SkPathStroker::lineTo() fails for line with length SK_ScalarNearlyZero')
static void DrawNearlyZeroLengthPathTestStep(SkCanvas* canvas, const TestData& d,
                                             skiatest::Reporter*, CanvasTestStep*) {
    SkPaint paint;
    paint.setStrokeWidth(SkIntToScalar(1));
    paint.setStyle(SkPaint::kStroke_Style);

    canvas->drawPath(d.fNearlyZeroLengthPath, paint);
}
TEST_STEP(DrawNearlyZeroLengthPath, DrawNearlyZeroLengthPathTestStep);

static void DrawVerticesShaderTestStep(SkCanvas* canvas, const TestData& d,
                                       skiatest::Reporter*, CanvasTestStep*) {
    SkPoint pts[4];
    pts[0].set(0, 0);
    pts[1].set(SkIntToScalar(d.fWidth), 0);
    pts[2].set(SkIntToScalar(d.fWidth), SkIntToScalar(d.fHeight));
    pts[3].set(0, SkIntToScalar(d.fHeight));
    SkPaint paint;
    paint.setShader(SkShader::MakeBitmapShader(d.fBitmap, SkShader::kClamp_TileMode,
                                               SkShader::kClamp_TileMode));
    canvas->drawVertices(SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode, 4, pts, pts,
                                              nullptr),
                         SkBlendMode::kModulate, paint);
}
// NYI: issue 240.
TEST_STEP_NO_PDF(DrawVerticesShader, DrawVerticesShaderTestStep);

static void DrawPictureTestStep(SkCanvas* canvas, const TestData& d,
                                skiatest::Reporter*, CanvasTestStep*) {
    SkPictureRecorder recorder;
    SkCanvas* testCanvas = recorder.beginRecording(SkIntToScalar(d.fWidth), SkIntToScalar(d.fHeight),
                                                   nullptr, 0);
    testCanvas->scale(SkIntToScalar(2), SkIntToScalar(1));
    testCanvas->clipRect(d.fRect);
    testCanvas->drawRect(d.fRect, d.fPaint);

    canvas->drawPicture(recorder.finishRecordingAsPicture());
}
TEST_STEP(DrawPicture, DrawPictureTestStep);

static void SaveRestoreTestStep(SkCanvas* canvas, const TestData& d,
                                skiatest::Reporter* reporter, CanvasTestStep* testStep) {
    int baseSaveCount = canvas->getSaveCount();
    int n = canvas->save();
    REPORTER_ASSERT(reporter, baseSaveCount == n, testStep->assertMessage());
    REPORTER_ASSERT(reporter, baseSaveCount + 1 == canvas->getSaveCount(),
                    testStep->assertMessage());
    canvas->save();
    canvas->save();
    REPORTER_ASSERT(reporter, baseSaveCount + 3 == canvas->getSaveCount(),
                    testStep->assertMessage());
    canvas->restoreToCount(baseSaveCount + 1);
    REPORTER_ASSERT(reporter, baseSaveCount + 1 == canvas->getSaveCount(),
                    testStep->assertMessage());

    // should this pin to 1, or be a no-op, or crash?
    canvas->restoreToCount(0);
    REPORTER_ASSERT(reporter, 1 == canvas->getSaveCount(), testStep->assertMessage());
}
TEST_STEP(SaveRestore, SaveRestoreTestStep);

static void NestedSaveRestoreWithSolidPaintTestStep(SkCanvas* canvas, const TestData& d,
                                                    skiatest::Reporter*, CanvasTestStep*) {
    // This test step challenges the TestDeferredCanvasStateConsistency
    // test cases because the opaque paint can trigger an optimization
    // that discards previously recorded commands. The challenge is to maintain
    // correct clip and matrix stack state.
    canvas->resetMatrix();
    canvas->rotate(SkIntToScalar(30));
    canvas->save();
    canvas->translate(SkIntToScalar(2), SkIntToScalar(1));
    canvas->save();
    canvas->scale(SkIntToScalar(3), SkIntToScalar(3));
    SkPaint paint;
    paint.setColor(0xFFFFFFFF);
    canvas->drawPaint(paint);
    canvas->restore();
    canvas->restore();
}
TEST_STEP(NestedSaveRestoreWithSolidPaint, \
    NestedSaveRestoreWithSolidPaintTestStep);

static void NestedSaveRestoreWithFlushTestStep(SkCanvas* canvas, const TestData& d,
                                               skiatest::Reporter*, CanvasTestStep*) {
    // This test step challenges the TestDeferredCanvasStateConsistency
    // test case because the canvas flush on a deferred canvas will
    // reset the recording session. The challenge is to maintain correct
    // clip and matrix stack state on the playback canvas.
    canvas->resetMatrix();
    canvas->rotate(SkIntToScalar(30));
    canvas->save();
    canvas->translate(SkIntToScalar(2), SkIntToScalar(1));
    canvas->save();
    canvas->scale(SkIntToScalar(3), SkIntToScalar(3));
    canvas->drawRect(d.fRect,d.fPaint);
    canvas->flush();
    canvas->restore();
    canvas->restore();
}
TEST_STEP(NestedSaveRestoreWithFlush, NestedSaveRestoreWithFlushTestStep);

static void TestPdfDevice(skiatest::Reporter* reporter, const TestData& d, CanvasTestStep* step) {
    SkDynamicMemoryWStream outStream;
    auto doc = SkPDF::MakeDocument(&outStream);
    if (!doc) {
        INFOF(reporter, "PDF disabled; TestPdfDevice test skipped.");
        return;
    }
    SkCanvas* canvas = doc->beginPage(SkIntToScalar(d.fWidth),
                                      SkIntToScalar(d.fHeight));
    REPORTER_ASSERT(reporter, canvas);
    step->setAssertMessageFormat(kPdfAssertMessageFormat);
    step->draw(canvas, d, reporter);
}

/*
 * This sub-test verifies that the test step passes when executed
 * with SkCanvas and with classes derrived from SkCanvas. It also verifies
 * that the all canvas derivatives report the same state as an SkCanvas
 * after having executed the test step.
 */
static void TestOverrideStateConsistency(skiatest::Reporter* reporter, const TestData& d,
                                         CanvasTestStep* testStep) {
    SkBitmap referenceStore;
    createBitmap(&referenceStore, 0xFFFFFFFF);
    SkCanvas referenceCanvas(referenceStore);
    testStep->setAssertMessageFormat(kCanvasDrawAssertMessageFormat);
    testStep->draw(&referenceCanvas, d, reporter);
}

static void test_newraster(skiatest::Reporter* reporter) {
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

DEF_TEST(Canvas, reporter) {
    TestData d;

    for (int testStep = 0; testStep < testStepArray().count(); testStep++) {
        TestOverrideStateConsistency(reporter, d, testStepArray()[testStep]);
        if (testStepArray()[testStep]->enablePdfTesting()) {
            TestPdfDevice(reporter, d, testStepArray()[testStep]);
        }
    }

    test_newraster(reporter);
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
    bool onFilter(SkTCopyOnFirstWrite<SkPaint>*, Type) const override { return true; }

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

#include "SkPaintImageFilter.h"

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

#include "SkBlurImageFilter.h"

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

