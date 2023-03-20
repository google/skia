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
 *  type:       The type of the uniform
 *  count:      Number of elements of 'type' in the array or kNonArray if not an array.
 */
class Uniform {
public:
    static constexpr int kNonArray = 0;

    constexpr Uniform(const char* name, SkSLType type) : Uniform(name, type, kNonArray) {}

    constexpr Uniform(const char* name, SkSLType type, int count)
            : fType      (static_cast<unsigned>(type))
            , fCount     (static_cast<unsigned>(count))
            , fName      (name) {
    }

    constexpr Uniform(const Uniform&) = default;
    Uniform& operator=(const Uniform&) = default;

    constexpr bool isInitialized() const { return this->type() != SkSLType::kVoid; }

    constexpr const char* name() const  { return fName; }
    constexpr SkSLType    type() const  { return static_cast<SkSLType>(fType);  }
    constexpr uint32_t    count() const { return static_cast<uint32_t>(fCount); }

private:
    uint32_t    fType    : 6;
    uint32_t    fCount   : 26;
    const char* fName;

    static_assert(kSkSLTypeCount <= (1 << 6));
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Uniform_DEFINED
