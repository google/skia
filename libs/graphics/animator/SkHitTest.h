#ifndef SkHitTest_DEFINED
#define SkHitTest_DEFINED

#include "SkDrawable.h"
#include "SkTypedArray.h"

class SkHitTest : public SkDrawable {
	DECLARE_MEMBER_INFO(HitTest);
	SkHitTest();
	virtual bool draw(SkAnimateMaker& );
	virtual bool enable(SkAnimateMaker& );
	virtual bool hasEnable() const;
	virtual const SkMemberInfo* preferredChild(SkDisplayTypes type);
private:
	SkTDDisplayableArray bullets;
	SkTDIntArray hits;
	SkTDDisplayableArray targets;
	SkBool value;
};

#endif // SkHitTest_DEFINED
