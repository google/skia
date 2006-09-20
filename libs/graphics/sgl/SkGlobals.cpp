#include "SkGlobals.h"
#include "SkThread.h"

SkGlobals::Rec::~Rec()
{
}

SkGlobals::Rec* SkGlobals::Find(U32 tag, Rec* (*create_proc)())
{
	SkGlobals::BootStrap&	bootstrap = SkGlobals::GetBootStrap();

	Rec* rec = bootstrap.fHead;
	while (rec)
	{
		if (rec->fTag == tag)
			return rec;
		rec = rec->fNext;
	}

	if (create_proc == nil)	// no create proc, just return not found
		return nil;

	// if we get here, we may need to create one. First grab the mutex, and
	// search again, creating one if its not found the 2nd time.

	bootstrap.fMutex.acquire();

	// search again, now that we have the mutex. Odds are it won't be there, but we check again
	// just in case it was added by another thread before we grabbed the mutex

	Rec*& head = bootstrap.fHead;
	rec = head;
	while (rec)
	{
		if (rec->fTag == tag)
			break;
		rec = rec->fNext;
	}

	if (rec == nil && (rec = create_proc()) != nil)
	{
		rec->fTag = tag;
		rec->fNext = head;
		bootstrap.fHead = rec;
	}

	bootstrap.fMutex.release();
	return rec;
}

void SkGlobals::Init()
{
}

void SkGlobals::Term()
{
	SkGlobals::BootStrap&	bootstrap = SkGlobals::GetBootStrap();

	bootstrap.fMutex.acquire();

	Rec*&	head = bootstrap.fHead;
	Rec*	rec = head;

	while (rec)
	{
		Rec* next = rec->fNext;
		SkDELETE(rec);
		rec = next;
	}

	bootstrap.fHead = nil;
	bootstrap.fMutex.release();
}


