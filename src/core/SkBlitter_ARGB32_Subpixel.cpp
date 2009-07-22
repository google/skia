/* libs/graphics/sgl/SkBlitter_ARGB32_Subpixel.cpp
**
** Copyright 2009, The Android Open Source Project
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

/* LCD blend functions:

   These functions take an alpha pixel of the following form:
      red, green, blue -> an alpha value for the given colour component.
      alpha -> the max of the red, green and blue alpha values.

   These alpha pixels result from subpixel renderering. The R/G/B values have
   already been corrected for RGB/BGR element ordering.

   The alpha pixel is blended with an original pixel and a source colour,
   resulting in a new pixel value.
*/

#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkMask.h"
#include "SkRect.h"

namespace skia_blitter_support {

/** Given a clip region which describes the desired location of a glyph and a
    bitmap to which an LCD glyph is to be blitted, return a pointer to the
    SkBitmap's pixels and output width and height adjusts for the glyph as well
    as a pointer into the glyph.

    Recall that LCD glyphs have extra rows (vertical mode) or columns
    (horizontal mode) at the edges as a result of low-pass filtering. If we
    wanted to put a glyph on the hard-left edge of bitmap, we would have to know
    to start one pixel into the glyph, as well as to only add 1 to the recorded
    glyph width etc. This function encapsulates that behaviour.

    @param mask    The glyph to be blitted.
    @param clip    The clip region describing the desired location of the glyph.
    @param device  The SkBitmap target for the blit.
    @param widthAdjustment  (output) a number to add to the glyph's nominal width.
    @param heightAdjustment (output) a number to add to the glyph's nominal width.
    @param alpha32 (output) a pointer into the 32-bit subpixel alpha data for the glyph
*/
uint32_t* adjustForSubpixelClip(const SkMask& mask,
                                const SkIRect& clip, const SkBitmap& device,
                                int* widthAdjustment, int* heightAdjustment,
                                const uint32_t** alpha32) {
    const bool lcdMode = mask.fFormat == SkMask::kHorizontalLCD_Format;
    const bool verticalLCDMode = mask.fFormat == SkMask::kVerticalLCD_Format;
    const int  leftOffset = clip.fLeft > 0 ? lcdMode : 0;
    const int  topOffset = clip.fTop > 0 ? verticalLCDMode : 0;
    const int  rightOffset = lcdMode && clip.fRight < device.width();
    const int  bottomOffset = verticalLCDMode && clip.fBottom < device.height();

    uint32_t* device32 = device.getAddr32(clip.fLeft - leftOffset, clip.fTop - topOffset);
    *alpha32 = mask.getAddrLCD(clip.fLeft + (lcdMode && !leftOffset),
                               clip.fTop + (verticalLCDMode && !topOffset));

    *widthAdjustment = leftOffset + rightOffset;
    *heightAdjustment = topOffset + bottomOffset;

    return device32;
}

uint32_t BlendLCDPixelWithColor(const uint32_t alphaPixel, const uint32_t originalPixel,
                                const uint32_t sourcePixel) {
    unsigned alphaRed = SkAlpha255To256(SkGetPackedR32(alphaPixel));
    unsigned alphaGreen = SkAlpha255To256(SkGetPackedG32(alphaPixel));
    unsigned alphaBlue = SkAlpha255To256(SkGetPackedB32(alphaPixel));

    unsigned sourceRed = SkGetPackedR32(sourcePixel);
    unsigned sourceGreen = SkGetPackedG32(sourcePixel);
    unsigned sourceBlue = SkGetPackedB32(sourcePixel);
    unsigned sourceAlpha = SkAlpha255To256(SkGetPackedA32(sourcePixel));

    alphaRed = (alphaRed * sourceAlpha) >> 8;
    alphaGreen = (alphaGreen * sourceAlpha) >> 8;
    alphaBlue = (alphaBlue * sourceAlpha) >> 8;
    unsigned alphaAlpha = SkMax32(SkMax32(alphaRed, alphaBlue), alphaGreen);

    unsigned originalRed = SkGetPackedR32(originalPixel);
    unsigned originalGreen = SkGetPackedG32(originalPixel);
    unsigned originalBlue = SkGetPackedB32(originalPixel);
    unsigned originalAlpha = SkGetPackedA32(originalPixel);

    return SkPackARGB32(SkMin32(255u, alphaAlpha + originalAlpha),
                        ((sourceRed * alphaRed) >> 8) + ((originalRed * (256 - alphaRed)) >> 8),
                        ((sourceGreen * alphaGreen) >> 8) + ((originalGreen * (256 - alphaGreen)) >> 8),
                        ((sourceBlue * alphaBlue) >> 8) + ((originalBlue * (256 - alphaBlue)) >> 8));

}

uint32_t BlendLCDPixelWithOpaqueColor(const uint32_t alphaPixel, const uint32_t originalPixel,
                                      const uint32_t sourcePixel) {
    unsigned alphaRed = SkAlpha255To256(SkGetPackedR32(alphaPixel));
    unsigned alphaGreen = SkAlpha255To256(SkGetPackedG32(alphaPixel));
    unsigned alphaBlue = SkAlpha255To256(SkGetPackedB32(alphaPixel));
    unsigned alphaAlpha = SkGetPackedA32(alphaPixel);

    unsigned sourceRed = SkGetPackedR32(sourcePixel);
    unsigned sourceGreen = SkGetPackedG32(sourcePixel);
    unsigned sourceBlue = SkGetPackedB32(sourcePixel);

    unsigned originalRed = SkGetPackedR32(originalPixel);
    unsigned originalGreen = SkGetPackedG32(originalPixel);
    unsigned originalBlue = SkGetPackedB32(originalPixel);
    unsigned originalAlpha = SkGetPackedA32(originalPixel);

    return SkPackARGB32(SkMin32(255u, alphaAlpha + originalAlpha),
                        ((sourceRed * alphaRed) >> 8) + ((originalRed * (256 - alphaRed)) >> 8),
                        ((sourceGreen * alphaGreen) >> 8) + ((originalGreen * (256 - alphaGreen)) >> 8),
                        ((sourceBlue * alphaBlue) >> 8) + ((originalBlue * (256 - alphaBlue)) >> 8));
}

uint32_t BlendLCDPixelWithBlack(const uint32_t alphaPixel, const uint32_t originalPixel) {
    unsigned alphaRed = SkAlpha255To256(SkGetPackedR32(alphaPixel));
    unsigned alphaGreen = SkAlpha255To256(SkGetPackedG32(alphaPixel));
    unsigned alphaBlue = SkAlpha255To256(SkGetPackedB32(alphaPixel));
    unsigned alphaAlpha = SkGetPackedA32(alphaPixel);

    unsigned originalRed = SkGetPackedR32(originalPixel);
    unsigned originalGreen = SkGetPackedG32(originalPixel);
    unsigned originalBlue = SkGetPackedB32(originalPixel);
    unsigned originalAlpha = SkGetPackedA32(originalPixel);

    return SkPackARGB32(SkMin32(255u, alphaAlpha + originalAlpha),
                        (originalRed * (256 - alphaRed)) >> 8,
                        (originalGreen * (256 - alphaGreen)) >> 8,
                        (originalBlue * (256 - alphaBlue)) >> 8);
}

}  // namespace skia_blitter_support
