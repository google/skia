#include "SkDrawFull.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"

bool SkFull::draw(SkAnimateMaker& maker) {
	SkBoundableAuto boundable(this, maker);
	maker.fCanvas->drawPaint(*maker.fPaint);
	return false;
}

