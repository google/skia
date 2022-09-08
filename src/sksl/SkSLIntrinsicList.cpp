/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLIntrinsicList.h"

namespace SkSL {

const IntrinsicMap& GetIntrinsicMap() {
    #define SKSL_INTRINSIC(name) {#name, k_##name##_IntrinsicKind},
    static const auto* kAllIntrinsics = new SkTHashMap<std::string_view, IntrinsicKind>{
        SKSL_INTRINSIC_LIST
    };
    #undef SKSL_INTRINSIC

    return *kAllIntrinsics;
}

}  // namespace SkSL
