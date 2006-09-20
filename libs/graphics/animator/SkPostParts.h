#ifndef SkPostParts_DEFINED
#define SkPostParts_DEFINED

#include "SkDisplayInput.h"

class SkPost;

class SkData: public SkInput {
	DECLARE_MEMBER_INFO(Data);
	SkData();
	bool add();
	virtual void dirty();
	virtual SkDisplayable* getParent() const;
	virtual void onEndElement(SkAnimateMaker& );
	virtual bool setParent(SkDisplayable* );
protected:
	SkPost* fParent;
	typedef SkInput INHERITED;
	friend class SkPost;
};

#endif // SkPostParts_DEFINED
