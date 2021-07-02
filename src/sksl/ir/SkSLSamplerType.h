/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SAMPLERTYPE
#define SKSL_SAMPLERTYPE

#include "src/sksl/ir/SkSLTextureType.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

class SamplerType : public Type {
public:
    static constexpr TypeKind kTypeKind = TypeKind::kSampler;

    SamplerType(const char* name, const TextureType& textureType)
        : INHERITED(name, "Z", kTypeKind)
        , fTextureType(textureType) {}

    const TextureType& textureType() const {
        return fTextureType;
    }

private:
    using INHERITED = Type;

    const TextureType& fTextureType;
};

}  // namespace SkSL

#endif
