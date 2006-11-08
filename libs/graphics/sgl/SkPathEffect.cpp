/* libs/graphics/sgl/SkPathEffect.cpp
**
** Copyright 2006, Google Inc.
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

#include "SkPathEffect.h"
#include "SkPath.h"
#include "SkBuffer.h"

SkFlattenable::Factory SkFlattenable::getFactory()
{
    return NULL;
}

void SkFlattenable::flatten(SkWBuffer&)
{
}

//////////////////////////////////////////////////////////////////////////////////

bool SkPathEffect::filterPath(SkPath*, const SkPath&, SkScalar*)
{
    return false;
}

static SkFlattenable* create_null_patheffect(SkRBuffer&)
{
    return SkNEW(SkPathEffect);
}

SkFlattenable::Factory SkPathEffect::getFactory()
{
    return create_null_patheffect;
}

//////////////////////////////////////////////////////////////////////////////////

SkPairPathEffect::SkPairPathEffect(SkPathEffect* pe0, SkPathEffect* pe1)
    : fPE0(pe0), fPE1(pe1)
{
    SkASSERT(pe0);
    SkASSERT(pe1);
    fPE0->ref();
    fPE1->ref();
}

SkPairPathEffect::~SkPairPathEffect()
{
    fPE0->unref();
    fPE1->unref();
}

/*
    Format: [oe0-factory][pe1-factory][pe0-size][pe0-data][pe1-data]
*/
void SkPairPathEffect::flatten(SkWBuffer& buffer)
{
    buffer.writePtr((void*)fPE0->getFactory());
    buffer.writePtr((void*)fPE1->getFactory());
    fPE0->flatten(buffer);
    fPE1->flatten(buffer);
}

SkPairPathEffect::SkPairPathEffect(SkRBuffer& buffer)
{
    Factory factory0 = (Factory)buffer.readPtr();
    Factory factory1 = (Factory)buffer.readPtr();
    
    fPE0 = (SkPathEffect*)factory0(buffer);
    fPE1 = (SkPathEffect*)factory1(buffer);
}

//////////////////////////////////////////////////////////////////////////////////

bool SkComposePathEffect::filterPath(SkPath* dst, const SkPath& src, SkScalar* width)
{
    SkPath          tmp;
    const SkPath*   ptr = &src;

    if (fPE1->filterPath(&tmp, src, width))
        ptr = &tmp;
    return fPE0->filterPath(dst, *ptr, width);
}

SkFlattenable* SkComposePathEffect::CreateProc(SkRBuffer& buffer)
{
    return SkNEW_ARGS(SkComposePathEffect, (buffer));
}

SkFlattenable::Factory SkComposePathEffect::getFactory()
{
    return SkComposePathEffect::CreateProc;
}

//////////////////////////////////////////////////////////////////////////////////

bool SkSumPathEffect::filterPath(SkPath* dst, const SkPath& src, SkScalar* width)
{
    // use bit-or so that we always call both, even if the first one succeeds
    return  fPE0->filterPath(dst, src, width) | fPE1->filterPath(dst, src, width);
}

SkFlattenable* SkSumPathEffect::CreateProc(SkRBuffer& buffer)
{
    return SkNEW_ARGS(SkSumPathEffect, (buffer));
}

SkFlattenable::Factory SkSumPathEffect::getFactory()
{
    return SkSumPathEffect::CreateProc;
}

/////////////////////////////////////////////////////////////////////////////////

#include "SkStroke.h"

SkStrokePathEffect::SkStrokePathEffect(const SkPaint& paint)
    : fWidth(paint.getStrokeWidth()), fMiter(paint.getStrokeMiter()),
      fStyle(SkToU8(paint.getStyle())), fJoin(SkToU8(paint.getStrokeJoin())), fCap(SkToU8(paint.getStrokeCap()))
{
}

SkStrokePathEffect::SkStrokePathEffect(SkScalar width, SkPaint::Style style, SkPaint::Join join, SkPaint::Cap cap, SkScalar miter)
    : fWidth(width), fMiter(miter), fStyle(SkToU8(style)), fJoin(SkToU8(join)), fCap(SkToU8(cap))
{
    if (miter < 0)  // signal they want the default
        fMiter = SK_DefaultMiterLimit;
}

bool SkStrokePathEffect::filterPath(SkPath* dst, const SkPath& src, SkScalar* width)
{
    if (fWidth < 0 || fStyle == SkPaint::kFill_Style)
        return false;

    if (fStyle == SkPaint::kStroke_Style && fWidth == 0)    // hairline
    {
        *width = 0;
        return true;
    }

    SkStroke    stroke;

    stroke.setWidth(fWidth);
    stroke.setMiterLimit(fMiter);
    stroke.setJoin((SkPaint::Join)fJoin);
    stroke.setCap((SkPaint::Cap)fCap);
    stroke.setDoFill(fStyle == SkPaint::kStrokeAndFill_Style);

    stroke.strokePath(src, dst);
    return true;
}

SkFlattenable::Factory SkStrokePathEffect::getFactory()
{
    return CreateProc;
}

SkFlattenable* SkStrokePathEffect::CreateProc(SkRBuffer& buffer)
{
    return SkNEW_ARGS(SkStrokePathEffect, (buffer));
}

void SkStrokePathEffect::flatten(SkWBuffer& buffer)
{
    this->INHERITED::flatten(buffer);

    buffer.writeScalar(fWidth);
    buffer.writeScalar(fMiter);
    buffer.write8(fStyle);
    buffer.write8(fJoin);
    buffer.write8(fCap);
    buffer.padToAlign4();
}

SkStrokePathEffect::SkStrokePathEffect(SkRBuffer& buffer)
    : SkPathEffect(buffer)
{
    fWidth = buffer.readScalar();
    fMiter = buffer.readScalar();
    fStyle = buffer.readU8();
    fJoin = buffer.readU8();
    fCap = buffer.readU8();
    buffer.skipToAlign4();
}


