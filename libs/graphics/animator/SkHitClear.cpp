#include "SkHitClear.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkHitClear::fInfo[] = {
	SK_MEMBER_ARRAY(targets, Displayable)
};

#endif

DEFINE_GET_MEMBER(SkHitClear);

bool SkHitClear::enable(SkAnimateMaker& maker) {
	for (int tIndex = 0; tIndex < targets.count(); tIndex++) {
		SkDisplayable* target = targets[tIndex];
		target->clearBounder();
	}
	return true;
}

bool SkHitClear::hasEnable() const {
	return true;
}

