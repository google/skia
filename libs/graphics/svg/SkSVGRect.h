#ifndef SkSVGRect_DEFINED
#define SkSVGRect_DEFINED

#include "SkSVGElements.h"

class SkSVGRect : public SkSVGElement {
	DECLARE_SVG_INFO(Rect);
	SkSVGRect();
private:
	SkString f_height;
	SkString f_width;
	SkString f_x;
	SkString f_y;
	typedef SkSVGElement INHERITED;
};

#endif // SkSVGRect_DEFINED
