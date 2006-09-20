#ifndef SkDrawOval_DEFINED
#define SkDrawOval_DEFINED

#include "SkDrawRectangle.h"

class SkOval : public SkDrawRect {
	DECLARE_MEMBER_INFO(Oval);
	virtual bool draw(SkAnimateMaker& );
private:
	typedef SkDrawRect INHERITED;
};

#endif // SkDrawOval_DEFINED

