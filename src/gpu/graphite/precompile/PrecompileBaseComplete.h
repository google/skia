/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileBaseComplete_DEFINED
#define skgpu_graphite_precompile_PrecompileBaseComplete_DEFINED

#include "include/gpu/graphite/precompile/PrecompileBase.h"

// This file simply provides the missing implementations of SelectOption and AddToKey.
// In practice PrecompileBase.h should never be used internally and this header should be
// used in its place.
namespace skgpu::graphite {

template<typename T>
std::pair<sk_sp<T>, int> PrecompileBase::SelectOption(SkSpan<const sk_sp<T>> options,
                                                      int desiredOption) {
    for (const sk_sp<T>& option : options) {
        if (desiredOption < (option ? option->numCombinations() : 1)) {
            return { option, desiredOption };
        }
        desiredOption -= option ? option->numCombinations() : 1;
    }
    return { nullptr, 0 };
}

template<typename T>
void PrecompileBase::AddToKey(const KeyContext& keyContext,
                              PaintParamsKeyBuilder* builder,
                              PipelineDataGatherer* gatherer,
                              SkSpan<const sk_sp<T>> options,
                              int desiredOption) {
    auto [option, childOptions] = SelectOption(options, desiredOption);
    if (option) {
        option->priv().addToKey(keyContext, builder, gatherer, childOptions);
    }
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileBaseComplete_DEFINED
