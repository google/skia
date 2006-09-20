#include "SkDrawOval.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkOval::fInfo[] = {
	SK_MEMBER_INHERITED,
};

#endif

DEFINE_GET_MEMBER(SkOval);

bool SkOval::draw(SkAnimateMaker& maker) {
	SkBoundableAuto boundable(this, maker);
	maker.fCanvas->drawOval(fRect, *maker.fPaint);
	return false;
}

