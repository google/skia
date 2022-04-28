/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkFloatToDecimal.h"

#include "include/core/SkTypes.h"

#include <cfloat>
#include <cmath>

#ifdef SK_DEBUG
#include <limits.h>
#endif

// returns `value * pow(base, e)`, assuming `e` is positive.
static double pow_by_squaring(double value, double base, int e) {
    // https://en.wikipedia.org/wiki/Exponentiation_by_squaring
    SkASSERT(e > 0);
    while (true) {
        if (e & 1) {
            value *= base;
        }
        e >>= 1;
        if (0 == e) {
            return value;
        }
        base *= base;
    }
}

// Return pow(10.0, e), optimized for common cases.
static double pow10(int e) {
    switch (e) {
        case 0:  return 1.0;  // common cases
        case 1:  return 10.0;
        case 2:  return 100.0;
        case 3:  return 1e+03;
        case 4:  return 1e+04;
        case 5:  return 1e+05;
        case 6:  return 1e+06;
        case 7:  return 1e+07;
        case 8:  return 1e+08;
        case 9:  return 1e+09;
        case 10: return 1e+10;
        case 11: return 1e+11;
        case 12: return 1e+12;
        case 13: return 1e+13;
        case 14: return 1e+14;
        case 15: return 1e+15;
        default:
            if (e > 15) {
                return pow_by_squaring(1e+15, 10.0, e - 15);
            } else {
                SkASSERT(e < 0);
                return pow_by_squaring(1.0, 0.1, -e);
            }
    }
}

/** Write a string into output, including a terminating '\0' (for
    unit testing).  Return strlen(output) (for SkWStream::write) The
    resulting string will be in the form /[-]?([0-9]*.)?[0-9]+/ and
    sscanf(output, "%f", &x) will return the original value iff the
    value is finite. This function accepts all possible input values.

    Motivation: "PDF does not support [numbers] in exponential format
    (such as 6.02e23)."  Otherwise, this function would rely on a
    sprintf-type function from the standard library. */
unsigned SkFloatToDecimal(float value, char output[kMaximumSkFloatToDecimalLength]) {
    /* The longest result is -FLT_MIN.
       We serialize it as "-.0000000000000000000000000000000000000117549435"
       which has 48 characters plus a terminating '\0'. */

    static_assert(kMaximumSkFloatToDecimalLength == 49, "");
    // 3 = '-', '.', and '\0' characters.
    // 9 = number of significant digits
    // abs(FLT_MIN_10_EXP) = number of zeros in FLT_MIN
    static_assert(kMaximumSkFloatToDecimalLength == 3 + 9 - FLT_MIN_10_EXP, "");

    /* section C.1 of the PDF1.4 spec (http://goo.gl/0SCswJ) says that
       most PDF rasterizers will use fixed-point scalars that lack the
       dynamic range of floats.  Even if this is the case, I want to
       serialize these (uncommon) very small and very large scalar
       values with enough precision to allow a floating-point
       rasterizer to read them in with perfect accuracy.
       Experimentally, rasterizers such as pdfium do seem to benefit
       from this.  Rasterizers that rely on fixed-point scalars should
       gracefully ignore these values that they can not parse. */
    char* output_ptr = &output[0];
    const char* const end = &output[kMaximumSkFloatToDecimalLength - 1];
    // subtract one to leave space for '\0'.

    /* This function is written to accept any possible input value,
       including non-finite values such as INF and NAN.  In that case,
       we ignore value-correctness and output a syntacticly-valid
       number. */
    if (value == INFINITY) {
        value = FLT_MAX;  // nearest finite float.
    }
    if (value == -INFINITY) {
        value = -FLT_MAX;  // nearest finite float.
    }
    if (!std::isfinite(value) || value == 0.0f) {
        // NAN is unsupported in PDF.  Always output a valid number.
        // Also catch zero here, as a special case.
        *output_ptr++ = '0';
        *output_ptr = '\0';
        return static_cast<unsigned>(output_ptr - output);
    }
    if (value < 0.0) {
        *output_ptr++ = '-';
        value = -value;
    }
    SkASSERT(value >= 0.0f);

    int binaryExponent;
    (void)std::frexp(value, &binaryExponent);
    static const double kLog2 = 0.3010299956639812;  // log10(2.0);
    int decimalExponent = static_cast<int>(std::floor(kLog2 * binaryExponent));
    int decimalShift = decimalExponent - 8;
    double power = pow10(-decimalShift);
    SkASSERT(value * power <= (double)INT_MAX);
    int d = static_cast<int>(value * power + 0.5);
    // SkASSERT(value == (float)(d * pow(10.0, decimalShift)));
    SkASSERT(d <= 999999999);
    if (d > 167772159) {  // floor(pow(10,1+log10(1<<24)))
       // need one fewer decimal digits for 24-bit precision.
       decimalShift = decimalExponent - 7;
       // SkASSERT(power * 0.1 = pow10(-decimalShift));
       // recalculate to get rounding right.
       d = static_cast<int>(value * (power * 0.1) + 0.5);
       SkASSERT(d <= 99999999);
    }
    while (d % 10 == 0) {
        d /= 10;
        ++decimalShift;
    }
    SkASSERT(d > 0);
    // SkASSERT(value == (float)(d * pow(10.0, decimalShift)));
    unsigned char buffer[9]; // decimal value buffer.
    int bufferIndex = 0;
    do {
        buffer[bufferIndex++] = d % 10;
        d /= 10;
    } while (d != 0);
    SkASSERT(bufferIndex <= (int)sizeof(buffer) && bufferIndex > 0);
    if (decimalShift >= 0) {
        do {
            --bufferIndex;
            *output_ptr++ = '0' + buffer[bufferIndex];
        } while (bufferIndex);
        for (int i = 0; i < decimalShift; ++i) {
            *output_ptr++ = '0';
        }
    } else {
        int placesBeforeDecimal = bufferIndex + decimalShift;
        if (placesBeforeDecimal > 0) {
            while (placesBeforeDecimal-- > 0) {
                --bufferIndex;
                *output_ptr++ = '0' + buffer[bufferIndex];
            }
            *output_ptr++ = '.';
        } else {
            *output_ptr++ = '.';
            int placesAfterDecimal = -placesBeforeDecimal;
            while (placesAfterDecimal-- > 0) {
                *output_ptr++ = '0';
            }
        }
        while (bufferIndex > 0) {
            --bufferIndex;
            *output_ptr++ = '0' + buffer[bufferIndex];
            if (output_ptr == end) {
                break;  // denormalized: don't need extra precision.
                // Note: denormalized numbers will not have the same number of
                // significantDigits, but do not need them to round-trip.
            }
        }
    }
    SkASSERT(output_ptr <= end);
    *output_ptr = '\0';
    return static_cast<unsigned>(output_ptr - output);
}
