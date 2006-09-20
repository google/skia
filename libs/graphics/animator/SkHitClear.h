#ifndef SkHitClear_DEFINED
#define SkHitClear_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"
#include "SkTypedArray.h"

class SkHitClear : public SkDisplayable {
	DECLARE_MEMBER_INFO(HitClear);
	virtual bool enable(SkAnimateMaker& );
	virtual bool hasEnable() const;
private:
	SkTDDisplayableArray targets;
};

#endif // SkHitClear_DEFINED
