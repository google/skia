#ifndef SkSVGBase_DEFINED
#define SkSVGBase_DEFINED

#include "SkSVGAttribute.h"

class SkSVGParser;

class SkSVGBase {
public:
	virtual ~SkSVGBase();
	virtual void addAttribute(SkSVGParser& parser, int attrIndex, 
		const char* attrValue, size_t attrLength);
	virtual int getAttributes(const SkSVGAttribute** attrPtr) = 0;
};

#endif // SkSVGBase_DEFINED