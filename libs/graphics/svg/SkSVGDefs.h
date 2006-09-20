#ifndef SkSVGDefs_DEFINED
#define SkSVGDefs_DEFINED

#include "SkSVGGroup.h"

class SkSVGDefs : public SkSVGGroup {
	DECLARE_SVG_INFO(Defs);
	virtual bool isDef();
	virtual bool isNotDef();
private:
	typedef SkSVGGroup INHERITED;
};

#endif // SkSVGDefs_DEFINED
