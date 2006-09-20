#ifndef SkUnitMapper_DEFINED
#define SkUnitMapper_DEFINED

#include "SkRefCnt.h"
#include "SkScalar.h"

class SkUnitMapper : public SkRefCnt {
public:
	/**	Given a value in [0..0xFFFF], return a value in the same range.
	*/
	virtual U16CPU mapUnit16(U16CPU x) = 0;
};

/**	This returns N values between [0...1]
*/
class SkDiscreteMapper : public SkUnitMapper {
public:
	SkDiscreteMapper(unsigned segments);
	// override
	virtual U16CPU mapUnit16(U16CPU x);
private:
	unsigned fSegments;
	SkFract fScale;
};

/**	This returns 1 - cos(x), to simulate lighting a sphere
*/
class SkFlipCosineMapper : public SkUnitMapper {
public:
	// override
	virtual U16CPU mapUnit16(U16CPU x);
};

#endif

