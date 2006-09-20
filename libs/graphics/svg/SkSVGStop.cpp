#include "SkSVGStop.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGStop::gAttributes[] = {
	SVG_ATTRIBUTE(offset)
};

DEFINE_SVG_INFO(Stop)

void SkSVGStop::translate(SkSVGParser& parser, bool defState) {
	parser._startElement("color");
	INHERITED::translate(parser, defState);
	parser._addAttribute("color", parser.getPaintLast(SkSVGPaint::kStopColor));
	parser._endElement();
}
