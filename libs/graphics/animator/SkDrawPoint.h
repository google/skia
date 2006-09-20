#ifndef SkDrawPoint_DEFINED
#define SkDrawPoint_DEFINED

#include "SkBoundable.h"
#include "SkMemberInfo.h"
#include "SkPoint.h"

struct Sk_Point {
	DECLARE_NO_VIRTUALS_MEMBER_INFO(_Point);
	Sk_Point();
private:
	SkPoint fPoint;
};

class SkDrawPoint : public SkDisplayable {
	DECLARE_MEMBER_INFO(DrawPoint);
	SkDrawPoint();
	virtual void getBounds(SkRect*  );
private:
	SkPoint fPoint;
	typedef SkDisplayable INHERITED;
};

#endif // SkDrawPoint_DEFINED
