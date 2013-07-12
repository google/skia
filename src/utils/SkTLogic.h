/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 *
 * This header provides some of the helpers (std::integral_constant) and
 * type transformations (std::conditional) which will become available with
 * C++11 in the type_traits header.
 *
 * Because we lack constexpr, we cannot mimic
 * std::integral_constant::'constexpr operator T()'.
 * As a result we introduce SkTBool and SkTIf similar to Boost in order to
 * minimize the visual noise of many uses of '::value'.
 */

#ifndef SkTLogic_DEFINED
#define SkTLogic_DEFINED

/** Represents a templated integer constant.
 *  Pre-C++11 version of std::integral_constant.
 */
template <typename T, T v> struct SkTIntegralConstant {
    static const T value = v;
    typedef T value_type;
    typedef SkTIntegralConstant<T, v> type;
};

/** Convenience specialization of SkTIntegralConstant. */
template <bool b> struct SkTBool : SkTIntegralConstant<bool, b> { };

/** Pre-C++11 version of std::true_type. */
typedef SkTBool<true> SkTrue;

/** Pre-C++11 version of std::false_type. */
typedef SkTBool<false> SkFalse;

/** SkTIf_c::type = (condition) ? T : F;
 *  Pre-C++11 version of std::conditional.
 */
template <bool condition, typename T, typename F> struct SkTIf_c {
    typedef F type;
};
template <typename T, typename F> struct SkTIf_c<true, T, F> {
    typedef T type;
};

/** SkTIf::type = (Condition::value) ? T : F; */
template <typename Condition, typename T, typename F> struct SkTIf {
    typedef typename SkTIf_c<static_cast<bool>(Condition::value), T, F>::type type;
};

/** SkTMux::type = (a && b) ? Both : (a) ? A : (b) ? B : Neither; */
template <typename a, typename b, typename Both, typename A, typename B, typename Neither>
struct SkTMux {
    typedef typename SkTIf<a, typename SkTIf<b, Both, A>::type,
                              typename SkTIf<b, B, Neither>::type>::type type;
};

#endif
