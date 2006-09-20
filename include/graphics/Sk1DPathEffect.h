#ifndef Sk1DPathEffect_DEFINED
#define Sk1DPathEffect_DEFINED

#include "SkPathEffect.h"
#include "SkPath.h"

class SkPathMeasure;

//  This class is not exported to java.
class Sk1DPathEffect : public SkPathEffect {
public:
	Sk1DPathEffect() {}

	virtual bool filterPath(SkPath* dst, const SkPath& src, SkScalar* width);

protected:
	/**	Called at the start of each contour, returns the initial offset
		into that contour.
	*/
	virtual SkScalar begin(SkScalar contourLength);
	/**	Called with the current distance along the path, with the current matrix
		for the point/tangent at the specified distance.
		Return the distance to travel for the next call. If return <= 0, then that
		contour is done.
	*/
	virtual SkScalar next(SkPath* dst, SkScalar distance, SkPathMeasure&);

	Sk1DPathEffect(SkRBuffer& buffer) : SkPathEffect(buffer) {}

private:
	// illegal
	Sk1DPathEffect(const Sk1DPathEffect&);
	Sk1DPathEffect& operator=(const Sk1DPathEffect&);

	typedef SkPathEffect INHERITED;
};

class SkPath1DPathEffect : public Sk1DPathEffect {
public:
    enum Style {
        kTranslate_Style,   // translate the shape to each position
        kRotate_Style,      // rotate the shape about its center
        kMorph_Style,       // transform each point, and turn lines into curves
        
        kStyleCount
    };
    SkPath1DPathEffect(const SkPath& path, SkScalar advance, SkScalar phase, Style);

    // This method is not exported to java.
	virtual bool filterPath(SkPath* dst, const SkPath& src, SkScalar* width);

protected:
    virtual SkScalar begin(SkScalar contourLength);
    virtual SkScalar next(SkPath* dst, SkScalar distance, SkPathMeasure&);
    
private:
    SkPath      fPath;
    SkScalar    fAdvance, fPhase;
    Style       fStyle;
    
    typedef Sk1DPathEffect INHERITED;
};


#endif
