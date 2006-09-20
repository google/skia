#include "SkDisplayScreenplay.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayScreenplay::fInfo[] = {
	SK_MEMBER(time, MSec)
};

#endif

DEFINE_GET_MEMBER(SkDisplayScreenplay);


