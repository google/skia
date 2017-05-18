/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTFitsIn_DEFINED
#define SkTFitsIn_DEFINED

#include "../private/SkTLogic.h"
#include <limits>
#include <type_traits>

namespace sktfitsin {
namespace Private {

/** SkTMux::type = (a && b) ? Both : (a) ? A : (b) ? B : Neither; */
template <bool a, bool b, typename Both, typename A, typename B, typename Neither>
struct SkTMux {
    using type = skstd::conditional_t<a, skstd::conditional_t<b, Both, A>,
                                         skstd::conditional_t<b, B, Neither>>;
};

/** SkTHasMoreDigits = (digits(A) >= digits(B)) ? true_type : false_type. */
template <typename A, typename B> struct SkTHasMoreDigits
    : skstd::bool_constant<std::numeric_limits<A>::digits >= std::numeric_limits<B>::digits>
{ };

/** A high or low side predicate which is used when it is statically known
 *  that source values are in the range of the Destination.
 */
template <typename S> struct SkTOutOfRange_False {
    using can_be_true = std::false_type;
    using source_type = S;
    static bool apply(S) {
        return false;
    }
};

/** A low side predicate which tests if the source value < Min(D).
 *  Assumes that Min(S) <= Min(D).
 */
template <typename D, typename S> struct SkTOutOfRange_LT_MinD {
    using can_be_true = std::true_type;
    using source_type = S;
    static bool apply(S s) {
        using precondition = SkTHasMoreDigits<S, D>;
        static_assert(precondition::value, "minS > minD");

        return s < static_cast<S>((std::numeric_limits<D>::min)());
    }
};

/** A low side predicate which tests if the source value is less than 0. */
template <typename D, typename S> struct SkTOutOfRange_LT_Zero {
    using can_be_true = std::true_type;
    using source_type = S;
    static bool apply(S s) {
        return s < static_cast<S>(0);
    }
};

/** A high side predicate which tests if the source value > Max(D).
 *  Assumes that Max(S) >= Max(D).
 */
template <typename D, typename S> struct SkTOutOfRange_GT_MaxD {
    using can_be_true = std::true_type;
    using source_type = S;
    static bool apply(S s) {
        using precondition = SkTHasMoreDigits<S, D>;
        static_assert(precondition::value, "maxS < maxD");

        return s > static_cast<S>((std::numeric_limits<D>::max)());
    }
};

/** Composes two SkTOutOfRange predicates.
 *  First checks OutOfRange_Low then, if in range, OutOfRange_High.
 */
template <typename OutOfRange_Low, typename OutOfRange_High> struct SkTOutOfRange_Either {
    using can_be_true = std::true_type;
    using source_type = typename OutOfRange_Low::source_type;
    static bool apply(source_type s) {
        bool outOfRange = OutOfRange_Low::apply(s);
        if (!outOfRange) {
            outOfRange = OutOfRange_High::apply(s);
        }
        return outOfRange;
    }
};

/** SkTCombineOutOfRange::type is an SkTOutOfRange_XXX type which is the
 *  optimal combination of OutOfRange_Low and OutOfRange_High.
 */
template <typename OutOfRange_Low, typename OutOfRange_High> struct SkTCombineOutOfRange {
    using Both = SkTOutOfRange_Either<OutOfRange_Low, OutOfRange_High>;
    using Neither = SkTOutOfRange_False<typename OutOfRange_Low::source_type>;

    using apply_low = typename OutOfRange_Low::can_be_true;
    using apply_high = typename OutOfRange_High::can_be_true;

    using type = typename SkTMux<apply_low::value, apply_high::value,
                                 Both, OutOfRange_Low, OutOfRange_High, Neither>::type;
};

template <typename D, typename S, typename OutOfRange_Low, typename OutOfRange_High>
struct SkTRangeChecker {
    /** This is the method which is called at runtime to do the range check. */
    static bool OutOfRange(S s) {
        using Combined = typename SkTCombineOutOfRange<OutOfRange_Low, OutOfRange_High>::type;
        return Combined::apply(s);
    }
};

/** SkTFitsIn_Unsigned2Unsiged::type is an SkTRangeChecker with an OutOfRange(S s) method
 *  the implementation of which is tailored for the source and destination types.
 *  Assumes that S and D are unsigned integer types.
 */
template <typename D, typename S> struct SkTFitsIn_Unsigned2Unsiged {
    using OutOfRange_Low = SkTOutOfRange_False<S>;
    using OutOfRange_High = SkTOutOfRange_GT_MaxD<D, S>;

    using HighSideOnlyCheck = SkTRangeChecker<D, S, OutOfRange_Low, OutOfRange_High>;
    using NoCheck = SkTRangeChecker<D, S, SkTOutOfRange_False<S>, SkTOutOfRange_False<S>>;

    // If std::numeric_limits<D>::digits >= std::numeric_limits<S>::digits, nothing to check.
    // This also protects the precondition of SkTOutOfRange_GT_MaxD.
    using sourceFitsInDesitination = SkTHasMoreDigits<D, S>;
    using type = skstd::conditional_t<sourceFitsInDesitination::value, NoCheck, HighSideOnlyCheck>;
};

/** SkTFitsIn_Signed2Signed::type is an SkTRangeChecker with an OutOfRange(S s) method
 *  the implementation of which is tailored for the source and destination types.
 *  Assumes that S and D are signed integer types.
 */
template <typename D, typename S> struct SkTFitsIn_Signed2Signed {
    using OutOfRange_Low = SkTOutOfRange_LT_MinD<D, S>;
    using OutOfRange_High = SkTOutOfRange_GT_MaxD<D, S>;

    using FullCheck = SkTRangeChecker<D, S, OutOfRange_Low, OutOfRange_High>;
    using NoCheck = SkTRangeChecker<D, S, SkTOutOfRange_False<S>, SkTOutOfRange_False<S>>;

    // If std::numeric_limits<D>::digits >= std::numeric_limits<S>::digits, nothing to check.
    // This also protects the precondition of SkTOutOfRange_LT_MinD and SkTOutOfRange_GT_MaxD.
    using sourceFitsInDesitination = SkTHasMoreDigits<D, S>;
    using type = skstd::conditional_t<sourceFitsInDesitination::value, NoCheck, FullCheck>;
};

/** SkTFitsIn_Signed2Unsigned::type is an SkTRangeChecker with an OutOfRange(S s) method
 *  the implementation of which is tailored for the source and destination types.
 *  Assumes that S is a signed integer type and D is an unsigned integer type.
 */
template <typename D, typename S> struct SkTFitsIn_Signed2Unsigned {
    using OutOfRange_Low = SkTOutOfRange_LT_Zero<D, S>;
    using OutOfRange_High = SkTOutOfRange_GT_MaxD<D, S>;

    using FullCheck = SkTRangeChecker<D, S, OutOfRange_Low, OutOfRange_High>;
    using LowSideOnlyCheck = SkTRangeChecker<D, S, OutOfRange_Low, SkTOutOfRange_False<S>>;

    // If std::numeric_limits<D>::max() >= std::numeric_limits<S>::max(),
    // no need to check the high side. (Until C++11, assume more digits means greater max.)
    // This also protects the precondition of SkTOutOfRange_GT_MaxD.
    using sourceCannotExceedDest = SkTHasMoreDigits<D, S>;
    using type = skstd::conditional_t<sourceCannotExceedDest::value, LowSideOnlyCheck, FullCheck>;
};

/** SkTFitsIn_Unsigned2Signed::type is an SkTRangeChecker with an OutOfRange(S s) method
 *  the implementation of which is tailored for the source and destination types.
 *  Assumes that S is an usigned integer type and D is a signed integer type.
 */
template <typename D, typename S> struct SkTFitsIn_Unsigned2Signed {
    using OutOfRange_Low = SkTOutOfRange_False<S>;
    using OutOfRange_High = SkTOutOfRange_GT_MaxD<D, S>;

    using HighSideOnlyCheck = SkTRangeChecker<D, S, OutOfRange_Low, OutOfRange_High>;
    using NoCheck = SkTRangeChecker<D, S, SkTOutOfRange_False<S>, SkTOutOfRange_False<S>>;

    // If std::numeric_limits<D>::max() >= std::numeric_limits<S>::max(), nothing to check.
    // (Until C++11, assume more digits means greater max.)
    // This also protects the precondition of SkTOutOfRange_GT_MaxD.
    using sourceCannotExceedDest = SkTHasMoreDigits<D, S>;
    using type = skstd::conditional_t<sourceCannotExceedDest::value, NoCheck, HighSideOnlyCheck>;
};

/** SkTFitsIn::type is an SkTRangeChecker with an OutOfRange(S s) method
 *  the implementation of which is tailored for the source and destination types.
 *  Assumes that S and D are integer types.
 */
template <typename D, typename S> struct SkTFitsIn {
    // One of the following will be the 'selector' type.
    using S2S = SkTFitsIn_Signed2Signed<D, S>;
    using S2U = SkTFitsIn_Signed2Unsigned<D, S>;
    using U2S = SkTFitsIn_Unsigned2Signed<D, S>;
    using U2U = SkTFitsIn_Unsigned2Unsiged<D, S>;

    using S_is_signed = skstd::bool_constant<std::numeric_limits<S>::is_signed>;
    using D_is_signed = skstd::bool_constant<std::numeric_limits<D>::is_signed>;

    using selector = typename SkTMux<S_is_signed::value, D_is_signed::value,
                                     S2S, S2U, U2S, U2U>::type;
    // This type is an SkTRangeChecker.
    using type = typename selector::type;
};

template <typename T, bool = std::is_enum<T>::value> struct underlying_type {
    using type = skstd::underlying_type_t<T>;
};
template <typename T> struct underlying_type<T, false> {
    using type = T;
};

} // namespace Private
} // namespace sktfitsin

/** Returns true if the integer source value 's' will fit in the integer destination type 'D'. */
template <typename D, typename S> inline bool SkTFitsIn(S s) {
    static_assert(std::is_integral<S>::value || std::is_enum<S>::value, "S must be integral.");
    static_assert(std::is_integral<D>::value || std::is_enum<D>::value, "D must be integral.");

    using RealS = typename sktfitsin::Private::underlying_type<S>::type;
    using RealD = typename sktfitsin::Private::underlying_type<D>::type;

    return !sktfitsin::Private::SkTFitsIn<RealD, RealS>::type::OutOfRange(s);
}

#endif
