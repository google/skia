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

#include <cstddef>
#include <type_traits>

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

template <typename T> using result_of_t = typename std::result_of<T>::type;

template <typename... T> using common_type_t = typename std::common_type<T...>::type;

template <std::size_t... Ints> struct index_sequence {
    using type = index_sequence;
    using value_type = std::size_t;
    static constexpr std::size_t size() noexcept { return sizeof...(Ints); }
};

template <typename S1, typename S2> struct make_index_sequence_combine;
template <std::size_t... I1, std::size_t... I2>
struct make_index_sequence_combine<skstd::index_sequence<I1...>, skstd::index_sequence<I2...>>
    : skstd::index_sequence<I1..., (sizeof...(I1)+I2)...>
{ };

template <std::size_t N> struct make_index_sequence
    : make_index_sequence_combine<typename skstd::make_index_sequence<    N/2>::type,
                                  typename skstd::make_index_sequence<N - N/2>::type>{};
template<> struct make_index_sequence<0> : skstd::index_sequence< >{};
template<> struct make_index_sequence<1> : skstd::index_sequence<0>{};

struct monostate {};

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
