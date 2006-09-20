#ifndef SkCornerPathEffect_DEFINED
#define SkCornerPathEffect_DEFINED

#include "SkPathEffect.h"

/**	\class SkCornerPathEffect

	SkCornerPathEffect is a subclass of SkPathEffect that can turn sharp corners
    into various treatments (e.g. rounded corners)
*/
class SkCornerPathEffect : public SkPathEffect {
public:
	/**	radius must be > 0 to have an effect. It specifies the distance from each corner
        that should be "rounded".
	*/
	SkCornerPathEffect(SkScalar radius);
	virtual ~SkCornerPathEffect();

	// overrides for SkPathEffect
    //  This method is not exported to java.
	virtual bool filterPath(SkPath* dst, const SkPath& src, SkScalar* width);

	// overrides for SkFlattenable
    //  This method is not exported to java.
	virtual Factory getFactory();
    //  This method is not exported to java.
	virtual void flatten(SkWBuffer&);

protected:
	SkCornerPathEffect(SkRBuffer&);

private:
    SkScalar    fRadius;

	static SkFlattenable* CreateProc(SkRBuffer&);
    
    // illegal
    SkCornerPathEffect(const SkCornerPathEffect&);
    SkCornerPathEffect& operator=(const SkCornerPathEffect&);
    
    typedef SkPathEffect INHERITED;
};

#endif

