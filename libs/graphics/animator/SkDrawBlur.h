#ifndef SkDrawBlur_DEFINED
#define SkDrawBlur_DEFINED

#include "SkPaintParts.h"
#include "SkBlurMaskFilter.h"

class SkDrawBlur : public SkDrawMaskFilter {
	DECLARE_DRAW_MEMBER_INFO(Blur);
	SkDrawBlur();
	virtual SkMaskFilter* getMaskFilter();
protected:
	SkScalar radius;
	int /*SkBlurMaskFilter::BlurStyle*/ blurStyle;
};

#endif // SkDrawBlur_DEFINED

