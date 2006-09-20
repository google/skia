#ifndef SkSVGMask_DEFINED
#define SkSVGMask_DEFINED

#include "SkSVGGroup.h"

class SkSVGMask : public SkSVGGroup {
	DECLARE_SVG_INFO(Mask);
	virtual bool isDef();
	virtual bool isNotDef();
protected:
	SkString f_height;
	SkString f_maskUnits;
	SkString f_width;
	SkString f_x;
	SkString f_y;
private:
	typedef SkSVGGroup INHERITED;
};

#endif // SkSVGMask_DEFINED
