#ifndef SkSVGPolyline_DEFINED
#define SkSVGPolyline_DEFINED

#include "SkSVGElements.h"
#include "SkString.h"

class SkSVGPolyline : public SkSVGElement {
	DECLARE_SVG_INFO(Polyline);
	virtual void addAttribute(SkSVGParser& , int attrIndex, 
		const char* attrValue, size_t attrLength);
protected:
	SkString f_clipRule;
	SkString f_fillRule;
	SkString f_points;
	typedef SkSVGElement INHERITED;
};

#endif // SkSVGPolyline_DEFINED
