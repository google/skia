#include "SkEmbossMaskFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkBlurMask.h"
#include "SkEmbossMask.h"
#include "SkBuffer.h"

SkMaskFilter* SkBlurMaskFilter::CreateEmboss(const SkScalar direction[3],
                                             SkScalar ambient, SkScalar specular,
                                             SkScalar blurRadius)
{
    if (direction == NULL)
        return NULL;

    // ambient should be 0...1 as a scalar
    int am = SkScalarToFixed(ambient) >> 8;
    if (am < 0) am = 0;
    else if (am > 0xFF) am = 0xFF;

    // specular should be 0..15.99 as a scalar
    int sp = SkScalarToFixed(specular) >> 12;
    if (sp < 0) sp = 0;
    else if (sp > 0xFF) sp = 0xFF;

    SkEmbossMaskFilter::Light   light;
    
    memcpy(light.fDirection, direction, sizeof(light.fDirection));
    light.fAmbient = SkToU8(am);
    light.fSpecular = SkToU8(sp);
    
    return SkNEW_ARGS(SkEmbossMaskFilter, (light, blurRadius));
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void normalize(SkScalar v[3])
{
	SkScalar mag = SkScalarSquare(v[0]) + SkScalarSquare(v[1]) + SkScalarSquare(v[2]);
	mag = SkScalarSqrt(mag);

	for (int i = 0; i < 3; i++)
		v[i] = SkScalarDiv(v[i], mag);
}

SkEmbossMaskFilter::SkEmbossMaskFilter(const Light& light, SkScalar blurRadius)
		: fLight(light), fBlurRadius(blurRadius)
{
	normalize(fLight.fDirection);
}

SkMask::Format SkEmbossMaskFilter::getFormat()
{
	return SkMask::k3D_Format;
}

bool SkEmbossMaskFilter::filterMask(SkMask* dst, const SkMask& src, const SkMatrix& matrix, SkPoint16* margin)
{
	SkScalar radius = matrix.mapRadius(fBlurRadius);

	SkBlurMask::Blur(dst, src, radius, SkBlurMask::kInner_Style);

	dst->fFormat = SkMask::k3D_Format;
	if (src.fImage == NULL)
		return true;

	// create a larger buffer for the other two channels (should force fBlur to do this for us)

	{
		U8*		alphaPlane = dst->fImage;
		size_t	planeSize = dst->computeImageSize();

		dst->fImage = SkMask::AllocImage(planeSize * 3);
		memcpy(dst->fImage, alphaPlane, planeSize);
		SkMask::FreeImage(alphaPlane);
	}

	// run the light direction through the matrix...
	Light	light = fLight;
	matrix.mapVectors((SkVector*)light.fDirection, (SkVector*)fLight.fDirection, 1);
	// now restore the length of the XY component
	((SkVector*)light.fDirection)->setLength(light.fDirection[0], light.fDirection[1],
											SkPoint::Length(fLight.fDirection[0], fLight.fDirection[1]));

	SkEmbossMask::Emboss(dst, light);

	// restore original alpha
	memcpy(dst->fImage, src.fImage, src.computeImageSize());

	return true;
}

SkFlattenable* SkEmbossMaskFilter::CreateProc(SkRBuffer& buffer)
{
    return SkNEW_ARGS(SkEmbossMaskFilter, (buffer));
}

SkFlattenable::Factory SkEmbossMaskFilter::getFactory()
{
	return CreateProc;
}

SkEmbossMaskFilter::SkEmbossMaskFilter(SkRBuffer& buffer) : SkMaskFilter(buffer)
{
    buffer.read(&fLight, sizeof(fLight));
    SkASSERT(fLight.fPad == 0);	// for the font-cache lookup to be clean
    fBlurRadius = buffer.readScalar();
}

void SkEmbossMaskFilter::flatten(SkWBuffer& buffer)
{
    this->INHERITED::flatten(buffer);

    fLight.fPad = 0;	// for the font-cache lookup to be clean
    buffer.write(&fLight, sizeof(fLight));
    buffer.writeScalar(fBlurRadius);
}

