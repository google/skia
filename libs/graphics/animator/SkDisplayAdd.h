#ifndef SkDisplayAdd_DEFINED
#define SkDisplayAdd_DEFINED

#include "SkDrawable.h"
#include "SkMemberInfo.h"

class SkAdd : public SkDrawable {
	DECLARE_MEMBER_INFO(Add);
	SkAdd();
	
	enum Mode {
		kMode_indirect,
		kMode_immediate
	};
	
	virtual SkDisplayable* deepCopy(SkAnimateMaker* );
	virtual bool draw(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
	virtual void dump(SkAnimateMaker* );
#endif
	virtual bool enable(SkAnimateMaker& );
	virtual bool hasEnable() const;
	virtual void initialize();
	virtual bool isDrawable() const;
protected:
//	struct _A {
		Mode mode;
		S32 offset;
		SkDrawable* use;
		SkDrawable* where;	// if nil, offset becomes index
//	} A;
private:
	typedef SkDrawable INHERITED;
};

class SkClear : public SkDisplayable {
	virtual bool enable(SkAnimateMaker& );
};

class SkMove : public SkAdd {
	DECLARE_MEMBER_INFO(Move);
private:
	typedef SkAdd INHERITED;
};

class SkRemove : public SkAdd {
	DECLARE_MEMBER_INFO(Remove);
	SkRemove();
protected:
	SkBool fDelete;
private:
	friend class SkAdd;
	typedef SkAdd INHERITED;
};

class SkReplace : public SkAdd {
	DECLARE_MEMBER_INFO(Replace);
private:
	typedef SkAdd INHERITED;
};

#endif // SkDisplayAdd_DEFINED


