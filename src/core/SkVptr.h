/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVptr_DEFINED
#define SkVptr_DEFINED

#include <string.h>

// Experimentally, see if we can get at the vptr of objects with one.

template <typename T>
void* SkVptr(const T& object) {
    void* vptr;
    memcpy(&vptr, (const void*)&object, sizeof(vptr));
    return vptr;
}

#endif//SkVptr_DEFINED
