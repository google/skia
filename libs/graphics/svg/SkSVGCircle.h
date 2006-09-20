#ifndef SkSVGCircle_DEFINED
#define SkSVGCircle_DEFINED

#include "SkSVGElements.h"

class SkSVGCircle : public SkSVGElement {
	DECLARE_SVG_INFO(Circle);
private:
	SkString f_cx;
	SkString f_cy;
	SkString f_r;
	typedef SkSVGElement INHERITED;
};

#endif // SkSVGCircle_DEFINED
