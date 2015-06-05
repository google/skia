/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

class SkSPRITE_CLASSNAME : public SkSpriteBlitter {
public:
    SkSPRITE_CLASSNAME(const SkPixmap& source SkSPRITE_ARGS) : SkSpriteBlitter(source) {
        SkSPRITE_INIT
    }

    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(width > 0 && height > 0);
        int srcX = x - fLeft;
        int srcY = y - fTop;
        SkSPRITE_DST_TYPE* SK_RESTRICT dst =fDst.SkSPRITE_DST_GETADDR(x, y);
        const SkSPRITE_SRC_TYPE* SK_RESTRICT src = fSource.SkSPRITE_SRC_GETADDR(srcX, srcY);
        size_t dstRB = fDst.rowBytes();
        size_t srcRB = fSource.rowBytes();

        SkDEBUGCODE((void)fDst.SkSPRITE_DST_GETADDR(x + width - 1, y + height - 1);)
        SkDEBUGCODE((void)fSource.SkSPRITE_SRC_GETADDR(srcX + width  - 1, srcY + height - 1);)

        SkSPRITE_PREAMBLE(fSource, srcX, srcY);

        do {
            SkSPRITE_DST_TYPE* d = dst;
            const SkSPRITE_SRC_TYPE* s = src;
#ifdef SkSPRITE_BEGIN_ROW
            SkSPRITE_BEGIN_ROW
#endif

#ifdef SkSPRITE_ROW_PROC
            SkSPRITE_ROW_PROC(d, s, width, x, y);
#else
            int w = width;
            do {
                SkSPRITE_SRC_TYPE sc = *s++;
                SkSPRITE_BLIT_PIXEL(d, sc);
                d += 1;
            } while (--w != 0);
#endif
            dst = (SkSPRITE_DST_TYPE* SK_RESTRICT)((char*)dst + dstRB);
            src = (const SkSPRITE_SRC_TYPE* SK_RESTRICT)((const char*)src + srcRB);
            SkSPRITE_NEXT_ROW
#ifdef SkSPRITE_ROW_PROC
            y += 1;
#endif
        } while (--height != 0);

        SkSPRITE_POSTAMBLE((*fSource));
    }

private:
    SkSPRITE_FIELDS
};

#undef SkSPRITE_BLIT_PIXEL
#undef SkSPRITE_CLASSNAME
#undef SkSPRITE_DST_TYPE
#undef SkSPRITE_SRC_TYPE
#undef SkSPRITE_DST_GETADDR
#undef SkSPRITE_SRC_GETADDR
#undef SkSPRITE_PREAMBLE
#undef SkSPRITE_POSTAMBLE
#undef SkSPRITE_ARGS
#undef SkSPRITE_FIELDS
#undef SkSPRITE_INIT
#undef SkSPRITE_NEXT_ROW
#undef SkSPRITE_BEGIN_ROW

#ifdef SkSPRITE_ROW_PROC
    #undef SkSPRITE_ROW_PROC
#endif
