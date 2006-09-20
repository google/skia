#ifndef SkDrawTo_DEFINED
#define SkDrawTo_DEFINED

#include "SkDrawGroup.h"
#include "SkMemberInfo.h"

class SkDrawBitmap;

class SkDrawTo : public SkGroup {
	DECLARE_MEMBER_INFO(DrawTo);
	SkDrawTo();
//	virtual ~SkDrawTo();
	virtual bool draw(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
#endif
protected:
	SkBool drawOnce;
	SkDrawBitmap* use;
private:
	typedef SkGroup INHERITED;
	SkBool fDrawnOnce;
};

#endif // SkDrawTo_DEFINED
