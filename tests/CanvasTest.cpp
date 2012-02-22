
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

static const int kWidth = 2;
static const int kHeight = 2;
// Maximum stream length for picture serialization
static const size_t kMaxPictureBufferSize = 1024; 

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
static const char* const kPictureReDrawAssertMessageFormat = 
    "Playing back test step %s from an SkPicture to another SkPicture";
static const char* const kDeferredDrawAssertMessageFormat = 
    "Drawing test step %s with SkDeferredCanvas";
static const char* const kProxyDrawAssertMessageFormat = 
    "Drawing test step %s with SkProxyCanvas";
static const char* const kNWayDrawAssertMessageFormat = 
    "Drawing test step %s with SkNWayCanvas";
static const char* const kRoundTripAssertMessageFormat = 
    "test step %s, SkPicture consistency after round trip";
static const char* const kPictureRecoringAssertMessageFormat = 
    "test step %s, SkPicture state consistency after recording";
static const char* const kPicturePlaybackAssertMessageFormat = 
    "test step %s, SkPicture state consistency in playback canvas";
static const char* const kDeferredPreFlushAssertMessageFormat = 
    "test step %s, SkDeferredCanvas state consistency before flush";
static const char* const kDeferredPostFlushAssertMessageFormat = 
    "test step %s, SkDeferredCanvas state consistency after flush";
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
    CanvasTestStep() {
        *testStepArray().append() = this;
        fAssertMessageFormat = kDefaultAssertMessageFormat;
    }

    virtual void draw(SkCanvas*, skiatest::Reporter*) = 0;
    virtual const char* name() const = 0;

    const char* assertMessage() {
        fAssertMessage.printf(fAssertMessageFormat, name());
        return fAssertMessage.c_str();
    }

    void setAssertMessageFormat(const char* format) {
        fAssertMessageFormat = format;
    }

private:
    SkString fAssertMessage;
    const char* fAssertMessageFormat;
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
SkPoint kTestPoint = SkPoint::Make(SkIntToScalar(0), SkIntToScalar(1));

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

// The following test steps are commented-out because they currently fail
// Issue: http://code.google.com/p/skia/issues/detail?id=506
//SIMPLE_TEST_STEP(SaveMatrix, save(SkCanvas::kMatrix_SaveFlag));
//SIMPLE_TEST_STEP(SaveClip, save(SkCanvas::kClip_SaveFlag));
//SIMPLE_TEST_STEP(SaveMatrixClip, save(SkCanvas::kMatrixClip_SaveFlag));
//SIMPLE_TEST_STEP(SaveLayer, saveLayer(NULL, NULL));
//SIMPLE_TEST_STEP(BoundedSaveLayer, saveLayer(&kTestRect, NULL));
//SIMPLE_TEST_STEP(PaintSaveLayer, saveLayer(NULL, &kTestPaint));
//SIMPLE_TEST_STEP_WITH_ASSERT(Translate,
//    translate(SkIntToScalar(1), SkIntToScalar(2)));
//SIMPLE_TEST_STEP_WITH_ASSERT(Scale,
//    scale(SkIntToScalar(1), SkIntToScalar(2)));
//SIMPLE_TEST_STEP_WITH_ASSERT(Rotate, rotate(SkIntToScalar(1)));
//SIMPLE_TEST_STEP_WITH_ASSERT(Skew,
//    skew(SkIntToScalar(1), SkIntToScalar(2)));
//SIMPLE_TEST_STEP_WITH_ASSERT(Concat, concat(kTestMatrix));
//SIMPLE_TEST_STEP(SetMatrix, setMatrix(kTestMatrix));
//SIMPLE_TEST_STEP_WITH_ASSERT(ClipRect, clipRect(kTestRect));
//SIMPLE_TEST_STEP_WITH_ASSERT(ClipPath, clipPath(kTestPath));
//SIMPLE_TEST_STEP_WITH_ASSERT(ClipRegion,
//    clipRegion(kTestRegion, SkRegion::kReplace_Op));
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
// The following test step is commented-out because it crashes SkDeferredCanvas
// Issue: http://code.google.com/p/skia/issues/detail?id=505
//SIMPLE_TEST_STEP(DrawBitmap, drawBitmap(kTestBitmap, 0, 0));
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
// The following test step is commented-out because it crashes SkDeferredCanvas
// Issue: http://code.google.com/p/skia/issues/detail?id=505
//SIMPLE_TEST_STEP(DrawSprite, drawSprite(kTestBitmap, 0, 0, NULL));
SIMPLE_TEST_STEP(DrawSpritePaint, drawSprite(kTestBitmap, 0, 0, &kTestPaint));
SIMPLE_TEST_STEP(DrawText, drawText(kTestText.c_str(), kTestText.size(),
    0, 1, kTestPaint));
SIMPLE_TEST_STEP(DrawPosText, drawPosText(kTestText.c_str(),
    kTestText.size(), &kTestPoint, kTestPaint));
SIMPLE_TEST_STEP(DrawTextOnPath, drawTextOnPath(kTestText.c_str(),
    kTestText.size(), kTestPath, NULL, kTestPaint));
SIMPLE_TEST_STEP(DrawTextOnPathMatrix, drawTextOnPath(kTestText.c_str(),
    kTestText.size(), kTestPath, &kTestMatrix, kTestPaint));
SIMPLE_TEST_STEP(SetExternalMatrix, setExternalMatrix(&kTestMatrix));
SIMPLE_TEST_STEP(DrawData, drawData(kTestText.c_str(), kTestText.size()));

///////////////////////////////////////////////////////////////////////////////
// Complex test steps

static void DrawVerticesShaderTestStep(SkCanvas* canvas, 
                                       skiatest::Reporter* reporter,
                                       CanvasTestStep* testStep) {
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
TEST_STEP(DrawVerticesShader, DrawVerticesShaderTestStep);

static void DrawPictureTestStep(SkCanvas* canvas, 
                                skiatest::Reporter* reporter,
                                CanvasTestStep* testStep) {
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
    REPORTER_ASSERT_MESSAGE(reporter, 1 == canvas->getSaveCount(), 
        testStep->assertMessage());
    size_t n = canvas->save();
    REPORTER_ASSERT_MESSAGE(reporter, 1 == n, testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, 2 == canvas->getSaveCount(),
        testStep->assertMessage());
    canvas->save();
    canvas->save();
    REPORTER_ASSERT_MESSAGE(reporter, 4 == canvas->getSaveCount(),
        testStep->assertMessage());
    canvas->restoreToCount(2);
    REPORTER_ASSERT_MESSAGE(reporter, 2 == canvas->getSaveCount(),
        testStep->assertMessage());

    // should this pin to 1, or be a no-op, or crash?
    canvas->restoreToCount(0);
    REPORTER_ASSERT_MESSAGE(reporter, 1 == canvas->getSaveCount(),
        testStep->assertMessage());
}
TEST_STEP(SaveRestore, SaveRestoreTestStep);

// The following test step is commented-out because it currently fails
// Issue: http://code.google.com/p/skia/issues/detail?id=506
/*
static void DrawLayerTestStep(SkCanvas* canvas, 
                              skiatest::Reporter* reporter,
                              CanvasTestStep* testStep) {
    REPORTER_ASSERT_MESSAGE(reporter, !canvas->isDrawingToLayer(),
        testStep->assertMessage());
    canvas->save();
    REPORTER_ASSERT_MESSAGE(reporter, !canvas->isDrawingToLayer(),
        testStep->assertMessage());
    
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
*/

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
        canvas1->getClipBounds(&bounds1, SkCanvas::kAA_EdgeType) ==
        canvas2->getClipBounds(&bounds2, SkCanvas::kAA_EdgeType),
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter, bounds1 == bounds2,
        testStep->assertMessage());
    REPORTER_ASSERT_MESSAGE(reporter,
        canvas1->getClipBounds(&bounds1, SkCanvas::kBW_EdgeType) ==
        canvas2->getClipBounds(&bounds2, SkCanvas::kBW_EdgeType),
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
    REPORTER_ASSERT_MESSAGE(reporter, canvas1->getTotalClipStack() ==
        canvas2->getTotalClipStack(), testStep->assertMessage());

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
    static void AssertFlattenedObjectsEqual(
        SkPictureRecord* referenceRecord,
        SkPictureRecord* testRecord,
        skiatest::Reporter* reporter,
        CanvasTestStep* testStep) {

        REPORTER_ASSERT_MESSAGE(reporter,
            referenceRecord->fBitmaps.count() ==
            testRecord->fBitmaps.count(), testStep->assertMessage());
        for (int i = 0; i < referenceRecord->fBitmaps.count(); ++i) {
            REPORTER_ASSERT_MESSAGE(reporter,
                SkFlatData::Compare(referenceRecord->fBitmaps[i],
                testRecord->fBitmaps[i]) == 0, testStep->assertMessage());
        }
        REPORTER_ASSERT_MESSAGE(reporter,
            referenceRecord->fMatrices.count() ==
            testRecord->fMatrices.count(), testStep->assertMessage());
        for (int i = 0; i < referenceRecord->fMatrices.count(); ++i) {
            REPORTER_ASSERT_MESSAGE(reporter,
                SkFlatData::Compare(referenceRecord->fMatrices[i],
                testRecord->fMatrices[i]) == 0,
                testStep->assertMessage());
        }
        REPORTER_ASSERT_MESSAGE(reporter,
            referenceRecord->fPaints.count() ==
            testRecord->fPaints.count(), testStep->assertMessage());
        for (int i = 0; i < referenceRecord->fPaints.count(); ++i) {
            REPORTER_ASSERT_MESSAGE(reporter,
                SkFlatData::Compare(referenceRecord->fPaints[i],
                testRecord->fPaints[i]) == 0, testStep->assertMessage());
        }
        REPORTER_ASSERT_MESSAGE(reporter,
            referenceRecord->fRegions.count() ==
            testRecord->fRegions.count(), testStep->assertMessage());
        for (int i = 0; i < referenceRecord->fRegions.count(); ++i) {
            REPORTER_ASSERT_MESSAGE(reporter,
                SkFlatData::Compare(referenceRecord->fRegions[i],
                testRecord->fRegions[i]) == 0, testStep->assertMessage());
        }
        REPORTER_ASSERT_MESSAGE(reporter,
            !referenceRecord->fPathHeap ==
            !testRecord->fPathHeap,
            testStep->assertMessage());
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
    
    }

public:

    static void TestPictureSerializationRoundTrip(skiatest::Reporter* reporter, 
                                                  CanvasTestStep* testStep) {
        testStep->setAssertMessageFormat(kPictureDrawAssertMessageFormat);
        SkPicture referencePicture;
        testStep->draw(referencePicture.beginRecording(kWidth, kHeight),
            reporter);
        SkPicture initialPicture;
        testStep->draw(initialPicture.beginRecording(kWidth, kHeight),
            reporter);
        testStep->setAssertMessageFormat(kPictureReDrawAssertMessageFormat);
        SkPicture roundTripPicture;
        initialPicture.draw(roundTripPicture.beginRecording(kWidth, kHeight));

        SkPictureRecord* referenceRecord = static_cast<SkPictureRecord*>(
            referencePicture.getRecordingCanvas());
        SkPictureRecord* roundTripRecord = static_cast<SkPictureRecord*>(
            roundTripPicture.getRecordingCanvas());

        testStep->setAssertMessageFormat(kPictureReDrawAssertMessageFormat);

        // Verify that deserialization-serialization round trip conserves all
        // data by comparing referenceRecord to roundTripRecord
        REPORTER_ASSERT_MESSAGE(reporter, referenceRecord->fBitmapIndex ==
            roundTripRecord->fBitmapIndex, testStep->assertMessage());
        REPORTER_ASSERT_MESSAGE(reporter, referenceRecord->fMatrixIndex ==
            roundTripRecord->fMatrixIndex, testStep->assertMessage());
        REPORTER_ASSERT_MESSAGE(reporter, referenceRecord->fPaintIndex ==
            roundTripRecord->fPaintIndex, testStep->assertMessage());
        REPORTER_ASSERT_MESSAGE(reporter, referenceRecord->fRegionIndex ==
            roundTripRecord->fRegionIndex, testStep->assertMessage());
        char referenceBuffer[kMaxPictureBufferSize];
        SkMemoryWStream referenceStream(referenceBuffer,
            kMaxPictureBufferSize);
        referenceRecord->fWriter.writeToStream(&referenceStream);
        char roundTripBuffer[kMaxPictureBufferSize];
        SkMemoryWStream roundTripStream(roundTripBuffer,
            kMaxPictureBufferSize);
        roundTripRecord->fWriter.writeToStream(&roundTripStream);
        REPORTER_ASSERT_MESSAGE(reporter,
            roundTripStream.bytesWritten() == referenceStream.bytesWritten(),
            testStep->assertMessage());
        REPORTER_ASSERT_MESSAGE(reporter, 0 == memcmp(referenceBuffer,
            roundTripBuffer, roundTripStream.bytesWritten()),
            testStep->assertMessage());
        REPORTER_ASSERT_MESSAGE(reporter, referenceRecord->fRecordFlags ==
            roundTripRecord->fRecordFlags, testStep->assertMessage());
        REPORTER_ASSERT_MESSAGE(reporter,
            referenceRecord->fRestoreOffsetStack ==
            roundTripRecord->fRestoreOffsetStack,
            testStep->assertMessage());
        AssertFlattenedObjectsEqual(referenceRecord, roundTripRecord,
            reporter, testStep);
        AssertCanvasStatesEqual(reporter, referenceRecord, roundTripRecord,
            testStep);
    }

    static void TestPictureFlattenedObjectReuse(skiatest::Reporter* reporter, 
                                         CanvasTestStep* testStep) {
        // Verify that when a test step is executed twice, no extra resources
        // are flattened during the second execution
        testStep->setAssertMessageFormat(kPictureDrawAssertMessageFormat);
        SkPicture referencePicture;
        SkCanvas* referenceCanvas = referencePicture.beginRecording(kWidth,
            kHeight);
        testStep->draw(referenceCanvas, reporter);
        SkPicture testPicture;
        SkCanvas* testCanvas = referencePicture.beginRecording(kWidth,
            kHeight);
        testStep->draw(referencePicture.beginRecording(kWidth, kHeight),
            reporter);
        testStep->setAssertMessageFormat(kPictureSecondDrawAssertMessageFormat);
        testStep->draw(referencePicture.beginRecording(kWidth, kHeight),
            reporter);

        SkPictureRecord* referenceRecord = static_cast<SkPictureRecord*>(
            referenceCanvas);
        SkPictureRecord* testRecord = static_cast<SkPictureRecord*>(
            testCanvas);
        // The following test currently fails on linux
        // Issue: http://code.google.com/p/skia/issues/detail?id=507
#if !defined(SK_BUILD_FOR_UNIX)
        testStep->setAssertMessageFormat(kPictureResourceReuseMessageFormat);
        AssertFlattenedObjectsEqual(referenceRecord, testRecord,
            reporter, testStep);
#endif
    }
};

static void TestPictureStateConsistency(skiatest::Reporter* reporter, 
                                        CanvasTestStep* testStep,
                                        const SkCanvas& referenceCanvas) {
    // Verify that the recording canvas's state is consistent
    // with that of a regular canvas
    SkPicture testPicture;
    SkCanvas* pictureCanvas = testPicture.beginRecording(kWidth, kHeight);
    testStep->setAssertMessageFormat(kPictureDrawAssertMessageFormat);
    testStep->draw(pictureCanvas, reporter);
    testStep->setAssertMessageFormat(kPictureRecoringAssertMessageFormat);
    AssertCanvasStatesEqual(reporter, pictureCanvas, &referenceCanvas,
        testStep);

    SkBitmap playbackStore;
    createBitmap(&playbackStore, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkDevice playbackDevice(playbackStore);
    SkCanvas playbackCanvas(&playbackDevice);
    testPicture.draw(&playbackCanvas);
    testStep->setAssertMessageFormat(kPicturePlaybackAssertMessageFormat);
    AssertCanvasStatesEqual(reporter, &playbackCanvas, &referenceCanvas,
        testStep);

    // The following test code is commented out because SkPicture is not
    // currently expected to preserve state when restarting recording.
    /*
    SkCanvas* pictureCanvas = testPicture.beginRecording(kWidth, kHeight);
    testStep->setAssertMessageFormat(kPictureResumeAssertMessageFormat);
    AssertCanvasStatesEqual(reporter, pictureCanvas, &referenceCanvas,
        testStep);
    */
}

static void TestDeferredCanvasStateConsistency(
    skiatest::Reporter* reporter,
    CanvasTestStep* testStep,
    const SkCanvas& referenceCanvas) {

    SkBitmap deferredStore;
    createBitmap(&deferredStore, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkDevice deferredDevice(deferredStore);
    SkDeferredCanvas deferredCanvas(&deferredDevice);
    testStep->setAssertMessageFormat(kDeferredDrawAssertMessageFormat);
    testStep->draw(&deferredCanvas, reporter);
    testStep->setAssertMessageFormat(kDeferredPreFlushAssertMessageFormat);
    AssertCanvasStatesEqual(reporter, &deferredCanvas, &referenceCanvas,
        testStep);

    // Verified that deferred canvas state is not affected by flushing
    // pending draw operations

    // The following test code is commented out because it currently fails.
    // Issue: http://code.google.com/p/skia/issues/detail?id=496
    /*
    deferredCanvas.flush();
    testStep->setAssertMessageFormat(kDeferredPostFlushAssertMessageFormat);
    AssertCanvasStatesEqual(reporter, &deferredCanvas, &referenceCanvas,
        testStep);
    */
}

static void TestProxyCanvasStateConsistency(
    skiatest::Reporter* reporter,
    CanvasTestStep* testStep,
    const SkCanvas& referenceCanvas) {

    SkBitmap indirectStore;
    createBitmap(&indirectStore, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkDevice indirectDevice(indirectStore);
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

static void TestNWayCanvasStateConsistency(
    skiatest::Reporter* reporter,
    CanvasTestStep* testStep,
    const SkCanvas& referenceCanvas) {

    SkBitmap indirectStore1;
    createBitmap(&indirectStore1, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkDevice indirectDevice1(indirectStore1);
    SkCanvas indirectCanvas1(&indirectDevice1);

    SkBitmap indirectStore2;
    createBitmap(&indirectStore2, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkDevice indirectDevice2(indirectStore2);
    SkCanvas indirectCanvas2(&indirectDevice2);

    SkNWayCanvas nWayCanvas;
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
    SkDevice referenceDevice(referenceStore);
    SkCanvas referenceCanvas(&referenceDevice);
    testStep->setAssertMessageFormat(kCanvasDrawAssertMessageFormat);
    testStep->draw(&referenceCanvas, reporter);

    TestPictureStateConsistency(reporter, testStep, referenceCanvas);
    TestDeferredCanvasStateConsistency(reporter, testStep, referenceCanvas);

    // The following test code is commented out because SkProxyCanvas is
    // missing a lot of virtual overrides on get* methods, which are used
    // to verify canvas state.
    // Issue: http://code.google.com/p/skia/issues/detail?id=500
    
    //TestProxyCanvasStateConsistency(reporter, testStep, referenceCanvas);

    // The following test code is commented out because SkNWayCanvas does not
    // report correct clipping and device bounds information
    // Issue: http://code.google.com/p/skia/issues/detail?id=501
    
    //TestNWayCanvasStateConsistency(reporter, testStep, referenceCanvas);
}

static void TestCanvas(skiatest::Reporter* reporter) {
    // Init global here because bitmap pixels cannot be alocated during
    // static initialization
    kTestBitmap = testBitmap();

    for (int testStep = 0; testStep < testStepArray().count(); testStep++) {
        TestOverrideStateConsistency(reporter, testStepArray()[testStep]);
        SkPictureTester::TestPictureSerializationRoundTrip(reporter, 
            testStepArray()[testStep]);
        SkPictureTester::TestPictureFlattenedObjectReuse(reporter,
            testStepArray()[testStep]);
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Canvas", TestCanvasClass, TestCanvas)
