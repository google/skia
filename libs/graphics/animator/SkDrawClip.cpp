#include "SkDrawClip.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkDrawRectangle.h"
#include "SkDrawPath.h"


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawClip::fInfo[] = {
	SK_MEMBER(path, Path),
	SK_MEMBER(rect, Rect)
};

#endif

DEFINE_GET_MEMBER(SkDrawClip);

SkDrawClip::SkDrawClip() : rect(nil), path(nil) {
}

bool SkDrawClip::draw(SkAnimateMaker& maker ) {
	if (rect != nil)
		maker.fCanvas->clipRect(rect->fRect);
	else {
		SkASSERT(path != nil);
		maker.fCanvas->clipPath(path->fPath);
	}
	return false;
}

