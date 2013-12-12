
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
#include "SkDeferredCanvas.h"
#include "SkDevice.h"
#include "SkMatrix.h"
#include "SkNWayCanvas.h"
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkPictureRecord.h"
#include "SkProxyCanvas.h"
#include "SkRect.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "Test.h"
#include "TestClassDef.h"

class Canvas2CanvasClipVisitor : public SkCanvas::ClipVisitor {
public:
    Canvas2CanvasClipVisitor(SkCanvas* target) : fTarget(target) {}

    virtual void clipRect(const SkRect& r, SkRegion::Op op, bool aa) {
        fTarget->clipRect(r, op, aa);
    }
    virtual void clipPath(const SkPath& p, SkRegion::Op op, bool aa) {
        fTarget->clipPath(p, op, aa);
    }

private:
    SkCanvas* fTarget;
};

static void test_clipVisitor(skiatest::Reporter* reporter, SkCanvas* canvas) {
    SkISize size = canvas->getDeviceSize();

    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, size.width(), size.height());
    SkCanvas c(bm);

    Canvas2CanvasClipVisitor visitor(&c);
    canvas->replayClips(&visitor);

    REPORTER_ASSERT(reporter, c.getTotalClip() == canvas->getTotalClip());
}

static const int kWidth = 2;
static const int kHeight = 2;

// Format strings that describe the test context.  The %s token is where
// the name of the test step is inserted.  The context is required for
// disambiguating the error in the case of failures that are reported in
// functions that are called multiple times in different contexts (test
// cases and test steps).
static const char* const kDefaultAssertMessageFormat = "%s";
static const char* const kCanvasDrawAssertMessageFormat =
    "Drawing test step %s with SkCanvas";
static const char* const kPictureDrawAssertMessageFormat =
    "Drawing test step %s with SkPicture";
static const char* const kPictureSecondDrawAssertMessageFormat =
    "Duplicate draw of test step %s with SkPicture";
static const char* const kDeferredDrawAssertMessageFormat =
    "Drawing test step %s with SkDeferredCanvas";
static const char* const kProxyDrawAssertMessageFormat =
    "Drawing test step %s with SkProxyCanvas";
static const char* const kNWayDrawAssertMessageFormat =
    "Drawing test step %s with SkNWayCanvas";
static const char* const kDeferredPreFlushAssertMessageFormat =
    "test step %s, SkDeferredCanvas state consistency before flush";
static const char* const kDeferredPostFlushPlaybackAssertMessageFormat =
    "test step %s, SkDeferredCanvas playback canvas state consistency after flush";
static const char* const kDeferredPostSilentFlushPlaybackAssertMessageFormat =
    "test step %s, SkDeferredCanvas playback canvas state consistency after silent flush";
static const char* const kPictureResourceReuseMessageFormat =
    "test step %s, SkPicture duplicate flattened object test";
static const char* const kProxyStateAssertMessageFormat =
    "test step %s, SkProxyCanvas state consistency";
static const char* const kProxyIndirectStateAssertMessageFormat =
    "test step %s, SkProxyCanvas indirect canvas state consistency";
static const char* const kNWayStateAssertMessageFormat =
    "test step %s, SkNWayCanvas state consistency";
static const char* const kNWayIndirect1StateAssertMessageFormat =
    "test step %s, SkNWayCanvas indirect canvas 1 state consistency";
static const char* const kNWayIndirect2StateAssertMessageFormat =
    "test step %s, SkNWayCanvas indirect canvas 2 state consistency";
static const char* const kPdfAssertMessageFormat =
    "PDF sanity check failed %s";

static void createBitmap(SkBitmap* bm, SkBitmap::Config config, SkColor color) {
    bm->setConfig(config, kWidth, kHeight);
    bm->allocPixels();
    bm->eraseColor(color);
}

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

    virtual void draw(SkCanvas*, skiatest::Reporter*) = 0;
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
// Constants used by test steps

const SkRect kTestRect =
    SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                     SkIntToScalar(2), SkIntToScalar(1));
static SkMatrix testMatrix() {
    SkMatrix matrix;
    matrix.reset();
    matrix.setScale(SkIntToScalar(2), SkIntToScalar(3));
    return matrix;
}
const SkMatrix kTestMatrix = testMatrix();
static SkPath testPath() {
    SkPath path;
    path.addRect(SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                                  SkIntToScalar(2), SkIntToScalar(1)));
    return path;
}
const SkPath kTestPath = testPath();
static SkRegion testRegion() {
    SkRegion region;
    SkIRect rect = SkIRect::MakeXYWH(0, 0, 2, 1);
    region.setRect(rect);
    return region;
}
const SkIRect kTestIRect = SkIRect::MakeXYWH(0, 0, 2, 1);
const SkRegion kTestRegion = testRegion();
const SkColor kTestColor = 0x01020304;
const SkPaint kTestPaint;
const SkPoint kTestPoints[3] = {
    {SkIntToScalar(0), SkIntToScalar(0)},
    {SkIntToScalar(2), SkIntToScalar(1)},
    {SkIntToScalar(0), SkIntToScalar(2)}
};
const size_t kTestPointCount = 3;
static SkBitmap testBitmap() {
    SkBitmap bitmap;
    createBitmap(&bitmap, SkBitmap::kARGB_8888_Config, 0x05060708);
    return bitmap;
}
SkBitmap kTestBitmap; // cannot be created during static init
SkString kTestText("Hello World");
SkPoint kTestPoints2[] = {
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
  { SkIntToScalar(10), SkIntToScalar(1) },
};


///////////////////////////////////////////////////////////////////////////////
// Macros for defining test steps

#define TEST_STEP(NAME, FUNCTION)                                       \
class NAME##_TestStep : public CanvasTestStep{                          \
public:                                                                 \
    virtual void draw(SkCanvas* canvas, skiatest::Reporter* reporter) { \
        FUNCTION (canvas, reporter, this);                              \
    }                                                                   \
    virtual const char* name() const {return #NAME ;}                   \
};                                                                      \
static NAME##_TestStep NAME##_TestStepInstance;

#define TEST_STEP_NO_PDF(NAME, FUNCTION)                                       \
class NAME##_TestStep : public CanvasTestStep{                          \
public:                                                                 \
    NAME##_TestStep() : CanvasTestStep(false) {}                        \
    virtual void draw(SkCanvas* canvas, skiatest::Reporter* reporter) { \
        FUNCTION (canvas, reporter, this);                              \
    }                                                                   \
    virtual const char* name() const {return #NAME ;}                   \
};                                                                      \
static NAME##_TestStep NAME##_TestStepInstance;

#define SIMPLE_TEST_STEP(NAME, CALL)                              \
static void NAME##TestStep(SkCanvas* canvas, skiatest::Reporter*, \
    CanvasTestStep*) {                                            \
    canvas-> CALL ;                                               \
}                                                                 \
TEST_STEP(NAME, NAME##TestStep )

#define SIMPLE_TEST_STEP_WITH_ASSERT(NAME, CALL)                           \
static void NAME##TestStep(SkCanvas* canvas, skiatest::Reporter* reporter, \
    CanvasTestStep* testStep) {                                            \
    REPORTER_ASSERT_MESSAGE(reporter, canvas-> CALL ,                      \
        testStep->assertMessage());                                        \
}                                                                          \
TEST_STEP(NAME, NAME##TestStep )


///////////////////////////////////////////////////////////////////////////////
// Basic test steps for most virtual methods in SkCanvas that draw or affect
// the state of the canvas.

SIMPLE_TEST_STEP_WITH_ASSERT(Translate,
    translate(SkIntToScalar(1), SkIntToScalar(2)));
SIMPLE_TEST_STEP_WITH_ASSERT(Scale,
    scale(SkIntToScalar(1), SkIntToScalar(2)));
SIMPLE_TEST_STEP_WITH_ASSERT(Rotate, rotate(SkIntToScalar(1)));
SIMPLE_TEST_STEP_WITH_ASSERT(Skew,
    skew(SkIntToScalar(1), SkIntToScalar(2)));
SIMPLE_TEST_STEP_WITH_ASSERT(Concat, concat(kTestMatrix));
SIMPLE_TEST_STEP(SetMatrix, setMatrix(kTestMatrix));
SIMPLE_TEST_STEP(ClipRect, clipRect(kTestRect));
SIMPLE_TEST_STEP(ClipPath, clipPath(kTestPath));
SIMPLE_TEST_STEP(ClipRegion,
    clipRegion(kTestRegion, SkRegion::kReplace_Op));
SIMPLE_TEST_STEP(Clear, clear(kTestColor));
SIMPLE_TEST_STEP(DrawPaint, drawPaint(kTestPaint));
SIMPLE_TEST_STEP(DrawPointsPoints, drawPoints(SkCanvas::kPoints_PointMode,
    kTestPointCount, kTestPoints, kTestPaint));
SIMPLE_TEST_STEP(DrawPointsLiness, drawPoints(SkCanvas::kLines_PointMode,
    kTestPointCount, kTestPoints, kTestPaint));
SIMPLE_TEST_STEP(DrawPointsPolygon, drawPoints(SkCanvas::kPolygon_PointMode,
    kTestPointCount, kTestPoints, kTestPaint));
SIMPLE_TEST_STEP(DrawRect, drawRect(kTestRect, kTestPaint));
SIMPLE_TEST_STEP(DrawPath, drawPath(kTestPath, kTestPaint));
SIMPLE_TEST_STEP(DrawBitmap, drawBitmap(kTestBitmap, 0, 0));
SIMPLE_TEST_STEP(DrawBitmapPaint, drawBitmap(kTestBitmap, 0, 0, &kTestPaint));
SIMPLE_TEST_STEP(DrawBitmapRect, drawBitmapRect(kTestBitmap, NULL, kTestRect,
    NULL));
SIMPLE_TEST_STEP(DrawBitmapRectSrcRect, drawBitmapRect(kTestBitmap,
    &kTestIRect, kTestRect, NULL));
SIMPLE_TEST_STEP(DrawBitmapRectPaint, drawBitmapRect(kTestBitmap, NULL,
    kTestRect, &kTestPaint));
SIMPLE_TEST_STEP(DrawBitmapMatrix, drawBitmapMatrix(kTestBitmap, kTestMatrix,
    NULL));
SIMPLE_TEST_STEP(DrawBitmapMatrixPaint, drawBitmapMatrix(kTestBitmap,
    kTestMatrix, &kTestPaint));
SIMPLE_TEST_STEP(DrawBitmapNine, drawBitmapNine(kTestBitmap, kTestIRect,
    kTestRect, NULL));
SIMPLE_TEST_STEP(DrawBitmapNinePaint, drawBitmapNine(kTestBitmap, kTestIRect,
    kTestRect, &kTestPaint));
SIMPLE_TEST_STEP(DrawSprite, drawSprite(kTestBitmap, 0, 0, NULL));
SIMPLE_TEST_STEP(DrawSpritePaint, drawSprite(kTestBitmap, 0, 0, &kTestPaint));
SIMPLE_TEST_STEP(DrawText, drawText(kTestText.c_str(), kTestText.size(),
    0, 1, kTestPaint));
SIMPLE_TEST_STEP(DrawPosText, drawPosText(kTestText.c_str(),
    kTestText.size(), kTestPoints2, kTestPaint));
SIMPLE_TEST_STEP(DrawTextOnPath, drawTextOnPath(kTestText.c_str(),
    kTestText.size(), kTestPath, NULL, kTestPaint));
SIMPLE_TEST_STEP(DrawTextOnPathMatrix, drawTextOnPath(kTestText.c_str(),
    kTestText.size(), kTestPath, &kTestMatrix, kTestPaint));
SIMPLE_TEST_STEP(DrawData, drawData(kTestText.c_str(), kTestText.size()));
SIMPLE_TEST_STEP(BeginGroup, beginCommentGroup(kTestText.c_str()));
SIMPLE_TEST_STEP(AddComment, addComment(kTestText.c_str(), kTestText.c_str()));
SIMPLE_TEST_STEP(EndGroup, endCommentGroup());

///////////////////////////////////////////////////////////////////////////////
// Complex test steps

// Save/restore calls cannot be in isolated simple test steps because the test
// cases that use SkPicture require that save and restore calls be balanced.
static void SaveMatrixStep(SkCanvas* canvas,
                           skiatest::Reporter* reporter,
                           CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->save(SkCanvas::kMatrix_SaveFlag);
    canvas->clipRegion(kTestRegion);
    canvas->translate(SkIntToScalar(1), SkIntToScalar(2));
    canvas->restore();
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getSaveCount() == saveCount,
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getTotalMatrix().isIdentity(),
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getTotalClip() == kTestRegion,
        testStep->assertMessage());
}
TEST_STEP(SaveMatrix, SaveMatrixStep);

static void SaveClipStep(SkCanvas* canvas,
                         skiatest::Reporter* reporter,
                         CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->save(SkCanvas::kClip_SaveFlag);
    canvas->translate(SkIntToScalar(1), SkIntToScalar(2));
    canvas->clipRegion(kTestRegion);
    canvas->restore();
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getSaveCount() == saveCount,
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, !canvas->getTotalMatrix().isIdentity(),
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getTotalClip() != kTestRegion,
        testStep->assertMessage());
}
TEST_STEP(SaveClip, SaveClipStep);

static void SaveMatrixClipStep(SkCanvas* canvas,
                               skiatest::Reporter* reporter,
                               CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->save(SkCanvas::kMatrixClip_SaveFlag);
    canvas->translate(SkIntToScalar(1), SkIntToScalar(2));
    canvas->clipRegion(kTestRegion);
    canvas->restore();
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getSaveCount() == saveCount,
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getTotalMatrix().isIdentity(),
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getTotalClip() != kTestRegion,
        testStep->assertMessage());
}
TEST_STEP(SaveMatrixClip, SaveMatrixClipStep);

static void SaveLayerStep(SkCanvas* canvas,
                          skiatest::Reporter* reporter,
                          CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->saveLayer(NULL, NULL);
    canvas->restore();
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getSaveCount() == saveCount,
        testStep->assertMessage());
}
TEST_STEP(SaveLayer, SaveLayerStep);

static void BoundedSaveLayerStep(SkCanvas* canvas,
                          skiatest::Reporter* reporter,
                          CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->saveLayer(&kTestRect, NULL);
    canvas->restore();
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getSaveCount() == saveCount,
        testStep->assertMessage());
}
TEST_STEP(BoundedSaveLayer, BoundedSaveLayerStep);

static void PaintSaveLayerStep(SkCanvas* canvas,
                          skiatest::Reporter* reporter,
                          CanvasTestStep* testStep) {
    int saveCount = canvas->getSaveCount();
    canvas->saveLayer(NULL, &kTestPaint);
    canvas->restore();
    REPORTER_ASSERT_MESSAGE(reporter, canvas->getSaveCount() == saveCount,
        testStep->assertMessage());
}
TEST_STEP(PaintSaveLayer, PaintSaveLayerStep);

static void TwoClipOpsStep(SkCanvas* canvas,
                           skiatest::Reporter*,
                           CanvasTestStep*) {
    // This test exercises a functionality in SkPicture that leads to the
    // recording of restore offset placeholders.  This test will trigger an
    // assertion at playback time if the placeholders are not properly
    // filled when the recording ends.
    canvas->clipRect(kTestRect);
    canvas->clipRegion(kTestRegion);
}
TEST_STEP(TwoClipOps, TwoClipOpsStep);

// exercise fix for http://code.google.com/p/skia/issues/detail?id=560
// ('SkPathStroker::lineTo() fails for line with length SK_ScalarNearlyZero')
static void DrawNearlyZeroLengthPathTestStep(SkCanvas* canvas,
                                             skiatest::Reporter*,
                                             CanvasTestStep*) {
    SkPaint paint;
    paint.setStrokeWidth(SkIntToScalar(1));
    paint.setStyle(SkPaint::kStroke_Style);

    SkPath path;
    SkPoint pt1 = { 0, 0 };
    SkPoint pt2 = { 0, SK_ScalarNearlyZero };
    SkPoint pt3 = { SkIntToScalar(1), 0 };
    SkPoint pt4 = { SkIntToScalar(1), SK_ScalarNearlyZero/2 };
    path.moveTo(pt1);
    path.lineTo(pt2);
    path.lineTo(pt3);
    path.lineTo(pt4);

    canvas->drawPath(path, paint);
}
TEST_STEP(DrawNearlyZeroLengthPath, DrawNearlyZeroLengthPathTestStep);

static void DrawVerticesShaderTestStep(SkCanvas* canvas,
                                       skiatest::Reporter*,
                                       CanvasTestStep*) {
    SkPoint pts[4];
    pts[0].set(0, 0);
    pts[1].set(SkIntToScalar(kWidth), 0);
    pts[2].set(SkIntToScalar(kWidth), SkIntToScalar(kHeight));
    pts[3].set(0, SkIntToScalar(kHeight));
    SkPaint paint;
    SkShader* shader = SkShader::CreateBitmapShader(kTestBitmap,
        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
    paint.setShader(shader)->unref();
    canvas->drawVertices(SkCanvas::kTriangleFan_VertexMode, 4, pts, pts,
                         NULL, NULL, NULL, 0, paint);
}
// NYI: issue 240.
TEST_STEP_NO_PDF(DrawVerticesShader, DrawVerticesShaderTestStep);

static void DrawPictureTestStep(SkCanvas* canvas,
                                skiatest::Reporter*,
                                CanvasTestStep*) {
    SkPicture* testPicture = SkNEW_ARGS(SkPicture, ());
    SkAutoUnref aup(testPicture);
    SkCanvas* testCanvas = testPicture->beginRecording(kWidth, kHeight);
    testCanvas->scale(SkIntToScalar(2), SkIntToScalar(1));
    testCanvas->clipRect(kTestRect);
    testCanvas->drawRect(kTestRect, kTestPaint);
    canvas->drawPicture(*testPicture);
}
TEST_STEP(DrawPicture, DrawPictureTestStep);

static void SaveRestoreTestStep(SkCanvas* canvas,
                                skiatest::Reporter* reporter,
                                CanvasTestStep* testStep) {
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

static void DrawLayerTestStep(SkCanvas* canvas,
                              skiatest::Reporter* reporter,
                              CanvasTestStep* testStep) {
    REPORTER_ASSERT_MESSAGE(reporter, !canvas->isDrawingToLayer(),
        testStep->assertMessage());
    canvas->save();
    REPORTER_ASSERT_MESSAGE(reporter, !canvas->isDrawingToLayer(),
        testStep->assertMessage());
    canvas->restore();

    const SkRect* bounds = NULL;    // null means include entire bounds
    const SkPaint* paint = NULL;

    canvas->saveLayer(bounds, paint);
    REPORTER_ASSERT_MESSAGE(reporter, canvas->isDrawingToLayer(),
        testStep->assertMessage());
    canvas->restore();
    REPORTER_ASSERT_MESSAGE(reporter, !canvas->isDrawingToLayer(),
        testStep->assertMessage());

    canvas->saveLayer(bounds, paint);
    canvas->saveLayer(bounds, paint);
    REPORTER_ASSERT_MESSAGE(reporter, canvas->isDrawingToLayer(),
        testStep->assertMessage());
    canvas->restore();
    REPORTER_ASSERT_MESSAGE(reporter, canvas->isDrawingToLayer(),
        testStep->assertMessage());
    canvas->restore();
    // now layer count should be 0
    REPORTER_ASSERT_MESSAGE(reporter, !canvas->isDrawingToLayer(),
        testStep->assertMessage());
}
TEST_STEP(DrawLayer, DrawLayerTestStep);

static void NestedSaveRestoreWithSolidPaintTestStep(SkCanvas* canvas,
                                      skiatest::Reporter*,
                                      CanvasTestStep*) {
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

static void NestedSaveRestoreWithFlushTestStep(SkCanvas* canvas,
                                      skiatest::Reporter*,
                                      CanvasTestStep*) {
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
    canvas->drawRect(kTestRect,kTestPaint);
    canvas->flush();
    canvas->restore();
    canvas->restore();
}
TEST_STEP(NestedSaveRestoreWithFlush, \
    NestedSaveRestoreWithFlushTestStep);

static void AssertCanvasStatesEqual(skiatest::Reporter* reporter,
                                    const SkCanvas* canvas1,
                                    const SkCanvas* canvas2,
                                    CanvasTestStep* testStep) {
    REPORTER_ASSERT_MESSAGE(reporter, canvas1->getDeviceSize() ==
        canvas2->getDeviceSize(), testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas1->getSaveCount() ==
        canvas2->getSaveCount(), testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas1->isDrawingToLayer() ==
        canvas2->isDrawingToLayer(), testStep->assertMessage());

    SkRect bounds1, bounds2;
    REPORTER_ASSERT_MESSAGE(reporter,
        canvas1->getClipBounds(&bounds1) == canvas2->getClipBounds(&bounds2),
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, bounds1 == bounds2,
                            testStep->assertMessage());

    REPORTER_ASSERT_MESSAGE(reporter, canvas1->getDrawFilter() ==
        canvas2->getDrawFilter(), testStep->assertMessage());
    SkIRect deviceBounds1, deviceBounds2;
    REPORTER_ASSERT_MESSAGE(reporter,
        canvas1->getClipDeviceBounds(&deviceBounds1) ==
        canvas2->getClipDeviceBounds(&deviceBounds2),
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, deviceBounds1 == deviceBounds2,
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas1->getBounder() ==
        canvas2->getBounder(), testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas1->getTotalMatrix() ==
        canvas2->getTotalMatrix(), testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas1->getClipType() ==
        canvas2->getClipType(), testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, canvas1->getTotalClip() ==
        canvas2->getTotalClip(), testStep->assertMessage());

    // The following test code is commented out because the test fails when
    // the canvas is an SkPictureRecord or SkDeferredCanvas
    // Issue: http://code.google.com/p/skia/issues/detail?id=498
    // Also, creating a LayerIter on an SkProxyCanvas crashes
    // Issue: http://code.google.com/p/skia/issues/detail?id=499
    /*
    SkCanvas::LayerIter layerIter1(const_cast<SkCanvas*>(canvas1), false);
    SkCanvas::LayerIter layerIter2(const_cast<SkCanvas*>(canvas2), false);
    while (!layerIter1.done() && !layerIter2.done()) {
        REPORTER_ASSERT_MESSAGE(reporter, layerIter1.matrix() ==
            layerIter2.matrix(), testStep->assertMessage());
        REPORTER_ASSERT_MESSAGE(reporter, layerIter1.clip() ==
            layerIter2.clip(), testStep->assertMessage());
        REPORTER_ASSERT_MESSAGE(reporter, layerIter1.paint() ==
            layerIter2.paint(), testStep->assertMessage());
        REPORTER_ASSERT_MESSAGE(reporter, layerIter1.x() ==
            layerIter2.x(), testStep->assertMessage());
        REPORTER_ASSERT_MESSAGE(reporter, layerIter1.y() ==
            layerIter2.y(), testStep->assertMessage());
        layerIter1.next();
        layerIter2.next();
    }
    REPORTER_ASSERT_MESSAGE(reporter, layerIter1.done(),
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, layerIter2.done(),
        testStep->assertMessage());
    */
}

// The following class groups static functions that need to access
// the privates members of SkPictureRecord
class SkPictureTester {
private:
    static int EQ(const SkFlatData* a, const SkFlatData* b) {
        return *a == *b;
    }

    static void AssertFlattenedObjectsEqual(
        SkPictureRecord* referenceRecord,
        SkPictureRecord* testRecord,
        skiatest::Reporter* reporter,
        CanvasTestStep* testStep) {

        REPORTER_ASSERT_MESSAGE(reporter,
            referenceRecord->fBitmapHeap->count() ==
            testRecord->fBitmapHeap->count(), testStep->assertMessage());
        REPORTER_ASSERT_MESSAGE(reporter,
            referenceRecord->fMatrices.count() ==
            testRecord->fMatrices.count(), testStep->assertMessage());
        for (int i = 0; i < referenceRecord->fMatrices.count(); ++i) {
            REPORTER_ASSERT_MESSAGE(reporter,
                EQ(referenceRecord->fMatrices[i], testRecord->fMatrices[i]),
                testStep->assertMessage());
        }
        REPORTER_ASSERT_MESSAGE(reporter,
            referenceRecord->fPaints.count() ==
            testRecord->fPaints.count(), testStep->assertMessage());
        for (int i = 0; i < referenceRecord->fPaints.count(); ++i) {
            REPORTER_ASSERT_MESSAGE(reporter,
                EQ(referenceRecord->fPaints[i], testRecord->fPaints[i]),
                                    testStep->assertMessage());
        }
        REPORTER_ASSERT_MESSAGE(reporter,
            referenceRecord->fRegions.count() ==
            testRecord->fRegions.count(), testStep->assertMessage());
        for (int i = 0; i < referenceRecord->fRegions.count(); ++i) {
            REPORTER_ASSERT_MESSAGE(reporter,
                EQ(referenceRecord->fRegions[i], testRecord->fRegions[i]),
                                    testStep->assertMessage());
        }
        REPORTER_ASSERT_MESSAGE(reporter,
            !referenceRecord->fPathHeap ==
            !testRecord->fPathHeap,
            testStep->assertMessage());
        // The following tests are commented out because they currently
        // fail. Issue: http://code.google.com/p/skia/issues/detail?id=507
        /*
        if (referenceRecord->fPathHeap) {
            REPORTER_ASSERT_MESSAGE(reporter,
                referenceRecord->fPathHeap->count() ==
                testRecord->fPathHeap->count(),
                testStep->assertMessage());
            for (int i = 0; i < referenceRecord->fPathHeap->count(); ++i) {
                REPORTER_ASSERT_MESSAGE(reporter,
                    (*referenceRecord->fPathHeap)[i] ==
                    (*testRecord->fPathHeap)[i], testStep->assertMessage());
            }
        }
        */

    }

public:

    static void TestPictureFlattenedObjectReuse(skiatest::Reporter* reporter,
                                                CanvasTestStep* testStep,
                                                uint32_t recordFlags) {
        // Verify that when a test step is executed twice, no extra resources
        // are flattened during the second execution
        testStep->setAssertMessageFormat(kPictureDrawAssertMessageFormat);
        SkPicture referencePicture;
        SkCanvas* referenceCanvas = referencePicture.beginRecording(kWidth,
            kHeight, recordFlags);
        testStep->draw(referenceCanvas, reporter);
        SkPicture testPicture;
        SkCanvas* testCanvas = testPicture.beginRecording(kWidth,
            kHeight, recordFlags);
        testStep->draw(testCanvas, reporter);
        testStep->setAssertMessageFormat(kPictureSecondDrawAssertMessageFormat);
        testStep->draw(testCanvas, reporter);

        SkPictureRecord* referenceRecord = static_cast<SkPictureRecord*>(
            referenceCanvas);
        SkPictureRecord* testRecord = static_cast<SkPictureRecord*>(
            testCanvas);
        testStep->setAssertMessageFormat(kPictureResourceReuseMessageFormat);
        AssertFlattenedObjectsEqual(referenceRecord, testRecord,
            reporter, testStep);
    }
};

static void TestPdfDevice(skiatest::Reporter* reporter,
                          CanvasTestStep* testStep) {
    SkISize pageSize = SkISize::Make(kWidth, kHeight);
    SkPDFDevice device(pageSize, pageSize, SkMatrix::I());
    SkCanvas canvas(&device);
    testStep->setAssertMessageFormat(kPdfAssertMessageFormat);
    testStep->draw(&canvas, reporter);
    SkPDFDocument doc;
    doc.appendPage(&device);
    SkDynamicMemoryWStream stream;
    doc.emitPDF(&stream);
}

// The following class groups static functions that need to access
// the privates members of SkDeferredCanvas
class SkDeferredCanvasTester {
public:
    static void TestDeferredCanvasStateConsistency(
        skiatest::Reporter* reporter,
        CanvasTestStep* testStep,
        const SkCanvas& referenceCanvas, bool silent) {

        SkBitmap deferredStore;
        createBitmap(&deferredStore, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
        SkBitmapDevice deferredDevice(deferredStore);
        SkAutoTUnref<SkDeferredCanvas> deferredCanvas(SkDeferredCanvas::Create(&deferredDevice));
        testStep->setAssertMessageFormat(kDeferredDrawAssertMessageFormat);
        testStep->draw(deferredCanvas, reporter);
        testStep->setAssertMessageFormat(kDeferredPreFlushAssertMessageFormat);
        AssertCanvasStatesEqual(reporter, deferredCanvas, &referenceCanvas,
            testStep);

        if (silent) {
            deferredCanvas->silentFlush();
        } else {
            deferredCanvas->flush();
        }

        testStep->setAssertMessageFormat(
            silent ? kDeferredPostSilentFlushPlaybackAssertMessageFormat :
            kDeferredPostFlushPlaybackAssertMessageFormat);
        AssertCanvasStatesEqual(reporter,
            deferredCanvas->immediateCanvas(),
            &referenceCanvas, testStep);

        // Verified that deferred canvas state is not affected by flushing
        // pending draw operations

        // The following test code is commented out because it currently fails.
        // Issue: http://code.google.com/p/skia/issues/detail?id=496
        /*
        testStep->setAssertMessageFormat(kDeferredPostFlushAssertMessageFormat);
        AssertCanvasStatesEqual(reporter, &deferredCanvas, &referenceCanvas,
            testStep);
        */
    }
};

// unused
static void TestProxyCanvasStateConsistency(
    skiatest::Reporter* reporter,
    CanvasTestStep* testStep,
    const SkCanvas& referenceCanvas) {

    SkBitmap indirectStore;
    createBitmap(&indirectStore, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkBitmapDevice indirectDevice(indirectStore);
    SkCanvas indirectCanvas(&indirectDevice);
    SkProxyCanvas proxyCanvas(&indirectCanvas);
    testStep->setAssertMessageFormat(kProxyDrawAssertMessageFormat);
    testStep->draw(&proxyCanvas, reporter);
    // Verify that the SkProxyCanvas reports consitent state
    testStep->setAssertMessageFormat(kProxyStateAssertMessageFormat);
    AssertCanvasStatesEqual(reporter, &proxyCanvas, &referenceCanvas,
        testStep);
    // Verify that the indirect canvas reports consitent state
    testStep->setAssertMessageFormat(kProxyIndirectStateAssertMessageFormat);
    AssertCanvasStatesEqual(reporter, &indirectCanvas, &referenceCanvas,
        testStep);
}

// unused
static void TestNWayCanvasStateConsistency(
    skiatest::Reporter* reporter,
    CanvasTestStep* testStep,
    const SkCanvas& referenceCanvas) {

    SkBitmap indirectStore1;
    createBitmap(&indirectStore1, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkBitmapDevice indirectDevice1(indirectStore1);
    SkCanvas indirectCanvas1(&indirectDevice1);

    SkBitmap indirectStore2;
    createBitmap(&indirectStore2, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkBitmapDevice indirectDevice2(indirectStore2);
    SkCanvas indirectCanvas2(&indirectDevice2);

    SkISize canvasSize = referenceCanvas.getDeviceSize();
    SkNWayCanvas nWayCanvas(canvasSize.width(), canvasSize.height());
    nWayCanvas.addCanvas(&indirectCanvas1);
    nWayCanvas.addCanvas(&indirectCanvas2);

    testStep->setAssertMessageFormat(kNWayDrawAssertMessageFormat);
    testStep->draw(&nWayCanvas, reporter);
    // Verify that the SkProxyCanvas reports consitent state
    testStep->setAssertMessageFormat(kNWayStateAssertMessageFormat);
    AssertCanvasStatesEqual(reporter, &nWayCanvas, &referenceCanvas,
        testStep);
    // Verify that the indirect canvases report consitent state
    testStep->setAssertMessageFormat(kNWayIndirect1StateAssertMessageFormat);
    AssertCanvasStatesEqual(reporter, &indirectCanvas1, &referenceCanvas,
        testStep);
    testStep->setAssertMessageFormat(kNWayIndirect2StateAssertMessageFormat);
    AssertCanvasStatesEqual(reporter, &indirectCanvas2, &referenceCanvas,
        testStep);
}

/*
 * This sub-test verifies that the test step passes when executed
 * with SkCanvas and with classes derrived from SkCanvas. It also verifies
 * that the all canvas derivatives report the same state as an SkCanvas
 * after having executed the test step.
 */
static void TestOverrideStateConsistency(skiatest::Reporter* reporter,
                                         CanvasTestStep* testStep) {
    SkBitmap referenceStore;
    createBitmap(&referenceStore, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkBitmapDevice referenceDevice(referenceStore);
    SkCanvas referenceCanvas(&referenceDevice);
    testStep->setAssertMessageFormat(kCanvasDrawAssertMessageFormat);
    testStep->draw(&referenceCanvas, reporter);

    SkDeferredCanvasTester::TestDeferredCanvasStateConsistency(reporter, testStep, referenceCanvas, false);

    SkDeferredCanvasTester::TestDeferredCanvasStateConsistency(reporter, testStep, referenceCanvas, true);

    // The following test code is disabled because SkProxyCanvas is
    // missing a lot of virtual overrides on get* methods, which are used
    // to verify canvas state.
    // Issue: http://code.google.com/p/skia/issues/detail?id=500

    if (false) { // avoid bit rot, suppress warning
        TestProxyCanvasStateConsistency(reporter, testStep, referenceCanvas);
    }

    // The following test code is disabled because SkNWayCanvas does not
    // report correct clipping and device bounds information
    // Issue: http://code.google.com/p/skia/issues/detail?id=501

    if (false) { // avoid bit rot, suppress warning
        TestNWayCanvasStateConsistency(reporter, testStep, referenceCanvas);
    }

    if (false) { // avoid bit rot, suppress warning
        test_clipVisitor(reporter, &referenceCanvas);
    }
}

DEF_TEST(Canvas, reporter) {
    // Init global here because bitmap pixels cannot be alocated during
    // static initialization
    kTestBitmap = testBitmap();

    for (int testStep = 0; testStep < testStepArray().count(); testStep++) {
        TestOverrideStateConsistency(reporter, testStepArray()[testStep]);
        SkPictureTester::TestPictureFlattenedObjectReuse(reporter,
            testStepArray()[testStep], 0);
        if (testStepArray()[testStep]->enablePdfTesting()) {
            TestPdfDevice(reporter, testStepArray()[testStep]);
        }
    }

    // Explicitly call reset(), so we don't leak the pixels (since kTestBitmap is a global)
    kTestBitmap.reset();
}
