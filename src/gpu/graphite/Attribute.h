/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Attribute_DEFINED
#define skgpu_graphite_Attribute_DEFINED

#include "include/private/SkAlign.h"
#include "include/private/SkAssert.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/graphite/DrawTypes.h"

#include <cstddef>

#include "include/core/SkMesh.h"

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

    // Since Attribute doesn't own its name, the passed in `attr` must outlive the returned
    // Attribute to ensure the name stays valid. Otherwise, Attribute should perform a deep-copy
    // to create a longer lived reference to its name.
    static Attribute MakeFromSkMeshAttribute(const SkMeshSpecification::Attribute& attr) {
        VertexAttribType cpuType;
        SkSLType gpuType;
        switch (attr.type) {
            case SkMeshSpecification::Attribute::Type::kFloat:
                cpuType = VertexAttribType::kFloat;
                gpuType = SkSLType::kFloat;
                break;
            case SkMeshSpecification::Attribute::Type::kFloat2:
                cpuType = VertexAttribType::kFloat2;
                gpuType = SkSLType::kFloat2;
                break;
            case SkMeshSpecification::Attribute::Type::kFloat3:
                cpuType = VertexAttribType::kFloat3;
                gpuType = SkSLType::kFloat3;
                break;
            case SkMeshSpecification::Attribute::Type::kFloat4:
                cpuType = VertexAttribType::kFloat4;
                gpuType = SkSLType::kFloat4;
                break;
            case SkMeshSpecification::Attribute::Type::kUByte4_unorm:
                cpuType = VertexAttribType::kUByte4_norm;
                gpuType = SkSLType::kHalf4;
                break;
        }
        return Attribute(attr.name.c_str(), cpuType, gpuType);
    }

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
