#ifndef SkTagList_DEFINED
#define SkTagList_DEFINED

#include "SkTypes.h"

enum SkTagListEnum {
	kListeners_SkTagList,
	kViewLayout_SkTagList,
	kViewArtist_SkTagList,

	kSkTagListCount
};

struct SkTagList {
	SkTagList*	fNext;
	U16			fExtra16;
	U8			fExtra8;
	U8			fTag;

	SkTagList(U8CPU tag) : fTag(SkToU8(tag))
	{
		SkASSERT(tag < kSkTagListCount);
		fNext		= nil;
		fExtra16	= 0;
		fExtra8		= 0;
	}
	virtual ~SkTagList();

	static SkTagList*	Find(SkTagList* head, U8CPU tag);
	static void			DeleteTag(SkTagList** headptr, U8CPU tag);
	static void			DeleteAll(SkTagList* head);
};

#endif
