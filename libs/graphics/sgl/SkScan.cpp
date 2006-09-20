#include "SkScan.h"
#include "SkBlitter.h"
#include "SkRegion.h"

void SkScan::FillRect(const SkRect& rect, const SkRegion* clip, SkBlitter* blitter)
{
	SkRect16 r;

	rect.round(&r);
	SkScan::FillDevRect(r, clip, blitter);
}

void SkScan::FillDevRect(const SkRect16& r, const SkRegion* clip, SkBlitter* blitter)
{
	if (!r.isEmpty())
	{
		if (clip)
		{
			SkRegion::Cliperator	cliper(*clip, r);
			const SkRect16&			rr = cliper.rect();

			while (!cliper.done())
			{
				blitter->blitRect(rr.fLeft, rr.fTop, rr.width(), rr.height());
				cliper.next();
			}
		}
		else
			blitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
	}
}

