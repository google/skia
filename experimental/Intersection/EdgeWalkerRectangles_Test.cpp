#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "SkBitmap.h"

static SkBitmap bitmap;

static void testSimplifyCoincidentInner() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(10, 10, 60, 60, SkPath::kCCW_Direction);
    path.addRect(20, 20, 50, 50, SkPath::kCW_Direction);
    path.addRect(20, 30, 40, 40, SkPath::kCW_Direction);
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyCoincidentVertical() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(10, 10, 30, 30);
    path.addRect(10, 30, 30, 40);
    simplify(path, true, out);
    SkRect rect;
    if (!out.isRect(&rect)) {
        SkDebugf("%s expected rect\n", __FUNCTION__);
    }
    if (rect != SkRect::MakeLTRB(10, 10, 30, 40)) {
        SkDebugf("%s expected union\n", __FUNCTION__);
    }
}

static void testSimplifyCoincidentHorizontal() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(10, 10, 30, 30);
    path.addRect(30, 10, 40, 30);
    simplify(path, true, out);
    SkRect rect;
    if (!out.isRect(&rect)) {
        SkDebugf("%s expected rect\n", __FUNCTION__);
    }
    if (rect != SkRect::MakeLTRB(10, 10, 40, 30)) {
        SkDebugf("%s expected union\n", __FUNCTION__);
    }
}

static void testSimplifyMulti() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(10, 10, 30, 30);
    path.addRect(20, 20, 40, 40);
    simplify(path, true, out);
    SkPath expected;
    expected.setFillType(SkPath::kEvenOdd_FillType);
    expected.moveTo(10,10); // two cutout corners
    expected.lineTo(10,30);
    expected.lineTo(20,30);
    expected.lineTo(20,40);
    expected.lineTo(40,40);
    expected.lineTo(40,20);
    expected.lineTo(30,20);
    expected.lineTo(30,10);
    expected.lineTo(10,10);
    expected.close();
    if (out != expected) {
        SkDebugf("%s expected equal\n", __FUNCTION__);
    }
    
    path = out;
    path.addRect(30, 10, 40, 20);
    path.addRect(10, 30, 20, 40);
    simplify(path, true, out);
    SkRect rect;
    if (!out.isRect(&rect)) {
        SkDebugf("%s expected rect\n", __FUNCTION__);
    }
    if (rect != SkRect::MakeLTRB(10, 10, 40, 40)) {
        SkDebugf("%s expected union\n", __FUNCTION__);
    }
    
    path = out;
    path.addRect(10, 10, 40, 40, SkPath::kCCW_Direction);
    simplify(path, true, out);
    if (!out.isEmpty()) {
        SkDebugf("%s expected empty\n", __FUNCTION__);
    }
}

static void testSimplifyAddL() {
    SkPath path, out;
    path.moveTo(10,10); // 'L' shape
    path.lineTo(10,40);
    path.lineTo(40,40);
    path.lineTo(40,20);
    path.lineTo(30,20);
    path.lineTo(30,10);
    path.lineTo(10,10);
    path.close();
    path.addRect(30, 10, 40, 20); // missing notch of 'L'
    simplify(path, true, out);
    SkRect rect;
    if (!out.isRect(&rect)) {
        SkDebugf("%s expected rect\n", __FUNCTION__);
    }
    if (rect != SkRect::MakeLTRB(10, 10, 40, 40)) {
        SkDebugf("%s expected union\n", __FUNCTION__);
    }
}

static void testSimplifyCoincidentCCW() {
    SkPath path, out;
    path.addRect(10, 10, 40, 40, SkPath::kCCW_Direction);
    path.addRect(10, 10, 40, 40, SkPath::kCCW_Direction);
    simplify(path, true, out);
    SkRect rect;
    if (!out.isRect(&rect)) {
        SkDebugf("%s expected rect\n", __FUNCTION__);
    }
    if (rect != SkRect::MakeLTRB(10, 10, 40, 40)) {
        SkDebugf("%s expected union\n", __FUNCTION__);
    }
}

static void testSimplifyCoincidentCW() {
    SkPath path, out;
    path.addRect(10, 10, 40, 40, SkPath::kCCW_Direction);
    path.addRect(10, 10, 40, 40, SkPath::kCW_Direction);
    simplify(path, true, out);
    if (!out.isEmpty()) {
        SkDebugf("%s expected empty\n", __FUNCTION__);
    }
}

static void testSimplifyCorner() {
    SkPath path, out;
    path.addRect(10, 10, 20, 20, SkPath::kCCW_Direction);
    path.addRect(20, 20, 40, 40, SkPath::kCW_Direction);
    simplify(path, true, out);
    SkTDArray<SkRect> boundsArray;
    contourBounds(out, boundsArray);
    if (boundsArray.count() != 2) {
        SkDebugf("%s expected 2 contours\n", __FUNCTION__);
        return;
    }
    SkRect one = SkRect::MakeLTRB(10, 10, 20, 20);
    SkRect two = SkRect::MakeLTRB(20, 20, 40, 40);
    if (boundsArray[0] != one && boundsArray[0] != two
            || boundsArray[1] != one && boundsArray[1] != two) {
        SkDebugf("%s expected match\n", __FUNCTION__);
    }
}

static void testSimplifyDiagonal() {
    SkRect rect2 = SkRect::MakeXYWH(10, 10, 10, 10);
    for (size_t outDir = SkPath::kCW_Direction; outDir <= SkPath::kCCW_Direction; ++outDir) {
        for (size_t inDir = SkPath::kCW_Direction; inDir <= SkPath::kCCW_Direction; ++inDir) {
            for (int x = 0; x <= 20; x += 20) {
                for (int y = 0; y <= 20; y += 20) {
                    SkPath path, out;
                    SkRect rect1 = SkRect::MakeXYWH(x, y, 10, 10);
                    path.addRect(rect1, static_cast<SkPath::Direction>(outDir));
                    path.addRect(rect2, static_cast<SkPath::Direction>(inDir));
                    simplify(path, true, out);
                    SkPath::Iter iter(out, false);
                    SkPoint pts[4], lastLine[2];
                    SkPath::Verb verb;
                    SkRect bounds[2];
                    bounds[0].setEmpty();
                    bounds[1].setEmpty();
                    SkRect* boundsPtr = bounds;
                    int count = 0, segments = 0;
                    bool lastLineSet = false;
                    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
                        switch (verb) {
                            case SkPath::kMove_Verb:
                                if (!boundsPtr->isEmpty()) {
                                    SkASSERT(boundsPtr == bounds);
                                    ++boundsPtr;
                                }
                                boundsPtr->set(pts[0].fX, pts[0].fY, pts[0].fX, pts[0].fY);
                                count = 0;
                                lastLineSet = false;
                                break;
                            case SkPath::kLine_Verb:
                                if (lastLineSet) {
                                    SkASSERT((lastLine[1].fX - lastLine[0].fX) *
                                        (pts[1].fY - lastLine[0].fY) !=
                                        (lastLine[1].fY - lastLine[0].fY) *
                                        (pts[1].fX - lastLine[0].fX));
                                }
                                lastLineSet = true;
                                lastLine[0] = pts[0];
                                lastLine[1] = pts[1];
                                count = 1;
                                ++segments;
                                break;
                            case SkPath::kClose_Verb:
                                count = 0;
                                break;
                            default:
                                SkDEBUGFAIL("bad verb");
                                return;
                        }
                        for (int i = 1; i <= count; ++i) {
                            boundsPtr->growToInclude(pts[i].fX, pts[i].fY);
                        }
                    }
                    if (boundsPtr != bounds) {
                        SkASSERT((bounds[0] == rect1 || bounds[1] == rect1) 
                                && (bounds[0] == rect2 || bounds[1] == rect2));
                    } else {
                        SkASSERT(segments == 8);
                    }
                }
            }
        }
    }
}

static void assertOneContour(const SkPath& out, bool edge, bool extend) {
    SkPath::Iter iter(out, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    SkRect bounds;
    bounds.setEmpty();
    int count = 0;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                SkASSERT(count == 0);
                break;
            case SkPath::kLine_Verb: 
                SkASSERT(pts[0].fX == pts[1].fX || pts[0].fY == pts[1].fY);
                ++count;
                break;
            case SkPath::kClose_Verb:
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
    }
    SkASSERT(count == (extend ? 4 : edge ? 6 : 8));
}

static void testSimplifyCoincident() {
    // outside to inside, outside to right, outside to outside
    // left to inside, left to right, left to outside
    // inside to right, inside to outside
    // repeat above for left, right, bottom
    SkScalar start[] = { 0, 10, 20 };
    size_t startCount = sizeof(start) / sizeof(start[0]);
    SkScalar stop[] = { 30, 40, 50 };
    size_t stopCount = sizeof(stop) / sizeof(stop[0]);
    SkRect rect2 = SkRect::MakeXYWH(10, 10, 30, 30);
    for (size_t outDir = SkPath::kCW_Direction; outDir <= SkPath::kCCW_Direction; ++outDir) {
        for (size_t inDir = SkPath::kCW_Direction; inDir <= SkPath::kCCW_Direction; ++inDir) {
            for (size_t startIndex = 0; startIndex < startCount; ++startIndex) {
                for (size_t stopIndex = 0; stopIndex < stopCount; ++stopIndex) {
                    bool extend = start[startIndex] == rect2.fLeft && stop[stopIndex] == rect2.fRight;
                    bool edge = start[startIndex] == rect2.fLeft || stop[stopIndex] == rect2.fRight;
                    SkRect rect1 = SkRect::MakeLTRB(start[startIndex], 0, stop[stopIndex], 10);
                    SkPath path, out;
                    path.addRect(rect1, static_cast<SkPath::Direction>(outDir));
                    path.addRect(rect2, static_cast<SkPath::Direction>(inDir));
                    simplify(path, true, out);
                    assertOneContour(out, edge, extend);
                    
                    path.reset();
                    rect1 = SkRect::MakeLTRB(start[startIndex], 40, stop[stopIndex], 50);
                    path.addRect(rect1, static_cast<SkPath::Direction>(outDir));
                    path.addRect(rect2, static_cast<SkPath::Direction>(inDir));
                    simplify(path, true, out);
                    assertOneContour(out, edge, extend);
                    
                    path.reset();
                    rect1 = SkRect::MakeLTRB(0, start[startIndex], 10, stop[stopIndex]);
                    path.addRect(rect1, static_cast<SkPath::Direction>(outDir));
                    path.addRect(rect2, static_cast<SkPath::Direction>(inDir));
                    simplify(path, true, out);
                    assertOneContour(out, edge, extend);
                    
                    path.reset();
                    rect1 = SkRect::MakeLTRB(40, start[startIndex], 50, stop[stopIndex]);
                    path.addRect(rect1, static_cast<SkPath::Direction>(outDir));
                    path.addRect(rect2, static_cast<SkPath::Direction>(inDir));
                    simplify(path, true, out);
                    assertOneContour(out, edge, extend);
                }
            }
        }
    }
}

static void testSimplifyOverlap() {
    SkScalar start[] = { 0, 10, 20 };
    size_t startCount = sizeof(start) / sizeof(start[0]);
    SkScalar stop[] = { 30, 40, 50 };
    size_t stopCount = sizeof(stop) / sizeof(stop[0]);
    SkRect rect2 = SkRect::MakeXYWH(10, 10, 30, 30);
    for (size_t dir = SkPath::kCW_Direction; dir <= SkPath::kCCW_Direction; ++dir) {
        for (size_t lefty = 0; lefty < startCount; ++lefty) {
            for (size_t righty = 0; righty < stopCount; ++righty) {
                for (size_t toppy = 0; toppy < startCount; ++toppy) {
                    for (size_t botty = 0; botty < stopCount; ++botty) {
                        SkRect rect1 = SkRect::MakeLTRB(start[lefty], start[toppy],
                                stop[righty], stop[botty]);
                        SkPath path, out;
                        path.addRect(rect1, static_cast<SkPath::Direction>(dir));
                        path.addRect(rect2, static_cast<SkPath::Direction>(dir));
                        testSimplify(path, true, out, bitmap);
                    }
                }
            }
        }
    }
}

static void testSimplifyOverlapTiny() {
    SkScalar start[] = { 0, 1, 2 };
    size_t startCount = sizeof(start) / sizeof(start[0]);
    SkScalar stop[] = { 3, 4, 5 };
    size_t stopCount = sizeof(stop) / sizeof(stop[0]);
    SkRect rect2 = SkRect::MakeXYWH(1, 1, 3, 3);
    for (size_t dir = SkPath::kCW_Direction; dir <= SkPath::kCCW_Direction; ++dir) {
        for (size_t lefty = 0; lefty < startCount; ++lefty) {
            for (size_t righty = 0; righty < stopCount; ++righty) {
                for (size_t toppy = 0; toppy < startCount; ++toppy) {
                    for (size_t botty = 0; botty < stopCount; ++botty) {
                        SkRect rect1 = SkRect::MakeLTRB(start[lefty], start[toppy],
                                stop[righty], stop[botty]);
                        SkPath path, out;
                        path.addRect(rect1, static_cast<SkPath::Direction>(dir));
                        path.addRect(rect2, static_cast<SkPath::Direction>(dir));
                        simplify(path, true, out);
                        comparePathsTiny(path, out);
                    }
                }
            }
        }
    }
}

static void testSimplifyDegenerate() {
    SkScalar start[] = { 0, 10, 20 };
    size_t startCount = sizeof(start) / sizeof(start[0]);
    SkScalar stop[] = { 30, 40, 50 };
    size_t stopCount = sizeof(stop) / sizeof(stop[0]);
    SkRect rect2 = SkRect::MakeXYWH(10, 10, 30, 30);
    for (size_t outDir = SkPath::kCW_Direction; outDir <= SkPath::kCCW_Direction; ++outDir) {
        for (size_t inDir = SkPath::kCW_Direction; inDir <= SkPath::kCCW_Direction; ++inDir) {
            for (size_t startIndex = 0; startIndex < startCount; ++startIndex) {
                for (size_t stopIndex = 0; stopIndex < stopCount; ++stopIndex) {
                    SkRect rect1 = SkRect::MakeLTRB(start[startIndex], 0, stop[stopIndex], 0);
                    SkPath path, out;
                    path.addRect(rect1, static_cast<SkPath::Direction>(outDir));
                    path.addRect(rect2, static_cast<SkPath::Direction>(inDir));
                    simplify(path, true, out);
                    SkRect rect;
                    if (!out.isRect(&rect)) {
                        SkDebugf("%s 1 expected rect\n", __FUNCTION__);
                    }
                    if (rect != rect2) {
                        SkDebugf("%s 1 expected union\n", __FUNCTION__);
                    }
                    
                    path.reset();
                    rect1 = SkRect::MakeLTRB(start[startIndex], 40, stop[stopIndex], 40);
                    path.addRect(rect1, static_cast<SkPath::Direction>(outDir));
                    path.addRect(rect2, static_cast<SkPath::Direction>(inDir));
                    simplify(path, true, out);
                    if (!out.isRect(&rect)) {
                        SkDebugf("%s 2 expected rect\n", __FUNCTION__);
                    }
                    if (rect != rect2) {
                        SkDebugf("%s 2 expected union\n", __FUNCTION__);
                    }
                    
                    path.reset();
                    rect1 = SkRect::MakeLTRB(0, start[startIndex], 0, stop[stopIndex]);
                    path.addRect(rect1, static_cast<SkPath::Direction>(outDir));
                    path.addRect(rect2, static_cast<SkPath::Direction>(inDir));
                    simplify(path, true, out);
                    if (!out.isRect(&rect)) {
                        SkDebugf("%s 3 expected rect\n", __FUNCTION__);
                    }
                    if (rect != rect2) {
                        SkDebugf("%s 3 expected union\n", __FUNCTION__);
                    }
                    
                    path.reset();
                    rect1 = SkRect::MakeLTRB(40, start[startIndex], 40, stop[stopIndex]);
                    path.addRect(rect1, static_cast<SkPath::Direction>(outDir));
                    path.addRect(rect2, static_cast<SkPath::Direction>(inDir));
                    simplify(path, true, out);
                    if (!out.isRect(&rect)) {
                        SkDebugf("%s 4 expected rect\n", __FUNCTION__);
                    }
                    if (rect != rect2) {
                        SkDebugf("%s 4 expected union\n", __FUNCTION__);
                    }
                }
            }
        }
    }
}

static void testSimplifyDegenerate1() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect( 0,  0,  0, 30);
    path.addRect(10, 10, 40, 40);
    simplify(path, true, out);
    SkRect rect;
    if (!out.isRect(&rect)) {
        SkDebugf("%s expected rect\n", __FUNCTION__);
    }
    if (rect != SkRect::MakeLTRB(10, 10, 40, 40)) {
        SkDebugf("%s expected union\n", __FUNCTION__);
    }
}

static void (*simplifyTests[])() = {
    testSimplifyCoincidentInner,
    testSimplifyOverlapTiny,
    testSimplifyDegenerate1,
    testSimplifyCorner,
    testSimplifyDegenerate,
    testSimplifyOverlap,
    testSimplifyDiagonal,
    testSimplifyCoincident,
    testSimplifyCoincidentCW,
    testSimplifyCoincidentCCW,
    testSimplifyCoincidentVertical, 
    testSimplifyCoincidentHorizontal,
    testSimplifyAddL,                
    testSimplifyMulti,               
};

static size_t simplifyTestsCount = sizeof(simplifyTests) / sizeof(simplifyTests[0]);

static void (*firstTest)() = 0;

void SimplifyRectangularPaths_Test() {
    size_t index = 0;
    if (firstTest) {
        while (index < simplifyTestsCount && simplifyTests[index] != firstTest) {
            ++index;
        }
    }
    for ( ; index < simplifyTestsCount; ++index) {
        if (simplifyTests[index] == testSimplifyCorner) {
            // testSimplifyCorner fails because it expects two contours, where
            // only one is returned. Both results are reasonable, but if two
            // contours are desirable, or if we provide an option to choose
            // between longer contours and more contours, turn this back on. For
            // the moment, testSimplifyDiagonal also checks the test case, and
            // permits either two rects or one non-crossing poly as valid
            // unreported results.
            continue;
        }
        (*simplifyTests[index])();
    }
}

