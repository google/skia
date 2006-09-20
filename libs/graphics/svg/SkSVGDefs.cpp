#include "SkSVGDefs.h"

DEFINE_SVG_NO_INFO(Defs)

bool SkSVGDefs::isDef() {
	return true;
}

bool SkSVGDefs::isNotDef() {
	return false;
}

void SkSVGDefs::translate(SkSVGParser& parser, bool defState) {
	INHERITED::translate(parser, defState);
}
