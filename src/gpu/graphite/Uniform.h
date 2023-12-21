/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Uniform_DEFINED
#define skgpu_graphite_Uniform_DEFINED

#include "src/core/SkSLTypeShared.h"

namespace skgpu::graphite {

// TODO: can SkRuntimeEffect::Uniform be absorbed into this class!?

/**
 * Describes a uniform. Uniforms consist of:
 *  name:       The constant string name to use in the generated SkSL (with mangling)
 *  type:       The type of the uniform
 *  count:      Number of elements of 'type' in the array or kNonArray if not an array.
 */
class Uniform {
public:
    static constexpr int kNonArray = 0;

    constexpr Uniform(const char* name, SkSLType type, int count = kNonArray)
            : Uniform(name, type, count, /*isPaintColor=*/false) {}

    /*
     * The paint color uniform is treated special and will only be added to the uniform block
     * once. Its name will not be mangled.
     */
    static constexpr Uniform PaintColor() {
        return Uniform("paintColor", SkSLType::kFloat4,
                       Uniform::kNonArray, /*isPaintColor=*/true);
    }

    constexpr Uniform(const Uniform&) = default;
    Uniform& operator=(const Uniform&) = default;

    constexpr const char* name()  const { return fName; }
    constexpr SkSLType    type()  const { return static_cast<SkSLType>(fType);  }
    constexpr int         count() const { return static_cast<int>(fCount); }

    constexpr bool isPaintColor() const { return static_cast<bool>(fIsPaintColor); }

private:
    constexpr Uniform(const char* name, SkSLType type, int count, bool isPaintColor)
            : fName(name)
            , fType(static_cast<unsigned>(type))
            , fIsPaintColor(static_cast<unsigned>(isPaintColor))
            , fCount(static_cast<unsigned>(count)) {
        SkASSERT(SkSLTypeCanBeUniformValue(type));
        SkASSERT(count >= 0);
    }

    const char* fName;

    // Uniform definitions for all encountered SkRuntimeEffects are stored permanently in the
    // ShaderCodeDictionary as part of the stable ShaderSnippet and code ID assigned to the
    // effect, including de-duplicating equivalent or re-created SkRuntimeEffects with the same
    // SkSL. To help keep this memory overhead as low as possible, we pack the Uniform fields
    // as tightly as possible.
    uint32_t    fType         : 6;
    uint32_t    fIsPaintColor : 1;
    uint32_t    fCount        : 25;

    static_assert(kSkSLTypeCount <= (1 << 6));
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Uniform_DEFINED
