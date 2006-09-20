#ifndef SkDrawFull_DEFINED
#define SkDrawFull_DEFINED

#include "SkBoundable.h"

class SkFull : public SkBoundable {
	DECLARE_EMPTY_MEMBER_INFO(Full);
	virtual bool draw(SkAnimateMaker& );
private:
	typedef SkBoundable INHERITED;
};

#endif // SkDrawFull_DEFINED
