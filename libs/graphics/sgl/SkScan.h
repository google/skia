#ifndef SkScan_DEFINED
#define SkScan_DEFINED

#include "SkRect.h"

class SkRegion;
class SkBlitter;
class SkPath;

class SkScan {
public:
	static void	FillDevRect(const SkRect16&, const SkRegion* clip, SkBlitter*);
	static void	FillRect(const SkRect&, const SkRegion* clip, SkBlitter*);
	static void FillPath(const SkPath&, const SkRegion* clip, SkBlitter*);

	static void HairLine(const SkPoint&, const SkPoint&, const SkRegion* clip, SkBlitter*);
	static void HairRect(const SkRect&, const SkRegion* clip, SkBlitter*);
	static void HairPath(const SkPath&, const SkRegion* clip, SkBlitter*);

	static void FrameRect(const SkRect&, SkScalar width, const SkRegion* clip, SkBlitter*);

	static void	AntiFillRect(const SkRect&, const SkRegion* clip, SkBlitter*);
	static void AntiFillPath(const SkPath&, const SkRegion* clip, SkBlitter*);

	static void AntiHairLine(const SkPoint&, const SkPoint&, const SkRegion* clip, SkBlitter*);
	static void AntiHairRect(const SkRect&, const SkRegion* clip, SkBlitter*);
	static void AntiHairPath(const SkPath&, const SkRegion* clip, SkBlitter*);
};

#endif
