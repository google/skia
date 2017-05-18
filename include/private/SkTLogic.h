/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 *
 * This header provides some of the helpers (like std::enable_if_t) which will
 * become available with C++14 in the type_traits header (in the skstd
 * namespace). This header also provides several Skia specific additions such
 * as SK_WHEN and the sknonstd namespace.
 */

#ifndef SkTLogic_DEFINED
#define SkTLogic_DEFINED

#include <stddef.h>
#include <stdint.h>
#include <type_traits>
#include <utility>

namespace skstd {

template <bool B> using bool_constant = std::integral_constant<bool, B>;

template <bool B, typename T, typename F> using conditional_t = typename std::conditional<B, T, F>::type;
template <bool B, typename T = void> using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T> using remove_const_t = typename std::remove_const<T>::type;
template <typename T> using remove_volatile_t = typename std::remove_volatile<T>::type;
template <typename T> using remove_cv_t = typename std::remove_cv<T>::type;
template <typename T> using remove_pointer_t = typename std::remove_pointer<T>::type;
template <typename T> using remove_reference_t = typename std::remove_reference<T>::type;
template <typename T> using remove_extent_t = typename std::remove_extent<T>::type;

template <typename T> using add_const_t = typename std::add_const<T>::type;
template <typename T> using add_volatile_t = typename std::add_volatile<T>::type;
template <typename T> using add_cv_t = typename std::add_cv<T>::type;
template <typename T> using add_pointer_t = typename std::add_pointer<T>::type;
template <typename T> using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;

template <typename... T> using common_type_t = typename std::common_type<T...>::type;

// Chromium currently requires gcc 4.8.2 or a recent clang compiler, but uses libstdc++4.6.4.
// Note that Precise actually uses libstdc++4.6.3.
// Unfortunately, libstdc++ STL before libstdc++4.7 do not define std::underlying_type.
// Newer gcc and clang compilers have __underlying_type which does not depend on runtime support.
// See https://gcc.gnu.org/onlinedocs/libstdc++/manual/abi.html for __GLIBCXX__ values.
// Unfortunately __GLIBCXX__ is a date, but no updates to versions before 4.7 are now anticipated.
#define SK_GLIBCXX_4_7_0 20120322
// Updates to versions before 4.7 but released after 4.7 was released.
#define SK_GLIBCXX_4_5_4 20120702
#define SK_GLIBCXX_4_6_4 20121127
#if defined(__GLIBCXX__) && (__GLIBCXX__ <  SK_GLIBCXX_4_7_0 || \
                             __GLIBCXX__ == SK_GLIBCXX_4_5_4 || \
                             __GLIBCXX__ == SK_GLIBCXX_4_6_4)
template <typename T> struct underlying_type {
    using type = __underlying_type(T);
};
template <typename T> using is_trivially_destructible = std::has_trivial_destructor<T>;
#else
template <typename T> using underlying_type = std::underlying_type<T>;
template <typename T> using is_trivially_destructible = std::is_trivially_destructible<T>;
#endif
template <typename T> using underlying_type_t = typename skstd::underlying_type<T>::type;

}  // namespace skstd

// The sknonstd namespace contains things we would like to be proposed and feel std-ish.
namespace sknonstd {

// The name 'copy' here is fraught with peril. In this case it means 'append', not 'overwrite'.
// Alternate proposed names are 'propagate', 'augment', or 'append' (and 'add', but already taken).
// std::experimental::propagate_const already exists for other purposes in TSv2.
// These also follow the <dest, source> pattern used by boost.
template <typename D, typename S> struct copy_const {
    using type = skstd::conditional_t<std::is_const<S>::value, skstd::add_const_t<D>, D>;
};
template <typename D, typename S> using copy_const_t = typename copy_const<D, S>::type;

template <typename D, typename S> struct copy_volatile {
    using type = skstd::conditional_t<std::is_volatile<S>::value, skstd::add_volatile_t<D>, D>;
};
template <typename D, typename S> using copy_volatile_t = typename copy_volatile<D, S>::type;

template <typename D, typename S> struct copy_cv {
    using type = copy_volatile_t<copy_const_t<D, S>, S>;
};
template <typename D, typename S> using copy_cv_t = typename copy_cv<D, S>::type;

// The name 'same' here means 'overwrite'.
// Alternate proposed names are 'replace', 'transfer', or 'qualify_from'.
// same_xxx<D, S> can be written as copy_xxx<remove_xxx_t<D>, S>
template <typename D, typename S> using same_const = copy_const<skstd::remove_const_t<D>, S>;
template <typename D, typename S> using same_const_t = typename same_const<D, S>::type;
template <typename D, typename S> using same_volatile =copy_volatile<skstd::remove_volatile_t<D>,S>;
template <typename D, typename S> using same_volatile_t = typename same_volatile<D, S>::type;
template <typename D, typename S> using same_cv = copy_cv<skstd::remove_cv_t<D>, S>;
template <typename D, typename S> using same_cv_t = typename same_cv<D, S>::type;

}  // namespace sknonstd

// Just a pithier wrapper for enable_if_t.
#define SK_WHEN(condition, T) skstd::enable_if_t<!!(condition), T>

#endif
