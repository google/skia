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

#include <stdint.h>

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

/** Pre-C++11 version of std::is_empty<T>. */
template <typename T>
class SkTIsEmpty {
    struct Derived : public T { char unused; };
public:
    static const bool value = sizeof(Derived) == sizeof(char);
};

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

/** SkTEnableIf_c::type = (condition) ? T : [does not exist]; */
template <bool condition, class T = void> struct SkTEnableIf_c { };
template <class T> struct SkTEnableIf_c<true, T> {
    typedef T type;
};

/** SkTEnableIf::type = (Condition::value) ? T : [does not exist]; */
template <class Condition, class T = void> struct SkTEnableIf
    : public SkTEnableIf_c<static_cast<bool>(Condition::value), T> { };

/** Use as a return type to enable a function only when cond_type::value is true,
 *  like C++14's std::enable_if_t.  E.g.  (N.B. this is a dumb example.)
 *  SK_WHEN(SkTrue, int) f(void* ptr) { return 1; }
 *  SK_WHEN(!SkTrue, int) f(void* ptr) { return 2; }
 */
#define SK_WHEN(cond_prefix, T) typename SkTEnableIf_c<cond_prefix::value, T>::type
#define SK_WHEN_C(cond, T) typename SkTEnableIf_c<cond, T>::type

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

namespace skstd {

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

template <typename T, typename U> struct is_same : SkFalse {};
template <typename T> struct is_same<T, T> : SkTrue {};

template <typename T> struct is_void : is_same<void, remove_cv_t<T>> {};

template <typename T> struct is_reference : SkFalse {};
template <typename T> struct is_reference<T&> : SkTrue {};
template <typename T> struct is_reference<T&&> : SkTrue {};

template <typename T> struct is_lvalue_reference : SkFalse {};
template <typename T> struct is_lvalue_reference<T&> : SkTrue {};

template <typename T> struct add_rvalue_reference {
    using type = typename SkTIf_c<is_void<T>::value || is_reference<T>::value, T, T&&>::type;
};
template <typename T> using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

}  // namespace skstd

/**
 *  SkTIsConst<T>::value is true if the type T is const.
 *  The type T is constrained not to be an array or reference type.
 */
template <typename T> struct SkTIsConst {
    static T* t;
    static uint16_t test(const volatile void*);
    static uint32_t test(volatile void *);
    static const bool value = (sizeof(uint16_t) == sizeof(test(t)));
};

#endif
