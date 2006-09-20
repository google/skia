#ifndef SkDrawTextBox_DEFINED
#define SkDrawTextBox_DEFINED

#include "SkDrawRectangle.h"
#include "SkTextBox.h"

class SkDrawTextBox : public SkDrawRect {
	DECLARE_DRAW_MEMBER_INFO(TextBox);
	SkDrawTextBox();

	// overrides
	virtual bool draw(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
#endif
	virtual bool getProperty(int index, SkScriptValue* value) const;
	virtual bool setProperty(int index, SkScriptValue& );

private:
	SkString fText;
	SkScalar fSpacingMul;
	SkScalar fSpacingAdd;
	int /*SkTextBox::Mode*/	 mode;
	int /*SkTextBox::SpacingAlign*/ spacingAlign;

	typedef SkDrawRect INHERITED;
};

#endif // SkDrawTextBox_DEFINED

