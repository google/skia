
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDiscretePathEffect.h"
#include "SkBuffer.h"
#include "SkPathMeasure.h"
#include "SkRandom.h"

static void Perterb(SkPoint* p, const SkVector& tangent, SkScalar scale) {
    SkVector normal = tangent;
    normal.rotateCCW();
    normal.setLength(scale);
    *p += normal;
}


SkDiscretePathEffect::SkDiscretePathEffect(SkScalar segLength, SkScalar deviation)
    : fSegLength(segLength), fPerterb(deviation)
{
}

bool SkDiscretePathEffect::filterPath(SkPath* dst, const SkPath& src,
                                      SkScalar* width) {
    bool doFill = *width < 0;

    SkPathMeasure   meas(src, doFill);
    uint32_t        seed = SkScalarRound(meas.getLength());
    SkRandom        rand(seed ^ ((seed << 16) | (seed >> 16)));
    SkScalar        scale = fPerterb;
    SkPoint         p;
    SkVector        v;

    do {
        SkScalar    length = meas.getLength();

        if (fSegLength * (2 + doFill) > length) {
            meas.getSegment(0, length, dst, true);  // to short for us to mangle
        } else {
            int         n = SkScalarRound(SkScalarDiv(length, fSegLength));
            SkScalar    delta = length / n;
            SkScalar    distance = 0;

            if (meas.isClosed()) {
                n -= 1;
                distance += delta/2;
            }
            
            if (meas.getPosTan(distance, &p, &v)) {
                Perterb(&p, v, SkScalarMul(rand.nextSScalar1(), scale));
                dst->moveTo(p);
            }
            while (--n >= 0) {
                distance += delta;
                if (meas.getPosTan(distance, &p, &v)) {
                    Perterb(&p, v, SkScalarMul(rand.nextSScalar1(), scale));
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

void SkDiscretePathEffect::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSegLength);
    buffer.writeScalar(fPerterb);
}

SkDiscretePathEffect::SkDiscretePathEffect(SkFlattenableReadBuffer& buffer) {
    fSegLength = buffer.readScalar();
    fPerterb = buffer.readScalar();
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR(SkDiscretePathEffect)

