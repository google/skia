#ifndef SkSVGSymbol_DEFINED
#define SkSVGSymbol_DEFINED

#include "SkSVGElements.h"

class SkSVGSymbol : public SkSVGElement {
	DECLARE_SVG_INFO(Symbol);
private:
	SkString f_viewBox;
	typedef SkSVGElement INHERITED;
};

#endif // SkSVGSymbol_DEFINED
