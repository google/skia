#ifndef SkSVGClipPath_DEFINED
#define SkSVGClipPath_DEFINED

#include "SkSVGElements.h"

class SkSVGClipPath : public SkSVGElement {
	DECLARE_SVG_INFO(ClipPath);
	virtual bool isDef();
	virtual bool isNotDef();
private:
	typedef SkSVGElement INHERITED;
};

#endif // SkSVGClipPath_DEFINED
