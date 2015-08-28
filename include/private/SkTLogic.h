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

#include <stdint.h>

namespace skstd {

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

template <typename T, typename U> struct is_same : false_type {};
template <typename T> struct is_same<T, T> : true_type {};

template <typename T> struct is_void : is_same<void, remove_cv_t<T>> {};

template <typename T> struct is_const : false_type {};
template <typename T> struct is_const<const T> : true_type {};

template <typename T> struct is_volatile : false_type {};
template <typename T> struct is_volatile<volatile T> : true_type {};

template <typename T> struct is_reference : false_type {};
template <typename T> struct is_reference<T&> : true_type {};
template <typename T> struct is_reference<T&&> : true_type {};

template <typename T> struct is_lvalue_reference : false_type {};
template <typename T> struct is_lvalue_reference<T&> : true_type {};

template <typename T> struct is_empty_detector {
    struct Derived : public remove_cv_t<T> { char unused; };
    static const bool value = sizeof(Derived) == sizeof(char);
};
template <typename T> struct is_empty : bool_constant<is_empty_detector<T>::value> {};

template <typename T> struct add_const { using type = const T; };
template <typename T> using add_const_t = typename add_const<T>::type;

template <typename T> struct add_volatile { using type = volatile T; };
template <typename T> using add_volatile_t = typename add_volatile<T>::type;

template <typename T> struct add_cv { using type = add_volatile_t<add_const_t<T>>; };
template <typename T> using add_cv_t = typename add_cv<T>::type;

template <typename T> struct add_rvalue_reference {
    using type = conditional_t<is_void<T>::value || is_reference<T>::value, T, T&&>;
};
template <typename T> using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

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

/** Use as a return type to enable a function only when cond_type::value is true,
 *  like C++14's std::enable_if_t.  E.g.  (N.B. this is a dumb example.)
 *  SK_WHEN(true_type, int) f(void* ptr) { return 1; }
 *  SK_WHEN(!true_type, int) f(void* ptr) { return 2; }
 */
#define SK_WHEN(cond_prefix, T) skstd::enable_if_t<cond_prefix::value, T>

// See http://en.wikibooks.org/wiki/More_C++_Idioms/Member_Detector
#define SK_CREATE_MEMBER_DETECTOR(member)                                           \
template <typename T>                                                               \
class HasMember_##member {                                                          \
    struct Fallback { int member; };                                                \
    struct Derived : T, Fallback {};                                                \
    template <typename U, U> struct Check;                                          \
    template <typename U> static uint8_t func(Check<int Fallback::*, &U::member>*); \
    template <typename U> static uint16_t func(...);                                \
public:                                                                             \
    typedef HasMember_##member type;                                                \
    static const bool value = sizeof(func<Derived>(NULL)) == sizeof(uint16_t);      \
}

// Same sort of thing as SK_CREATE_MEMBER_DETECTOR, but checks for the existence of a nested type.
#define SK_CREATE_TYPE_DETECTOR(type)                                   \
template <typename T>                                                   \
class HasType_##type {                                                  \
    template <typename U> static uint8_t func(typename U::type*);       \
    template <typename U> static uint16_t func(...);                    \
public:                                                                 \
    static const bool value = sizeof(func<T>(NULL)) == sizeof(uint8_t); \
}

#endif
