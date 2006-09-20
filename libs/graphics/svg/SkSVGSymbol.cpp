#include "SkSVGSymbol.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGSymbol::gAttributes[] = {
	SVG_ATTRIBUTE(viewBox)
};

DEFINE_SVG_INFO(Symbol)

void SkSVGSymbol::translate(SkSVGParser& parser, bool defState) {
	INHERITED::translate(parser, defState);
	// !!! children need to be written into document 
}
