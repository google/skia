/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Uniform_DEFINED
#define skgpu_Uniform_DEFINED

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"

namespace skgpu {

/**
 * Describes a uniform. Uniforms consist of:
 *  type:       The type of the uniform
 *  count:      Number of elements of 'type' in the array or kNonArray if not an array.
 */
class Uniform {
public:
    static constexpr int kNonArray = 0;

    constexpr Uniform(const char* name, SLType type) : Uniform(name, type, kNonArray) {}

    constexpr Uniform(const char* name, SLType type, int count)
            : fType      (static_cast<unsigned>(type))
            , fCount     (static_cast<unsigned>(count))
#ifdef SK_DEBUG
            , fName      (name)
#endif
    {
    }

    constexpr Uniform(const Uniform&) = default;
    Uniform& operator=(const Uniform&) = default;

    constexpr bool isInitialized() const { return this->type() != SLType::kVoid; }

#ifdef SK_DEBUG
    constexpr const char* name() const  { return fName; }
#endif
    constexpr SLType   type() const  { return static_cast<SLType>(fType);  }
    constexpr uint32_t count() const { return static_cast<uint32_t>(fCount); }

private:
    uint32_t    fType    : 6;
    uint32_t    fCount   : 26;
#ifdef SK_DEBUG
    const char* fName;
#endif

    static_assert(kSLTypeCount <= (1 << 6));
};

} // namespace skgpu

#endif // skgpu_Uniform_DEFINED
