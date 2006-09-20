#ifndef SkTransparentShader_DEFINED
#define SkTransparentShader_DEFINED

#include "SkShader.h"

class SkTransparentShader : public SkShader {
public:
	virtual U32		getFlags();
	virtual bool	setContext(	const SkBitmap& device,
								const SkPaint& paint,
								const SkMatrix& matrix);
	virtual void	shadeSpan(int x, int y, SkPMColor[], int count);
	virtual void	shadeSpanOpaque16(int x, int y, U16 span[], int count);

private:
	SkBitmap	fDevice;
	U8			fAlpha;

	typedef SkShader INHERITED;
};

#endif

