/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

// skcms_Transform.h contains skcms implementation details.
// Please don't use this header from outside the skcms repo.

namespace skcms_private {

#if defined(__clang__)
    template <int N, typename T> using Vec = T __attribute__((ext_vector_type(N)));
#elif defined(__GNUC__)
    // Unfortunately, GCC does not allow us to omit the struct. This will not compile:
    //   template <int N, typename T> using Vec = T __attribute__((vector_size(N*sizeof(T))));
    template <int N, typename T> struct VecHelper {
        typedef T __attribute__((vector_size(N * sizeof(T)))) V;
    };
    template <int N, typename T> using Vec = typename VecHelper<N, T>::V;
#endif

}  // namespace skcms_private
