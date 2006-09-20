#ifndef SkFlattenable_DEFINED
#define SkFlattenable_DEFINED

#include "SkRefCnt.h"
#include "SkBuffer.h"

/**	\class SkFlattenable

	SkFlattenable is the base class for objects that need to be flattened
	into a data stream for either transport or as part of the key to the
	font cache.
*/
//  This class is not exported to java.
class SkFlattenable : public SkRefCnt {
public:
	typedef SkFlattenable* (*Factory)(SkRBuffer&);

	virtual Factory	getFactory();
	virtual void	flatten(SkWBuffer&);
};

#endif

