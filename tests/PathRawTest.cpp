/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRaw.h"

#include "tests/Test.h"

#include <functional>

namespace {

class SkSPathRawBuilder {
public:
    SkSPathRawBuilder(SkSpan<SkPoint> ptStore, SkSpan<SkPathVerb> vbStore, SkSpan<float> cnStore)
    : fPtStorage(ptStore)
    , fVbStorage(vbStore)
    , fCnStorage(cnStore)
    , fPts(0), fCns(0), fVbs(0)
    {}

    void moveTo(SkPoint);
    void lineTo(SkPoint);
    void quadTo(SkPoint, SkPoint);
    void conicTo(SkPoint, SkPoint, SkScalar w);
    void cubicTo(SkPoint, SkPoint, SkPoint);
    void close();

    SkPathRaw raw(SkPathFillType, SkPathConvexity) const;

private:
    SkSpan<SkPoint>    fPtStorage;
    SkSpan<SkPathVerb> fVbStorage;
    SkSpan<SkScalar>   fCnStorage;
    size_t fPts, fCns, fVbs;

    void check_extend_pts(size_t n) const {
        SkASSERT(fPts + n <= fPtStorage.size());
    }
    void check_extend_vbs(size_t n) const {
        SkASSERT(fVbs + n <= fVbStorage.size());
    }
    void check_extend_cns(size_t n) const {
        SkASSERT(fCns + n <= fCnStorage.size());
    }
};

} // namespace

void SkSPathRawBuilder::moveTo(SkPoint p) {
    check_extend_pts(1);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p;
    fVbStorage[fVbs++] = SkPathVerb::kMove;
}

void SkSPathRawBuilder::lineTo(SkPoint p) {
    check_extend_pts(1);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p;
    fVbStorage[fVbs++] = SkPathVerb::kLine;
}

void SkSPathRawBuilder::quadTo(SkPoint p1, SkPoint p2) {
    check_extend_pts(2);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fVbStorage[fVbs++] = SkPathVerb::kQuad;
}

void SkSPathRawBuilder::conicTo(SkPoint p1, SkPoint p2, SkScalar w) {
    check_extend_pts(2);
    check_extend_cns(1);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fCnStorage[fCns++] = w;
    fVbStorage[fVbs++] = SkPathVerb::kConic;
}

void SkSPathRawBuilder::cubicTo(SkPoint p1, SkPoint p2, SkPoint p3) {
    check_extend_pts(3);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fPtStorage[fPts++] = p3;
    fVbStorage[fVbs++] = SkPathVerb::kCubic;
}

void SkSPathRawBuilder::close() {
    check_extend_vbs(1);
    fVbStorage[fVbs++] = SkPathVerb::kClose;
}

SkPathRaw SkSPathRawBuilder::raw(SkPathFillType ft, SkPathConvexity convexity) const {
    const auto ptSpan = fPtStorage.first(fPts);
    return {
            ptSpan,
            fVbStorage.first(fVbs),
            fCnStorage.first(fCns),
            SkRect::BoundsOrEmpty(ptSpan),
            ft,
            convexity,
            SkPathPriv::ComputeSegmentMask(fVbStorage.first(fVbs)),
    };
}

static void check_iter(skiatest::Reporter* reporter, const SkPathRaw& raw,
                       SkSpan<SkPoint> pts, SkSpan<const SkPathVerb> vbs, SkSpan<const float> cns) {
    size_t pIndex = 0, vIndex = 0, cIndex = 0;
    size_t moveToIndex = 0; // track the start of each contour

    auto iter = raw.iter();
    while (auto r = iter.next()) {
        REPORTER_ASSERT(reporter, vIndex < vbs.size());
        REPORTER_ASSERT(reporter, vbs[vIndex++] == r->fVerb);
        switch (r->fVerb) {
            case SkPathVerb::kMove:
                moveToIndex = pIndex;
                REPORTER_ASSERT(reporter, pIndex < pts.size());
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->fPoints[0]);
                break;
            case SkPathVerb::kLine:
                REPORTER_ASSERT(reporter, pIndex < pts.size());
                REPORTER_ASSERT(reporter, pts[pIndex-1] == r->fPoints[0]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->fPoints[1]);
                break;
            case SkPathVerb::kQuad:
                REPORTER_ASSERT(reporter, pIndex+1 < pts.size());
                REPORTER_ASSERT(reporter, pts[pIndex-1] == r->fPoints[0]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->fPoints[1]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->fPoints[2]);
                break;
            case SkPathVerb::kConic:
                REPORTER_ASSERT(reporter, pIndex+1 < pts.size());
                REPORTER_ASSERT(reporter, pts[pIndex-1] == r->fPoints[0]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->fPoints[1]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->fPoints[2]);
                REPORTER_ASSERT(reporter, cIndex < cns.size());
                REPORTER_ASSERT(reporter, cns[cIndex++] == r->fConicWeight);
                break;
            case SkPathVerb::kCubic:
                REPORTER_ASSERT(reporter, pIndex+2 < pts.size());
                REPORTER_ASSERT(reporter, pts[pIndex-1] == r->fPoints[0]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->fPoints[1]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->fPoints[2]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->fPoints[3]);
                break;
            case SkPathVerb::kClose:
                REPORTER_ASSERT(reporter, pts[pIndex-1]    == r->fPoints[0]);   // last  pt
                REPORTER_ASSERT(reporter, pts[moveToIndex] == r->fPoints[1]);   // first pt
                break;
        }
    }

    // make sure the iter is really done
    REPORTER_ASSERT(reporter, !iter.next());
}

DEF_TEST(pathraw_iter, reporter) {
    SkPoint pts[11];
    SkPathVerb vbs[8];
    float cns[1];

    constexpr size_t N = 11;
    SkPoint p[N];
    for (size_t i = 0; i < N; ++i) {
        p[i] = {SkScalar(i), SkScalar(i)};
    }

    SkSPathRawBuilder bu(pts, vbs, cns);

    const SkPathVerb verbs[] = {
        SkPathVerb::kMove,
        SkPathVerb::kLine,
        SkPathVerb::kQuad,
        SkPathVerb::kCubic,
        SkPathVerb::kClose,
        SkPathVerb::kMove,
        SkPathVerb::kLine,
        SkPathVerb::kConic,
    };

    bu.moveTo(p[0]);
    bu.lineTo(p[1]);
    bu.quadTo(p[2], p[3]);
    bu.cubicTo(p[4], p[5], p[6]);
    bu.close();
    bu.moveTo(p[7]);
    bu.lineTo(p[8]);
    bu.conicTo(p[9], p[10], 2);

    auto raw = bu.raw(SkPathFillType::kWinding, SkPathConvexity::kUnknown);

    REPORTER_ASSERT(reporter, raw.fPoints.size() == N);
    REPORTER_ASSERT(reporter, raw.fVerbs.size() == 8);
    REPORTER_ASSERT(reporter, raw.fConics.size() == 1);

    check_iter(reporter, raw, p, verbs, cns);

    // now make sure pathbuilder generates the same results

    SkPathBuilder pb;

    pb.moveTo(p[0]);
    pb.lineTo(p[1]);
    pb.quadTo(p[2], p[3]);
    pb.cubicTo(p[4], p[5], p[6]);
    pb.close();
    pb.moveTo(p[7]);
    pb.lineTo(p[8]);
    pb.conicTo(p[9], p[10], 2);

    auto path = pb.detach();
    raw = SkPathPriv::Raw(path, SkResolveConvexity::kNo).value();

    check_iter(reporter, raw, p, verbs, cns);
}

DEF_TEST(pathraw_segmentmask, reporter) {
    auto check_mask = [reporter](const char* desc,
                                 uint32_t expectedMask,
                                 const std::function<void(SkSPathRawBuilder&)>& build) {
        skiatest::ReporterContext context(reporter, desc);
        // Make these buffers plenty big to hold any of the paths in the tests
        SkPoint pts[20];
        SkPathVerb vbs[20];
        float cns[10];
        SkSPathRawBuilder bu(pts, vbs, cns);
        build(bu);
        auto raw = bu.raw(SkPathFillType::kWinding, SkPathConvexity::kUnknown);
        REPORTER_ASSERT(reporter, raw.fSegmentMask == expectedMask);
    };

    check_mask("move-only", 0, [](SkSPathRawBuilder& bu) { bu.moveTo({0, 0}); });

    check_mask("line", SkPath::kLine_SegmentMask, [](SkSPathRawBuilder& bu) {
        bu.moveTo({0, 0});
        bu.lineTo({1, 1});
    });

    check_mask("quad", SkPath::kQuad_SegmentMask, [](SkSPathRawBuilder& bu) {
        bu.moveTo({0, 0});
        bu.quadTo({1, 1}, {2, 2});
    });

    check_mask("conic", SkPath::kConic_SegmentMask, [](SkSPathRawBuilder& bu) {
        bu.moveTo({0, 0});
        bu.conicTo({1, 1}, {2, 2}, 0.5f);
    });

    check_mask("cubic", SkPath::kCubic_SegmentMask, [](SkSPathRawBuilder& bu) {
        bu.moveTo({0, 0});
        bu.cubicTo({1, 1}, {2, 2}, {3, 3});
    });

    check_mask("line-quad",
               SkPath::kLine_SegmentMask | SkPath::kQuad_SegmentMask,
               [](SkSPathRawBuilder& bu) {
                   bu.moveTo({0, 0});
                   bu.lineTo({1, 1});
                   bu.quadTo({2, 2}, {3, 3});
               });

    check_mask("conic-cubic",
               SkPath::kConic_SegmentMask | SkPath::kCubic_SegmentMask,
               [](SkSPathRawBuilder& bu) {
                   bu.moveTo({0, 0});
                   bu.conicTo({1, 1}, {2, 2}, 0.5f);
                   bu.cubicTo({3, 3}, {4, 4}, {5, 5});
               });

    check_mask("all",
               SkPath::kLine_SegmentMask | SkPath::kQuad_SegmentMask | SkPath::kConic_SegmentMask |
                       SkPath::kCubic_SegmentMask,
               [](SkSPathRawBuilder& bu) {
                   bu.moveTo({0, 0});
                   bu.lineTo({1, 1});
                   bu.quadTo({2, 2}, {3, 3});
                   bu.conicTo({4, 4}, {5, 5}, 0.5f);
                   bu.cubicTo({6, 6}, {7, 7}, {8, 8});
                   bu.close();
               });

    check_mask("empty path", 0, [](SkSPathRawBuilder& bu) {
        bu.moveTo({0, 0});
        bu.close();
    });
}
