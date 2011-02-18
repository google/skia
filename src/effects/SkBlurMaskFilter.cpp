/* libs/graphics/effects/SkBlurMaskFilter.cpp
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

#include "SkBlurMaskFilter.h"
#include "SkBlurMask.h"
#include "SkBuffer.h"
#include "SkMaskFilter.h"

class SkBlurMaskFilterImpl : public SkMaskFilter {
public:
    SkBlurMaskFilterImpl(SkScalar radius, SkBlurMaskFilter::BlurStyle style, uint32_t flags);

    // overrides from SkMaskFilter
    virtual SkMask::Format getFormat();
    virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix& matrix, SkIPoint* margin);

    // overrides from SkFlattenable
    // This method is not exported to java.
    virtual Factory getFactory();
    // This method is not exported to java.
    virtual void flatten(SkFlattenableWriteBuffer&);

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer&);

private:
    SkScalar                    fRadius;
    SkBlurMaskFilter::BlurStyle fBlurStyle;
    uint32_t                    fBlurFlags;

    SkBlurMaskFilterImpl(SkFlattenableReadBuffer&);
    
    typedef SkMaskFilter INHERITED;
};

SkMaskFilter* SkBlurMaskFilter::Create(SkScalar radius, SkBlurMaskFilter::BlurStyle style,
                                       uint32_t flags)
{
    if (radius <= 0 || (unsigned)style >= SkBlurMaskFilter::kBlurStyleCount 
        || flags > SkBlurMaskFilter::kAll_BlurFlag)
        return NULL;

    return SkNEW_ARGS(SkBlurMaskFilterImpl, (radius, style, flags));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkScalar radius, SkBlurMaskFilter::BlurStyle style,
                                           uint32_t flags)
    : fRadius(radius), fBlurStyle(style), fBlurFlags(flags)
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
    SkASSERT(flags <= SkBlurMaskFilter::kAll_BlurFlag);
}

SkMask::Format SkBlurMaskFilterImpl::getFormat()
{
    return SkMask::kA8_Format;
}

bool SkBlurMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src, const SkMatrix& matrix, SkIPoint* margin)
{
    SkScalar radius;
    if (fBlurFlags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag)
        radius = fRadius;
    else
        radius = matrix.mapRadius(fRadius);

    // To avoid unseemly allocation requests (esp. for finite platforms like
    // handset) we limit the radius so something manageable. (as opposed to
    // a request like 10,000)
    static const SkScalar MAX_RADIUS = SkIntToScalar(128);
    radius = SkMinScalar(radius, MAX_RADIUS);
    SkBlurMask::Quality blurQuality = (fBlurFlags & SkBlurMaskFilter::kHighQuality_BlurFlag) ? 
        SkBlurMask::kHigh_Quality : SkBlurMask::kLow_Quality;

    if (SkBlurMask::Blur(dst, src, radius, (SkBlurMask::Style)fBlurStyle, blurQuality))
    {
        if (margin) {
            // we need to integralize radius for our margin, so take the ceil
            // just to be safe.
            margin->set(SkScalarCeil(radius), SkScalarCeil(radius));
        }
        return true;
    }
    return false;
}

SkFlattenable* SkBlurMaskFilterImpl::CreateProc(SkFlattenableReadBuffer& buffer)
{
    return SkNEW_ARGS(SkBlurMaskFilterImpl, (buffer));
}

SkFlattenable::Factory SkBlurMaskFilterImpl::getFactory()
{
    return CreateProc;
}

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkFlattenableReadBuffer& buffer) : SkMaskFilter(buffer)
{
    fRadius = buffer.readScalar();
    fBlurStyle = (SkBlurMaskFilter::BlurStyle)buffer.readS32();
    fBlurFlags = buffer.readU32() & SkBlurMaskFilter::kAll_BlurFlag;
    SkASSERT(fRadius >= 0);
    SkASSERT((unsigned)fBlurStyle < SkBlurMaskFilter::kBlurStyleCount);
}

void SkBlurMaskFilterImpl::flatten(SkFlattenableWriteBuffer& buffer)
{
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fRadius);
    buffer.write32(fBlurStyle);
    buffer.write32(fBlurFlags);
}

///////////////////////////////////////////////////////////////////////////////

static SkFlattenable::Registrar gReg("SkBlurMaskFilter",
                                     SkBlurMaskFilterImpl::CreateProc);

