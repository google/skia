#include "SkCoreBlitters.h"

SkA1_Blitter::SkA1_Blitter(const SkBitmap& device, const SkPaint& paint)
	: fDevice(device)
{
	fSrcA = SkToU8(SkColorGetA(paint.getColor()));
}

void SkA1_Blitter::blitH(int x, int y, int width)
{
	SkASSERT(x >= 0 && y >= 0 && (unsigned)(x + width) <= fDevice.width());

	if (fSrcA <= 0x7F)
		return;

	U8* dst = fDevice.getAddr1(x, y);
	int	right = x + width;

	int left_mask = 0xFF >> (x & 7);
	int rite_mask = 0xFF << (8 - (right & 7));
	int full_runs = (right >> 3) - ((x + 7) >> 3);

	// check for empty right mask, so we don't read off the end (or go slower than we need to)
	if (rite_mask == 0)
	{
		SkASSERT(full_runs >= 0);
		full_runs -= 1;
		rite_mask = 0xFF;
	}
	if (left_mask == 0xFF)
		full_runs -= 1;

	if (full_runs < 0)
	{
		SkASSERT((left_mask & rite_mask) != 0);
		*dst |= (left_mask & rite_mask);
	}
	else
	{
		*dst++ |= left_mask;
		memset(dst, 0xFF, full_runs);
		dst += full_runs;
		*dst |= rite_mask;
	}
}

