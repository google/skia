#ifndef SkSVGText_DEFINED
#define SkSVGText_DEFINED

#include "SkSVGElements.h"

class SkSVGText : public SkSVGElement {
	DECLARE_SVG_INFO(Text);
protected:
	SkString f_x;
	SkString f_y;
	SkString f_text;	// not an attribute
private:
	typedef SkSVGElement INHERITED;
	friend class SkSVGParser;
};

class SkSVGTspan : public SkSVGText {
	DECLARE_SVG_INFO(Tspan);
private:
	typedef SkSVGText INHERITED;
};

#endif // SkSVGText_DEFINED
