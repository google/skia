/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"
#include "src/utils/SkArcLengthParameterization.h"

namespace {

SkPath makeCubics() {
    SkRandom r;
    SkPath path;

    const auto randPt = [&r]() -> SkPoint {
        constexpr float min = -2000, max = 2000;
        return SkPoint::Make(r.nextRangeF(min, max), r.nextRangeF(min, max));
    };

    constexpr int kNumCubics = 1000;
    for (int i = 0; i < kNumCubics; i++) {
        if (i == 0) {
            const SkPoint p0 = randPt();
            path.moveTo(p0);
        }
        path.cubicTo(randPt(), randPt(), randPt());
        // path.lineTo(randPt());
        // path.quadTo(randPt(), randPt());
        // path.conicTo(randPt(), randPt(), r.nextRangeF(0.1f, 10.f));
    }

    return path;
}

}  // namespace

class PathMeasureBench : public Benchmark {
    SkString fName;

public:
    PathMeasureBench(const char name[]) { fName.printf("pathmeasure_%s", name); }

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    virtual void performTest() = 0;

protected:
    virtual int mulLoopCount() const { return 1; }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            this->performTest();
        }
    }

private:
    using INHERITED = Benchmark;
};

class CurrentBench : public PathMeasureBench {
public:
    CurrentBench() : INHERITED("current") { fPath = makeCubics(); }

protected:
    void performTest() override {
        SkPathMeasure meas(fPath, false);
        // xoring into a volatile prevents the compiler from optimizing these away
        volatile int junk = 0;
        // SkDebugf("length = %f\n", meas.getLength());
        junk ^= int(meas.getLength());
    }

private:
    SkPath fPath;

    using INHERITED = PathMeasureBench;
};

class NewBench : public PathMeasureBench {
public:
    NewBench() : INHERITED("new") { fPath = makeCubics(); }

protected:
    void performTest() override {
        SkArcLengthParameterization param(fPath);
        // xoring into a volatile prevents the compiler from optimizing these away
        volatile int junk = 0;
        // SkDebugf("new length = %f\n", param.totalLength());
        junk ^= int(param.totalLength());
    }

private:
    SkPath fPath;

    using INHERITED = PathMeasureBench;
};

class CurrentDashBench : public PathMeasureBench {
public:
    CurrentDashBench() : INHERITED("current_dash") { fPath = makeCubics(); }

protected:
    void performTest() override {
        constexpr std::array<float, 2> fIntervals = {10, 5};

        int index = 0;
        float dlen = fIntervals[index];
        SkPath dst;

        SkPathMeasure meas(fPath, false);
        const float totalLen = meas.getLength();
        double distance = 0;
        while (distance < totalLen) {
            meas.getSegment(distance, distance + dlen, &dst, true);
            distance += dlen;

            // next interval
            index = (index + 1) % fIntervals.size();
            dlen = fIntervals[index];

            // skip gaps
            if (index % 2 == 1) {
                index = (index + 1) % fIntervals.size();
                dlen = fIntervals[index];
                distance += dlen;
            }
        }

        // xoring into a volatile prevents the compiler from optimizing these away
        volatile int junk = 0;
        junk ^= int(dst.getBounds().fBottom);
    }

private:
    SkPath fPath;

    using INHERITED = PathMeasureBench;
};

class NewDashBench : public PathMeasureBench {
public:
    NewDashBench() : INHERITED("new_dash") { fPath = makeCubics(); }

protected:
    void performTest() override {
        constexpr std::array<float, 2> fIntervals = {10, 5};

        int index = 0;
        float dlen = fIntervals[index];
        SkPath dst;

        SkArcLengthParameterization param(fPath);
        SkArcLengthSegmentIter iter(param);
        const float totalLen = param.totalLength();
        while (iter.currU() < 1.0f) {
            const float du = dlen / totalLen;
            iter.getSegment(du, &dst);

            // next interval
            index = (index + 1) % fIntervals.size();
            dlen = fIntervals[index];

            // skip gaps
            if (index % 2 == 1) {
                iter.advance(dlen / totalLen);
                index = (index + 1) % fIntervals.size();
                dlen = fIntervals[index];
            }
        }

        // xoring into a volatile prevents the compiler from optimizing these away
        volatile int junk = 0;
        junk ^= int(dst.getBounds().fBottom);
    }

private:
    SkPath fPath;

    using INHERITED = PathMeasureBench;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new CurrentBench();)
DEF_BENCH(return new NewBench();)

DEF_BENCH(return new CurrentDashBench();)
DEF_BENCH(return new NewDashBench();)
