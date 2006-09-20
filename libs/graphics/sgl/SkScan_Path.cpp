#include "SkScanPriv.h"
#include "SkBlitter.h"
#include "SkEdge.h"
#include "SkGeometry.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkTemplates.h"

#define kEDGE_HEAD_Y	SK_MinS16
#define kEDGE_TAIL_Y	SK_MaxS16

#ifdef SK_DEBUG
	static void validate_sort(const SkEdge* edge)
	{
		int	y = kEDGE_HEAD_Y;

		while (edge->fFirstY != SK_MaxS16)
		{
			edge->validate();
			SkASSERT(y <= edge->fFirstY);

			y = edge->fFirstY;
			edge = edge->fNext;
		}
	}
#else
	#define validate_sort(edge)
#endif

static inline void remove_edge(SkEdge* edge)
{
	edge->fPrev->fNext = edge->fNext;
	edge->fNext->fPrev = edge->fPrev;
}

static inline void swap_edges(SkEdge* prev, SkEdge* next)
{
	SkASSERT(prev->fNext == next && next->fPrev == prev);

	// remove prev from the list
	prev->fPrev->fNext = next;
	next->fPrev = prev->fPrev;

	// insert prev after next
	prev->fNext = next->fNext;
	next->fNext->fPrev = prev;
	next->fNext = prev;
	prev->fPrev = next;
}

static void backward_insert_edge_based_on_x(SkEdge* edge SkDECLAREPARAM(int, curr_y))
{
	SkFixed	x = edge->fX;

	for (;;)
	{
		SkEdge* prev = edge->fPrev;
		
		// add 1 to curr_y since we may have added new edges (built from curves)
		// that start on the next scanline
		SkASSERT(prev && prev->fFirstY <= curr_y + 1);

		if (prev->fX <= x)
			break;

		swap_edges(prev, edge);
	}
}

static void insert_new_edges(SkEdge* newEdge, int curr_y)
{
	SkASSERT(newEdge->fFirstY >= curr_y);

	while (newEdge->fFirstY == curr_y)
	{
		SkEdge* next = newEdge->fNext;
		backward_insert_edge_based_on_x(newEdge  SkPARAM(curr_y));
		newEdge = next;
	}
}

#ifdef SK_DEBUG
static void validate_edges_for_y(const SkEdge* edge, int curr_y)
{
	while (edge->fFirstY <= curr_y)
	{
		SkASSERT(edge->fPrev && edge->fNext);
		SkASSERT(edge->fPrev->fNext == edge);
		SkASSERT(edge->fNext->fPrev == edge);
		SkASSERT(edge->fFirstY <= edge->fLastY);

		SkASSERT(edge->fPrev->fX <= edge->fX);
		edge = edge->fNext;
	}
}
#else
	#define validate_edges_for_y(edge, curr_y)
#endif

#if defined _WIN32 && _MSC_VER >= 1300	// disable warning : local variable used without having been initialized
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

static void walk_edges(SkEdge* prevHead, SkPath::FillType fillType, SkBlitter* blitter,
					   int stop_y)
{
	validate_sort(prevHead->fNext);

	int	curr_y = prevHead->fNext->fFirstY;
	int windingMask = (fillType == SkPath::kWinding_FillType) ? -1 : 1;

	for (;;)
	{
		int		w = 0;
		int		left SK_INIT_TO_AVOID_WARNING;
		bool	in_interval	= false;
		SkEdge* currE = prevHead->fNext;
		SkFixed	prevX = prevHead->fX;

		validate_edges_for_y(currE, curr_y);

		while (currE->fFirstY <= curr_y)
		{
			SkASSERT(currE->fLastY >= curr_y);

			int x = (currE->fX + SK_Fixed1/2) >> 16;
			w += currE->fWinding;
			if ((w & windingMask) == 0)	// we finished an interval
			{
				SkASSERT(in_interval);
				int width = x - left;
				SkASSERT(width >= 0);
				if (width)
					blitter->blitH(left, curr_y, width);
				in_interval = false;
			}
			else if (!in_interval)
			{
				left = x;
				in_interval = true;
			}

			SkEdge* next = currE->fNext;
			SkFixed	newX;

			if (currE->fLastY == curr_y)	// are we done with this edge?
			{
				if (currE->fCurveCount < 0)
				{
					if (((SkCubicEdge*)currE)->updateCubic())
					{
						SkASSERT(currE->fFirstY == curr_y + 1);
						
						newX = currE->fX;
						goto NEXT_X;
					}
				}
				else if (currE->fCurveCount > 0)
				{
					if (((SkQuadraticEdge*)currE)->updateQuadratic())
					{
						newX = currE->fX;
						goto NEXT_X;
					}
				}
				remove_edge(currE);
			}
			else
			{
				SkASSERT(currE->fLastY > curr_y);
				newX = currE->fX + currE->fDX;
				currE->fX = newX;
			NEXT_X:
				if (newX < prevX)	// ripple currE backwards until it is x-sorted
					backward_insert_edge_based_on_x(currE  SkPARAM(curr_y));
				else
					prevX = newX;
			}
			currE = next;
			SkASSERT(currE);
		}

		curr_y += 1;
		if (curr_y >= stop_y)
			break;

		// now currE points to the first edge with a Yint larger than curr_y
		insert_new_edges(currE, curr_y);
	}
}

#if defined _WIN32 && _MSC_VER >= 1300
#pragma warning ( pop )
#endif

/*	Our line edge relies on the maximum span being <= 512, so that it can
	use FDot6 and keep the dx,dy in 16bits (for much faster slope divide).
	This function returns true if the specified line is too big.
*/
static inline bool line_too_big(const SkPoint pts[2])
{
	SkScalar dx = pts[1].fX - pts[0].fX;
	SkScalar dy = pts[1].fY - pts[0].fY;

	return	SkScalarAbs(dx) > SkIntToScalar(511) ||
			SkScalarAbs(dy) > SkIntToScalar(511);
}

static int build_edges(SkEdge edge[], const SkPath& path, const SkRect16* clipRect, SkEdge* list[], int shiftUp)
{
	SkEdge**		start = list;
	SkPath::Iter	iter(path, true);
	SkPoint			pts[4];
	SkPath::Verb	verb;

	while ((verb = iter.next(pts)) != SkPath::kDone_Verb)
	{
		switch (verb) {
		case SkPath::kLine_Verb:
			if (edge->setLine(pts, clipRect, shiftUp))
			{
				*list++ = edge;
				edge = (SkEdge*)((char*)edge + sizeof(SkEdge));
			}
			break;
		case SkPath::kQuad_Verb:
			{
				SkPoint	tmp[5];
				SkPoint* p = tmp;
				int		count = SkChopQuadAtYExtrema(pts, tmp);

				do {
					if (((SkQuadraticEdge*)edge)->setQuadratic(p, clipRect, shiftUp))
					{
						*list++ = edge;
						edge = (SkEdge*)((char*)edge + sizeof(SkQuadraticEdge));
					}
					p += 2;
				} while (--count >= 0);
			}
			break;
		case SkPath::kCubic_Verb:
			{
				SkPoint	tmp[10];
				SkPoint* p = tmp;
				int		count = SkChopCubicAtYExtrema(pts, tmp);				
				SkASSERT(count >= 0 && count <= 2);

				do {
					if (((SkCubicEdge*)edge)->setCubic(p, clipRect, shiftUp))
					{
						*list++ = edge;
						edge = (SkEdge*)((char*)edge + sizeof(SkCubicEdge));
					}
					p += 3;
				} while (--count >= 0);
			}
			break;
		default:
			break;
		}
	}
	return (int)(list - start);
}

extern "C" {
	static int edge_compare(const void* a, const void* b)
	{
		const SkEdge* edgea = *(const SkEdge**)a;
		const SkEdge* edgeb = *(const SkEdge**)b;

		int valuea = edgea->fFirstY;
		int valueb = edgeb->fFirstY;

		if (valuea == valueb)
		{
			valuea = edgea->fX;
			valueb = edgeb->fX;
		}
		return valuea - valueb;
	}
}

static SkEdge* sort_edges(SkEdge* list[], int count, SkEdge** last)
{
	qsort(list, count, sizeof(SkEdge*), edge_compare);

	// now make the edges linked in sorted order
	for (int i = 1; i < count; i++)
	{
		list[i - 1]->fNext = list[i];
		list[i]->fPrev = list[i - 1];
	}

	*last = list[count - 1];
	return list[0];
}

static int worst_case_edge_count(const SkPath& path, size_t* storage)
{
	size_t	size = 0;
	int		edgeCount = 0;

	SkPath::Iter	iter(path, true);
	SkPath::Verb	verb;

	while ((verb = iter.next(nil)) != SkPath::kDone_Verb)
	{
		switch (verb) {
		case SkPath::kLine_Verb:
			edgeCount += 1;
			size += sizeof(SkQuadraticEdge);	// treat line like Quad (in case its > 512)
			break;
		case SkPath::kQuad_Verb:
			edgeCount += 2;						// might need 2 edges when we chop on Y extrema
			size += 2 * sizeof(SkQuadraticEdge);
			break;
		case SkPath::kCubic_Verb:
			edgeCount += 3;						// might need 3 edges when we chop on Y extrema
			size += 3 * sizeof(SkCubicEdge);
			break;
		default:
			break;
		}
	}

	SkASSERT(storage);
	*storage = size;
	return edgeCount;
}

void sk_fill_path(const SkPath& path, const SkRect16* clipRect, SkBlitter* blitter,
				  const SkRect16& ir, int shiftEdgesUp)
{
	SkASSERT(&path && blitter);

	size_t	size;
	int		maxCount = worst_case_edge_count(path, &size);

	SkAutoMalloc	memory(maxCount * sizeof(SkEdge*) + size);
	SkEdge**		list = (SkEdge**)memory.get();
	SkEdge*			edge = (SkEdge*)(list + maxCount);
	int				count = build_edges(edge, path, clipRect, list, shiftEdgesUp);
	SkEdge			headEdge, tailEdge, *last;

	SkASSERT(count <= maxCount);
	if (count == 0)
		return;
	SkASSERT(count > 1);

	// this returns the first and last edge after they're sorted into a dlink list
	edge = sort_edges(list, count, &last);

	headEdge.fPrev = nil;
	headEdge.fNext = edge;
	headEdge.fFirstY = kEDGE_HEAD_Y;
	headEdge.fX = SK_MinS32;
	edge->fPrev = &headEdge;

	tailEdge.fPrev = last;
	tailEdge.fNext = nil;
	tailEdge.fFirstY = kEDGE_TAIL_Y;
	last->fNext = &tailEdge;

	// now edge is the head of the sorted linklist
	int	stop_y = ir.fBottom;
	if (clipRect && stop_y > clipRect->fBottom)
		stop_y = clipRect->fBottom;
	walk_edges(&headEdge, path.getFillType(), blitter, stop_y);
}

/////////////////////////////////////////////////////////////////////////////////////

SkScanClipper::SkScanClipper(SkBlitter* blitter, const SkRegion* clip, const SkRect16& ir)
{
	fBlitter = nil;		// nil means blit nothing
	fClipRect = nil;

	if (clip)
	{
		fClipRect = &clip->getBounds();
		if (!SkRect16::Intersects(*fClipRect, ir))	// completely clipped out
			return;

		if (clip->isRect())
		{
			if (fClipRect->contains(ir))
				fClipRect = nil;
			else
			{
				// only need a wrapper blitter if we're horizontally clipped
				if (fClipRect->fLeft > ir.fLeft || fClipRect->fRight < ir.fRight)
				{
					fRectBlitter.init(blitter, *fClipRect);
					blitter = &fRectBlitter;
				}
			}
		}
		else
		{
			fRgnBlitter.init(blitter, clip);
			blitter = &fRgnBlitter;
		}
	}
	fBlitter = blitter;
}

/////////////////////////////////////////////////////////////////////////////////////

void SkScan::FillPath(const SkPath& path, const SkRegion* clip, SkBlitter* blitter)
{
	if (clip && clip->isEmpty())
		return;

	SkRect		r;
	SkRect16	ir;

	path.computeBounds(&r, SkPath::kFast_BoundsType);
	r.round(&ir);
	if (ir.isEmpty())
		return;

	SkScanClipper	clipper(blitter, clip, ir);

	blitter = clipper.getBlitter();
	if (blitter)
		sk_fill_path(path, clipper.getClipRect(), blitter, ir, 0);
}

