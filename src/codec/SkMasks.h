/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMasks_DEFINED
#define SkMasks_DEFINED

#include "SkTypes.h"

/*
 *
 * Contains useful mask routines for SkMaskSwizzler
 *
 */
class SkMasks {
public:

    /*
     *
     * Input bit masks format
     *
     */
    struct InputMasks {
        uint32_t red;
        uint32_t green;
        uint32_t blue;
        uint32_t alpha;
    };

    /*
     *
     * Contains all of the information for a single mask
     *
     */
     struct MaskInfo {
        uint32_t mask;
        uint32_t shift;
        uint32_t size;
     };

    /*
     *
     * Create the masks object
     *
     */
    static SkMasks* CreateMasks(InputMasks masks, uint32_t bpp);

    /*
     *
     * Get a color component
     *
     */
    uint8_t getRed(uint32_t pixel) const;
    uint8_t getGreen(uint32_t pixel) const;
    uint8_t getBlue(uint32_t pixel) const;
    uint8_t getAlpha(uint32_t pixel) const;

    /*
     *
     * Getter for the alpha mask
     * The alpha mask may be used in other decoding modes
     *
     */
     uint32_t getAlphaMask() const {
        return fAlpha.mask;
     }

private:

    /*
     *
     * Constructor
     *
     */
    SkMasks(const MaskInfo& red, const MaskInfo& green, const MaskInfo& blue,
            const MaskInfo& alpha);

    const MaskInfo fRed;
    const MaskInfo fGreen;
    const MaskInfo fBlue;
    const MaskInfo fAlpha;
};

#endif
