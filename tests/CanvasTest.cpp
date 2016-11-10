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
 *              REPORTER_ASSERT_MESSAGE(reporter, (...), \
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
#include "SkCanvas.h"
#include "SkClipStack.h"
#include "SkDocument.h"
#include "SkMatrix.h"
#include "SkNWayCanvas.h"
#include "SkPaint.h"
#include "SkPaintFilterCanvas.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkPictureRecord.h"
#include "SkPictureRecorder.h"
#include "SkRasterClip.h"
#include "SkRect.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTemplates.h"
#include "SkTDArray.h"
#include "Test.h"

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

class Canvas2CanvasClipVisitor : public SkCanvas::ClipVisitor {
public:
    Canvas2CanvasClipVisitor(SkCanvas* target) : fTarget(target) {}

    void clipRect(const SkRect& r, SkCanvas::ClipOp op, bool aa) override {
        fTarget->clipRect(r, op, aa);
    }
    void clipRRect(const SkRRect& r, SkCanvas::ClipOp op, bool aa) override {
        fTarget->clipRRect(r, op, aa);
    }
    void clipPath(const SkPath& p, SkCanvas::ClipOp op, bool aa) override {
        fTarget->clipPath(p, op, aa);
    }

private:
    SkCanvas* fTarget;
};

static void test_clipstack(skiatest::Reporter* reporter) {
    // The clipstack is refcounted, and needs to be able to out-live the canvas if a client has
    // ref'd it.
    const SkClipStack* cs = nullptr;
    {
        SkCanvas canvas(10, 10);
        cs = SkRef(canvas.getClipStack());
    }
    REPORTER_ASSERT(reporter, cs->unique());
    cs->unref();
}

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

#define SIMPLE_TEST_STEP_WITH_ASSERT(NAME, CALL)                           \
static void NAME##TestStep(SkCanvas* canvas, const TestData& d,     \
    skiatest::Reporter*, CanvasTestStep* testStep) {                       \
    REPORTER_ASSERT_MESSAGE(reporter, canvas-> CALL ,                      \
        testStep->assertMessage());                                        \
}                                                                          \
TEST_STEP(NAME, NAME##TestStep )


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
SIMPLE_TEST_STEP(ClipRegion, clipRegion(d.fRegion, SkCanvas::kReplace_Op));
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
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getSaveCount() == saveCount,
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getTotalMatrix().isIdentity(),
        testStep->assertMessage());
//    REPORTER_ASSERT_MESSAGE(reporter, canvas->getTotalClip() != kTestRegion, testStep->assertMessage());
}
TEST_STEP(SaveMatrixClip, SaveMatrixClipStep);

static void SaveLayerStep(SkCanvas* canvas, const TestData& d,
                          skiatest::Reporter* reporter, CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->saveLayer(nullptr, nullptr);
    canvas->restore();
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getSaveCount() == saveCount,
        testStep->assertMessage());
}
TEST_STEP(SaveLayer, SaveLayerStep);

static void BoundedSaveLayerStep(SkCanvas* canvas, const TestData& d,
                                 skiatest::Reporter* reporter, CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->saveLayer(&d.fRect, nullptr);
    canvas->restore();
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getSaveCount() == saveCount,
        testStep->assertMessage());
}
TEST_STEP(BoundedSaveLayer, BoundedSaveLayerStep);

static void PaintSaveLayerStep(SkCanvas* canvas, const TestData& d,
                               skiatest::Reporter* reporter, CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->saveLayer(nullptr, &d.fPaint);
    canvas->restore();
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getSaveCount() == saveCount,
        testStep->assertMessage());
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
    canvas->drawVertices(SkCanvas::kTriangleFan_VertexMode, 4, pts, pts,
                         nullptr, SkBlendMode::kModulate, nullptr, 0, paint);
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
    REPORTER_ASSERT_MESSAGE(reporter, baseSaveCount == n, testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, baseSaveCount + 1 == canvas->getSaveCount(),
        testStep->assertMessage());
    canvas->save();
    canvas->save();
    REPORTER_ASSERT_MESSAGE(reporter, baseSaveCount + 3 == canvas->getSaveCount(),
        testStep->assertMessage());
    canvas->restoreToCount(baseSaveCount + 1);
    REPORTER_ASSERT_MESSAGE(reporter, baseSaveCount + 1 == canvas->getSaveCount(),
        testStep->assertMessage());

    // should this pin to 1, or be a no-op, or crash?
    canvas->restoreToCount(0);
    REPORTER_ASSERT_MESSAGE(reporter, 1 == canvas->getSaveCount(),
        testStep->assertMessage());
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

static void DescribeTopLayerTestStep(SkCanvas* canvas,
                                     const TestData& d,
                                     skiatest::Reporter* reporter,
                                     CanvasTestStep* testStep) {
    SkMatrix m;
    SkIRect r;
    // NOTE: adjustToTopLayer() does *not* reduce the clip size, even if the canvas
    // is smaller than 10x10!

    canvas->temporary_internal_describeTopLayer(&m, &r);
    REPORTER_ASSERT_MESSAGE(reporter, m.isIdentity(), testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, r == SkIRect::MakeXYWH(0, 0, 2, 2),
                            testStep->assertMessage());

    // Putting a full-canvas layer on it should make no change to the results.
    SkRect layerBounds = SkRect::MakeXYWH(0.f, 0.f, 10.f, 10.f);
    canvas->saveLayer(layerBounds, nullptr);
    canvas->temporary_internal_describeTopLayer(&m, &r);
    REPORTER_ASSERT_MESSAGE(reporter, m.isIdentity(), testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, r == SkIRect::MakeXYWH(0, 0, 2, 2),
                            testStep->assertMessage());
    canvas->restore();

    // Adding a translated layer translates the results.
    // Default canvas is only 2x2, so can't offset our layer by very much at all;
    // saveLayer() aborts if the bounds don't intersect.
    layerBounds = SkRect::MakeXYWH(1.f, 1.f, 6.f, 6.f);
    canvas->saveLayer(layerBounds, nullptr);
    canvas->temporary_internal_describeTopLayer(&m, &r);
    REPORTER_ASSERT_MESSAGE(reporter, m == SkMatrix::MakeTrans(-1.f, -1.f),
                            testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, r == SkIRect::MakeXYWH(0, 0, 1, 1),
                            testStep->assertMessage());
    canvas->restore();

}
TEST_STEP(DescribeTopLayer, DescribeTopLayerTestStep);


static void TestPdfDevice(skiatest::Reporter* reporter, const TestData& d, CanvasTestStep* step) {
    SkDynamicMemoryWStream outStream;
    sk_sp<SkDocument> doc(SkDocument::MakePDF(&outStream));
    REPORTER_ASSERT(reporter, doc);
    if (!doc) {
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

    test_clipstack(reporter);
}

static void test_newraster(skiatest::Reporter* reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);
    const size_t minRowBytes = info.minRowBytes();
    const size_t size = info.getSafeSize(minRowBytes);
    SkAutoTMalloc<SkPMColor> storage(size);
    SkPMColor* baseAddr = storage.get();
    sk_bzero(baseAddr, size);

    SkCanvas* canvas = SkCanvas::NewRasterDirect(info, baseAddr, minRowBytes);
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
    delete canvas;

    // now try a deliberately bad info
    info = info.makeWH(-1, info.height());
    REPORTER_ASSERT(reporter, nullptr == SkCanvas::NewRasterDirect(info, baseAddr, minRowBytes));

    // too big
    info = info.makeWH(1 << 30, 1 << 30);
    REPORTER_ASSERT(reporter, nullptr == SkCanvas::NewRasterDirect(info, baseAddr, minRowBytes));

    // not a valid pixel type
    info = SkImageInfo::Make(10, 10, kUnknown_SkColorType, info.alphaType());
    REPORTER_ASSERT(reporter, nullptr == SkCanvas::NewRasterDirect(info, baseAddr, minRowBytes));

    // We should succeed with a zero-sized valid info
    info = SkImageInfo::MakeN32Premul(0, 0);
    canvas = SkCanvas::NewRasterDirect(info, baseAddr, minRowBytes);
    REPORTER_ASSERT(reporter, canvas);
    delete canvas;
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

#define SHADOW_TEST_CANVAS_CONST 10
#ifdef SK_EXPERIMENTAL_SHADOWING
class SkShadowTestCanvas : public SkPaintFilterCanvas {
public:

    SkShadowTestCanvas(int x, int y, skiatest::Reporter* reporter)
        : INHERITED(x,y)
        , fReporter(reporter) {}

    bool onFilter(SkTCopyOnFirstWrite<SkPaint>* paint, Type type) const {
        REPORTER_ASSERT(this->fReporter, this->getZ() == SHADOW_TEST_CANVAS_CONST);

        return true;
    }

    void testUpdateDepth(skiatest::Reporter *reporter) {
        // set some depths (with picture enabled), then check them as they get set

        REPORTER_ASSERT(reporter, this->getZ() == 0);
        this->translateZ(-10);
        REPORTER_ASSERT(reporter, this->getZ() == -10);

        this->save();
        this->translateZ(20);
        REPORTER_ASSERT(reporter, this->getZ() == 10);

        this->restore();
        REPORTER_ASSERT(reporter, this->getZ() == -10);

        this->translateZ(13.14f);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(this->getZ(), 3.14f));
    }

private:
    skiatest::Reporter* fReporter;

    typedef SkPaintFilterCanvas INHERITED;
};
#endif

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

    SkRect clip1, clip2;

    MockFilterCanvas filterCanvas(&canvas);
    REPORTER_ASSERT(reporter, canvas.getTotalMatrix() == filterCanvas.getTotalMatrix());
    REPORTER_ASSERT(reporter, canvas.getClipBounds(&clip1) == filterCanvas.getClipBounds(&clip2));
    REPORTER_ASSERT(reporter, clip1 == clip2);

    filterCanvas.clipRect(SkRect::MakeXYWH(30.5f, 30.7f, 100, 100));
    filterCanvas.scale(0.75f, 0.5f);
    REPORTER_ASSERT(reporter, canvas.getTotalMatrix() == filterCanvas.getTotalMatrix());
    REPORTER_ASSERT(reporter, canvas.getClipBounds(&clip1) == filterCanvas.getClipBounds(&clip2));
    REPORTER_ASSERT(reporter, clip1 == clip2);

#ifdef SK_EXPERIMENTAL_SHADOWING
    SkShadowTestCanvas* tCanvas = new SkShadowTestCanvas(100,100, reporter);
    tCanvas->testUpdateDepth(reporter);
    delete(tCanvas);

    SkPictureRecorder recorder;
    SkShadowTestCanvas *tSCanvas = new SkShadowTestCanvas(100, 100, reporter);
    SkCanvas *tPCanvas = recorder.beginRecording(SkRect::MakeIWH(100, 100));

    tPCanvas->translateZ(SHADOW_TEST_CANVAS_CONST);
    sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
    tSCanvas->drawPicture(pic);

    delete(tSCanvas);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkDeferredCanvas.h"
#include "SkDumpCanvas.h"

DEF_TEST(DeferredCanvas, r) {
    SkDebugfDumper dumper;
    SkDumpCanvas dumpC(&dumper);

    SkDeferredCanvas canvas(&dumpC);

    SkPaint paint;
//    paint.setShader(SkShader::MakeColorShader(SK_ColorRED));

    canvas.save();
    canvas.clipRect(SkRect::MakeWH(55, 55));
    canvas.translate(10, 20);
    canvas.drawRect(SkRect::MakeWH(50, 50), paint);
    canvas.restore();
}

