#include "SkSVGFilter.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGFilter::gAttributes[] = {
	SVG_ATTRIBUTE(filterUnits),
	SVG_ATTRIBUTE(height),
	SVG_ATTRIBUTE(width),
	SVG_ATTRIBUTE(x),
	SVG_ATTRIBUTE(y)
};

DEFINE_SVG_INFO(Filter)

void SkSVGFilter::translate(SkSVGParser& parser, bool defState) {
//	INHERITED::translate(parser, defState);
}
