/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Attribute_DEFINED
#define skgpu_graphite_Attribute_DEFINED

#include "include/private/base/SkAlign.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/graphite/DrawTypes.h"

namespace skgpu::graphite {

 /** Describes a vertex or instance attribute. */
class Attribute {
public:
    constexpr Attribute() = default;
    constexpr Attribute(const char* name,
                        VertexAttribType cpuType,
                        SkSLType gpuType)
            : fName(name), fCPUType(cpuType), fGPUType(gpuType) {
        SkASSERT(name && gpuType != SkSLType::kVoid);
    }
    constexpr Attribute(const Attribute&) = default;

    Attribute& operator=(const Attribute&) = default;

    constexpr bool isInitialized() const { return fGPUType != SkSLType::kVoid; }

    constexpr const char*      name()    const { return fName; }
    constexpr VertexAttribType cpuType() const { return fCPUType; }
    constexpr SkSLType         gpuType() const { return fGPUType; }

    constexpr size_t size()       const { return VertexAttribTypeSize(fCPUType); }
    constexpr size_t sizeAlign4() const { return SkAlign4(this->size()); }

private:
    const char* fName = nullptr;
    VertexAttribType fCPUType = VertexAttribType::kFloat;
    SkSLType fGPUType = SkSLType::kVoid;
};

enum class Interpolation {
    // The default perspective-correct interpolation for floating point types.
    kPerspective,
    // Screen-space linear interpolation for floating point types.
    kLinear,
    // No guarantee on what the provoking vertex is, should be used when all vertices have the same
    // value so that is irrelevant.
    //
    // The only supported interpolation option for integer types.
    kFlat
};

/**Describes an interpolated value passed between a vertex and fragment shader. */
class Varying {
public:
    constexpr Varying() = default;
    constexpr Varying(const char* name,
                      SkSLType gpuType,
                      Interpolation interpolation = Interpolation::kPerspective)
            : fName(name)
            , fGPUType(gpuType)
            , fInterpolation(SkSLTypeIsIntegralType(gpuType) ? Interpolation::kFlat
                                                             : interpolation) {
        SkASSERT(name && gpuType != SkSLType::kVoid);
        SkASSERT(SkSLTypeVecLength(gpuType) >= 1); // Only scalar/vector types allowed as varyings.
        // Allow kPerspective for integer types since that's the default arg and will be replaced
        // with kFlat; but explicitly requesting kLinear for integer types is not allowed.
        SkASSERT(SkSLTypeIsFloatType(gpuType) || interpolation != Interpolation::kLinear);
    }

    constexpr Varying(const Varying&) = default;

    Varying& operator=(const Varying&) = default;

    constexpr bool isInitialized() const { return fGPUType != SkSLType::kVoid; }

    constexpr const char*   name()          const { return fName; }
    constexpr SkSLType      gpuType()       const { return fGPUType; }
    constexpr Interpolation interpolation() const { return fInterpolation; }

private:
    const char* fName = nullptr;
    SkSLType fGPUType = SkSLType::kVoid;
    Interpolation fInterpolation = Interpolation::kPerspective;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Attribute_DEFINED
