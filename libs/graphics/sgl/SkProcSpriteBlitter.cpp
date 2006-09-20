#if 0	// experimental

class SkProcSpriteBlitter : public SkSpriteBlitter {
public:
	typedef void (*Proc)(void* dst, const void* src, int count, const U32 ctable[]);

	SkProcSpriteBlitter(const SkBitmap& source, Proc proc, unsigned srcShift, unsigned dstShift)
		: SkSpriteBlitter(source), fProc(proc), fSrcShift(SkToU8(srcShift)), fDstShift(SkToU8(dstShift)) {}

	virtual void blitRect(int x, int y, int width, int height)
	{
		size_t		dstRB = fDevice.rowBytes();
		size_t		srcRB = fSource.rowBytes();
		char*		dst = (char*)fDevice.getPixels() + y * dstRB + (x << fDstShift);
		const char*	src = (const char*)fSource.getPixels() + (y - fTop) * srcRB + ((x - fLeft) << fSrcShift);
		Proc		proc = fProc;
		const U32*	ctable = nil;

		if fSource.getColorTable())
			ctable = fSource.getColorTable()->lockColors();

		while (--height >= 0)
		{
			proc(dst, src, width, ctable);
			dst += dstRB;
			src += srcRB;
		}

		if fSource.getColorTable())
			fSource.getColorTable()->unlockColors(false);
	}

private:
	Proc	fProc;
	U8		fSrcShift, fDstShift;
};

#endif
