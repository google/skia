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

#include "SkColorPriv.h"

namespace skia_blitter_support {

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
