#ifndef SkDrawDiscrete_DEFINED
#define SkDrawDiscrete_DEFINED

#include "SkPaintParts.h"

class SkDiscrete : public SkDrawPathEffect {
	DECLARE_MEMBER_INFO(Discrete);
	SkDiscrete();
	virtual SkPathEffect* getPathEffect();
private:
	SkScalar deviation;
	SkScalar segLength;
};

#endif //SkDrawDiscrete_DEFINED
