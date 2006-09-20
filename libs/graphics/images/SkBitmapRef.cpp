#include "SkBitmapRefPriv.h"
#include "SkTemplates.h"
#include "SkThread.h"
#include "SkString.h"
#include "SkGlobals.h"
#include "SkThread.h"

SkGlobals::Rec* SkBitmapRef_Globals::Create()
{
	SkBitmapRef_Globals* rec = SkNEW(SkBitmapRef_Globals);
	rec->fCache = nil;
	return rec;
}

///////////////////////////////////////////////////////////////////////////////////////////

SkBitmapRef::SkBitmapRef(Rec* rec) : fRec(rec)
{
	SkASSERT(rec);
	rec->fRefCnt += 1;
}

SkBitmapRef::SkBitmapRef(const SkBitmap& src, bool transferOwnsPixels)
{
    fRec = SkNEW_ARGS(SkBitmapRef::Rec, (src));
    fRec->fIsCache = false;
    if (transferOwnsPixels)
    {
        fRec->fBM.setOwnsPixels(src.getOwnsPixels());
        ((SkBitmap*)&src)->setOwnsPixels(false);
    }
}
  
SkBitmapRef::~SkBitmapRef()
{
    if (fRec->fIsCache)
    {
        SkBitmapRef_Globals& globals = *(SkBitmapRef_Globals*)SkGlobals::Find(kBitmapRef_GlobalsTag,
                                                                              SkBitmapRef_Globals::Create);
        SkAutoMutexAcquire	ac(globals.fMutex);

        SkASSERT(fRec->fRefCnt > 0);
        fRec->fRefCnt -= 1;
    }
    else
    {
        SkDEBUGF(("~SkBitmapRef[%d %d]\n", fRec->fBM.width(), fRec->fBM.height()));
        SkDELETE(fRec);
    }
}

const SkBitmap& SkBitmapRef::bitmap()
{
	return fRec->fBM;
}

///////////////////////////////////////////////////////////////////////////////////////////

SkBitmapRef* SkBitmapRef::create(const SkBitmap& src, bool transferOwnsPixels)
{
    return SkNEW_ARGS(SkBitmapRef, (src, transferOwnsPixels));
}

void SkBitmapRef::PurgeCacheAll()
{
	SkBitmapRef_Globals* globals = (SkBitmapRef_Globals*)SkGlobals::Find(kBitmapRef_GlobalsTag, nil);
	if (globals == nil)
		return;

	SkAutoMutexAcquire	ac(globals->fMutex);
	SkBitmapRef::Rec*	rec = globals->fCache;
    SkDEBUGCODE(int     count = 0;)

	while (rec)
	{
        SkDEBUGCODE(count += 1;)
		SkASSERT(rec->fRefCnt == 0);
		Rec* next = rec->fNext;
		SkDELETE(rec);
		rec = next;
	}
	globals->fCache = nil;
    
    SkDEBUGF(("PurgeCacheAll freed %d bitmaps\n", count));
}

bool SkBitmapRef::PurgeCacheOne()
{
	SkBitmapRef_Globals* globals = (SkBitmapRef_Globals*)SkGlobals::Find(kBitmapRef_GlobalsTag, nil);
	if (globals == nil)
		return false;

	SkAutoMutexAcquire	ac(globals->fMutex);
	SkBitmapRef::Rec*	rec = globals->fCache;
	SkBitmapRef::Rec*	prev = nil;
    SkDEBUGCODE(int     count = 0;)

	while (rec)
	{
        SkDEBUGCODE(count += 1;)

		SkBitmapRef::Rec* next = rec->fNext;
		if (rec->fRefCnt == 0)
		{
			if (prev)
				prev = next;
			else
				globals->fCache = next;

            SkDEBUGF(("PurgeCacheOne for bitmap[%d %d]\n", rec->fBM.width(), rec->fBM.height()));
			SkDELETE(rec);
			return true;
		}
		prev = rec;
		rec = next;
	}

    SkDEBUGF(("PurgeCacheOne: nothing to purge from %d bitmaps\n", count));
	return false;
}

