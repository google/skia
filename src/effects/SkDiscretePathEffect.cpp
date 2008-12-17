/* libs/graphics/effects/SkDiscretePathEffect.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkDiscretePathEffect.h"
#include "SkBuffer.h"
#include "SkPathMeasure.h"
#include "SkRandom.h"

static void Perterb(SkPoint* p, const SkVector& tangent, SkScalar scale)
{
    SkVector normal = tangent;
    normal.rotateCCW();
    normal.setLength(scale);
    *p += normal;
}


SkDiscretePathEffect::SkDiscretePathEffect(SkScalar segLength, SkScalar deviation)
    : fSegLength(segLength), fPerterb(deviation)
{
}

bool SkDiscretePathEffect::filterPath(SkPath* dst, const SkPath& src, SkScalar* width)
{
    bool doFill = *width < 0;

    SkPathMeasure   meas(src, doFill);
    uint32_t        seed = SkScalarRound(meas.getLength());
    SkRandom        rand(seed ^ ((seed << 16) | (seed >> 16)));
    SkScalar        scale = fPerterb;
    SkPoint         p;
    SkVector        v;

    do {
        SkScalar    length = meas.getLength();

        if (fSegLength * (2 + doFill) > length)
        {
            meas.getSegment(0, length, dst, true);  // to short for us to mangle
        }
        else
        {
            int         n = SkScalarRound(SkScalarDiv(length, fSegLength));
            SkScalar    delta = length / n;
            SkScalar    distance = 0;

            if (meas.isClosed())
            {
                n -= 1;
                distance += delta/2;
            }
            meas.getPosTan(distance, &p, &v);
            Perterb(&p, v, SkScalarMul(rand.nextSScalar1(), scale));
            dst->moveTo(p);
            while (--n >= 0)
            {
                distance += delta;
                meas.getPosTan(distance, &p, &v);
                Perterb(&p, v, SkScalarMul(rand.nextSScalar1(), scale));
                dst->lineTo(p);
            }
            if (meas.isClosed())
                dst->close();
        }
    } while (meas.nextContour());
    return true;
}

SkFlattenable::Factory SkDiscretePathEffect::getFactory()
{
    return CreateProc;
}

SkFlattenable* SkDiscretePathEffect::CreateProc(SkFlattenableReadBuffer& buffer)
{
    return SkNEW_ARGS(SkDiscretePathEffect, (buffer));
}

void SkDiscretePathEffect::flatten(SkFlattenableWriteBuffer& buffer)
{
    buffer.writeScalar(fSegLength);
    buffer.writeScalar(fPerterb);
}

SkDiscretePathEffect::SkDiscretePathEffect(SkFlattenableReadBuffer& buffer)
{
    fSegLength = buffer.readScalar();
    fPerterb = buffer.readScalar();
}


