#include "Test.h"
#include "SkPath.h"
#include "SkPaint.h"

// found and fixed for webkit: mishandling when we hit recursion limit on
// mostly degenerate cubic flatness test
static void regression_cubic(skiatest::Reporter* reporter) {
    SkPath path, stroke;
    SkPaint paint;

    path.moveTo(460.2881309415525, 303.250847066498);
    path.cubicTo(463.36378422175284, 302.1169735073363,
                 456.32239330810046, 304.720354932878,
                 453.15255460013304, 305.788586869862);
    
    SkRect fillR, strokeR;
    fillR = path.getBounds();

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkIntToScalar(2));
    paint.getFillPath(path, &stroke);
    strokeR = stroke.getBounds();

    SkRect maxR = fillR;
    SkScalar miter = SkMaxScalar(SK_Scalar1, paint.getStrokeMiter());
    SkScalar inset = paint.getStrokeJoin() == SkPaint::kMiter_Join ?
                            SkScalarMul(paint.getStrokeWidth(), miter) :
                            paint.getStrokeWidth();
    maxR.inset(-inset, -inset);

    // test that our stroke didn't explode
    REPORTER_ASSERT(reporter, maxR.contains(strokeR));
}

static void TestPaint(skiatest::Reporter* reporter) {
    // TODO add general paint tests

    // regression tests
    regression_cubic(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Paint", TestPaintClass, TestPaint)
