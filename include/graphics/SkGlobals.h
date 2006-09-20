#ifndef SkGlobals_DEFINED
#define SkGlobals_DEFINED

#include "SkThread.h"

class SkGlobals {
public:
	class Rec {
	public:
		virtual	~Rec();
	private:
		Rec*	fNext;
		U32		fTag;

		friend class SkGlobals;
	};

	/**	Look for a matching Rec for the specified tag. If one is found, return it.
		If one is not found, if create_proc is nil, return nil, else
		call the proc, and if it returns a Rec, add it to the global list
		and return it.

		create_proc can NOT call back into SkGlobals::Find (it would deadlock)
	*/
	static Rec*	Find(U32 tag, Rec* (*create_proc)());
	/**	Helper for Find, when you want to assert that the Rec is already in the list
	*/
	static Rec* Get(U32 tag)
	{
		Rec* rec = SkGlobals::Find(tag, nil);
		SkASSERT(rec);
		return rec;
	}

	// used by porting layer
	struct BootStrap {
		SkMutex	fMutex;
		Rec*	fHead;
	};

private:
	static void	Init();
	static void Term();
	friend class SkGraphics;

	//	This last function is implemented in the porting layer
	static BootStrap& GetBootStrap();
};

#endif

