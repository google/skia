// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Int96_DEFINED
#define Int96_DEFINED

#include <cstdint>

namespace bentleyottmann {
struct Int96 {
    int64_t hi;
    uint32_t lo;

    static Int96 Make(int32_t a);
    static Int96 Make(int64_t a);
};
bool operator== (const Int96& a, const Int96& b);
bool operator< (const Int96& a, const Int96& b);
Int96 operator+ (const Int96& a, const Int96& b);
Int96 multiply(int64_t a, int32_t b);
Int96 multiply(int32_t a, int64_t b);


}  // namespace bentleyottmann
#endif  // Int96_DEFINED
