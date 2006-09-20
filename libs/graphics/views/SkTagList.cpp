#include "SkTagList.h"

SkTagList::~SkTagList()
{
}

SkTagList* SkTagList::Find(SkTagList* rec, U8CPU tag)
{
	SkASSERT(tag < kSkTagListCount);

	while (rec != nil)
	{
		if (rec->fTag == tag)
			break;
		rec = rec->fNext;
	}
	return rec;
}

void SkTagList::DeleteTag(SkTagList** head, U8CPU tag)
{
	SkASSERT(tag < kSkTagListCount);

	SkTagList* rec = *head;
	SkTagList* prev = nil;

	while (rec != nil)
	{
		SkTagList* next = rec->fNext;

		if (rec->fTag == tag)
		{
			if (prev)
				prev->fNext = next;
			else
				*head = next;
			delete rec;
			break;
		}
		prev = rec;
		rec = next;
	}
}

void SkTagList::DeleteAll(SkTagList* rec)
{
	while (rec)
	{
		SkTagList* next = rec->fNext;
		delete rec;
		rec = next;
	}
}

