/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFloatUtils_DEFINED
#define SkFloatUtils_DEFINED

#include "SkTypes.h"
#include <limits.h>
#include <float.h>

template <size_t size>
class SkTypeWithSize {
public:
    // Prevents using SkTypeWithSize<N> with non-specialized N.
    typedef void UInt;
};

template <>
class SkTypeWithSize<32> {
public:
    typedef uint32_t UInt;
};

template <>
class SkTypeWithSize<64> {
public:
    typedef uint64_t UInt;
};

template <typename RawType>
struct SkNumericLimits {
    static const int digits = 0;
};

template <>
struct SkNumericLimits<double> {
    static const int digits = DBL_MANT_DIG;
};

template <>
struct SkNumericLimits<float> {
    static const int digits = FLT_MANT_DIG;
};

//See
//http://stackoverflow.com/questions/17333/most-effective-way-for-float-and-double-comparison/3423299#3423299
//http://code.google.com/p/googletest/source/browse/trunk/include/gtest/internal/gtest-internal.h
//http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm

template <typename RawType, unsigned int ULPs>
class SkFloatingPoint {
public:
    /** Bits is a unsigned integer the same size as the floating point number. */
    typedef typename SkTypeWithSize<sizeof(RawType) * CHAR_BIT>::UInt Bits;

    /** # of bits in a number. */
    static const size_t kBitCount = CHAR_BIT * sizeof(RawType);

    /** # of fraction bits in a number. */
    static const size_t kFractionBitCount = SkNumericLimits<RawType>::digits - 1;

    /** # of exponent bits in a number. */
    static const size_t kExponentBitCount = kBitCount - 1 - kFractionBitCount;

    /** The mask for the sign bit. */
    static const Bits kSignBitMask = static_cast<Bits>(1) << (kBitCount - 1);

    /** The mask for the fraction bits. */
    static const Bits kFractionBitMask =
        ~static_cast<Bits>(0) >> (kExponentBitCount + 1);

    /** The mask for the exponent bits. */
    static const Bits kExponentBitMask = ~(kSignBitMask | kFractionBitMask);

    /** How many ULP's (Units in the Last Place) to tolerate when comparing. */
    static const size_t kMaxUlps = ULPs;

    /**
     *  Constructs a FloatingPoint from a raw floating-point number.
     *
     *  On an Intel CPU, passing a non-normalized NAN (Not a Number)
     *  around may change its bits, although the new value is guaranteed
     *  to be also a NAN.  Therefore, don't expect this constructor to
     *  preserve the bits in x when x is a NAN.
     */
    explicit SkFloatingPoint(const RawType& x) { fU.value = x; }

    /** Returns the exponent bits of this number. */
    Bits exponent_bits() const { return kExponentBitMask & fU.bits; }

    /** Returns the fraction bits of this number. */
    Bits fraction_bits() const { return kFractionBitMask & fU.bits; }

    /** Returns true iff this is NAN (not a number). */
    bool is_nan() const {
        // It's a NAN if both of the folloowing are true:
        // * the exponent bits are all ones
        // * the fraction bits are not all zero.
        return (exponent_bits() == kExponentBitMask) && (fraction_bits() != 0);
    }

    /**
     *  Returns true iff this number is at most kMaxUlps ULP's away from ths.
     *  In particular, this function:
     *   - returns false if either number is (or both are) NAN.
     *   - treats really large numbers as almost equal to infinity.
     *   - thinks +0.0 and -0.0 are 0 DLP's apart.
     */
    bool AlmostEquals(const SkFloatingPoint& rhs) const {
        // Any comparison operation involving a NAN must return false.
        if (is_nan() || rhs.is_nan()) return false;

        const Bits dist = DistanceBetweenSignAndMagnitudeNumbers(fU.bits,
                                                                 rhs.fU.bits);
        //SkDEBUGF(("(%f, %f, %d) ", u_.value_, rhs.u_.value_, dist));
        return dist <= kMaxUlps;
    }

private:
    /** The data type used to store the actual floating-point number. */
    union FloatingPointUnion {
        /** The raw floating-point number. */
        RawType value;
        /** The bits that represent the number. */
        Bits bits;
    };

    /**
     *  Converts an integer from the sign-and-magnitude representation to
     *  the biased representation. More precisely, let N be 2 to the
     *  power of (kBitCount - 1), an integer x is represented by the
     *  unsigned number x + N.
     *
     *  For instance,
     *
     *    -N + 1 (the most negative number representable using
     *           sign-and-magnitude) is represented by 1;
     *    0      is represented by N; and
     *    N - 1  (the biggest number representable using
     *           sign-and-magnitude) is represented by 2N - 1.
     *
     *  Read http://en.wikipedia.org/wiki/Signed_number_representations
     *  for more details on signed number representations.
     */
    static Bits SignAndMagnitudeToBiased(const Bits &sam) {
        if (kSignBitMask & sam) {
            // sam represents a negative number.
            return ~sam + 1;
        } else {
            // sam represents a positive number.
            return kSignBitMask | sam;
        }
    }

    /**
     *  Given two numbers in the sign-and-magnitude representation,
     *  returns the distance between them as an unsigned number.
     */
    static Bits DistanceBetweenSignAndMagnitudeNumbers(const Bits &sam1,
                                                       const Bits &sam2) {
        const Bits biased1 = SignAndMagnitudeToBiased(sam1);
        const Bits biased2 = SignAndMagnitudeToBiased(sam2);
        return (biased1 >= biased2) ? (biased1 - biased2) : (biased2 - biased1);
    }

    FloatingPointUnion fU;
};

#endif
