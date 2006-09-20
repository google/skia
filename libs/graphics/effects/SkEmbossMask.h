#ifndef SkEmbossMask_DEFINED
#define SkEmbossMask_DEFINED

#include "SkEmbossMaskFilter.h"

class SkEmbossMask {
public:
	static void Emboss(SkMask* mask, const SkEmbossMaskFilter::Light&);
};

#endif

