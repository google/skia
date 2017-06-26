/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <array>
#include <tuple>
#include <vector>
#include "SkLinearBitmapPipeline.h"
#include "SkColor.h"
#include "SkNx.h"
#include "SkPoint.h"
#include "SkPM4f.h"
#include "Test.h"
#include "SkLinearBitmapPipeline_tile.h"


DEF_TEST(LBPBilerpEdge, reporter) {

}

static SkString dump(SkScalar cut, Span prefix, Span remainder) {
    SkPoint prefixStart; SkScalar prefixLen; int prefixCount;
    std::tie(prefixStart, prefixLen, prefixCount) = prefix;
    SkPoint remainderStart; SkScalar remainderLen; int remainderCount;
    std::tie(remainderStart, remainderLen, remainderCount) = remainder;
    return SkStringPrintf("cut: %f prefix: (%f, %f), %f, %d - remainder: (%f, %f), %f, %d",
                          cut,
                          prefixStart.fX, prefixStart.fY, prefixLen, prefixCount,
                          remainderStart.fX, remainderStart.fY, remainderLen, remainderCount);
}

static void check_span_result(
    skiatest::Reporter* reporter,
    Span span, SkScalar dx, SkScalar cut, SkPoint start, SkScalar len, int count) {
    SkPoint originalStart; SkScalar originalLen; int originalCount;
    std::tie(originalStart, originalLen, originalCount) = span;

    Span prefix = span.breakAt(cut, dx);

    SkPoint prefixStart; SkScalar prefixLen; int prefixCount;
    std::tie(prefixStart, prefixLen, prefixCount) = prefix;

    REPORTER_ASSERT_MESSAGE(reporter, prefixStart == start, dump(cut, prefix, span));
    REPORTER_ASSERT_MESSAGE(reporter, prefixLen == len, dump(cut, prefix, span));
    REPORTER_ASSERT_MESSAGE(reporter, prefixCount == count, dump(cut, prefix, span));
    SkPoint expectedRemainderStart;
    SkScalar expectedRemainderLen;
    int expectedRemainderCount;
    if (prefix.isEmpty()) {
        expectedRemainderStart = originalStart;
        expectedRemainderLen = originalLen;
        expectedRemainderCount = originalCount;
    } else {
        expectedRemainderStart = SkPoint::Make(originalStart.fX + prefixLen + dx, originalStart.fY);
        expectedRemainderLen = originalLen - prefixLen - dx;
        expectedRemainderCount = originalCount - prefixCount;
    }

    if (!span.isEmpty()) {
        SkPoint remainderStart;
        SkScalar remainderLen;
        int remainderCount;
        std::tie(remainderStart, remainderLen, remainderCount) = span;
        // Remainder span
        REPORTER_ASSERT_MESSAGE(reporter, expectedRemainderStart == remainderStart,
                                dump(cut, prefix, span));
        REPORTER_ASSERT_MESSAGE(reporter,
                                expectedRemainderLen == remainderLen,
                                dump(cut, prefix, span));
        REPORTER_ASSERT_MESSAGE(reporter,
                                expectedRemainderCount == remainderCount,
                                dump(cut, prefix, span));
    }
}

DEF_TEST(LBPSpanOps, reporter) {
    {
        SkScalar dx = 1.0f;
        SkPoint start = SkPoint::Make(-5, -5);
        Span span{start, 9.0f, 10};
        check_span_result(reporter, span, dx,  0.0f, start, 4.0f, 5);
        check_span_result(reporter, span, dx, -6.0f, SkPoint::Make(0, 0), 0.0f, 0);
        check_span_result(reporter, span, dx, -5.0f, SkPoint::Make(0, 0), 0.0f, 0);
        check_span_result(reporter, span, dx, -4.0f, SkPoint::Make(-5, -5), 0.0f, 1);
        check_span_result(reporter, span, dx,  4.0f, SkPoint::Make(-5, -5), 8.0f, 9);
        check_span_result(reporter, span, dx,  5.0f, SkPoint::Make(-5, -5), 9.0f, 10);
        check_span_result(reporter, span, dx,  6.0f, SkPoint::Make(-5, -5), 9.0f, 10);
    }
    {
        SkScalar dx = -1.0f;
        SkPoint start = SkPoint::Make(5, 5);
        Span span{start, -9.0f, 10};
        check_span_result(reporter, span, dx,  0.0f, start, -5.0f, 6);
        check_span_result(reporter, span, dx, -6.0f, SkPoint::Make(5, 5), -9.0f, 10);
        check_span_result(reporter, span, dx, -5.0f, SkPoint::Make(5, 5), -9.0f, 10);
        check_span_result(reporter, span, dx, -4.0f, SkPoint::Make(5, 5), -9.0f, 10);
        check_span_result(reporter, span, dx,  4.0f, SkPoint::Make(5, 5), -1.0f, 2);
        check_span_result(reporter, span, dx,  5.0f, SkPoint::Make(5, 5), 0.0f, 1);
        check_span_result(reporter, span, dx,  6.0f, SkPoint::Make(0, 0), 0.0f, 0);
    }
}

DEF_TEST(LBPBilerpSpanOps, reporter) {

}

template <typename XTiler, typename YTiler>
static bool compare_tiler_case(
    XTiler& xTiler, YTiler& yTiler, Span span, skiatest::Reporter* reporter) {
    Span originalSpan = span;
    std::vector<SkPoint> listPoints;
    std::vector<SkPoint> spanPoints;
    struct Sink {
        void SK_VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) {
            SkASSERT(0 < n && n < 4);
            if (n >= 1) storePoint({xs[0], ys[0]});
            if (n >= 2) storePoint({xs[1], ys[1]});
            if (n >= 3) storePoint({xs[2], ys[2]});
        }

        void SK_VECTORCALL pointList4(Sk4s xs, Sk4s ys) {
            storePoint({xs[0], ys[0]});
            storePoint({xs[1], ys[1]});
            storePoint({xs[2], ys[2]});
            storePoint({xs[3], ys[3]});
        }

        void pointSpan(Span span) {
            span_fallback(span, this);
        }

        void storePoint(SkPoint pt) {
            fPoints->push_back({SkScalarFloorToScalar(X(pt)), SkScalarFloorToScalar(Y(pt))});
        }

        std::vector<SkPoint>* fPoints;
    };

    Sink listSink = {&listPoints};
    Sink spanSink = {&spanPoints};

    SkPoint start; SkScalar length; int count;
    std::tie(start, length, count) = span;

    SkScalar dx = length / (count - 1);
    Sk4f xs = Sk4f{X(start)} + Sk4f{0.0f, dx, 2 * dx, 3 * dx};
    Sk4f ys = Sk4f{Y(start)};
    while (count >= 4) {
        Sk4f txs = xs;
        Sk4f tys = ys;
        xTiler.tileXPoints(&txs);
        yTiler.tileYPoints(&tys);
        listSink.pointList4(txs, tys);
        xs = xs + 4.0f * dx;
        count -= 4;
    }
    if (count > 0) {
        xTiler.tileXPoints(&xs);
        yTiler.tileYPoints(&ys);
        listSink.pointListFew(count, xs, ys);
    }

    std::tie(start, length, count) = originalSpan;
    SkScalar x = X(start);
    SkScalar y = yTiler.tileY(Y(start));
    Span yAdjustedSpan{{x, y}, length, count};

    bool handledSpan = xTiler.maybeProcessSpan(yAdjustedSpan, &spanSink);
    if (handledSpan) {
        auto firstNotTheSame = std::mismatch(
            listPoints.begin(), listPoints.end(), spanPoints.begin());
        if (firstNotTheSame.first != listSink.fPoints->end()) {
            auto element = std::distance(listPoints.begin(), firstNotTheSame.first);
            SkASSERT(element >= 0);
            std::tie(start, length, count) = originalSpan;
            ERRORF(reporter, "Span: {%f, %f}, %f, %d", start.fX, start.fY, length, count);
            ERRORF(reporter, "Size points: %d, size span: %d",
                   listPoints.size(), spanPoints.size());
            if ((unsigned)element >= spanPoints.size()) {
                ERRORF(reporter, "Size points: %d, size span: %d",
                       listPoints.size(), spanPoints.size());
                // Mismatch off the end
                ERRORF(reporter,
                       "The mismatch is at position %d and has value %f, %f - it is off the end "
                           "of the other.",
                       element, X(*firstNotTheSame.first), Y(*firstNotTheSame.first));
            } else {
                ERRORF(reporter,
                       "Mismatch at %d - points: %f, %f - span: %f, %f",
                       element, listPoints[element].fX, listPoints[element].fY,
                       spanPoints[element].fX, spanPoints[element].fY);
            }
            SkFAIL("aha");
        }
    }
    return true;
}

template <typename XTiler, typename YTiler>
static bool compare_tiler_spans(int width, int height, skiatest::Reporter* reporter) {
    XTiler xTiler{width};
    YTiler yTiler{height};
    INFOF(reporter, "w: %d, h: %d \n", width, height);
    std::array<int, 8> interestingX {{-5, -1, 0, 1, width - 1, width, width + 1, width + 5}};
    std::array<int, 8> interestingY {{-5, -1, 0, 1, height - 1, height, height + 1, height + 5}};
    std::array<int, 6> interestingCount {{1, 2, 3, 4, 5, 10}};
    std::array<SkScalar, 7> interestingScale {{0.0f, 1.0f, 0.5f, 2.1f, -2.1f, -1.0f, -0.5f}};
    for (auto scale : interestingScale) {
        for (auto startX : interestingX) {
            for (auto count : interestingCount) {
                for (auto y : interestingY) {
                    Span span{
                        SkPoint::Make((SkScalar)startX, (SkScalar)y), (count-1.0f) * scale, count};
                    if (!compare_tiler_case(xTiler, yTiler, span, reporter)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

template <typename XTiler, typename YTiler>
static void test_tiler(skiatest::Reporter* reporter) {
    std::array<int, 6> interestingSize {{1, 2, 3, 4, 5, 10}};
    for (auto width : interestingSize) {
        for (auto height : interestingSize) {
            if (!compare_tiler_spans<XTiler, YTiler>(width, height, reporter)) { return; }
        }
    }
}
/*
DEF_TEST(LBPStrategyClampTile, reporter) {
#if 0
    ClampStrategy tiler{SkSize::Make(1, 1)};
    Span span{SkPoint::Make(0, -5), 1.0f, 2};
    compare_tiler_case<ClampStrategy>(tiler, span, reporter);
#else
    test_tiler<XClampStrategy, YClampStrategy>(reporter);
#endif
}

DEF_TEST(LBPStrategyRepeatTile, reporter) {
#if 0
    RepeatStrategy tiler{SkSize::Make(3, 1)};
    Span span{SkPoint::Make(-5, -5), 20 * 2.1f, 100};
    compare_tiler_case<RepeatStrategy>(tiler, span, reporter);
#else
    test_tiler<XRepeatStrategy, YRepeatStrategy>(reporter);
#endif
}
*/
