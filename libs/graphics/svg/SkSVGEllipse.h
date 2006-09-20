#ifndef SkSVGEllipse_DEFINED
#define SkSVGEllipse_DEFINED

#include "SkSVGElements.h"

class SkSVGEllipse : public SkSVGElement {
	DECLARE_SVG_INFO(Ellipse);
private:
	SkString f_cx;
	SkString f_cy;
	SkString f_rx;
	SkString f_ry;
	typedef SkSVGElement INHERITED;
};

#endif // SkSVGEllipse_DEFINED
