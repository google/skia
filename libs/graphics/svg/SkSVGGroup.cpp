#include "SkSVGGroup.h"
#include "SkSVGParser.h"

SkSVGGroup::SkSVGGroup() {
	fIsNotDef = false;
}

SkSVGElement* SkSVGGroup::getGradient() {
	for (SkSVGElement** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
		SkSVGElement* result = (*ptr)->getGradient();
		if (result != nil)
			return result;
	}
	return nil;
}

bool SkSVGGroup::isDef() {
	return fParent ? fParent->isDef() : false;
}

bool SkSVGGroup::isFlushable() {
	return false;
}

bool SkSVGGroup::isGroup() {
	return true;
}

bool SkSVGGroup::isNotDef() {
	return fParent ? fParent->isNotDef() : false;
}

void SkSVGGroup::translate(SkSVGParser& parser, bool defState) {
	for (SkSVGElement** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++)
		parser.translate(*ptr, defState);
}
