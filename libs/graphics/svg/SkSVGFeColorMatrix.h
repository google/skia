#ifndef SkSVGFeColorMatrix_DEFINED
#define SkSVGFeColorMatrix_DEFINED

#include "SkSVGElements.h"

class SkSVGFeColorMatrix : public SkSVGElement {
	DECLARE_SVG_INFO(FeColorMatrix);
protected:
	SkString f_color_interpolation_filters;
	SkString f_result;
	SkString f_type;
	SkString f_values;
private:
	typedef SkSVGElement INHERITED;
};

#endif // SkSVGFeColorMatrix_DEFINED
