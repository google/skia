#ifndef SkDisplayList_DEFINED
#define SkDisplayList_DEFINED

#include "SkOperand.h"
#include "SkIntArray.h"
#include "SkBounder.h"
#include "SkRect.h"

class SkAnimateMaker;
class SkActive;
class SkApply;
class SkDrawable;
class SkGroup;

class SkDisplayList : public SkBounder {
public:
	SkDisplayList();
	virtual ~SkDisplayList();
	void append(SkActive* );
	void clear() { fDrawList.reset(); }
	int count() { return fDrawList.count(); }
	bool draw(SkAnimateMaker& , SkMSec time);
#ifdef SK_DUMP_ENABLED
	void dump(SkAnimateMaker* maker);
	void dumpInner(SkAnimateMaker* maker);
	static int fIndent;
	static int fDumpIndex;
#endif
	int findGroup(SkDrawable* match, SkTDDrawableArray** list, 
		SkGroup** parent, SkGroup** found, SkTDDrawableArray** grandList); 
	SkDrawable* get(int index) { return fDrawList[index]; }
	SkMSec getTime() { return fInTime; }
	SkTDDrawableArray* getDrawList() { return &fDrawList; }
	void hardReset();
	virtual bool onIRect(const SkRect16& r);
	void reset();
	void remove(SkActive* );
#ifdef SK_DEBUG
	void validate();
#else
	void validate() {}
#endif
	static int SearchForMatch(SkDrawable* match, SkTDDrawableArray** list,
		SkGroup** parent, SkGroup** found, SkTDDrawableArray**grandList);
	static bool SearchGroupForMatch(SkDrawable* draw, SkDrawable* match, 
		SkTDDrawableArray** list, SkGroup** parent, SkGroup** found, SkTDDrawableArray** grandList, 
		int &index);
public:
	SkRect16 fBounds;
	SkRect16 fInvalBounds;
	bool fDrawBounds;
	bool fHasUnion;
	bool fUnionBounds;
private:
	SkTDDrawableArray fDrawList;
	SkTDActiveArray fActiveList;
	SkMSec fInTime;
	friend class SkEvents;
};

#endif // SkDisplayList_DEFINED

