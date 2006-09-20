#ifndef SkEmbossMaskFilter_DEFINED
#define SkEmbossMaskFilter_DEFINED

#include "SkMaskFilter.h"

/** \class SkEmbossMaskFilter

    This mask filter creates a 3D emboss look, by specifying a light and blur amount.
*/
class SkEmbossMaskFilter : public SkMaskFilter {
public:
	struct Light {
		SkScalar	fDirection[3];	// x,y,z
		U16			fPad;
		U8			fAmbient;
		U8			fSpecular;		// exponent, 4.4 right now
	};

	SkEmbossMaskFilter(const Light& light, SkScalar blurRadius);

	// overrides from SkMaskFilter
    //  This method is not exported to java.
	virtual SkMask::Format getFormat();
    //  This method is not exported to java.
	virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix& matrix, SkPoint16* margin);

	// overrides from SkFlattenable

    //  This method is not exported to java.
	virtual Factory getFactory();
    //  This method is not exported to java.
	virtual void flatten(SkWBuffer&);

protected:
    SkEmbossMaskFilter(SkRBuffer&);

private:
	Light		fLight;
	SkScalar	fBlurRadius;

	static SkFlattenable* CreateProc(SkRBuffer&);
    
    typedef SkMaskFilter INHERITED;
};

#endif

