#ifndef SkSVGGroup_DEFINED
#define SkSVGGroup_DEFINED

#include "SkSVGElements.h"

class SkSVGGroup : public SkSVGElement {
public:
	SkSVGGroup();
	virtual SkSVGElement* getGradient();
	virtual bool isDef();
	virtual bool isFlushable();
	virtual bool isGroup();
	virtual bool isNotDef();
	void translate(SkSVGParser& , bool defState);
private:
	typedef SkSVGElement INHERITED;
};

#endif // SkSVGGroup_DEFINED
