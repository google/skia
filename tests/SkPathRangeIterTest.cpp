/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/core/SkPathPriv.h"
#include "tests/Test.h"

SkPoint next_point(SkRandom& rand) { return {rand.nextF(), rand.nextF()}; }

DEF_TEST(SkPath_RangeIter, r) {
    enum class Verb {
        kMove = (int)SkPathVerb::kMove,
        kLine = (int)SkPathVerb::kLine,
        kQuad = (int)SkPathVerb::kQuad,
        kConic = (int)SkPathVerb::kConic,
        kCubic = (int)SkPathVerb::kCubic,
        kClose = (int)SkPathVerb::kClose,
        kImplicitMove
    };

    Verb verbs[] = {
        Verb::kImplicitMove,
        Verb::kLine,
        Verb::kConic,
        Verb::kClose,
        Verb::kImplicitMove,
        Verb::kCubic,
        Verb::kMove,
        Verb::kConic,
        Verb::kLine,
        Verb::kClose,
        Verb::kMove,
        Verb::kMove
    };

    class : SkRandom {
    public:
        SkPoint p() { return {this->SkRandom::nextF(), this->SkRandom::nextF()}; }
        float w() { return this->SkRandom::nextF(); }
    } genData, testData;

    for (int i = 0; i < 10; ++i) {
        if (genData.p() != testData.p() || genData.w() != testData.w()) {
            ERRORF(r, "genData and testData not in sync.");
            return;
        }
    }

    // Build the path.
    SkPath path;
    for (Verb verb : verbs) {
        switch (verb) {
            case Verb::kImplicitMove:
                break;
            case Verb::kMove:
                path.moveTo(genData.p());
                break;
            case Verb::kLine:
                path.lineTo(genData.p());
                break;
            case Verb::kQuad: {
                auto a = genData.p();
                auto b = genData.p();
                path.quadTo(a, b);
                break;
            }
            case Verb::kCubic: {
                auto a = genData.p();
                auto b = genData.p();
                auto c = genData.p();
                path.cubicTo(a, b, c);
                break;
            }
            case Verb::kConic: {
                auto a = genData.p();
                auto b = genData.p();
                path.conicTo(a, b, genData.w());
                break;
            }
            case Verb::kClose:
                path.close();
                break;
        }
    }

    // Verify sure the RangeIter works as expected.
    SkPathPriv::Iterate iterate(path);
    auto iter = iterate.begin();
    SkPoint startPt = {0,0};
    SkPoint lastPt = {0,0};
    for (Verb verb : verbs) {
        auto [pathVerb, pathPts, pathWt] = *iter++;
        switch (verb) {
            case Verb::kImplicitMove:
                REPORTER_ASSERT(r, pathPts[0] == startPt);
                lastPt = pathPts[0];
                break;
            case Verb::kMove:
                REPORTER_ASSERT(r, pathPts[0] == testData.p());
                startPt = lastPt = pathPts[0];
                break;
            case Verb::kLine:
                REPORTER_ASSERT(r, pathPts[0] == lastPt);
                REPORTER_ASSERT(r, pathPts[1] == testData.p());
                lastPt = pathPts[1];
                break;
            case Verb::kQuad:
                REPORTER_ASSERT(r, pathPts[0] == lastPt);
                REPORTER_ASSERT(r, pathPts[1] == testData.p());
                REPORTER_ASSERT(r, pathPts[2] == testData.p());
                lastPt = pathPts[2];
                break;
            case Verb::kCubic:
                REPORTER_ASSERT(r, pathPts[0] == lastPt);
                REPORTER_ASSERT(r, pathPts[1] == testData.p());
                REPORTER_ASSERT(r, pathPts[2] == testData.p());
                REPORTER_ASSERT(r, pathPts[3] == testData.p());
                lastPt = pathPts[3];
                break;
            case Verb::kConic:
                REPORTER_ASSERT(r, pathPts[0] == lastPt);
                REPORTER_ASSERT(r, pathPts[1] == testData.p());
                REPORTER_ASSERT(r, pathPts[2] == testData.p());
                REPORTER_ASSERT(r, *pathWt == testData.w());
                lastPt = pathPts[2];
                break;
            case Verb::kClose:
                REPORTER_ASSERT(r, pathPts[0] == lastPt);
                break;
        }
    }
    REPORTER_ASSERT(r, iter == iterate.end());
}
