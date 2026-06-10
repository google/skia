/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/Tiger.h"
#include "include/core/SkPath.h"
#include "include/private/SkTArray.h"
#include "src/gpu/ganesh/geometry/GrPathUtils.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"

#include <vector>

namespace {
using namespace skgpu::graphite;

template <FlattenMode kMode> class FlattenBench : public Benchmark {
public:
    FlattenBench() : fPaths(Tiger::GetTigerPaths()) {
        fName.printf("SparseStrips_Flatten_%s", kMode == FlattenMode::kScalar ? "Scalar" : "SIMD");
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        Flatten flattener;
        Polyline polyline;
        for (int i = 0; i < loops; ++i) {
            for (auto& path : fPaths) {
                flattener.processPaths<kMode>(
                        path, SkMatrix(), Tiger::kTigerWidthF, Tiger::kTigerHeightF, &polyline);
            }
            polyline.reset();
        }
    }

private:
    SkString fName;
    std::vector<SkPath> fPaths;
};

class GrLegacyBench : public Benchmark {
public:
    // Match the tolerances of Flatten.
    static constexpr SkScalar kTolerance = Flatten::kQuadErrTolerance;
    static constexpr SkScalar kToleranceSqd = Flatten::kQuadTolerance2;

    GrLegacyBench() : fName("SparseStrips_FlattenLegacy_Ganesh"), fPaths(Tiger::GetTigerPaths()) {}

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        SkAutoConicToQuads converter;
        skia_private::TArray<SkPoint> pointBuffer;
        skia_private::TArray<SkPoint, true> cubicQuadsBuffer;

        for (int i = 0; i < loops; ++i) {
            for (auto& path : fPaths) {
                SkPath::Iter iter(path, false);
                SkPoint pts[4];
                SkPath::Verb verb;

                while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
                    switch (verb) {
                        case SkPath::kMove_Verb:
                        case SkPath::kLine_Verb:
                            pointBuffer.push_back(pts[0]);
                            if (verb == SkPath::kLine_Verb) {
                                pointBuffer.push_back(pts[1]);
                            }
                            break;

                        case SkPath::kQuad_Verb: {
                            this->flattenQuad(pts, &pointBuffer);
                            break;
                        }

                        case SkPath::kConic_Verb: {
                            const SkPoint* quadPts =
                                    converter.computeQuads(pts, iter.conicWeight(), kTolerance);
                            int quadCount = converter.countQuads();
                            for (int j = 0; j < quadCount; ++j) {
                                this->flattenQuad(&quadPts[j * 2], &pointBuffer);
                            }
                            break;
                        }

                        case SkPath::kCubic_Verb: {
                            cubicQuadsBuffer.clear();
                            GrPathUtils::convertCubicToQuads(pts, kTolerance, &cubicQuadsBuffer);
                            int quadCount = cubicQuadsBuffer.empty()
                                                    ? 0
                                                    : (cubicQuadsBuffer.size() - 1) / 2;
                            for (int j = 0; j < quadCount; ++j) {
                                this->flattenQuad(&cubicQuadsBuffer[j * 2], &pointBuffer);
                            }
                            break;
                        }
                        case SkPath::kClose_Verb:
                        default:
                            break;
                    }
                }
            }
            pointBuffer.clear();
        }
    }

private:
    void flattenQuad(const SkPoint pts[3], skia_private::TArray<SkPoint>* pointBuffer) const {
        uint32_t count = GrPathUtils::quadraticPointCount(pts, kTolerance);
        SkPoint* outPtr = pointBuffer->push_back_n(count);
        GrPathUtils::generateQuadraticPoints(pts[0], pts[1], pts[2], kToleranceSqd, &outPtr, count);
    }

    SkString fName;
    std::vector<SkPath> fPaths;
};

}  // namespace

DEF_BENCH(return (new FlattenBench<skgpu::graphite::FlattenMode::kScalar>());)
DEF_BENCH(return (new FlattenBench<skgpu::graphite::FlattenMode::kSimd>());)
DEF_BENCH(return (new GrLegacyBench());)
