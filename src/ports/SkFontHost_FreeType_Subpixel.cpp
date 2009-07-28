/* libs/graphics/ports/SkFontHost_FreeType_Subpixel.cpp
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

/* This file contains functions for converting Freetype's subpixel output
   formats into the format used by SkMask for subpixel masks. See the comments
   in SkMask.h for details on the format.
*/

#include "SkColorPriv.h"
#include "SkFontHost.h"
#include "SkMask.h"
#include "SkScalerContext.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CONFIG_OPTIONS_H

#if 0
// Also include the files by name for build tools which require this.
#include <freetype/freetype.h>
#endif

namespace skia_freetype_support {

#if defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
static const int kFilterBorderWidth = 3;
#else
static const int kFilterBorderWidth = 0;
#endif

void CopyFreetypeBitmapToLCDMask(const SkGlyph& dest, const FT_Bitmap& source)
{
    // |source| has three alpha values per pixel and has an extra column at the
    // left and right edges (if FT_CONFIG_OPTION_SUBPIXEL_RENDERING).

    //                    ----- <--- a single pixel in the output
    // source .oOo..      |.oO|o..
    //        .OOO..      -----
    //        .oOo.. -->   .OO O..
    //                     .oO o..

    uint8_t* output = reinterpret_cast<uint8_t*>(dest.fImage);
    const unsigned outputPitch = SkAlign4((source.width - 2 * kFilterBorderWidth) / 3);
    const uint8_t* input = source.buffer;

    // First we calculate the A8 mask.
    for (int y = 0; y < source.rows; ++y) {
        const uint8_t* inputRow = input;
        uint8_t* outputRow = output;
        inputRow += kFilterBorderWidth;  // skip the extra column on the left
        for (int x = kFilterBorderWidth; x < source.width - kFilterBorderWidth; x += 3) {
            const uint8_t averageAlpha = (static_cast<unsigned>(inputRow[0]) + inputRow[1] + inputRow[2] + 1) / 3;
            *outputRow++ = averageAlpha;
            inputRow += 3;
        }

        input += source.pitch;
        output += outputPitch;
    }

    // Align the 32-bit plane on a word boundary
    uint32_t* output32 = (uint32_t*) SkAlign4((uintptr_t) output);

    // Now we build the 32-bit alpha mask and RGB order correct.
    const int isBGR = SkFontHost::GetSubpixelOrder() == SkFontHost::kBGR_LCDOrder;
    input = source.buffer;

    for (int y = 0; y < source.rows; ++y) {
        const uint8_t* inputRow = input;
        if (!kFilterBorderWidth)
          *output32++ = SkPackARGB32(0, 0, 0, 0);
        for (int x = 0; x < source.width; x += 3) {
            const uint8_t alphaRed = isBGR ? inputRow[2] : inputRow[0];
            const uint8_t alphaGreen = inputRow[1];
            const uint8_t alphaBlue = isBGR ? inputRow[0] : inputRow[2];
            const uint8_t maxAlpha = SkMax32(alphaRed, SkMax32(alphaGreen, alphaBlue));
            *output32++ = SkPackARGB32(maxAlpha, alphaRed, alphaGreen, alphaBlue);

            inputRow += 3;
        }
        if (!kFilterBorderWidth)
          *output32++ = SkPackARGB32(0, 0, 0, 0);

        input += source.pitch;
    }
}

void CopyFreetypeBitmapToVerticalLCDMask(const SkGlyph& dest, const FT_Bitmap& source)
{
    // |source| has three times as many rows as normal, and an extra triple on the
    // top and bottom (if FT_CONFIG_OPTION_SUBPIXEL_RENDERING).

    // source .oOo..      |.|oOo..
    //        .OOO.. -->  |.|OOO..
    //        .oOo..      |.|oOo..
    //                     ^
    //                     |-------- A single pixel in the output

    uint8_t* output = reinterpret_cast<uint8_t*>(dest.fImage);
    const unsigned outputPitch = dest.rowBytes();
    const uint8_t* input = source.buffer;

    // First we calculate the A8 mask.
    input += kFilterBorderWidth * source.pitch;   // skip the extra at the beginning
    for (int y = kFilterBorderWidth; y < source.rows - kFilterBorderWidth; y += 3) {
        const uint8_t* inputRow = input;
        uint8_t* outputRow = output;
        for (int x = 0; x < source.width; ++x) {
            const uint8_t averageAlpha = (static_cast<unsigned>(*inputRow) + inputRow[source.pitch] + inputRow[source.pitch * 2] + 1) / 3;
            *outputRow++ = averageAlpha;
            inputRow++;
        }

        input += source.pitch * 3;
        output += outputPitch;
    }

    // Align the 32-bit plane on a word boundary
    uint32_t* output32 = (uint32_t*) SkAlign4((uintptr_t) output);

    // Now we build the 32-bit alpha mask and RGB order correct.
    const int isBGR = SkFontHost::GetSubpixelOrder() == SkFontHost::kBGR_LCDOrder;
    input = source.buffer;

    if (!kFilterBorderWidth) {
        for (int x = 0; x < source.width; ++x)
            *output32++ = SkPackARGB32(0, 0, 0, 0);
    }

    for (int y = 0; y < source.rows; y += 3) {
        const uint8_t* inputRow = input;
        for (int x = 0; x < source.width; ++x) {
            const uint8_t alphaRed = isBGR ? inputRow[source.pitch * 2] : inputRow[0];
            const uint8_t alphaGreen = inputRow[source.pitch];
            const uint8_t alphaBlue = isBGR ? inputRow[0] : inputRow[2 * source.pitch];
            const uint8_t maxAlpha = SkMax32(alphaRed, SkMax32(alphaGreen, alphaBlue));
            *output32++ = SkPackARGB32(maxAlpha, alphaRed, alphaGreen, alphaBlue);
            inputRow++;
        }

        input += source.pitch * 3;
    }

    if (!kFilterBorderWidth) {
        for (int x = 0; x < source.width; ++x)
            *output32++ = SkPackARGB32(0, 0, 0, 0);
    }
}

}  // namespace skia_freetype_support
