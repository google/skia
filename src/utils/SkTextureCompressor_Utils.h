/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkTextureCompressorUtils_DEFINED
#define SkTextureCompressorUtils_DEFINED

namespace SkTextureCompressor {

    // In some compression formats used for grayscale alpha, i.e. coverage masks, three
    // bit indices are used to represent each pixel. A compression scheme must therefore
    // quantize the full eight bits of grayscale to three bits. The simplest way to do
    // this is to take the top three bits of the grayscale value. However, this does not
    // provide an accurate quantization: 192 will be quantized to 219 instead of 185. In
    // our compression schemes, we let these three-bit indices represent the full range
    // of grayscale values, and so when we go from three bits to eight bits, we replicate
    // the three bits into the lower bits of the eight bit value. Below are two different
    // techniques that offer a quality versus speed tradeoff in terms of quantization.
#if 1
    // Divides each byte in the 32-bit argument by three.
    static inline uint32_t MultibyteDiv3(uint32_t x) {
        const uint32_t a = (x >> 2) & 0x3F3F3F3F;
        const uint32_t ar = (x & 0x03030303) << 4;

        const uint32_t b = (x >> 4) & 0x0F0F0F0F;
        const uint32_t br = (x & 0x0F0F0F0F) << 2;

        const uint32_t c = (x >> 6) & 0x03030303;
        const uint32_t cr = x & 0x3F3F3F3F;

        return a + b + c + (((ar + br + cr) >> 6) & 0x03030303);
    }

    // Takes a loaded 32-bit integer of four 8-bit greyscale values and returns their
    // quantization into 3-bit values, used by LATC and R11 EAC. Instead of taking the
    // top three bits, the function computes the best three-bit value such that its
    // reconstruction into an eight bit value via bit replication will yield the best
    // results. In a 32-bit integer taking the range of values from 0-255 we would add
    // 18 and divide by 36 (255 / 36 ~= 7). However, since we are working in constrained
    // 8-bit space, our algorithm is the following:
    // 1. Shift right by one to give room for overflow
    // 2. Add 9 (18/2)
    // 3. Divide by 18 (divide by two, then by three twice)
    static inline uint32_t ConvertToThreeBitIndex(uint32_t x) {
        x = (x >> 1) & 0x7F7F7F7F; // 1
        x = x + 0x09090909;        // 2

        // Need to divide by 18... so first divide by two
        x = (x >> 1) & 0x7F7F7F7F;

        // Now divide by three twice
        x = MultibyteDiv3(x);
        x = MultibyteDiv3(x);
        return x;
    }
#else
    // Moves the top three bits of each byte in the 32-bit argument to the least
    // significant bits of their respective byte.
    static inline uint32_t ConvertToThreeBitIndex(uint32_t x) {
        return (x >> 5) & 0x07070707;
    }
#endif
}

#endif // SkTextureCompressorUtils_DEFINED
