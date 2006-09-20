#include "SkDrawLine.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkPaint.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkLine::fInfo[] = {
	SK_MEMBER(x1, Float),
	SK_MEMBER(x2, Float),
	SK_MEMBER(y1, Float),
	SK_MEMBER(y2, Float)
};

#endif

DEFINE_GET_MEMBER(SkLine);

SkLine::SkLine() : x1(0), x2(0), y1(0), y2(0) { 
}

bool SkLine::draw(SkAnimateMaker& maker) {
	SkPoint start = {x1, y1};
	SkPoint stop = {x2, y2};
	SkBoundableAuto boundable(this, maker);
	maker.fCanvas->drawLine(start, stop, *maker.fPaint);
	return false;
}
