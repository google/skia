#ifndef SkTextToPath_DEFINED
#define SkTextToPath_DEFINED

#include "SkDrawPath.h"
#include "SkMemberInfo.h"

class SkDrawPaint;
class SkDrawPath;
class SkText;

class SkTextToPath : public SkDrawable {
	DECLARE_MEMBER_INFO(TextToPath);
	SkTextToPath();
	virtual bool draw(SkAnimateMaker& );
	virtual void onEndElement(SkAnimateMaker& );
private:
	SkDrawPaint* paint;
	SkDrawPath* path;
	SkText* text;
};

#endif // SkTextToPath_DEFINED

