/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/effects/SkBlenders.h"
#include "src/core/SkPathPriv.h"

using namespace skia_private;

// We don't always want to test NaNs and infinities.
static void fuzz_nice_float(Fuzz* fuzz, float* f) {
    float v;
    fuzz->next(&v);
    constexpr float kLimit = 1.0e35f;  // FLT_MAX?
    *f = (v == v && v <= kLimit && v >= -kLimit) ? v : 0.0f;
}

template <typename... Args>
static void fuzz_nice_float(Fuzz* fuzz, float* f, Args... rest) {
    fuzz_nice_float(fuzz, f);
    fuzz_nice_float(fuzz, rest...);
}

static void fuzz_nice_rect(Fuzz* fuzz, SkRect* r) {
    fuzz_nice_float(fuzz, &r->fLeft, &r->fTop, &r->fRight, &r->fBottom);
    r->sort();
}

// allows some float values for path points
void FuzzNicePath(Fuzz* fuzz, SkPath* path, int maxOps) {
    if (maxOps <= 0 || fuzz->exhausted() || path->countPoints() > 100000) {
        return;
    }
    uint8_t fillType;
    fuzz->nextRange(&fillType, 0, (uint8_t)SkPathFillType::kInverseEvenOdd);
    path->setFillType((SkPathFillType)fillType);
    uint8_t numOps;
    fuzz->nextRange(&numOps, 0, maxOps);
    for (uint8_t i = 0; i < numOps; ++i) {
        // When we start adding the path to itself, the fuzzer can make an
        // exponentially long path, which causes timeouts.
        if (path->countPoints() > 100000) {
            return;
        }
        // How many items in the switch statement below.
        constexpr uint8_t MAX_PATH_OPERATION = 32;
        uint8_t op;
        fuzz->nextRange(&op, 0, MAX_PATH_OPERATION);
        bool test;
        SkPath p;
        SkMatrix m;
        SkRRect rr;
        SkRect r;
        SkPathDirection dir;
        unsigned int ui;
        SkScalar a, b, c, d, e, f;
        switch (op) {
            case 0:
                fuzz_nice_float(fuzz, &a, &b);
                path->moveTo(a, b);
                break;
            case 1:
                fuzz_nice_float(fuzz, &a, &b);
                path->rMoveTo(a, b);
                break;
            case 2:
                fuzz_nice_float(fuzz, &a, &b);
                path->lineTo(a, b);
                break;
            case 3:
                fuzz_nice_float(fuzz, &a, &b);
                path->rLineTo(a, b);
                break;
            case 4:
                fuzz_nice_float(fuzz, &a, &b, &c, &d);
                path->quadTo(a, b, c, d);
                break;
            case 5:
                fuzz_nice_float(fuzz, &a, &b, &c, &d);
                path->rQuadTo(a, b, c, d);
                break;
            case 6:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e);
                path->conicTo(a, b, c, d, e);
                break;
            case 7:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e);
                path->rConicTo(a, b, c, d, e);
                break;
            case 8:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e, &f);
                path->cubicTo(a, b, c, d, e, f);
                break;
            case 9:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e, &f);
                path->rCubicTo(a, b, c, d, e, f);
                break;
            case 10:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e);
                path->arcTo(a, b, c, d, e);
                break;
            case 11:
                fuzz_nice_float(fuzz, &a, &b);
                fuzz_nice_rect(fuzz, &r);
                fuzz->next(&test);
                path->arcTo(r, a, b, test);
                break;
            case 12:
                path->close();
                break;
            case 13:
                fuzz_nice_rect(fuzz, &r);
                fuzz->nextRange(&ui, 0, 1);
                dir = static_cast<SkPathDirection>(ui);
                path->addRect(r, dir);
                break;
            case 14:
                fuzz->nextRange(&ui, 0, 1);
                dir = static_cast<SkPathDirection>(ui);
                fuzz_nice_rect(fuzz, &r);
                fuzz->next(&ui);
                path->addRect(r, dir, ui);
                break;
            case 15:
                fuzz->nextRange(&ui, 0, 1);
                dir = static_cast<SkPathDirection>(ui);
                fuzz_nice_rect(fuzz, &r);
                path->addOval(r, dir);
                break;
            case 16:
                fuzz->nextRange(&ui, 0, 1);
                dir = static_cast<SkPathDirection>(ui);
                fuzz_nice_rect(fuzz, &r);
                fuzz->next(&ui);
                path->addOval(r, dir, ui);
                break;
            case 17:
                fuzz->nextRange(&ui, 0, 1);
                dir = static_cast<SkPathDirection>(ui);
                fuzz_nice_float(fuzz, &a, &b, &c);
                path->addCircle(a, b, c, dir);
                break;
            case 18:
                fuzz_nice_rect(fuzz, &r);
                fuzz_nice_float(fuzz, &a, &b);
                path->addArc(r, a, b);
                break;
            case 19:
                fuzz_nice_float(fuzz, &a, &b);
                fuzz_nice_rect(fuzz, &r);
                fuzz->nextRange(&ui, 0, 1);
                dir = static_cast<SkPathDirection>(ui);
                path->addRoundRect(r, a, b, dir);
                break;
            case 20:
                FuzzNiceRRect(fuzz, &rr);
                fuzz->nextRange(&ui, 0, 1);
                dir = static_cast<SkPathDirection>(ui);
                path->addRRect(rr, dir);
                break;
            case 21:
                fuzz->nextRange(&ui, 0, 1);
                dir = static_cast<SkPathDirection>(ui);
                FuzzNiceRRect(fuzz, &rr);
                path->addRRect(rr, dir, ui);
                break;
            case 22: {
                fuzz->nextRange(&ui, 0, 1);
                SkPath::AddPathMode mode = static_cast<SkPath::AddPathMode>(ui);
                FuzzNiceMatrix(fuzz, &m);
                FuzzNicePath(fuzz, &p, maxOps-1);
                path->addPath(p, m, mode);
                break;
            }
            case 23: {
                fuzz->nextRange(&ui, 0, 1);
                SkPath::AddPathMode mode = static_cast<SkPath::AddPathMode>(ui);
                FuzzNiceMatrix(fuzz, &m);
                path->addPath(*path, m, mode);
                break;
            }
            case 24:
                FuzzNicePath(fuzz, &p, maxOps-1);
                path->reverseAddPath(p);
                break;
            case 25:
                path->addPath(*path);
                break;
            case 26:
                path->reverseAddPath(*path);
                break;
            case 27:
                fuzz_nice_float(fuzz, &a, &b);
                path->offset(a, b, path);
                break;
            case 28:
                FuzzNicePath(fuzz, &p, maxOps-1);
                fuzz_nice_float(fuzz, &a, &b);
                p.offset(a, b, path);
                break;
            case 29:
                FuzzNiceMatrix(fuzz, &m);
                path->transform(m, path);
                break;
            case 30:
                FuzzNicePath(fuzz, &p, maxOps-1);
                // transform can explode path sizes so skip this op if p too big
                if (p.countPoints() <= 100000) {
                    FuzzNiceMatrix(fuzz, &m);
                    p.transform(m, path);
                }
                break;
            case 31:
                fuzz_nice_float(fuzz, &a, &b);
                path->setLastPt(a, b);
                break;
            case MAX_PATH_OPERATION:
                SkPathPriv::ShrinkToFit(path);
                break;

            default:
                SkASSERT(false);
                break;
        }
        SkASSERTF(       path->isValid(),        "path->isValid() failed at op %d, case %d", i, op);
    }
}

// allows all float values for path points
void FuzzEvilPath(Fuzz* fuzz, SkPath* path, int last_verb) {
  while (!fuzz->exhausted()) {
    // Use a uint8_t to conserve bytes.  This makes our "fuzzed bytes footprint"
    // smaller, which leads to more efficient fuzzing.
    uint8_t operation;
    fuzz->next(&operation);
    SkScalar a,b,c,d,e,f;

    switch (operation % (last_verb + 1)) {
      case SkPath::Verb::kMove_Verb:
        fuzz->next(&a, &b);
        path->moveTo(a, b);
        break;

      case SkPath::Verb::kLine_Verb:
        fuzz->next(&a, &b);
        path->lineTo(a, b);
        break;

      case SkPath::Verb::kQuad_Verb:
        fuzz->next(&a, &b, &c, &d);
        path->quadTo(a, b, c, d);
        break;

      case SkPath::Verb::kConic_Verb:
        fuzz->next(&a, &b, &c, &d, &e);
        path->conicTo(a, b, c, d, e);
        break;

      case SkPath::Verb::kCubic_Verb:
        fuzz->next(&a, &b, &c, &d, &e, &f);
        path->cubicTo(a, b, c, d, e, f);
        break;

      case SkPath::Verb::kClose_Verb:
        path->close();
        break;

      case SkPath::Verb::kDone_Verb:
        // In this case, simply exit.
        return;
    }
  }
}

void FuzzNiceRRect(Fuzz* fuzz, SkRRect* rr) {
    SkRect r;
    fuzz_nice_rect(fuzz, &r);

    SkVector radii[4];
    for (SkVector& vec : radii) {
        fuzz->nextRange(&vec.fX, 0.0f, 1.0f);
        vec.fX *= 0.5f * r.width();
        fuzz->nextRange(&vec.fY, 0.0f, 1.0f);
        vec.fY *= 0.5f * r.height();
    }
    rr->setRectRadii(r, radii);
    SkASSERT(rr->isValid());
}

void FuzzNiceMatrix(Fuzz* fuzz, SkMatrix* m) {
    constexpr int kArrayLength = 9;
    SkScalar buffer[kArrayLength];
    int matrixType;
    fuzz->nextRange(&matrixType, 0, 4);
    switch (matrixType) {
        case 0:  // identity
            *m = SkMatrix::I();
            return;
        case 1:  // translate
            fuzz->nextRange(&buffer[0], -4000.0f, 4000.0f);
            fuzz->nextRange(&buffer[1], -4000.0f, 4000.0f);
            *m = SkMatrix::Translate(buffer[0], buffer[1]);
            return;
        case 2:  // translate + scale
            fuzz->nextRange(&buffer[0], -400.0f, 400.0f);
            fuzz->nextRange(&buffer[1], -400.0f, 400.0f);
            fuzz->nextRange(&buffer[2], -4000.0f, 4000.0f);
            fuzz->nextRange(&buffer[3], -4000.0f, 4000.0f);
            *m = SkMatrix::Scale(buffer[0], buffer[1]);
            m->postTranslate(buffer[2], buffer[3]);
            return;
        case 3:  // affine
            fuzz->nextN(buffer, 6);
            m->setAffine(buffer);
            return;
        case 4:  // perspective
            fuzz->nextN(buffer, kArrayLength);
            m->set9(buffer);
            return;
        default:
            SkASSERT(false);
            return;
    }
}

void FuzzNiceRegion(Fuzz* fuzz, SkRegion* region, int maxN) {
    uint8_t N;
    fuzz->nextRange(&N, 0, maxN);
    for (uint8_t i = 0; i < N; ++i) {
        SkIRect r;
        SkRegion::Op op;
        // Avoid the sentinel value used by Region.
        fuzz->nextRange(&r.fLeft,   -2147483646, 2147483646);
        fuzz->nextRange(&r.fTop,    -2147483646, 2147483646);
        fuzz->nextRange(&r.fRight,  -2147483646, 2147483646);
        fuzz->nextRange(&r.fBottom, -2147483646, 2147483646);
        r.sort();
        fuzz->nextEnum(&op, SkRegion::kLastOp);
        if (!region->op(r, op)) {
            return;
        }
    }
}

void FuzzCreateValidInputsForRuntimeEffect(SkRuntimeEffect* effect,
                                           sk_sp<SkData>& uniformBytes,
                                           TArray<SkRuntimeEffect::ChildPtr>& children) {
    // Create storage for our uniforms.
    uniformBytes = SkData::MakeZeroInitialized(effect->uniformSize());
    void* uniformData = uniformBytes->writable_data();

    for (const SkRuntimeEffect::Uniform& u : effect->uniforms()) {
        // We treat scalars, vectors, matrices and arrays the same. We just figure out how many
        // uniform slots need to be filled, and write consecutive numbers into those slots.
        static_assert(sizeof(int) == 4 && sizeof(float) == 4);
        size_t numFields = u.sizeInBytes() / 4;

        if (u.type == SkRuntimeEffect::Uniform::Type::kInt ||
            u.type == SkRuntimeEffect::Uniform::Type::kInt2 ||
            u.type == SkRuntimeEffect::Uniform::Type::kInt3 ||
            u.type == SkRuntimeEffect::Uniform::Type::kInt4) {
            int intVal = 0;
            while (numFields--) {
                // Assign increasing integer values to each slot (0, 1, 2, ...).
                *static_cast<int*>(uniformData) = intVal++;
                uniformData = static_cast<int*>(uniformData) + 1;
            }
        } else {
            float floatVal = 0.0f;
            while (numFields--) {
                // Assign increasing float values to each slot (0.0, 1.0, 2.0, ...).
                *static_cast<float*>(uniformData) = floatVal++;
                uniformData = static_cast<float*>(uniformData) + 1;
            }
        }
    }

    // Create valid children for any requested child effects.
    children.clear();
    children.reserve(effect->children().size());
    for (const SkRuntimeEffect::Child& c : effect->children()) {
        switch (c.type) {
            case SkRuntimeEffect::ChildType::kShader:
                children.push_back(SkShaders::Color(SK_ColorRED));
                break;
            case SkRuntimeEffect::ChildType::kColorFilter:
                children.push_back(SkColorFilters::Blend(SK_ColorBLUE, SkBlendMode::kModulate));
                break;
            case SkRuntimeEffect::ChildType::kBlender:
                children.push_back(SkBlenders::Arithmetic(0.50f, 0.25f, 0.10f, 0.05f, false));
                break;
        }
    }
}
