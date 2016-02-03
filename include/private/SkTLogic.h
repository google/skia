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

#include "SkTypes.h"

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

// template<typename R, typename... Args> struct is_function<
//     R [calling-convention] (Args...[, ...]) [const] [volatile] [&|&&]> : std::true_type {};
// The cv and ref-qualified versions are strange types we're currently avoiding, so not supported.
// These aren't supported in msvc either until vs2015u2.
// On all platforms, variadic functions only exist in the c calling convention.
// mcvc 2013 introduced __vectorcall, but it wan't until 2015 that it was added to is_function.
template <typename> struct is_function : std::false_type {};
#if !defined(SK_BUILD_FOR_WIN)
template <typename R, typename... Args> struct is_function<R(Args...)> : std::true_type {};
#else
#if defined(_M_IX86)
template <typename R, typename... Args> struct is_function<R __cdecl (Args...)> : std::true_type {};
template <typename R, typename... Args> struct is_function<R __stdcall (Args...)> : std::true_type {};
template <typename R, typename... Args> struct is_function<R __fastcall (Args...)> : std::true_type {};
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
template <typename R, typename... Args> struct is_function<R __vectorcall (Args...)> : std::true_type {};
#endif
#else
template <typename R, typename... Args> struct is_function<R __cdecl (Args...)> : std::true_type {};
template <typename R, typename... Args> struct is_function<R __vectorcall (Args...)> : std::true_type {};
#endif
#endif
template <typename R, typename... Args> struct is_function<R(Args..., ...)> : std::true_type {};

template <typename T> using add_const_t = typename std::add_const<T>::type;
template <typename T> using add_volatile_t = typename std::add_volatile<T>::type;
template <typename T> using add_cv_t = typename std::add_cv<T>::type;
template <typename T> using add_pointer_t = typename std::add_pointer<T>::type;
template <typename T> using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;

template <typename S, typename D,
          bool=std::is_void<S>::value || is_function<D>::value || std::is_array<D>::value>
struct is_convertible_detector {
    static const/*expr*/ bool value = std::is_void<D>::value;
};
template <typename S, typename D> struct is_convertible_detector<S, D, false> {
    using yes_type = uint8_t;
    using no_type = uint16_t;

    template <typename To> static void param_convertable_to(To);

    template <typename From, typename To>
    static decltype(param_convertable_to<To>(std::declval<From>()), yes_type()) convertible(int);

    template <typename, typename> static no_type convertible(...);

    static const/*expr*/ bool value = sizeof(convertible<S, D>(0)) == sizeof(yes_type);
};
// std::is_convertable is known to be broken (not work with incomplete types) in Android clang NDK.
// This is currently what prevents us from using std::unique_ptr.
template<typename S, typename D> struct is_convertible
    : bool_constant<is_convertible_detector<S, D>::value> {};

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
