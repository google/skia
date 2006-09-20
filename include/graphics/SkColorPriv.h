#ifndef SkColorPriv_DEFINED
#define SkColorPriv_DEFINED

#include "SkColor.h"

inline unsigned SkAlpha255To256(U8CPU alpha)
{
	SkASSERT(SkToU8(alpha) == alpha);
	return alpha + (alpha >> 7);
}

#define SkAlphaMul(value, alpha256)		((value) * (alpha256) >> 8)

//	The caller may want negative values, so keep all params signed (int)
//	so we don't accidentally slip into unsigned math and lose the sign
//	extension when we shift (in SkAlphaMul)
inline int SkAlphaBlend(int src, int dst, int scale256)
{
	SkASSERT((unsigned)scale256 <= 256);
	return dst + SkAlphaMul(src - dst, scale256);
}

#define SK_R16_BITS		5
#define SK_G16_BITS		6
#define SK_B16_BITS		5

#define SK_R16_SHIFT	(SK_B16_BITS + SK_G16_BITS)
#define SK_G16_SHIFT	(SK_B16_BITS)
#define SK_B16_SHIFT	0

#define SK_R16_MASK		((1 << SK_R16_BITS) - 1)
#define SK_G16_MASK		((1 << SK_G16_BITS) - 1)
#define SK_B16_MASK		((1 << SK_B16_BITS) - 1)

#define SkGetPackedR16(color)	(((unsigned)(color) >> SK_R16_SHIFT) & SK_R16_MASK)
#define SkGetPackedG16(color)	(((unsigned)(color) >> SK_G16_SHIFT) & SK_G16_MASK)
#define SkGetPackedB16(color)	(((unsigned)(color) >> SK_B16_SHIFT) & SK_B16_MASK)

inline uint16_t SkPackRGB16(unsigned r, unsigned g, unsigned b)
{
	SkASSERT(r <= SK_R16_MASK);
	SkASSERT(g <= SK_G16_MASK);
	SkASSERT(b <= SK_B16_MASK);

	return SkToU16((r << SK_R16_SHIFT) | (g << SK_G16_SHIFT) | (b << SK_B16_SHIFT));
}

inline int SkShouldDitherXY(int x, int y)
{
    return (x ^ y) & 1;
}

inline uint16_t SkDitherPack888ToRGB16(U8CPU r, U8CPU g, U8CPU b)
{
    r = ((r << 1) - ((r >> (8 - SK_R16_BITS) << (8 - SK_R16_BITS)) | (r >> SK_R16_BITS))) >> (8 - SK_R16_BITS);
    g = ((g << 1) - ((g >> (8 - SK_G16_BITS) << (8 - SK_G16_BITS)) | (g >> SK_G16_BITS))) >> (8 - SK_G16_BITS);
    b = ((b << 1) - ((b >> (8 - SK_B16_BITS) << (8 - SK_B16_BITS)) | (b >> SK_B16_BITS))) >> (8 - SK_B16_BITS);

   return SkPackRGB16(r, g, b);
}

#define SK_R16_MASK_IN_PLACE		(SK_R16_MASK << SK_R16_SHIFT)
#define SK_G16_MASK_IN_PLACE		(SK_G16_MASK << SK_G16_SHIFT)
#define SK_B16_MASK_IN_PLACE		(SK_B16_MASK << SK_B16_SHIFT)

#define SK_R16B16_MASK_IN_PLACE		(SK_R16_MASK_IN_PLACE | SK_B16_MASK_IN_PLACE)

inline U16CPU SkAlphaMulRGB16(U16CPU c, unsigned scale)
{
#if SK_G16_MASK_IN_PLACE != 0x07E0
	return SkPackRGB16(	SkAlphaMul(SkGetPackedR16(c), scale),
						SkAlphaMul(SkGetPackedG16(c), scale),
						SkAlphaMul(SkGetPackedB16(c), scale));
#else
	scale >>= (8 - SK_G16_BITS);
	uint32_t rb = (c & SK_R16B16_MASK_IN_PLACE) * scale >> SK_G16_BITS;
	uint32_t  g = (c & SK_G16_MASK_IN_PLACE) * scale >> SK_G16_BITS;
	return (g & SK_G16_MASK_IN_PLACE) | (rb & SK_R16B16_MASK_IN_PLACE);
#endif
}

inline U16CPU SkBlendRGB16(U16CPU src, U16CPU dst, unsigned scale)
{
	SkASSERT(scale <= 256);

	return SkPackRGB16(	SkAlphaBlend(SkGetPackedR16(src), SkGetPackedR16(dst), scale),
						SkAlphaBlend(SkGetPackedG16(src), SkGetPackedG16(dst), scale),
						SkAlphaBlend(SkGetPackedB16(src), SkGetPackedB16(dst), scale));
}

/////////////////////////////////////////////////////////////////////////////////////////////

#define SK_A32_BITS		8
#define SK_R32_BITS		8
#define SK_G32_BITS		8
#define SK_B32_BITS		8

#ifdef TEST_INTEL_MAC

#define SK_A32_SHIFT	0
#define SK_R32_SHIFT	8
#define SK_G32_SHIFT	16
#define SK_B32_SHIFT	24

#else

#define SK_A32_SHIFT	24
#define SK_R32_SHIFT	16
#define SK_G32_SHIFT	8
#define SK_B32_SHIFT	0

#endif

#define SK_A32_MASK		((1 << SK_A32_BITS) - 1)
#define SK_R32_MASK		((1 << SK_R32_BITS) - 1)
#define SK_G32_MASK		((1 << SK_G32_BITS) - 1)
#define SK_B32_MASK		((1 << SK_B32_BITS) - 1)

#define SkGetPackedA32(packed)		((uint32_t)((packed) << (24 - SK_A32_SHIFT)) >> 24)
#define SkGetPackedR32(packed)		((uint32_t)((packed) << (24 - SK_R32_SHIFT)) >> 24)
#define SkGetPackedG32(packed)		((uint32_t)((packed) << (24 - SK_G32_SHIFT)) >> 24)
#define SkGetPackedB32(packed)		((uint32_t)((packed) << (24 - SK_B32_SHIFT)) >> 24)

inline SkPMColor SkPackARGB32(U8CPU a, U8CPU r, U8CPU g, U8CPU b)
{
	SkASSERT(a <= SK_A32_MASK);
	SkASSERT(r <= a);
	SkASSERT(g <= a);
	SkASSERT(b <= a);

	return (a << SK_A32_SHIFT) | (r << SK_R32_SHIFT) | (g << SK_G32_SHIFT) | (b << SK_B32_SHIFT);
}

inline uint32_t SkAlphaMulQ(uint32_t c, unsigned scale)
{
	uint32_t rb = ((c & 0xFF00FF) * scale) >> 8;
	uint32_t ag = ((c >> 8) & 0xFF00FF) * scale;
	return (rb & 0xFF00FF) | (ag & ~0xFF00FF);
}

inline SkPMColor SkPMSrcOver(SkPMColor src, SkPMColor dst)
{
	return src + SkAlphaMulQ(dst, SkAlpha255To256(255 - SkGetPackedA32(src)));
}

inline SkPMColor SkBlendARGB32(SkPMColor src, SkPMColor dst, U8CPU aa)
{
	SkASSERT((unsigned)aa <= 255);

	unsigned src_scale = SkAlpha255To256(aa);
	unsigned dst_scale = SkAlpha255To256(255 - SkAlphaMul(SkGetPackedA32(src), src_scale));

	return SkAlphaMulQ(src, src_scale) + SkAlphaMulQ(dst, dst_scale);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Convert a 32bit pixel to a 16bit pixel (no dither)

#define SkR32ToR16(r)	((unsigned)(r) >> (SK_R32_BITS - SK_R16_BITS))
#define SkG32ToG16(g)	((unsigned)(g) >> (SK_G32_BITS - SK_G16_BITS))
#define SkB32ToB16(b)	((unsigned)(b) >> (SK_B32_BITS - SK_B16_BITS))

#define SkPacked32ToR16(c)	(((unsigned)(c) >> (SK_R32_SHIFT + SK_R32_BITS - SK_R16_BITS)) & SK_R16_MASK)
#define SkPacked32ToG16(c)	(((unsigned)(c) >> (SK_G32_SHIFT + SK_G32_BITS - SK_G16_BITS)) & SK_G16_MASK)
#define SkPacked32ToB16(c)	(((unsigned)(c) >> (SK_B32_SHIFT + SK_B32_BITS - SK_B16_BITS)) & SK_B16_MASK)

inline U16CPU SkPixel32ToPixel16(SkPMColor src)
{
#if 0
	return	(SkPacked32ToR16(src) << SK_R16_SHIFT) |
			(SkPacked32ToG16(src) << SK_G16_SHIFT) |
			(SkPacked32ToB16(src) << SK_B16_SHIFT);
#else	// only works if the components are in the same order in both formats (i.e. foo32_shift >= foo16_shift)
	return	((src >> (SK_R32_SHIFT + SK_R32_BITS - SK_R16_BITS - SK_R16_SHIFT)) & (SK_R16_MASK << SK_R16_SHIFT)) |
			((src >> (SK_G32_SHIFT + SK_G32_BITS - SK_G16_BITS - SK_G16_SHIFT)) & (SK_G16_MASK << SK_G16_SHIFT)) |
			((src >> (SK_B32_SHIFT + SK_B32_BITS - SK_B16_BITS - SK_B16_SHIFT)) & (SK_B16_MASK << SK_B16_SHIFT));
#endif
}

#define SkPixel32ToPixel16_ToU16(src)	SkToU16(SkPixel32ToPixel16(src))

////////////////////////////////////////////////////////////////////////////////////////////
// Convert a 16bit pixel to a 32bit pixel

inline unsigned SkR16ToR32(unsigned r)
{
	return (r << (8 - SK_R16_BITS)) | (r >> (2 * SK_R16_BITS - 8));
}
inline unsigned SkG16ToG32(unsigned g)
{
	return (g << (8 - SK_G16_BITS)) | (g >> (2 * SK_G16_BITS - 8));
}
inline unsigned SkB16ToB32(unsigned b)
{
	return (b << (8 - SK_B16_BITS)) | (b >> (2 * SK_B16_BITS - 8));
}

#define SkPacked16ToR32(c)		SkR16ToR32(SkGetPackedR16(c))
#define SkPacked16ToG32(c)		SkG16ToG32(SkGetPackedG16(c))
#define SkPacked16ToB32(c)		SkB16ToB32(SkGetPackedB16(c))

inline SkPMColor SkPixel16ToPixel32(U16CPU src)
{
	SkASSERT(src == SkToU16(src));

	unsigned	r = SkPacked16ToR32(src);
	unsigned	g = SkPacked16ToG32(src);
	unsigned	b = SkPacked16ToB32(src);

	SkASSERT((r >> (8 - SK_R16_BITS)) == SkGetPackedR16(src));
	SkASSERT((g >> (8 - SK_G16_BITS)) == SkGetPackedG16(src));
	SkASSERT((b >> (8 - SK_B16_BITS)) == SkGetPackedB16(src));

	return SkPackARGB32(0xFF, r, g, b);
}

#endif

