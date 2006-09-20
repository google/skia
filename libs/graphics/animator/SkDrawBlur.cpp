#include "SkDrawBlur.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawBlur::fInfo[] = {
	SK_MEMBER(blurStyle, MaskFilterBlurStyle),
	SK_MEMBER(radius, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawBlur);

SkDrawBlur::SkDrawBlur() : radius(-1), 
	blurStyle(SkBlurMaskFilter::kNormal_BlurStyle) {
}

SkMaskFilter* SkDrawBlur::getMaskFilter() {
	if (radius < 0)
		return nil;
	return SkBlurMaskFilter::Create(radius, (SkBlurMaskFilter::BlurStyle) blurStyle);
}

