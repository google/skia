#ifndef SkBoundable_DEFINED
#define SkBoundable_DEFINED

#include "SkDrawable.h"
#include "SkRect.h"

class SkBoundable : public SkDrawable {
public:
	SkBoundable();
	virtual void clearBounder();
	virtual void enableBounder();
	virtual void getBounds(SkRect* );
	bool hasBounds() { return fBounds.fLeft != (S16)0x8000U; }
	void setBounds(SkRect16& bounds) { fBounds = bounds; }
protected:
	void clearBounds() { fBounds.fLeft = (S16) SkToU16(0x8000); }; // mark bounds as unset
	SkRect16 fBounds;
private:
	typedef SkDrawable INHERITED;
};

class SkBoundableAuto {
public:
	SkBoundableAuto(SkBoundable* boundable, SkAnimateMaker& maker);
	~SkBoundableAuto();
private:
	SkBoundable* fBoundable;
	SkAnimateMaker& fMaker;
	SkBoundableAuto& operator= (const SkBoundableAuto& );
};

#endif // SkBoundable_DEFINED

