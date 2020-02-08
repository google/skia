/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/core/SkPathMeasure.h"
#include "include/core/SkStrokeRec.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/private/SkFixed.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

sk_sp<SkPathEffect> SkDiscretePathEffect::Make(SkScalar segLength, SkScalar deviation,
                                               uint32_t seedAssist) {
    if (!SkScalarIsFinite(segLength) || !SkScalarIsFinite(deviation)) {
        return nullptr;
    }
    if (segLength <= SK_ScalarNearlyZero) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkDiscretePathEffect(segLength, deviation, seedAssist));
}

static void Perterb(SkPoint* p, const SkVector& tangent, SkScalar scale) {
    SkVector normal = tangent;
    SkPointPriv::RotateCCW(&normal);
    normal.setLength(scale);
    *p += normal;
}

SkDiscretePathEffect::SkDiscretePathEffect(SkScalar segLength,
                                           SkScalar deviation,
                                           uint32_t seedAssist)
    : fSegLength(segLength), fPerterb(deviation), fSeedAssist(seedAssist)
{
}

/** \class LCGRandom

    Utility class that implements pseudo random 32bit numbers using a fast
    linear equation. Unlike rand(), this class holds its own seed (initially
    set to 0), so that multiple instances can be used with no side-effects.

    Copied from the original implementation of SkRandom. Only contains the
    methods used by SkDiscretePathEffect::filterPath, with methods that were
    not called directly moved to private.
*/

class LCGRandom {
public:
    LCGRandom(uint32_t seed) : fSeed(seed) {}

    /** Return the next pseudo random number expressed as a SkScalar
        in the range [-SK_Scalar1..SK_Scalar1).
    */
    SkScalar nextSScalar1() { return SkFixedToScalar(this->nextSFixed1()); }

private:
    /** Return the next pseudo random number as an unsigned 32bit value.
    */
    uint32_t nextU() { uint32_t r = fSeed * kMul + kAdd; fSeed = r; return r; }

    /** Return the next pseudo random number as a signed 32bit value.
     */
    int32_t nextS() { return (int32_t)this->nextU(); }

    /** Return the next pseudo random number expressed as a signed SkFixed
     in the range [-SK_Fixed1..SK_Fixed1).
     */
    SkFixed nextSFixed1() { return this->nextS() >> 15; }

    //  See "Numerical Recipes in C", 1992 page 284 for these constants
    enum {
        kMul = 1664525,
        kAdd = 1013904223
    };
    uint32_t fSeed;
};

bool SkDiscretePathEffect::onFilterPath(SkPath* dst, const SkPath& src,
                                        SkStrokeRec* rec, const SkRect*) const {
    bool doFill = rec->isFillStyle();

    SkPathMeasure   meas(src, doFill);

    /* Caller may supply their own seed assist, which by default is 0 */
    uint32_t seed = fSeedAssist ^ SkScalarRoundToInt(meas.getLength());

    LCGRandom   rand(seed ^ ((seed << 16) | (seed >> 16)));
    SkScalar    scale = fPerterb;
    SkPoint     p;
    SkVector    v;

    do {
        SkScalar    length = meas.getLength();

        if (fSegLength * (2 + doFill) > length) {
            meas.getSegment(0, length, dst, true);  // to short for us to mangle
        } else {
            int         n = SkScalarRoundToInt(length / fSegLength);
            constexpr int kMaxReasonableIterations = 100000;
            n = std::min(n, kMaxReasonableIterations);
            SkScalar    delta = length / n;
            SkScalar    distance = 0;

            if (meas.isClosed()) {
                n -= 1;
                distance += delta/2;
            }

            if (meas.getPosTan(distance, &p, &v)) {
                Perterb(&p, v, rand.nextSScalar1() * scale);
                dst->moveTo(p);
            }
            while (--n >= 0) {
                distance += delta;
                if (meas.getPosTan(distance, &p, &v)) {
                    Perterb(&p, v, rand.nextSScalar1() * scale);
                    dst->lineTo(p);
                }
            }
            if (meas.isClosed()) {
                dst->close();
            }
        }
    } while (meas.nextContour());
    return true;
}

sk_sp<SkFlattenable> SkDiscretePathEffect::CreateProc(SkReadBuffer& buffer) {
    SkScalar segLength = buffer.readScalar();
    SkScalar perterb = buffer.readScalar();
    uint32_t seed = buffer.readUInt();
    return Make(segLength, perterb, seed);
}

void SkDiscretePathEffect::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fSegLength);
    buffer.writeScalar(fPerterb);
    buffer.writeUInt(fSeedAssist);
}
