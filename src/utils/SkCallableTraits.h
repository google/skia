/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCallableTraits_DEFINED
#define SkCallableTraits_DEFINED

#include <type_traits>

template <typename R, typename... Args> struct sk_base_callable_traits {
    using return_type = R;
    static constexpr std::size_t arity = sizeof...(Args);
    template <std::size_t N> struct argument {
        static_assert(N < arity, "");
        using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };
};

#define SK_CALLABLE_TRAITS__COMMA ,

#define SK_CALLABLE_TRAITS__VARARGS(quals, _) \
SK_CALLABLE_TRAITS__INSTANCE(quals,) \
SK_CALLABLE_TRAITS__INSTANCE(quals, SK_CALLABLE_TRAITS__COMMA ...)

#ifdef __cpp_noexcept_function_type
#define SK_CALLABLE_TRAITS__NE_VARARGS(quals, _) \
SK_CALLABLE_TRAITS__VARARGS(quals,) \
SK_CALLABLE_TRAITS__VARARGS(quals noexcept,)
#else
#define SK_CALLABLE_TRAITS__NE_VARARGS(quals, _) \
SK_CALLABLE_TRAITS__VARARGS(quals,)
#endif

#define SK_CALLABLE_TRAITS__REF_NE_VARARGS(quals, _) \
SK_CALLABLE_TRAITS__NE_VARARGS(quals,) \
SK_CALLABLE_TRAITS__NE_VARARGS(quals &,) \
SK_CALLABLE_TRAITS__NE_VARARGS(quals &&,)

#define SK_CALLABLE_TRAITS__CV_REF_NE_VARARGS() \
SK_CALLABLE_TRAITS__REF_NE_VARARGS(,) \
SK_CALLABLE_TRAITS__REF_NE_VARARGS(const,) \
SK_CALLABLE_TRAITS__REF_NE_VARARGS(volatile,) \
SK_CALLABLE_TRAITS__REF_NE_VARARGS(const volatile,)

/** Infer the return_type and argument<N> of a callable type T. */
template <typename T> struct SkCallableTraits : SkCallableTraits<decltype(&T::operator())> {};

// function (..., (const, volatile), (&, &&), noexcept)
#define SK_CALLABLE_TRAITS__INSTANCE(quals, varargs) \
template <typename R, typename... Args> \
struct SkCallableTraits<R(Args... varargs) quals> : sk_base_callable_traits<R, Args...> {};

SK_CALLABLE_TRAITS__CV_REF_NE_VARARGS()
#undef SK_CALLABLE_TRAITS__INSTANCE

// pointer to function (..., noexcept)
#define SK_CALLABLE_TRAITS__INSTANCE(quals, varargs) \
template <typename R, typename... Args> \
struct SkCallableTraits<R(*)(Args... varargs) quals> : sk_base_callable_traits<R, Args...> {};

SK_CALLABLE_TRAITS__NE_VARARGS(,)
#undef SK_CALLABLE_TRAITS__INSTANCE

// pointer to method (..., (const, volatile), (&, &&), noexcept)
#define SK_CALLABLE_TRAITS__INSTANCE(quals, varargs) \
template <typename T, typename R, typename... Args> \
struct SkCallableTraits<R(T::*)(Args... varargs) quals> : sk_base_callable_traits<R, Args...> {};

SK_CALLABLE_TRAITS__CV_REF_NE_VARARGS()
#undef SK_CALLABLE_TRAITS__INSTANCE

// pointer to field
template <typename T, typename R>
struct SkCallableTraits<R T::*> : sk_base_callable_traits<typename std::add_lvalue_reference<R>::type> {};

#undef SK_CALLABLE_TRAITS__CV_REF_NE_VARARGS
#undef SK_CALLABLE_TRAITS__REF_NE_VARARGS
#undef SK_CALLABLE_TRAITS__NE_VARARGS
#undef SK_CALLABLE_TRAITS__VARARGS
#undef SK_CALLABLE_TRAITS__COMMA

#endif
