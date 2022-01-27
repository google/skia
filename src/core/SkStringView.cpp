/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkStringView.h"

#include <algorithm>

namespace skstd {

bool operator==(string_view left, string_view right) {
    if (left.length() != right.length()) {
        return false;
    }
    return !string_view::traits_type::compare(left.data(), right.data(), left.length());
}

bool operator!=(string_view left, string_view right) {
    return !(left == right);
}

bool operator<(string_view left, string_view right) {
    int result = string_view::traits_type::compare(left.data(), right.data(),
                                                   std::min(left.length(), right.length()));
    if (!result) {
        result = left.length() - right.length();
    }
    return result < 0;
}

bool operator<=(string_view left, string_view right) {
    return !(left > right);
}

bool operator>(string_view left, string_view right) {
    return right < left;
}

bool operator>=(string_view left, string_view right) {
    return !(left < right);
}

} // namespace skstd
