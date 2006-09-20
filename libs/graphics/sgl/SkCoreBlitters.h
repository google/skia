#ifndef SkCoreBlitters_DEFINED
#define SkCoreBlitters_DEFINED

#include "SkBlitter.h"

class SkA8_Blitter : public SkBlitter {
public:
	SkA8_Blitter(const SkBitmap& device, const SkPaint& paint);
	virtual void blitH(int x, int y, int width);
	virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[]);
	virtual void blitV(int x, int y, int height, SkAlpha alpha);
	virtual void blitRect(int x, int y, int width, int height);
	virtual void blitMask(const SkMask&, const SkRect16&);

private:
	const SkBitmap&	fDevice;
	unsigned		fSrcA;

	// illegal
	SkA8_Blitter& operator=(const SkA8_Blitter&);
};

class SkA8_Shader_Blitter : public SkBlitter {
public:
	SkA8_Shader_Blitter(const SkBitmap& device, const SkPaint& paint);
	virtual ~SkA8_Shader_Blitter();
	virtual void blitH(int x, int y, int width);
	virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[]);
	virtual void blitMask(const SkMask&, const SkRect16&);

private:
	const SkBitmap&	fDevice;
	SkShader*	fShader;
	SkXfermode*	fXfermode;
	SkPMColor*	fBuffer;
	U8*			fAAExpand;

    typedef SkBlitter INHERITED;

	// illegal
	SkA8_Shader_Blitter& operator=(const SkA8_Shader_Blitter&);
};

////////////////////////////////////////////////////////////////

class SkARGB32_Blitter : public SkBlitter {
public:
	SkARGB32_Blitter(const SkBitmap& device, const SkPaint& paint);
	virtual void blitH(int x, int y, int width);
	virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[]);
	virtual void blitV(int x, int y, int height, SkAlpha alpha);
	virtual void blitRect(int x, int y, int width, int height);
	virtual void blitMask(const SkMask&, const SkRect16&);

protected:
	const SkBitmap&	fDevice;

private:
	SkColor		fPMColor;
	unsigned	fSrcA, fSrcR, fSrcG, fSrcB;

	// illegal
	SkARGB32_Blitter& operator=(const SkARGB32_Blitter&);
};

class SkARGB32_Black_Blitter : public SkARGB32_Blitter {
public:
	SkARGB32_Black_Blitter(const SkBitmap& device, const SkPaint& paint)
		: SkARGB32_Blitter(device, paint) {}
	virtual void blitMask(const SkMask&, const SkRect16&);
};

class SkARGB32_Shader_Blitter : public SkBlitter {
public:
	SkARGB32_Shader_Blitter(const SkBitmap& device, const SkPaint& paint);
	virtual ~SkARGB32_Shader_Blitter();
	virtual void blitH(int x, int y, int width);
	virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[]);

private:
	const SkBitmap&	fDevice;
	SkShader*	fShader;
	SkXfermode*	fXfermode;
	SkPMColor*	fBuffer;

	typedef SkBlitter INHERITED;

	// illegal
	SkARGB32_Shader_Blitter& operator=(const SkARGB32_Shader_Blitter&);
};

////////////////////////////////////////////////////////////////

class SkRGB16_Blitter : public SkBlitter {
public:
	SkRGB16_Blitter(const SkBitmap& device, const SkPaint& paint);
	virtual void blitH(int x, int y, int width);
	virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[]);
	virtual void blitV(int x, int y, int height, SkAlpha alpha);
	virtual void blitRect(int x, int y, int width, int height);
	virtual void blitMask(const SkMask&, const SkRect16&);

protected:
	const SkBitmap&	fDevice;

private:
	unsigned	fScale;
	U16			fColor16;
	U16			fRawColor16;

	// illegal
	SkRGB16_Blitter& operator=(const SkRGB16_Blitter&);
};

class SkRGB16_Black_Blitter : public SkRGB16_Blitter {
public:
	SkRGB16_Black_Blitter(const SkBitmap& device, const SkPaint& paint)
		: SkRGB16_Blitter(device, paint) {}
	virtual void blitMask(const SkMask&, const SkRect16&);
};

class SkRGB16_Shader_Blitter : public SkBlitter {
public:
	SkRGB16_Shader_Blitter(const SkBitmap& device, const SkPaint& paint);
	virtual ~SkRGB16_Shader_Blitter();
	virtual void blitH(int x, int y, int width);
	virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[]);

private:
	const SkBitmap&	fDevice;
	SkShader*	fShader;
	SkPMColor*	fBuffer;

	typedef SkBlitter INHERITED;

	// illegal
	SkRGB16_Shader_Blitter& operator=(const SkRGB16_Shader_Blitter&);
};

class SkRGB16_Shader_Xfermode_Blitter : public SkBlitter {
public:
	SkRGB16_Shader_Xfermode_Blitter(const SkBitmap& device, const SkPaint& paint);
	virtual ~SkRGB16_Shader_Xfermode_Blitter();
	virtual void blitH(int x, int y, int width);
	virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[]);

private:
	const SkBitmap&	fDevice;
	SkShader*	fShader;
	SkXfermode*	fXfermode;
	SkPMColor*	fBuffer;
	U8*			fAAExpand;

	typedef SkBlitter INHERITED;

	// illegal
	SkRGB16_Shader_Xfermode_Blitter& operator=(const SkRGB16_Shader_Xfermode_Blitter&);
};

/////////////////////////////////////////////////////////////////////////////

class SkA1_Blitter : public SkBlitter {
public:
	SkA1_Blitter(const SkBitmap& device, const SkPaint& paint);
	virtual void blitH(int x, int y, int width);

private:
	const SkBitmap&	fDevice;
	U8				fSrcA;

	// illegal
	SkA1_Blitter& operator=(const SkA1_Blitter&);
};


#endif

