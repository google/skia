/* libs/graphics/effects/Sk2DPathEffect.cpp
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

#include "Sk2DPathEffect.h"
#include "SkBlitter.h"
#include "SkPath.h"
#include "SkScan.h"

class Sk2DPathEffectBlitter : public SkBlitter {
public:
    Sk2DPathEffectBlitter(Sk2DPathEffect* pe, SkPath* dst)
        : fPE(pe), fDst(dst)
    {}
    virtual void blitH(int x, int y, int count)
    {
        fPE->nextSpan(x, y, count, fDst);
    }
private:
    Sk2DPathEffect* fPE;
    SkPath*         fDst;
};

////////////////////////////////////////////////////////////////////////////////////

Sk2DPathEffect::Sk2DPathEffect(const SkMatrix& mat) : fMatrix(mat)
{
    mat.invert(&fInverse);
}

bool Sk2DPathEffect::filterPath(SkPath* dst, const SkPath& src, SkScalar* width)
{
    Sk2DPathEffectBlitter   blitter(this, dst);
    SkPath                  tmp;
    SkIRect                 ir;

    src.transform(fInverse, &tmp);
    tmp.getBounds().round(&ir);
    if (!ir.isEmpty()) {
        // need to pass a clip to fillpath, required for inverse filltypes,
        // even though those do not make sense for this patheffect
        SkRegion clip(ir);
        
        this->begin(ir, dst);
        SkScan::FillPath(tmp, clip, &blitter);
        this->end(dst);
    }
    return true;
}

void Sk2DPathEffect::nextSpan(int x, int y, int count, SkPath* path)
{
    const SkMatrix& mat = this->getMatrix();
    SkPoint src, dst;

    src.set(SkIntToScalar(x) + SK_ScalarHalf, SkIntToScalar(y) + SK_ScalarHalf);
    do {
        mat.mapPoints(&dst, &src, 1);
        this->next(dst, x++, y, path);
        src.fX += SK_Scalar1;
    } while (--count > 0);
}

void Sk2DPathEffect::begin(const SkIRect& uvBounds, SkPath* dst) {}
void Sk2DPathEffect::next(const SkPoint& loc, int u, int v, SkPath* dst) {}
void Sk2DPathEffect::end(SkPath* dst) {}

////////////////////////////////////////////////////////////////////////////////

void Sk2DPathEffect::flatten(SkFlattenableWriteBuffer& buffer)
{
    char storage[SkMatrix::kMaxFlattenSize];
    uint32_t size = fMatrix.flatten(storage);
    buffer.write32(size);
    buffer.write(storage, size);
}

Sk2DPathEffect::Sk2DPathEffect(SkFlattenableReadBuffer& buffer)
{
    char storage[SkMatrix::kMaxFlattenSize];
    uint32_t size = buffer.readS32();
    SkASSERT(size <= sizeof(storage));
    buffer.read(storage, size);
    fMatrix.unflatten(storage);
    fMatrix.invert(&fInverse);
}

SkFlattenable::Factory Sk2DPathEffect::getFactory()
{
    return CreateProc;
}

SkFlattenable* Sk2DPathEffect::CreateProc(SkFlattenableReadBuffer& buffer)
{
    return SkNEW_ARGS(Sk2DPathEffect, (buffer));
}



