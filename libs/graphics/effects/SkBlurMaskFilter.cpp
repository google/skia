/* libs/graphics/effects/SkBlurMaskFilter.cpp
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

#include "SkBlurMaskFilter.h"
#include "SkBlurMask.h"
#include "SkBuffer.h"

class SkBlurMaskFilterImpl : public SkMaskFilter {
public:
    SkBlurMaskFilterImpl(SkScalar radius, SkBlurMaskFilter::BlurStyle style);

    // overrides from SkMaskFilter
    virtual SkMask::Format getFormat();
    virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix& matrix, SkPoint16* margin);

    // overrides from SkFlattenable
    // This method is not exported to java.
    virtual Factory getFactory();
    // This method is not exported to java.
    virtual void flatten(SkWBuffer&);

private:
    SkScalar                    fRadius;
    SkBlurMaskFilter::BlurStyle fBlurStyle;

    static SkFlattenable* CreateProc(SkRBuffer&);
    SkBlurMaskFilterImpl(SkRBuffer&);
    
    typedef SkMaskFilter INHERITED;
};

SkMaskFilter* SkBlurMaskFilter::Create(SkScalar radius, SkBlurMaskFilter::BlurStyle style)
{
    if (radius <= 0 || (unsigned)style >= SkBlurMaskFilter::kBlurStyleCount)
        return NULL;

    return SkNEW_ARGS(SkBlurMaskFilterImpl, (radius, style));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkScalar radius, SkBlurMaskFilter::BlurStyle style)
    : fRadius(radius), fBlurStyle(style)
{
#if 0
    fGamma = NULL;
    if (gammaScale)
    {
        fGamma = new U8[256];
        if (gammaScale > 0)
            SkBlurMask::BuildSqrGamma(fGamma, gammaScale);
        else
            SkBlurMask::BuildSqrtGamma(fGamma, -gammaScale);
    }
#endif
    SkASSERT(radius >= 0);
    SkASSERT((unsigned)style < SkBlurMaskFilter::kBlurStyleCount);
}

SkMask::Format SkBlurMaskFilterImpl::getFormat()
{
    return SkMask::kA8_Format;
}

bool SkBlurMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src, const SkMatrix& matrix, SkPoint16* margin)
{
    SkScalar radius = matrix.mapRadius(fRadius);

    if (margin)
        margin->set(SkScalarCeil(radius), SkScalarCeil(radius));

    return SkBlurMask::Blur(dst, src, radius, (SkBlurMask::Style)fBlurStyle);
}

SkFlattenable* SkBlurMaskFilterImpl::CreateProc(SkRBuffer& buffer)
{
    return SkNEW_ARGS(SkBlurMaskFilterImpl, (buffer));
}

SkFlattenable::Factory SkBlurMaskFilterImpl::getFactory()
{
    return CreateProc;
}

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkRBuffer& buffer) : SkMaskFilter(buffer)
{
    fRadius = buffer.readScalar();
    fBlurStyle = (SkBlurMaskFilter::BlurStyle)buffer.readS32();
    SkASSERT(fRadius >= 0);
    SkASSERT((unsigned)fBlurStyle < SkBlurMaskFilter::kBlurStyleCount);
}

void SkBlurMaskFilterImpl::flatten(SkWBuffer& buffer)
{
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fRadius);
    buffer.write32(fBlurStyle);
}

