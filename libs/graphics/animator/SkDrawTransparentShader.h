#ifndef SkDrawTransparentShader_DEFINED
#define SkDrawTransparentShader_DEFINED

#include "SkPaintParts.h"

class SkDrawTransparentShader : public SkDrawShader {
	DECLARE_EMPTY_MEMBER_INFO(TransparentShader);
	virtual SkShader* getShader();
};

#endif // SkDrawTransparentShader_DEFINED

