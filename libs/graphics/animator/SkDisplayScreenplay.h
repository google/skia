#ifndef SkDisplayScreenplay_DEFINED
#define SkDisplayScreenplay_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"

class SkDisplayScreenplay : public SkDisplayable {
	DECLARE_DISPLAY_MEMBER_INFO(Screenplay);
	SkMSec time;
};

#endif // SkDisplayScreenplay_DEFINED
