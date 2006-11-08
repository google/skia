/* libs/graphics/sgl/SkSpriteBlitterTemplate.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/


class SkSPRITE_CLASSNAME : public SkSpriteBlitter {
public:
    SkSPRITE_CLASSNAME(const SkBitmap& source SkSPRITE_ARGS)
        : SkSpriteBlitter(source)
    {
        SkSPRITE_INIT
    }
    virtual void blitRect(int x, int y, int width, int height)
    {
        SkASSERT(width > 0 && height > 0);
        int srcX = x - fLeft;
        int srcY = y - fTop;
        SkSPRITE_DST_TYPE*          dst = fDevice->SkSPRITE_DST_GETADDR(x, y);
        const SkSPRITE_SRC_TYPE*    src = fSource->SkSPRITE_SRC_GETADDR(srcX, srcY);
        unsigned                    dstRB = fDevice->rowBytes();
        unsigned                    srcRB = fSource->rowBytes();

        SkDEBUGCODE((void)fDevice->SkSPRITE_DST_GETADDR(x + width - 1, y + height - 1);)
        SkDEBUGCODE((void)fSource->SkSPRITE_SRC_GETADDR(srcX + width  - 1, srcY + height - 1);)

        SkSPRITE_PREAMBLE((*fSource), srcX, srcY);

        do {
            SkSPRITE_DST_TYPE* d = dst;
            const SkSPRITE_SRC_TYPE* s = src;
#ifdef SkSPRITE_BEGIN_ROW
            SkSPRITE_BEGIN_ROW
#endif
            int w = width;
            do {
                SkSPRITE_SRC_TYPE sc = *s++;
                SkSPRITE_BLIT_PIXEL(d, sc);
                d += 1;
            } while (--w != 0);
            dst = (SkSPRITE_DST_TYPE*)((char*)dst + dstRB);
            src = (const SkSPRITE_SRC_TYPE*)((const char*)src + srcRB);
            SkSPRITE_NEXT_ROW
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

