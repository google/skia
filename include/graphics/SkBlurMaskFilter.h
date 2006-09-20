#ifndef SkBlurMaskFilter_DEFINED
#define SkBlurMaskFilter_DEFINED

#include "SkMaskFilter.h"

class SkBlurMaskFilter : public SkMaskFilter {
public:
	enum BlurStyle {
		kNormal_BlurStyle,	//!< fuzzy inside and outside
		kSolid_BlurStyle,	//!< solid inside, fuzzy outside
		kOuter_BlurStyle,	//!< nothing inside, fuzzy outside
		kInner_BlurStyle,	//!< fuzzy inside, nothing outside

		kBlurStyleCount
	};

    /** Create a blur maskfilter.
        @param radius   The radius to extend the blur from the original mask. Must be > 0.
        @param style    The BlurStyle to use
        @return The new blur maskfilter
    */
    static SkMaskFilter* Create(SkScalar radius, BlurStyle style);

    /** Create an emboss maskfilter
        @param direction    array of 3 scalars [x, y, z] specifying the direction of the light source
        @param ambient      0...1 amount of ambient light
        @param specular     coefficient for specular highlights (e.g. 8)
        @param blurRadius   amount to blur before applying lighting (e.g. 3)
        @return the emboss maskfilter
    */
    static SkMaskFilter* CreateEmboss(  const SkScalar direction[3],
                                        SkScalar ambient, SkScalar specular,
                                        SkScalar blurRadius);
};

#endif

