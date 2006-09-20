#include "SkSpriteBlitter.h"
#include "SkTemplates.h"
#include "SkUtils.h"
#include "SkColorPriv.h"

#define D32_S32A_Opaque_Pixel(dst, sc)																\
do {																								\
	if (sc)																							\
	{																								\
		unsigned srcA = SkGetPackedA32(sc);															\
		U32 result = sc;													\
		if (srcA != 0xFF)																			\
			result += SkAlphaMulQ(*dst, SkAlpha255To256(255 - srcA));							\
		*dst = result;																				\
	}																								\
} while (0)

#define SkSPRITE_CLASSNAME					Sprite_D32_S32A_Opaque
#define SkSPRITE_ARGS
#define SkSPRITE_FIELDS
#define SkSPRITE_INIT
#define SkSPRITE_DST_TYPE					uint32_t
#define SkSPRITE_SRC_TYPE					uint32_t
#define SkSPRITE_DST_GETADDR				getAddr32
#define SkSPRITE_SRC_GETADDR				getAddr32
#define SkSPRITE_PREAMBLE(srcBM, x, y)
#define SkSPRITE_BLIT_PIXEL(dst, src)		D32_S32A_Opaque_Pixel(dst, src)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)
#include "SkSpriteBlitterTemplate.h"

class Sprite_D32_S32_Opaque : public SkSpriteBlitter {
public:
	Sprite_D32_S32_Opaque(const SkBitmap& source) : SkSpriteBlitter(source) {}

	virtual void blitRect(int x, int y, int width, int height)
	{
		SkASSERT(width > 0 && height > 0);
		uint32_t*		dst = fDevice->getAddr32(x, y);
		const uint32_t*	src = fSource->getAddr32(x - fLeft, y - fTop);
		unsigned		dstRB = fDevice->rowBytes();
		unsigned		srcRB = fSource->rowBytes();
        size_t          size = width * sizeof(uint32_t);

		do {
            memcpy(dst, src, size);
			dst = (uint32_t*)((char*)dst + dstRB);
			src = (const uint32_t*)((const char*)src + srcRB);
		} while (--height != 0);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include "SkTemplatesPriv.h"

SkSpriteBlitter* SkSpriteBlitter::ChooseD32(const SkBitmap& source, SkXfermode* mode, U8 alpha,
											void* storage, size_t storageSize)
{
	SkSpriteBlitter* blitter = nil;

	switch (source.getConfig()) {
	case SkBitmap::kARGB_8888_Config:
		if (mode == nil)
		{
			if (alpha == 255)
            {
                if (source.isOpaque())
                    SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S32_Opaque, storage, storageSize, (source));
                else
                    SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S32A_Opaque, storage, storageSize, (source));
            }
		}
		break;
	default:
		break;
	}
	return blitter;
}

