#ifndef SkBlurMask_DEFINED
#define SkBlurMask_DEFINED

#include "SkShader.h"

class SkBlurMask {
public:
	enum Style {
		kNormal_Style,	//!< fuzzy inside and outside
		kSolid_Style,	//!< solid inside, fuzzy outside
		kOuter_Style,	//!< nothing inside, fuzzy outside
		kInner_Style,	//!< fuzzy inside, nothing outside

		kStyleCount
	};

	static bool Blur(SkMask* dst, const SkMask& src, SkScalar radius, Style);
};

#endif



