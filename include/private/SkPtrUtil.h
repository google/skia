/*
 * Copyright 2020 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPtrUtil_DEFINED
#define SkPtrUtil_DEFINED

#include <memory>
#include <type_traits>

/**
 * Adopts ownership from a raw pointer and transfers it to the returned `std::unique_ptr`, whose
 * type is deduced. Because of this deduction, *do not* specify the template type `T` when calling
 * `SkWrapUnique`.
 *
 * Example:
 *   Foo* MakeFoo(int);
 *   auto x = SkWrapUnique(MakeFoo(123));  // 'x' is std::unique_ptr<Foo>.
 *
 * Do not call SkWrapUnique with an explicit type, as in `SkWrapUnique<Foo>(MakeFoo(123))`.  The
 * purpose of SkWrapUnique is to automatically deduce the pointer type. If you wish to make the type
 * explicit, just use `std::unique_ptr` directly.
 *
 *   auto x = std::unique_ptr<X>(MakeFoo(123));
 *                  - or -
 *   std::unique_ptr<X> x(MakeFoo(123));
 *
 * While `SkWrapUnique` is useful for capturing the output of a raw pointer factory, prefer
 * 'std::make_unique<T>(args...)' over 'SkWrapUnique(new T(args...))'.
 *
 *   auto x = SkWrapUnique(new Foo(123));  // works, but nonideal.
 *   auto x = std::make_unique<Foo>(123);  // safer, standard, avoids raw 'new'.
 *
 * Note that `SkWrapUnique(p)` is valid only if `delete p` is a valid expression. In particular,
 * `SkWrapUnique()` cannot wrap pointers to arrays, functions or void, and it must not be used to
 * capture pointers obtained from array-new expressions (even though that would compile!).
 */
template <typename T>
std::unique_ptr<T> SkWrapUnique(T* ptr) {
    static_assert(!std::is_array<T>::value, "array types are unsupported");
    static_assert(std::is_object<T>::value, "non-object types are unsupported");
    return std::unique_ptr<T>(ptr);
}

#endif
