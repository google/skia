#ifndef SkScalerContext_DEFINED
#define SkScalerContext_DEFINED

#include "SkMatrix.h"
#include "SkPath.h"
#include "SkPoint.h"

class SkDescriptor;
class SkMaskFilter;
class SkPaint;
class SkPathEffect;
class SkRasterizer;

#define SK_UnknownAuxScalerContextID    0
#define SK_MaxAuxScalerContextID        16

struct SkGlyph {
	void*       fImage;
	SkPath*     fPath;
	SkFixed     fAdvanceX, fAdvanceY;

	uint16_t	fGlyphID;
	uint16_t	fWidth, fHeight, fRowBytes;
	int16_t		fTop, fLeft;

	uint16_t	fCharCode;	// might go away with line layout. really wants 20bits
	uint8_t		fMaskFormat;
	SkBool8     fUseAuxContext; // just need 1-bit for this field

	size_t computeImageSize() const;
};

class SkScalerContext {
public:
	struct Rec {
		SkScalar	fTextSize, fPreScaleX, fPreSkewX;
		SkScalar	fPost2x2[2][2];
		SkScalar	fFrameWidth, fMiterLimit;
		SkBool8		fUseHints;
		SkBool8		fFrameAndFill;
		SkBool8		fDoAA;
		uint8_t		fStrokeJoin;

		void	getMatrixFrom2x2(SkMatrix*) const;
		void	getLocalMatrix(SkMatrix*) const;
		void	getSingleMatrix(SkMatrix*) const;
	};

	SkScalerContext(const SkDescriptor* desc);
	virtual	~SkScalerContext();

	void	getMetrics(SkGlyph*);
	void	getImage(const SkGlyph&);
	void	getPath(const SkGlyph&, SkPath*);
	void	getLineHeight(SkPoint* above, SkPoint* below);

	static inline void MakeRec(const SkPaint&, const SkMatrix*, Rec* rec);
	static SkScalerContext* Create(const SkDescriptor*);

protected:
	Rec	fRec;

	virtual void generateMetrics(SkGlyph*) = 0;
	virtual void generateImage(const SkGlyph&) = 0;
	virtual void generatePath(const SkGlyph&, SkPath*) = 0;
	virtual void generateLineHeight(SkPoint* above, SkPoint* below) = 0;

private:
	SkPathEffect*	fPathEffect;
	SkMaskFilter*	fMaskFilter;
    SkRasterizer*   fRasterizer;
	SkScalar		fDevFrameWidth;

    void internalGetPath(const SkGlyph& glyph, SkPath* fillPath, SkPath* devPath, SkMatrix* fillToDevMatrix);

    // we index into this with scalerContextID-1
    SkScalerContext* fAuxContext[SK_MaxAuxScalerContextID];
};

#define kRec_SkDescriptorTag			SkSetFourByteTag('s', 'r', 'e', 'c')
#define kTypeface_SkDescriptorTag		SkSetFourByteTag('t', 'p', 'f', 'c')
#define kPathEffect_SkDescriptorTag		SkSetFourByteTag('p', 't', 'h', 'e')
#define kMaskFilter_SkDescriptorTag		SkSetFourByteTag('m', 's', 'k', 'f')
#define kRasterizer_SkDescriptorTag		SkSetFourByteTag('r', 'a', 's', 't')

#endif

