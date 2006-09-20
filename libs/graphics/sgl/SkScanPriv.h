#ifndef SkScanPriv_DEFINED
#define SkScanPriv_DEFINED

#include "SkScan.h"
#include "SkBlitter.h"

class SkScanClipper {
public:
	SkScanClipper(SkBlitter* blitter, const SkRegion* clip, const SkRect16& bounds);

	SkBlitter*		getBlitter() const { return fBlitter; }
	const SkRect16*	getClipRect() const { return fClipRect; }

private:
	SkRectClipBlitter	fRectBlitter;
	SkRgnClipBlitter	fRgnBlitter;
	SkBlitter*			fBlitter;
	const SkRect16*		fClipRect;
};

void sk_fill_path(const SkPath& path, const SkRect16* clipRect, SkBlitter* blitter,
				  const SkRect16& ir, int shiftEdgesUp);

#endif

