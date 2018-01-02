/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFloatToDecimal_DEFINED
#define SkFloatToDecimal_DEFINED

constexpr unsigned kMaximumSkFloatToDecimalLength = 49;

/** \fn SkFloatToDecimal
    Convert a float into a decimal string.

    The resulting string will be in the form `[-]?([0-9]*\.)?[0-9]+` (It does
    not use scientific notation.) and `sscanf(output, "%f", &x)` will return
    the original value if the value is finite. This function accepts all
    possible input values.

    INFINITY and -INFINITY are rounded to FLT_MAX and -FLT_MAX.

    NAN values are converted to 0.

    This function will always add a terminating '\0' to the output.

    @param value  Any floating-point number
    @param output The buffer to write the string into.  Must be non-null.

    @return strlen(output)
*/
unsigned SkFloatToDecimal(float value, char output[kMaximumSkFloatToDecimalLength]);

#endif  // SkFloatToDecimal_DEFINED
