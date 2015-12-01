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
 */

#ifndef SkTLogic_DEFINED
#define SkTLogic_DEFINED

#include "SkTypes.h"

#include <stddef.h>
#include <stdint.h>

namespace skstd {

using nullptr_t = decltype(nullptr);

template <typename T, T v> struct integral_constant {
    static const/*expr*/ T value = v;
    using value_type = T;
    using type = integral_constant<T, v>;
    //constexpr operator value_type() const noexcept { return value; }
    //constexpr value_type operator()() const noexcept { return value; }
};

template <bool B> using bool_constant = integral_constant<bool, B>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

template <bool B, typename T, typename F> struct conditional { using type = T; };
template <typename T, typename F> struct conditional<false, T, F> { using type = F; };
template <bool B, typename T, typename F> using conditional_t = typename conditional<B, T, F>::type;

template <bool B, typename T = void> struct enable_if { using type = T; };
template <typename T> struct enable_if<false, T> {};
template <bool B, typename T = void> using enable_if_t = typename enable_if<B, T>::type;

template <typename T> struct remove_const { using type = T; };
template <typename T> struct remove_const<const T> { using type = T; };
template <typename T> using remove_const_t = typename remove_const<T>::type;

template <typename T> struct remove_volatile { using type = T; };
template <typename T> struct remove_volatile<volatile T> { using type = T; };
template <typename T> using remove_volatile_t = typename remove_volatile<T>::type;

template <typename T> struct remove_cv { using type = remove_volatile_t<remove_const_t<T>>; };
template <typename T> using remove_cv_t = typename remove_cv<T>::type;

template <typename T> struct remove_reference { using type = T; };
template <typename T> struct remove_reference<T&> { using type = T; };
template <typename T> struct remove_reference<T&&> { using type = T; };
template <typename T> using remove_reference_t = typename remove_reference<T>::type;

template <typename T> struct remove_extent { using type = T; };
template <typename T> struct remove_extent<T[]> { using type = T; };
template <typename T, size_t N> struct remove_extent<T[N]> { using type = T;};
template <typename T> using remove_extent_t = typename remove_extent<T>::type;

template <typename T, typename U> struct is_same : false_type {};
template <typename T> struct is_same<T, T> : true_type {};

template <typename T> struct is_void : is_same<void, remove_cv_t<T>> {};

template <typename T> struct is_const : false_type {};
template <typename T> struct is_const<const T> : true_type {};

template <typename T> struct is_volatile : false_type {};
template <typename T> struct is_volatile<volatile T> : true_type {};

template <typename T> struct is_pointer_detector : false_type {};
template <typename T> struct is_pointer_detector<T*> : true_type {};
template <typename T> struct is_pointer : is_pointer_detector<remove_cv_t<T>> {};

template <typename T> struct is_reference : false_type {};
template <typename T> struct is_reference<T&> : true_type {};
template <typename T> struct is_reference<T&&> : true_type {};

template <typename T> struct is_lvalue_reference : false_type {};
template <typename T> struct is_lvalue_reference<T&> : true_type {};

template <typename T> struct is_rvalue_reference : false_type {};
template <typename T> struct is_rvalue_reference<T&&> : true_type {};

template <typename T> struct is_class_detector {
    using yes_type = uint8_t;
    using no_type = uint16_t;
    template <typename U> static yes_type clazz(int U::*);
    template <typename U> static no_type clazz(...);
    static const/*expr*/ bool value = sizeof(clazz<T>(0)) == sizeof(yes_type) /*&& !is_union<T>::value*/;
};
template <typename T> struct is_class : bool_constant<is_class_detector<T>::value> {};

template <typename T, bool = is_class<T>::value> struct is_empty_detector {
    struct Derived : public T { char unused; };
    static const/*expr*/ bool value = sizeof(Derived) == sizeof(char);
};
template <typename T> struct is_empty_detector<T, false> {
    static const/*expr*/ bool value = false;
};
template <typename T> struct is_empty : bool_constant<is_empty_detector<T>::value> {};

template <typename T> struct is_array : false_type {};
template <typename T> struct is_array<T[]> : true_type {};
template <typename T, size_t N> struct is_array<T[N]> : true_type {};

// template<typename R, typename... Args> struct is_function<
//     R [calling-convention] (Args...[, ...]) [const] [volatile] [&|&&]> : true_type {};
// The cv and ref-qualified versions are strange types we're currently avoiding, so not supported.
// On all platforms, variadic functions only exist in the c calling convention.
template <typename> struct is_function : false_type { };
#if !defined(SK_BUILD_FOR_WIN)
template <typename R, typename... Args> struct is_function<R(Args...)> : true_type {};
#else
#if defined(_M_IX86)
template <typename R, typename... Args> struct is_function<R __cdecl (Args...)> : true_type {};
template <typename R, typename... Args> struct is_function<R __stdcall (Args...)> : true_type {};
template <typename R, typename... Args> struct is_function<R __fastcall (Args...)> : true_type {};
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
template <typename R, typename... Args> struct is_function<R __vectorcall (Args...)> : true_type {};
#endif
#else
template <typename R, typename... Args> struct is_function<R __cdecl (Args...)> : true_type {};
template <typename R, typename... Args> struct is_function<R __vectorcall (Args...)> : true_type {};
#endif
#endif
template <typename R, typename... Args> struct is_function<R(Args..., ...)> : true_type {};

template <typename T> struct add_const { using type = const T; };
template <typename T> using add_const_t = typename add_const<T>::type;

template <typename T> struct add_volatile { using type = volatile T; };
template <typename T> using add_volatile_t = typename add_volatile<T>::type;

template <typename T> struct add_cv { using type = add_volatile_t<add_const_t<T>>; };
template <typename T> using add_cv_t = typename add_cv<T>::type;

template <typename T> struct add_pointer { using type = remove_reference_t<T>*; };
template <typename T> using add_pointer_t = typename add_pointer<T>::type;

template <typename T, bool=is_void<T>::value> struct add_lvalue_reference_init { using type = T; };
template <typename T> struct add_lvalue_reference_init<T, false> { using type = T&; };
template <typename T> struct add_lvalue_reference : add_lvalue_reference_init<T> { };
template <typename T> using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

template <typename T, bool=is_void<T>::value> struct add_rvalue_reference_init { using type = T; };
template <typename T> struct add_rvalue_reference_init<T, false> { using type = T&&; };
template <typename T> struct add_rvalue_reference : add_rvalue_reference_init<T> {};
template <typename T> using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

/* This is 'just' a forward declaration. */
template <typename T> add_rvalue_reference_t<T> declval() /*noexcept*/;

template <typename S, typename D, bool=is_void<S>::value||is_function<D>::value||is_array<D>::value>
struct is_convertible_detector {
    static const/*expr*/ bool value = is_void<D>::value;
};
template <typename S, typename D> struct is_convertible_detector<S, D, false> {
    using yes_type = uint8_t;
    using no_type = uint16_t;

    template <typename To> static void param_convertable_to(To);

    template <typename From, typename To>
    static decltype(param_convertable_to<To>(declval<From>()), yes_type()) convertible(int);

    template <typename, typename> static no_type convertible(...);

    static const/*expr*/ bool value = sizeof(convertible<S, D>(0)) == sizeof(yes_type);
};
template<typename S, typename D> struct is_convertible
    : bool_constant<is_convertible_detector<S, D>::value> { };

template <typename T> struct decay {
    using U = remove_reference_t<T>;
    using type = conditional_t<is_array<U>::value,
        remove_extent_t<U>*,
        conditional_t<is_function<U>::value, add_pointer_t<U>, remove_cv_t<U>>>;
};
template <typename T> using decay_t = typename decay<T>::type;

}  // namespace skstd

// The sknonstd namespace contains things we would like to be proposed and feel std-ish.
namespace sknonstd {

// The name 'copy' here is fraught with peril. In this case it means 'append', not 'overwrite'.
// Alternate proposed names are 'propagate', 'augment', or 'append' (and 'add', but already taken).
// std::experimental::propagate_const already exists for other purposes in TSv2.
// These also follow the <dest, source> pattern used by boost.
template <typename D, typename S> struct copy_const {
    using type = skstd::conditional_t<skstd::is_const<S>::value, skstd::add_const_t<D>, D>;
};
template <typename D, typename S> using copy_const_t = typename copy_const<D, S>::type;

template <typename D, typename S> struct copy_volatile {
    using type = skstd::conditional_t<skstd::is_volatile<S>::value, skstd::add_volatile_t<D>, D>;
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
