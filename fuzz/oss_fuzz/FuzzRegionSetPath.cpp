/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../Fuzz.h"
#include "SkData.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkTypes.h"

// We don't always want to test NaNs and infinities.
static void fuzz_nice_float(Fuzz* fuzz, float* f) {
    float v;
    fuzz->next(&v);
    constexpr float kLimit = 1.0e35f;  // FLT_MAX?
    *f = (v == v && v <= kLimit && v >= -kLimit) ? v : 0.0f;
}

template <typename... Args>
inline void fuzz_nice_float(Fuzz* fuzz, float* f, Args... rest) {
    fuzz_nice_float(fuzz, f);
    fuzz_nice_float(fuzz, rest...);
}

static void fuzz_path(Fuzz* fuzz, SkPath* path, int maxOps) {
    if (maxOps < 2) {
        maxOps = 2;
    }
    uint8_t fillType;
    fuzz->nextRange(&fillType, 0, (uint8_t)SkPath::kInverseEvenOdd_FillType);
    path->setFillType((SkPath::FillType)fillType);
    uint8_t numOps;
    fuzz->nextRange(&numOps, 2, maxOps);
    for (uint8_t i = 0; i < numOps; ++i) {
        uint8_t op;
        fuzz->nextRange(&op, 0, 6);
        SkScalar a, b, c, d, e, f;
        switch (op) {
            case 0:
                fuzz_nice_float(fuzz, &a, &b);
                path->moveTo(a, b);
                break;
            case 1:
                fuzz_nice_float(fuzz, &a, &b);
                path->lineTo(a, b);
                break;
            case 2:
                fuzz_nice_float(fuzz, &a, &b, &c, &d);
                path->quadTo(a, b, c, d);
                break;
            case 3:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e);
                path->conicTo(a, b, c, d, e);
                break;
            case 4:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e, &f);
                path->cubicTo(a, b, c, d, e, f);
                break;
            case 5:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e);
                path->arcTo(a, b, c, d, e);
                break;
            case 6:
                path->close();
                break;
            default:
                break;
        }
    }
}

template <>
inline void Fuzz::next(SkRegion* region) {
    uint8_t N;
    this->nextRange(&N, 0, 10);
    for (uint8_t i = 0; i < N; ++i) {
        SkIRect r;
        uint8_t op;
        this->next(&r);
        r.sort();
        this->nextRange(&op, 0, (uint8_t)SkRegion::kLastOp);
        if (!region->op(r, (SkRegion::Op)op)) {
            return;
        }
    }
}

int FuzzRegionSetPath(Fuzz* fuzz) {
    SkPath p;
    int maxOps;
    fuzz->nextRange(&maxOps, 0, 1000);
    fuzz_path(fuzz, &p, maxOps);
    SkRegion r1;
    bool initR1;
    fuzz->next(&initR1);
    if (initR1) {
        fuzz->next(&r1);
    }
    SkRegion r2;
    fuzz->next(&r2);

    r1.setPath(p, r2);

    // Do some follow on computations to make sure region is well-formed.
    r1.computeRegionComplexity();
    r1.isComplex();
    if (r1 == r2) {
        r1.contains(0,0);
    } else {
        r1.contains(1,1);
    }

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    sk_sp<SkData> bytes(SkData::MakeWithoutCopy(data, size));
    Fuzz fuzz(bytes);
    return FuzzRegionSetPath(&fuzz);
}

