#ifndef SkDrawText_DEFINED
#define SkDrawText_DEFINED

#include "SkBoundable.h"
#include "SkMemberInfo.h"

class SkText : public SkBoundable {
	DECLARE_MEMBER_INFO(Text);
	SkText();
	virtual ~SkText();
	virtual bool draw(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
	virtual void dump(SkAnimateMaker* );
#endif
	virtual bool getProperty(int index, SkScriptValue* value) const ; 
	const char* getText() { return text.c_str(); }
	size_t getSize() { return text.size(); }
protected:
	SkString text;
	SkScalar x;
	SkScalar y;
private:
	friend class SkTextToPath;
	typedef SkBoundable INHERITED;
};

#endif // SkDrawText_DEFINED
