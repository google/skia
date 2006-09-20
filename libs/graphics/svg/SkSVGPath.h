#ifndef SkSVGPath_DEFINED
#define SkSVGPath_DEFINED

#include "SkSVGElements.h"

class SkSVGPath : public SkSVGElement {
	DECLARE_SVG_INFO(Path);
private:
	SkString f_d;
	typedef SkSVGElement INHERITED;
};

#endif // SkSVGPath_DEFINED
