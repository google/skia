/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if 0
// snippets that one day may be useful, unused for now...

// get sign, exponent, mantissa from double
// Translate the double into sign, exponent and mantissa.
    long bits = BitConverter.DoubleToInt64Bits(d);
    // Note that the shift is sign-extended, hence the test against -1 not 1
    bool negative = (bits < 0);
    int exponent = (int) ((bits >> 52) & 0x7ffL);
    long mantissa = bits & 0xfffffffffffffL;

    // Subnormal numbers; exponent is effectively one higher,
    // but there's no extra normalisation bit in the mantissa
    if (exponent==0)
    {
        exponent++;
    }
    // Normal numbers; leave exponent as it is but add extra
    // bit to the front of the mantissa
    else
    {
        mantissa = mantissa | (1L<<52);
    }

    // Bias the exponent. It's actually biased by 1023, but we're
    // treating the mantissa as m.0 rather than 0.m, so we need
    // to subtract another 52 from it.
    exponent -= 1075;

    if (mantissa == 0)
    {
        return "0";
    }

    /* Normalize */
    while((mantissa & 1) == 0)
    {    /*  i.e., Mantissa is even */
        mantissa >>= 1;
        exponent++;
    }
#endif
