#ifndef SkDrawShader_DEFINED
#define SkDrawShader_DEFINED

#include "SkPaintParts.h"
#include "SkShader.h"

class SkBaseBitmap;

class SkDrawBitmapShader : public SkDrawShader {
	DECLARE_DRAW_MEMBER_INFO(BitmapShader);
	SkDrawBitmapShader();
	virtual bool add();
	virtual SkShader* getShader();
protected:
	int /*SkPaint::FilterType*/ filterType;
	SkBaseBitmap* image;
private:
	typedef SkDrawShader INHERITED;
};

#endif // SkDrawShader_DEFINED
