#ifndef SkSVGMetadata_DEFINED
#define SkSVGMetadata_DEFINED

#include "SkSVGElements.h"

class SkSVGMetadata : public SkSVGElement {
	DECLARE_SVG_INFO(Metadata);
	virtual bool isDef();
	virtual bool isNotDef();
private:
	typedef SkSVGElement INHERITED;
};

#endif // SkSVGMetadata_DEFINED
