#include "SkSVGMetadata.h"
#include "SkSVGParser.h"

DEFINE_SVG_NO_INFO(Metadata)

bool SkSVGMetadata::isDef() {
	return false;
}

bool SkSVGMetadata::isNotDef() {
	return false;
}

void SkSVGMetadata::translate(SkSVGParser& parser, bool defState) {
}
