#ifndef SkDrawDash_DEFINED
#define SkDrawDash_DEFINED

#include "SkPaintParts.h"
#include "SkIntArray.h"

class SkDash : public SkDrawPathEffect {
	DECLARE_MEMBER_INFO(Dash);
	SkDash();
	virtual ~SkDash();
	virtual SkPathEffect* getPathEffect();
private:
	SkTDScalarArray intervals;
	SkScalar phase;
};

#endif // SkDrawDash_DEFINED

