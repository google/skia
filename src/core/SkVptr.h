/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVptr_DEFINED
#define SkVptr_DEFINED

#include <string.h>
#include <type_traits>

// Experimentally, see if we can get at the vptr of objects with one.

template <typename T>
static inline void* SkVptr(const T& object) {
    static_assert(std::has_virtual_destructor<T>::value, "");
    void* vptr;
    memcpy(&vptr, (const void*)&object, sizeof(vptr));
    return vptr;
}

#endif//SkVptr_DEFINED
