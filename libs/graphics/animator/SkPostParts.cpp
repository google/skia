#include "SkPostParts.h"
#include "SkDisplayPost.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkData::fInfo[] = {
	SK_MEMBER_INHERITED
};

#endif

DEFINE_GET_MEMBER(SkData);

SkData::SkData() : fParent(nil) {}

bool SkData::add() {
	SkASSERT(name.size() > 0);
	const char* dataName = name.c_str();
	if (fInt != (int) SK_NaN32)
		fParent->fEvent.setS32(dataName, fInt);
	else if (SkScalarIsNaN(fFloat) == false)
		fParent->fEvent.setScalar(dataName, fFloat);
	else if (string.size() > 0) 
		fParent->fEvent.setString(dataName, string);
//	else
//		SkASSERT(0);
	return false;
}

void SkData::dirty() {
	fParent->dirty();
}

SkDisplayable* SkData::getParent() const {
	return fParent;
}

bool SkData::setParent(SkDisplayable* displayable) {
	if (displayable->isPost() == false)
		return true;
	fParent = (SkPost*) displayable;
	return false;
}

void SkData::onEndElement(SkAnimateMaker&) {
	add();
}

