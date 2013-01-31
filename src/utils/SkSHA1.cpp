/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * The following code is based on the description in RFC 3174.
 * http://www.ietf.org/rfc/rfc3174.txt
 */

#include "SkTypes.h"
#include "SkSHA1.h"
#include <string.h>

/** SHA1 basic transformation. Transforms state based on block. */
static void transform(uint32_t state[5], const uint8_t block[64]);

/** Encodes input into output (5 big endian 32 bit values). */
static void encode(uint8_t output[20], const uint32_t input[5]);

/** Encodes input into output (big endian 64 bit value). */
static void encode(uint8_t output[8], const uint64_t input);

SkSHA1::SkSHA1() : byteCount(0) {
    // These are magic numbers from the specification. The first four are the same as MD5.
    this->state[0] = 0x67452301;
    this->state[1] = 0xefcdab89;
    this->state[2] = 0x98badcfe;
    this->state[3] = 0x10325476;
    this->state[4] = 0xc3d2e1f0;
}

void SkSHA1::update(const uint8_t* input, size_t inputLength) {
    unsigned int bufferIndex = (unsigned int)(this->byteCount & 0x3F);
    unsigned int bufferAvailable = 64 - bufferIndex;

    unsigned int inputIndex;
    if (inputLength >= bufferAvailable) {
        if (bufferIndex) {
            memcpy(&this->buffer[bufferIndex], input, bufferAvailable);
            transform(this->state, this->buffer);
            inputIndex = bufferAvailable;
        } else {
            inputIndex = 0;
        }

        for (; inputIndex + 63 < inputLength; inputIndex += 64) {
            transform(this->state, &input[inputIndex]);
        }

        bufferIndex = 0;
    } else {
        inputIndex = 0;
    }

    memcpy(&this->buffer[bufferIndex], &input[inputIndex], inputLength - inputIndex);

    this->byteCount += inputLength;
}

void SkSHA1::finish(Digest& digest) {
    // Get the number of bits before padding.
    uint8_t bits[8];
    encode(bits, this->byteCount << 3);

    // Pad out to 56 mod 64.
    unsigned int bufferIndex = (unsigned int)(this->byteCount & 0x3F);
    unsigned int paddingLength = (bufferIndex < 56) ? (56 - bufferIndex) : (120 - bufferIndex);
    static uint8_t PADDING[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    this->update(PADDING, paddingLength);

    // Append length (length before padding, will cause final update).
    this->update(bits, 8);

    // Write out digest.
    encode(digest.data, this->state);

#if defined(SK_SHA1_CLEAR_DATA)
    // Clear state.
    memset(this, 0, sizeof(*this));
#endif
}

struct F1 { uint32_t operator()(uint32_t B, uint32_t C, uint32_t D) {
    return (B & C) | ((~B) & D);
    //return D ^ (B & (C ^ D));
    //return (B & C) ^ ((~B) & D);
    //return (B & C) + ((~B) & D);
    //return _mm_or_ps(_mm_andnot_ps(B, D), _mm_and_ps(B, C)); //SSE2
    //return vec_sel(D, C, B); //PPC
}};

struct F2 { uint32_t operator()(uint32_t B, uint32_t C, uint32_t D) {
    return B ^ C ^ D;
}};

struct F3 { uint32_t operator()(uint32_t B, uint32_t C, uint32_t D) {
    return (B & C) | (B & D) | (C & D);
    //return (B & C) | (D & (B | C));
    //return (B & C) | (D & (B ^ C));
    //return (B & C) + (D & (B ^ C));
    //return (B & C) ^ (B & D) ^ (C & D);
}};

/** Rotates x left n bits. */
static inline uint32_t rotate_left(uint32_t x, uint8_t n) {
    return (x << n) | (x >> (32 - n));
}

template <typename T>
static inline void operation(T operation,
                             uint32_t A, uint32_t& B, uint32_t C, uint32_t D, uint32_t& E,
                             uint32_t w, uint32_t k) {
    E += rotate_left(A, 5) + operation(B, C, D) + w + k;
    B = rotate_left(B, 30);
}

static void transform(uint32_t state[5], const uint8_t block[64]) {
    uint32_t A = state[0], B = state[1], C = state[2], D = state[3], E = state[4];

    // Round constants defined in SHA-1.
    static const uint32_t K[] = {
        0x5A827999, //sqrt(2) * 2^30
        0x6ED9EBA1, //sqrt(3) * 2^30
        0x8F1BBCDC, //sqrt(5) * 2^30
        0xCA62C1D6, //sqrt(10) * 2^30
    };

    uint32_t W[80];

    // Initialize the array W.
    size_t i = 0;
    for (size_t j = 0; i < 16; ++i, j += 4) {
        W[i] = (((uint32_t)block[j  ]) << 24) |
               (((uint32_t)block[j+1]) << 16) |
               (((uint32_t)block[j+2]) <<  8) |
               (((uint32_t)block[j+3])      );
    }
    for (; i < 80; ++i) {
       W[i] = rotate_left(W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16], 1);
       //The following is equivelent and speeds up SSE implementations, but slows non-SSE.
       //W[i] = rotate_left(W[i-6] ^ W[i-16] ^ W[i-28] ^ W[i-32], 2);
    }

    // Round 1
    operation(F1(), A, B, C, D, E, W[ 0], K[0]);
    operation(F1(), E, A, B, C, D, W[ 1], K[0]);
    operation(F1(), D, E, A, B, C, W[ 2], K[0]);
    operation(F1(), C, D, E, A, B, W[ 3], K[0]);
    operation(F1(), B, C, D, E, A, W[ 4], K[0]);
    operation(F1(), A, B, C, D, E, W[ 5], K[0]);
    operation(F1(), E, A, B, C, D, W[ 6], K[0]);
    operation(F1(), D, E, A, B, C, W[ 7], K[0]);
    operation(F1(), C, D, E, A, B, W[ 8], K[0]);
    operation(F1(), B, C, D, E, A, W[ 9], K[0]);
    operation(F1(), A, B, C, D, E, W[10], K[0]);
    operation(F1(), E, A, B, C, D, W[11], K[0]);
    operation(F1(), D, E, A, B, C, W[12], K[0]);
    operation(F1(), C, D, E, A, B, W[13], K[0]);
    operation(F1(), B, C, D, E, A, W[14], K[0]);
    operation(F1(), A, B, C, D, E, W[15], K[0]);
    operation(F1(), E, A, B, C, D, W[16], K[0]);
    operation(F1(), D, E, A, B, C, W[17], K[0]);
    operation(F1(), C, D, E, A, B, W[18], K[0]);
    operation(F1(), B, C, D, E, A, W[19], K[0]);

    // Round 2
    operation(F2(), A, B, C, D, E, W[20], K[1]);
    operation(F2(), E, A, B, C, D, W[21], K[1]);
    operation(F2(), D, E, A, B, C, W[22], K[1]);
    operation(F2(), C, D, E, A, B, W[23], K[1]);
    operation(F2(), B, C, D, E, A, W[24], K[1]);
    operation(F2(), A, B, C, D, E, W[25], K[1]);
    operation(F2(), E, A, B, C, D, W[26], K[1]);
    operation(F2(), D, E, A, B, C, W[27], K[1]);
    operation(F2(), C, D, E, A, B, W[28], K[1]);
    operation(F2(), B, C, D, E, A, W[29], K[1]);
    operation(F2(), A, B, C, D, E, W[30], K[1]);
    operation(F2(), E, A, B, C, D, W[31], K[1]);
    operation(F2(), D, E, A, B, C, W[32], K[1]);
    operation(F2(), C, D, E, A, B, W[33], K[1]);
    operation(F2(), B, C, D, E, A, W[34], K[1]);
    operation(F2(), A, B, C, D, E, W[35], K[1]);
    operation(F2(), E, A, B, C, D, W[36], K[1]);
    operation(F2(), D, E, A, B, C, W[37], K[1]);
    operation(F2(), C, D, E, A, B, W[38], K[1]);
    operation(F2(), B, C, D, E, A, W[39], K[1]);

    // Round 3
    operation(F3(), A, B, C, D, E, W[40], K[2]);
    operation(F3(), E, A, B, C, D, W[41], K[2]);
    operation(F3(), D, E, A, B, C, W[42], K[2]);
    operation(F3(), C, D, E, A, B, W[43], K[2]);
    operation(F3(), B, C, D, E, A, W[44], K[2]);
    operation(F3(), A, B, C, D, E, W[45], K[2]);
    operation(F3(), E, A, B, C, D, W[46], K[2]);
    operation(F3(), D, E, A, B, C, W[47], K[2]);
    operation(F3(), C, D, E, A, B, W[48], K[2]);
    operation(F3(), B, C, D, E, A, W[49], K[2]);
    operation(F3(), A, B, C, D, E, W[50], K[2]);
    operation(F3(), E, A, B, C, D, W[51], K[2]);
    operation(F3(), D, E, A, B, C, W[52], K[2]);
    operation(F3(), C, D, E, A, B, W[53], K[2]);
    operation(F3(), B, C, D, E, A, W[54], K[2]);
    operation(F3(), A, B, C, D, E, W[55], K[2]);
    operation(F3(), E, A, B, C, D, W[56], K[2]);
    operation(F3(), D, E, A, B, C, W[57], K[2]);
    operation(F3(), C, D, E, A, B, W[58], K[2]);
    operation(F3(), B, C, D, E, A, W[59], K[2]);

    // Round 4
    operation(F2(), A, B, C, D, E, W[60], K[3]);
    operation(F2(), E, A, B, C, D, W[61], K[3]);
    operation(F2(), D, E, A, B, C, W[62], K[3]);
    operation(F2(), C, D, E, A, B, W[63], K[3]);
    operation(F2(), B, C, D, E, A, W[64], K[3]);
    operation(F2(), A, B, C, D, E, W[65], K[3]);
    operation(F2(), E, A, B, C, D, W[66], K[3]);
    operation(F2(), D, E, A, B, C, W[67], K[3]);
    operation(F2(), C, D, E, A, B, W[68], K[3]);
    operation(F2(), B, C, D, E, A, W[69], K[3]);
    operation(F2(), A, B, C, D, E, W[70], K[3]);
    operation(F2(), E, A, B, C, D, W[71], K[3]);
    operation(F2(), D, E, A, B, C, W[72], K[3]);
    operation(F2(), C, D, E, A, B, W[73], K[3]);
    operation(F2(), B, C, D, E, A, W[74], K[3]);
    operation(F2(), A, B, C, D, E, W[75], K[3]);
    operation(F2(), E, A, B, C, D, W[76], K[3]);
    operation(F2(), D, E, A, B, C, W[77], K[3]);
    operation(F2(), C, D, E, A, B, W[78], K[3]);
    operation(F2(), B, C, D, E, A, W[79], K[3]);

    state[0] += A;
    state[1] += B;
    state[2] += C;
    state[3] += D;
    state[4] += E;

#if defined(SK_SHA1_CLEAR_DATA)
    // Clear sensitive information.
    memset(W, 0, sizeof(W));
#endif
}

static void encode(uint8_t output[20], const uint32_t input[5]) {
    for (size_t i = 0, j = 0; i < 5; i++, j += 4) {
        output[j  ] = (uint8_t)((input[i] >> 24) & 0xff);
        output[j+1] = (uint8_t)((input[i] >> 16) & 0xff);
        output[j+2] = (uint8_t)((input[i] >>  8) & 0xff);
        output[j+3] = (uint8_t)((input[i]      ) & 0xff);
    }
}

static void encode(uint8_t output[8], const uint64_t input) {
    output[0] = (uint8_t)((input >> 56) & 0xff);
    output[1] = (uint8_t)((input >> 48) & 0xff);
    output[2] = (uint8_t)((input >> 40) & 0xff);
    output[3] = (uint8_t)((input >> 32) & 0xff);
    output[4] = (uint8_t)((input >> 24) & 0xff);
    output[5] = (uint8_t)((input >> 16) & 0xff);
    output[6] = (uint8_t)((input >>  8) & 0xff);
    output[7] = (uint8_t)((input      ) & 0xff);
}
