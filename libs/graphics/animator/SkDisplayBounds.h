#ifndef SkDisplayBounds_DEFINED
#define SkDisplayBounds_DEFINED

#include "SkDrawRectangle.h"

class SkDisplayBounds : public SkDrawRect {
	DECLARE_DISPLAY_MEMBER_INFO(Bounds);
	SkDisplayBounds();
	virtual bool draw(SkAnimateMaker& );
private:
	SkBool inval;
	typedef SkDrawRect INHERITED;
};

#endif // SkDisplayBounds_DEFINED

