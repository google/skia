#ifndef SkSVGUse_DEFINED
#define SkSVGUse_DEFINED

#include "SkSVGElements.h"

class SkSVGUse : public SkSVGElement {
	DECLARE_SVG_INFO(Use);
protected:
	SkString f_height;
	SkString f_width;
	SkString f_x;
	SkString f_xlink_href;
	SkString f_y;
private:
	typedef SkSVGElement INHERITED;
	friend class SkSVGClipPath;
};

#endif // SkSVGUse_DEFINED
