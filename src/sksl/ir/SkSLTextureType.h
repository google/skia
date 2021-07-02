/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TEXTURETYPE
#define SKSL_TEXTURETYPE

#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

class TextureType : public Type {
public:
    static constexpr TypeKind kTypeKind = TypeKind::kTexture;

    TextureType(const char* name, SpvDim_ dimensions, bool isDepth, bool isArrayed,
                bool isMultisampled, bool isSampled)
        : INHERITED(name, "T", kTypeKind)
        , fDimensions(dimensions)
        , fIsDepth(isDepth)
        , fIsArrayed(isArrayed)
        , fIsMultisampled(isMultisampled)
        , fIsSampled(isSampled) {}

    SpvDim_ dimensions() const {
        return fDimensions;
    }

    bool isDepth() const {
        return fIsDepth;
    }

    bool isArrayed() const {
        return fIsArrayed;
    }

    bool isMultisampled() const {
        return fIsMultisampled;
    }

    bool isSampled() const {
        return fIsSampled;
    }

private:
    using INHERITED = Type;

    SpvDim_ fDimensions;
    bool fIsDepth : 1;
    bool fIsArrayed : 1;
    bool fIsMultisampled : 1;
    bool fIsSampled : 1;
};

}  // namespace SkSL

#endif
