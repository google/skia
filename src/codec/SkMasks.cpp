/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecPriv.h"
#include "SkMasks.h"
#include "SkTypes.h"

/*
 *
 * Used to convert 1-7 bit color components into 8-bit color components
 *
 */
static constexpr uint8_t n_bit_to_8_bit_lookup_table[] = {
    // 1 bit
    0, 255,
    // 2 bits
    0, 85, 170, 255,
    // 3 bits
    0, 36, 73, 109, 146, 182, 219, 255,
    // 4 bits
    0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255,
    // 5 bits
    0, 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115, 123, 132, 140,
    148, 156, 165, 173, 181, 189, 197, 206, 214, 222, 230, 239, 247, 255,
    // 6 bits
    0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 45, 49, 53, 57, 61, 65, 69, 73,
    77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 130, 134, 138,
    142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190, 194, 198,
    202, 206, 210, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255,
    // 7 bits
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38,
    40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76,
    78, 80, 82, 84, 86, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
    112, 114, 116, 118, 120, 122, 124, 126, 129, 131, 133, 135, 137, 139, 141,
    143, 145, 147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167, 169, 171,
    173, 175, 177, 179, 181, 183, 185, 187, 189, 191, 193, 195, 197, 199, 201,
    203, 205, 207, 209, 211, 213, 215, 217, 219, 221, 223, 225, 227, 229, 231,
    233, 235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255
};

/*
 *
 * Convert an n bit component to an 8-bit component
 *
 */
static uint8_t convert_to_8(uint8_t component, uint32_t n) {
    if (0 == n) {
        return 0;
    } else if (8 > n) {
        return n_bit_to_8_bit_lookup_table[(1 << n) - 2 + component];
    } else {
        SkASSERT(8 == n);
        return component;
    }
}

static uint8_t get_comp(uint32_t pixel, uint32_t mask, uint32_t shift,
                        uint32_t size) {
    return convert_to_8((pixel & mask) >> shift, size);
}

/*
 *
 * Get a color component
 *
 */
uint8_t SkMasks::getRed(uint32_t pixel) const {
    return get_comp(pixel, fRed.mask, fRed.shift, fRed.size);
}
uint8_t SkMasks::getGreen(uint32_t pixel) const {
    return get_comp(pixel, fGreen.mask, fGreen.shift, fGreen.size);
}
uint8_t SkMasks::getBlue(uint32_t pixel) const {
    return get_comp(pixel, fBlue.mask, fBlue.shift, fBlue.size);
}
uint8_t SkMasks::getAlpha(uint32_t pixel) const {
    return get_comp(pixel, fAlpha.mask, fAlpha.shift, fAlpha.size);
}

/*
 *
 * Process an input mask to obtain the necessary information
 *
 */
static const SkMasks::MaskInfo process_mask(uint32_t mask) {
    // Determine properties of the mask
    uint32_t tempMask = mask;
    uint32_t shift = 0;
    uint32_t size = 0;
    if (tempMask != 0) {
        // Count trailing zeros on masks
        for (; (tempMask & 1) == 0; tempMask >>= 1) {
            shift++;
        }
        // Count the size of the mask
        for (; tempMask & 1; tempMask >>= 1) {
            size++;
        }
        // Verify that the mask is continuous
        if (tempMask) {
            SkCodecPrintf("Warning: Bit mask is not continuous.\n");
            // Finish processing the mask
            for (; tempMask; tempMask >>= 1) {
                size++;
            }
        }
        // Truncate masks greater than 8 bits
        if (size > 8) {
            shift += size - 8;
            size = 8;
            mask &= 0xFF << shift;
        }
    }

    return { mask, shift, size };
}

/*
 *
 * Create the masks object
 *
 */
SkMasks* SkMasks::CreateMasks(InputMasks masks, int bytesPerPixel) {
    SkASSERT(0 < bytesPerPixel && bytesPerPixel <= 4);

    // Trim the input masks to match bytesPerPixel.
    if (bytesPerPixel < 4) {
        int bitsPerPixel = 8*bytesPerPixel;
        masks.red   &= (1 << bitsPerPixel) - 1;
        masks.green &= (1 << bitsPerPixel) - 1;
        masks.blue  &= (1 << bitsPerPixel) - 1;
        masks.alpha &= (1 << bitsPerPixel) - 1;
    }

    // Check that masks do not overlap.
    if (((masks.red   & masks.green) |
         (masks.red   & masks.blue ) |
         (masks.red   & masks.alpha) |
         (masks.green & masks.blue ) |
         (masks.green & masks.alpha) |
         (masks.blue  & masks.alpha) ) != 0) {
        return nullptr;
    }

    return new SkMasks(process_mask(masks.red  ),
                       process_mask(masks.green),
                       process_mask(masks.blue ),
                       process_mask(masks.alpha));
}


SkMasks::SkMasks(const MaskInfo& red, const MaskInfo& green,
                 const MaskInfo& blue, const MaskInfo& alpha)
    : fRed(red)
    , fGreen(green)
    , fBlue(blue)
    , fAlpha(alpha)
{}
