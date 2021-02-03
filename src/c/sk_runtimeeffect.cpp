/*
 * Copyright 2020 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkShader.h"
#include "include/effects/SkRuntimeEffect.h"

#include "include/c/sk_types.h"
#include "include/c/sk_runtimeeffect.h"

#include "src/c/sk_types_priv.h"

// sk_runtimeeffect_t

sk_runtimeeffect_t* sk_runtimeeffect_make(sk_string_t* sksl, sk_string_t* error) {
    auto [effect, errorMessage] = SkRuntimeEffect::Make(AsString(*sksl));
    if (error && errorMessage.size() > 0)
        AsString(error)->swap(errorMessage);
    return ToRuntimeEffect(effect.release());
}

void sk_runtimeeffect_unref(sk_runtimeeffect_t* effect) {
    SkSafeUnref(AsRuntimeEffect(effect));
}

sk_shader_t* sk_runtimeeffect_make_shader(sk_runtimeeffect_t* effect, sk_data_t* uniforms, sk_shader_t** children, size_t childCount, const sk_matrix_t* localMatrix, bool isOpaque) {
    sk_sp<SkShader>* skChildren = new sk_sp<SkShader>[childCount];
    for (int i = 0; i < childCount; i++) {
        skChildren[i] = sk_ref_sp(AsShader(children[i]));
    }

    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    
    sk_sp<SkShader> shader = AsRuntimeEffect(effect)->makeShader(
        sk_ref_sp(AsData(uniforms)),
        skChildren,
        childCount,
        localMatrix ? &m : nullptr,
        isOpaque);

    delete[] skChildren;

    return ToShader(shader.release());
}

sk_colorfilter_t* sk_runtimeeffect_make_color_filter(sk_runtimeeffect_t* effect, sk_data_t* uniforms, sk_colorfilter_t** children, size_t childCount) {
    sk_sp<SkColorFilter>* skChildren = new sk_sp<SkColorFilter>[childCount];
    for (int i = 0; i < childCount; i++) {
        skChildren[i] = sk_ref_sp(AsColorFilter(children[i]));
    }

    sk_sp<SkColorFilter> shader = AsRuntimeEffect(effect)->makeColorFilter(
        sk_ref_sp(AsData(uniforms)),
        skChildren,
        childCount);

    delete[] skChildren;

    return ToColorFilter(shader.release());
}

size_t sk_runtimeeffect_get_uniform_size(const sk_runtimeeffect_t* effect) {
    return AsRuntimeEffect(effect)->uniformSize();
}

size_t sk_runtimeeffect_get_uniforms_count(const sk_runtimeeffect_t* effect) {
    return AsRuntimeEffect(effect)->uniforms().count();
}

void sk_runtimeeffect_get_uniform_name(const sk_runtimeeffect_t* effect, int index, sk_string_t* name) {
    auto vector = AsRuntimeEffect(effect)->uniforms();
    auto item = vector.begin() + index;
    AsString(name)->set(item->fName);
}

const sk_runtimeeffect_uniform_t* sk_runtimeeffect_get_uniform_from_index(const sk_runtimeeffect_t* effect, int index) {
    auto vector = AsRuntimeEffect(effect)->uniforms();
    auto item = vector.begin() + index;
    return ToRuntimeEffectUniform(&(*item));
}

const sk_runtimeeffect_uniform_t* sk_runtimeeffect_get_uniform_from_name(const sk_runtimeeffect_t* effect, const char* name, size_t len) {
    return ToRuntimeEffectUniform(AsRuntimeEffect(effect)->findUniform(name));
}

size_t sk_runtimeeffect_get_children_count(const sk_runtimeeffect_t* effect) {
    return AsRuntimeEffect(effect)->children().count();
}

void sk_runtimeeffect_get_child_name(const sk_runtimeeffect_t* effect, int index, sk_string_t* name) {
    auto vector = AsRuntimeEffect(effect)->children();
    auto item = vector.begin() + index;
    AsString(name)->set(*item);
}

// sk_runtimeeffect_uniform_t

size_t sk_runtimeeffect_uniform_get_offset(const sk_runtimeeffect_uniform_t* variable) {
    return AsRuntimeEffectUniform(variable)->fOffset;
}

size_t sk_runtimeeffect_uniform_get_size_in_bytes(const sk_runtimeeffect_uniform_t* variable) {
    return AsRuntimeEffectUniform(variable)->sizeInBytes();
}
