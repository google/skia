/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageInfo.h"
#include "Test.h"
#include <type_traits>

DEF_TEST(Namespace, reporter) {
    static_assert(std::is_same<SkImageInfo, skia::SkImageInfo>::value, "");
    static_assert(&SkColorTypeBytesPerPixel == &skia::SkColorTypeBytesPerPixel, "");
}
