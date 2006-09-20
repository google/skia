#ifndef SkSVGPolygon_DEFINED
#define SkSVGPolygon_DEFINED

#include "SkSVGPolyline.h"

class SkSVGPolygon : public SkSVGPolyline {
	DECLARE_SVG_INFO(Polygon);
	virtual void addAttribute(SkSVGParser& , int attrIndex, 
		const char* attrValue, size_t attrLength);
private:
	typedef SkSVGPolyline INHERITED;
};

#endif // SkSVGPolygon_DEFINED
