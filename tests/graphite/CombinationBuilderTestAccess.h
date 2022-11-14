/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef CombinationBuilderTestAccess_DEFINED
#define CombinationBuilderTestAccess_DEFINED

#include "include/gpu/graphite/CombinationBuilder.h"
#include "include/private/SkUniquePaintParamsID.h"

namespace skgpu::graphite {

class CombinationBuilderTestAccess {
public:
    static int NumCombinations(skgpu::graphite::CombinationBuilder* builder) {
        return builder->numCombinations();
    }
    static std::vector<SkUniquePaintParamsID> BuildCombinations(
                ShaderCodeDictionary* dict,
                skgpu::graphite::CombinationBuilder* builder) {
        std::vector<SkUniquePaintParamsID> uniqueIDs;

        builder->buildCombinations(dict,
                                   [&](SkUniquePaintParamsID uniqueID) {
                                       uniqueIDs.push_back(uniqueID);
                                   });

        return uniqueIDs;
    }
#ifdef SK_DEBUG
    static int Epoch(const skgpu::graphite::CombinationBuilder& builder) {
        return builder.epoch();
    }
    static int Epoch(const skgpu::graphite::CombinationOption& option) {
        return option.epoch();
    }
#endif
};

} // namespace skgpu::graphite

#endif // CombinationBuilderTestAccess_DEFINED
