/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkStringView.h"
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

IntrinsicKind FindIntrinsicKind(std::string_view functionName) {
    if (skstd::starts_with(functionName, '$')) {
        functionName.remove_prefix(1);
    }

    const IntrinsicMap& intrinsicMap = GetIntrinsicMap();
    IntrinsicKind* kind = intrinsicMap.find(functionName);
    return kind ? *kind : kNotIntrinsic;
}

}  // namespace SkSL
