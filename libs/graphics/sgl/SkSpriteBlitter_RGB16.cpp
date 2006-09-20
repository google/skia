#include "SkSpriteBlitter.h"
#include "SkTemplates.h"
#include "SkUtils.h"
#include "SkColorPriv.h"

#define D16_S32A_Opaque_Pixel(dst, sc)																\
do {																								\
	if (sc)																							\
	{																								\
		unsigned srcA = SkGetPackedA32(sc);															\
		unsigned result = SkPixel32ToPixel16(sc);													\
		if (srcA != 0xFF)																			\
			result += SkAlphaMulRGB16(*dst, SkAlpha255To256(255 - srcA));							\
		*dst = SkToU16(result);																				\
	}																								\
} while (0)

#define SkSPRITE_CLASSNAME					Sprite_D16_S32A_Opaque
#define SkSPRITE_ARGS
#define SkSPRITE_FIELDS
#define SkSPRITE_INIT
#define SkSPRITE_DST_TYPE					uint16_t
#define SkSPRITE_SRC_TYPE					uint32_t
#define SkSPRITE_DST_GETADDR				getAddr16
#define SkSPRITE_SRC_GETADDR				getAddr32
#define SkSPRITE_PREAMBLE(srcBM, x, y)
#define SkSPRITE_BLIT_PIXEL(dst, src)		D16_S32A_Opaque_Pixel(dst, src)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)
#include "SkSpriteBlitterTemplate.h"

static inline void D16_S32A_Blend_Pixel_helper(U16* dst, U32 sc, unsigned src_scale)
{
	uint16_t dc = *dst;
	unsigned sa = SkGetPackedA32(sc);
	unsigned dr, dg, db;

	if (sa == 255)
	{
		dr = SkAlphaBlend(SkPacked32ToR16(sc), SkGetPackedR16(dc), src_scale);
		dg = SkAlphaBlend(SkPacked32ToG16(sc), SkGetPackedG16(dc), src_scale);
		db = SkAlphaBlend(SkPacked32ToB16(sc), SkGetPackedB16(dc), src_scale);
	}
	else
	{
		unsigned dst_scale = 255 - SkAlphaMul(sa, src_scale);
		dr = (SkPacked32ToR16(sc) * src_scale + SkGetPackedR16(dc) * dst_scale) >> 8;
		dg = (SkPacked32ToG16(sc) * src_scale + SkGetPackedG16(dc) * dst_scale) >> 8;
		db = (SkPacked32ToB16(sc) * src_scale + SkGetPackedB16(dc) * dst_scale) >> 8;
	}
	*dst = SkPackRGB16(dr, dg, db);
}

#define D16_S32A_Blend_Pixel(dst, sc, src_scale)	do { if (sc) D16_S32A_Blend_Pixel_helper(dst, sc, src_scale); } while (0)


#define SkSPRITE_CLASSNAME					Sprite_D16_S32A_Blend
#define SkSPRITE_ARGS						, U8 alpha
#define SkSPRITE_FIELDS						U8 fSrcAlpha;
#define SkSPRITE_INIT						fSrcAlpha = alpha;
#define SkSPRITE_DST_TYPE					uint16_t
#define SkSPRITE_SRC_TYPE					uint32_t
#define SkSPRITE_DST_GETADDR				getAddr16
#define SkSPRITE_SRC_GETADDR				getAddr32
#define SkSPRITE_PREAMBLE(srcBM, x, y)		unsigned src_scale = SkAlpha255To256(fSrcAlpha);
#define SkSPRITE_BLIT_PIXEL(dst, src)		D16_S32A_Blend_Pixel(dst, src, src_scale)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)
#include "SkSpriteBlitterTemplate.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

#define SkSPRITE_CLASSNAME					Sprite_D16_S32_Opaque
#define SkSPRITE_ARGS
#define SkSPRITE_FIELDS
#define SkSPRITE_INIT
#define SkSPRITE_DST_TYPE					uint16_t
#define SkSPRITE_SRC_TYPE					uint32_t
#define SkSPRITE_DST_GETADDR				getAddr16
#define SkSPRITE_SRC_GETADDR				getAddr32
#define SkSPRITE_PREAMBLE(srcBM, x, y)
#define SkSPRITE_BLIT_PIXEL(dst, src)		*dst = SkPixel32ToPixel16_ToU16(src)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)
#include "SkSpriteBlitterTemplate.h"

#if 1
#define D16_S32_Blend_Pixel(dst, sc, scale)												\
do {																					\
	U16 dc = *dst;																		\
	*dst = SkPackRGB16(	SkAlphaBlend(SkPacked32ToR16(sc), SkGetPackedR16(dc), scale),	\
						SkAlphaBlend(SkPacked32ToG16(sc), SkGetPackedG16(dc), scale),	\
						SkAlphaBlend(SkPacked32ToB16(sc), SkGetPackedB16(dc), scale));	\
} while (0)
#else
static inline void D16_S32_Blend_Pixel(uint16_t* dst, uint32_t sc, int scale)
{
	U16 dc = *dst;
	*dst = SkPackRGB16(	SkAlphaBlend(SkPacked32ToR16(sc), SkGetPackedR16(dc), scale),
						SkAlphaBlend(SkPacked32ToG16(sc), SkGetPackedG16(dc), scale),
						SkAlphaBlend(SkPacked32ToB16(sc), SkGetPackedB16(dc), scale));
}
#endif

#define SkSPRITE_CLASSNAME					Sprite_D16_S32_Blend
#define SkSPRITE_ARGS						, uint8_t alpha
#define SkSPRITE_FIELDS						uint8_t	fSrcAlpha;
#define SkSPRITE_INIT						fSrcAlpha = alpha;
#define SkSPRITE_DST_TYPE					uint16_t
#define SkSPRITE_SRC_TYPE					uint32_t
#define SkSPRITE_DST_GETADDR				getAddr16
#define SkSPRITE_SRC_GETADDR				getAddr32
#define SkSPRITE_PREAMBLE(srcBM, x, y)		int src_scale = SkAlpha255To256(fSrcAlpha);
#define SkSPRITE_BLIT_PIXEL(dst, src)		D16_S32_Blend_Pixel(dst, src, src_scale)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)
#include "SkSpriteBlitterTemplate.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

class Sprite_D16_S16_Opaque : public SkSpriteBlitter {
public:
	Sprite_D16_S16_Opaque(const SkBitmap& source)
		: SkSpriteBlitter(source) {}

	// overrides
	virtual void blitRect(int x, int y, int width, int height)
	{
		uint16_t*		dst = fDevice->getAddr16(x, y);
		const uint16_t*	src = fSource->getAddr16(x - fLeft, y - fTop);
		unsigned        dstRB = fDevice->rowBytes();
		unsigned        srcRB = fSource->rowBytes();

		while (--height >= 0)
		{
			memcpy(dst, src, width << 1);
			dst = (uint16_t*)((char*)dst + dstRB);
			src = (const uint16_t*)((const char*)src + srcRB);
		}
	}
};

#define D16_S16_Blend_Pixel(dst, sc, scale)				\
	do {												\
		uint16_t dc = *dst;								\
		*dst = SkToU16(SkBlendRGB16(sc, dc, scale));	\
	} while (0)

#define SkSPRITE_CLASSNAME					Sprite_D16_S16_Blend
#define SkSPRITE_ARGS						, U8 alpha
#define SkSPRITE_FIELDS						U8	fSrcAlpha;
#define SkSPRITE_INIT						fSrcAlpha = alpha;
#define SkSPRITE_DST_TYPE					uint16_t
#define SkSPRITE_SRC_TYPE					uint16_t
#define SkSPRITE_DST_GETADDR				getAddr16
#define SkSPRITE_SRC_GETADDR				getAddr16
#define SkSPRITE_PREAMBLE(srcBM, x, y)		int scale = SkAlpha255To256(fSrcAlpha);
#define SkSPRITE_BLIT_PIXEL(dst, src)		D16_S16_Blend_Pixel(dst, src, scale)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)
#include "SkSpriteBlitterTemplate.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

#define SkSPRITE_CLASSNAME					Sprite_D16_SIndex8A_Opaque
#define SkSPRITE_ARGS
#define SkSPRITE_FIELDS
#define SkSPRITE_INIT
#define SkSPRITE_DST_TYPE					uint16_t
#define SkSPRITE_SRC_TYPE					uint8_t
#define SkSPRITE_DST_GETADDR				getAddr16
#define SkSPRITE_SRC_GETADDR				getAddr8
#define SkSPRITE_PREAMBLE(srcBM, x, y)		const SkPMColor* ctable = srcBM.getColorTable()->lockColors()
#define SkSPRITE_BLIT_PIXEL(dst, src)		D16_S32A_Opaque_Pixel(dst, ctable[src])
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)			srcBM.getColorTable()->unlockColors(false)
#include "SkSpriteBlitterTemplate.h"

#define SkSPRITE_CLASSNAME					Sprite_D16_SIndex8A_Blend
#define SkSPRITE_ARGS						, uint8_t alpha
#define SkSPRITE_FIELDS						uint8_t fSrcAlpha;
#define SkSPRITE_INIT						fSrcAlpha = alpha;
#define SkSPRITE_DST_TYPE					uint16_t
#define SkSPRITE_SRC_TYPE					uint8_t
#define SkSPRITE_DST_GETADDR				getAddr16
#define SkSPRITE_SRC_GETADDR				getAddr8
#define SkSPRITE_PREAMBLE(srcBM, x, y)		const SkPMColor* ctable = srcBM.getColorTable()->lockColors(); unsigned src_scale = SkAlpha255To256(fSrcAlpha);
#define SkSPRITE_BLIT_PIXEL(dst, src)		D16_S32A_Blend_Pixel(dst, ctable[src], src_scale)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)			srcBM.getColorTable()->unlockColors(false);
#include "SkSpriteBlitterTemplate.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

#define SkSPRITE_CLASSNAME					Sprite_D16_SIndex8_Opaque
#define SkSPRITE_ARGS
#define SkSPRITE_FIELDS
#define SkSPRITE_INIT
#define SkSPRITE_DST_TYPE					uint16_t
#define SkSPRITE_SRC_TYPE					uint8_t
#define SkSPRITE_DST_GETADDR				getAddr16
#define SkSPRITE_SRC_GETADDR				getAddr8
#define SkSPRITE_PREAMBLE(srcBM, x, y)		const uint16_t* ctable = srcBM.getColorTable()->lock16BitCache()
#define SkSPRITE_BLIT_PIXEL(dst, src)		*dst = ctable[src]
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)			srcBM.getColorTable()->unlock16BitCache()
#include "SkSpriteBlitterTemplate.h"

#define SkSPRITE_CLASSNAME					Sprite_D16_SIndex8_Blend
#define SkSPRITE_ARGS						, uint8_t alpha
#define SkSPRITE_FIELDS						uint8_t fSrcAlpha;
#define SkSPRITE_INIT						fSrcAlpha = alpha;
#define SkSPRITE_DST_TYPE					uint16_t
#define SkSPRITE_SRC_TYPE					uint8_t
#define SkSPRITE_DST_GETADDR				getAddr16
#define SkSPRITE_SRC_GETADDR				getAddr8
#define SkSPRITE_PREAMBLE(srcBM, x, y)		const uint16_t* ctable = srcBM.getColorTable()->lock16BitCache(); unsigned src_scale = SkAlpha255To256(fSrcAlpha);
#define SkSPRITE_BLIT_PIXEL(dst, src)		D16_S16_Blend_Pixel(dst, ctable[src], src_scale)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)			srcBM.getColorTable()->unlock16BitCache();
#include "SkSpriteBlitterTemplate.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include "SkTemplatesPriv.h"

SkSpriteBlitter* SkSpriteBlitter::ChooseD16(const SkBitmap& source, SkXfermode* mode, U8 alpha,
											void* storage, size_t storageSize)
{
	SkSpriteBlitter* blitter = nil;

	switch (source.getConfig()) {
	case SkBitmap::kARGB_8888_Config:
        if (mode == nil)
        {
            bool srcIsOpaque = source.isOpaque();
            if (alpha == 255)
            {
                if (srcIsOpaque)
                    SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S32_Opaque, storage, storageSize, (source));
                else
                    SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S32A_Opaque, storage, storageSize, (source));
            }
            else
            {
                if (srcIsOpaque)
                    SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S32_Blend, storage, storageSize, (source, alpha));
                else
                    SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S32A_Blend, storage, storageSize, (source, alpha));
            }
        }
		break;
	case SkBitmap::kRGB_565_Config:
#ifdef SK_SUPPORT_16_8_BITMAP
		if (source.getA8Plane())
		{
			if (mode == nil)
			{
				if (alpha == 255)
					SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S816_Opaque, storage, storageSize, (source));
				else
					SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S816_Blend, storage, storageSize, (source, alpha));
			}
		}
		else
#endif
		if (mode == nil)
		{
			if (alpha == 255)
				SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S16_Opaque, storage, storageSize, (source));
			else
				SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S16_Blend, storage, storageSize, (source, alpha));
		}
		break;
	case SkBitmap::kIndex8_Config:
		if (mode == nil)
		{
			if (source.getColorTable()->getFlags() & SkColorTable::kColorsAreOpaque_Flag)
			{
				if (alpha == 255)
					SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_SIndex8_Opaque, storage, storageSize, (source));
				else
					SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_SIndex8_Blend, storage, storageSize, (source, alpha));
			}
			else
			{
				if (alpha == 255)
					SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_SIndex8A_Opaque, storage, storageSize, (source));
				else
					SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_SIndex8A_Blend, storage, storageSize, (source, alpha));
			}
		}
		break;
	default:
		break;
	}
	return blitter;
}

