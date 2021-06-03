/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkStringView.h"

namespace skstd {
    
bool operator==(string_view left, string_view right) {
    if (left.length() != right.length()) {
        return false;
    }
    return left.length() == 0 || !memcmp(left.data(), right.data(), left.length());
}

bool operator!=(string_view left, string_view right) {
    return !(left == right);
}

} // namespace skstd
