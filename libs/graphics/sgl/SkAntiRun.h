#ifndef SkAntiRun_DEFINED
#define SkAntiRun_DEFINED

#include "SkBlitter.h"

inline int sk_make_nonzero_neg_one(int x)
{
	return (x | -x) >> 31;
}

#if 0
template <int kShift> class SkAntiRun {
	static U8 coverage_to_alpha(int aa)
	{
		aa <<= 8 - 2*kShift;
		aa -= aa >> (8 - kShift - 1);
		return SkToU8(aa);
	}
public:
	void set(int start, int stop)
	{
		SkASSERT(start >= 0 && stop > start);

#if 1
		int	fb = start & kMask;
		int fe = stop & kMask;
		int	n = (stop >> kShift) - (start >> kShift) - 1;

		if (n < 0)
		{
			fb = fe - fb;
			n = 0;
			fe = 0;
		}
		else
		{
			if (fb == 0)
				n += 1;
			else
				fb = (1 << kShift) - fb;
		}

		fStartAlpha = coverage_to_alpha(fb);
		fMiddleCount = n;
		fStopAlpha = coverage_to_alpha(fe);
#else
		int	x0 = start >> kShift;
		int x1 = (stop - 1) >> kShift;
		int middle = x1 - x0;
		int aa;

		if (middle == 0)
		{
			aa = stop - start;
			aa <<= 8 - 2*kShift;
			aa -= aa >> (8 - kShift - 1);
			SkASSERT(aa > 0 && aa < kMax);
			fStartAlpha = SkToU8(aa);
			fMiddleCount = 0;
			fStopAlpha = 0;
		}
		else
		{
			int aa = start & kMask;
			aa <<= 8 - 2*kShift;
			aa -= aa >> (8 - kShift - 1);
			SkASSERT(aa >= 0 && aa < kMax);
			if (aa)
				fStartAlpha = SkToU8(kMax - aa);
			else
			{
				fStartAlpha = 0;
				middle += 1;
			}
			aa = stop & kMask;
			aa <<= 8 - 2*kShift;
			aa -= aa >> (8 - kShift - 1);
			SkASSERT(aa >= 0 && aa < kMax);
			middle += sk_make_nonzero_neg_one(aa);

			fStopAlpha = SkToU8(aa);
			fMiddleCount = middle;
		}
		SkASSERT(fStartAlpha < kMax);
		SkASSERT(fStopAlpha < kMax);
#endif
	}

	void blit(int x, int y, SkBlitter* blitter)
	{
		S16	runs[2];
		runs[0] = 1;
		runs[1] = 0;

		if (fStartAlpha)
		{
			blitter->blitAntiH(x, y, &fStartAlpha, runs);
			x += 1;
		}
		if (fMiddleCount)
		{
			blitter->blitH(x, y, fMiddleCount);
			x += fMiddleCount;
		}
		if (fStopAlpha)
			blitter->blitAntiH(x, y, &fStopAlpha, runs);
	}

	U8	getStartAlpha() const { return fStartAlpha; }
	int	getMiddleCount() const { return fMiddleCount; }
	U8	getStopAlpha() const { return fStopAlpha; }

private:
	U8	fStartAlpha, fStopAlpha;
	int	fMiddleCount;

	enum {
		kMask = (1 << kShift) - 1,
		kMax = (1 << (8 - kShift)) - 1
	};
};
#endif

////////////////////////////////////////////////////////////////////////////////////

class SkAlphaRuns {
public:
	S16*	fRuns;
	U8*		fAlpha;

	bool	empty() const
	{
		SkASSERT(fRuns[0] > 0);
		return fAlpha[0] == 0 && fRuns[fRuns[0]] == 0;
	}
	void	reset(int width);
	void	add(int x, U8CPU startAlpha, int middleCount, U8CPU stopAlpha, U8CPU maxValue);
	SkDEBUGCODE(void assertValid(int y, int maxStep) const;)
	SkDEBUGCODE(void dump() const;)

	static void Break(S16 runs[], U8 alpha[], int x, int count);
	static void BreakAt(S16 runs[], U8 alpha[], int x)
	{
		while (x > 0)
		{
			int	n = runs[0];
			SkASSERT(n > 0);

			if (x < n)
			{
				alpha[x] = alpha[0];
				runs[0] = SkToS16(x);
				runs[x] = SkToS16(n - x);
				break;
			}
			runs += n;
			alpha += n;
			x -= n;
		}
	}

private:
	SkDEBUGCODE(int fWidth;)
	SkDEBUGCODE(void validate() const;)
};

#endif

