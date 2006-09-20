#ifndef SkBitmapRefPriv_DEFINED
#define SkBitmapRefPriv_DEFINED

#include "SkBitmapRef.h"
#include "SkGlobals.h"
#include "SkThread.h"
#include "SkBitmap.h"
#include "SkString.h"

#define kBitmapRef_GlobalsTag	SkSetFourByteTag('s', 'k', 'b', 'r')

class SkBitmapRef_Globals : public SkGlobals::Rec {
public:
	SkMutex				fMutex;
	SkBitmapRef::Rec*	fCache;
    
    static Rec* Create();
};

struct SkBitmapRef::Rec {
    Rec(bool isCache): fIsCache(isCache) {}
    Rec(const SkBitmap& src): fBM(src) {}
    
    Rec*		fNext;
    int32_t		fRefCnt;
    SkString	fPath;
    SkBitmap	fBM;
    bool        fIsCache;
};

#endif
