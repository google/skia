#include "SkDrawText.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkPaint.h"

enum SkText_Properties {
	SK_PROPERTY(length)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkText::fInfo[] = {
	SK_MEMBER_PROPERTY(length, Int),
	SK_MEMBER(text, String),
	SK_MEMBER(x, Float),
	SK_MEMBER(y, Float)
};

#endif

DEFINE_GET_MEMBER(SkText);

SkText::SkText() : x(0), y(0) {
}

SkText::~SkText() {
}

bool SkText::draw(SkAnimateMaker& maker) {
	SkBoundableAuto boundable(this, maker);
	maker.fCanvas->drawText(text.c_str(), text.size(), x, y, *maker.fPaint);
	return false;
}

#ifdef SK_DUMP_ENABLED
void SkText::dump(SkAnimateMaker* maker) {
	INHERITED::dump(maker);
}
#endif

bool SkText::getProperty(int index, SkScriptValue* value) const {
	SkASSERT(index == SK_PROPERTY(length));
	value->fType = SkType_Int;
	value->fOperand.fS32 = (S32) text.size();
	return true;
}

