/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCustomMeshPriv_DEFINED
#define SkCustomMeshPriv_DEFINED

#include "include/core/SkCustomMesh.h"

#ifdef SK_ENABLE_SKSL
#include "include/private/GrTypesPriv.h"
#include "src/core/SkSLTypeShared.h"

struct SkCustomMeshSpecificationPriv {
    using Varying   = SkCustomMeshSpecification::Varying;
    using Attribute = SkCustomMeshSpecification::Attribute;
    using ColorType = SkCustomMeshSpecification::ColorType;

    static SkSpan<const Varying> Varyings(const SkCustomMeshSpecification& spec) {
        return SkMakeSpan(spec.fVaryings);
    }

    static const SkSL::Program* VS(const SkCustomMeshSpecification& spec) { return spec.fVS.get(); }
    static const SkSL::Program* FS(const SkCustomMeshSpecification& spec) { return spec.fFS.get(); }

    static int Hash(const SkCustomMeshSpecification& spec) { return spec.fHash; }

    static ColorType GetColorType(const SkCustomMeshSpecification& spec) { return spec.fColorType; }
    static bool HasColors(const SkCustomMeshSpecification& spec) {
        return GetColorType(spec) != ColorType::kNone;
    }

    static SkColorSpace* ColorSpace(const SkCustomMeshSpecification& spec) {
        return spec.fColorSpace.get();
    }

    static SkAlphaType AlphaType(const SkCustomMeshSpecification& spec) { return spec.fAlphaType; }

    static bool HasLocalCoords(const SkCustomMeshSpecification& spec) {
        return spec.fHasLocalCoords;
    }

    static SkSLType VaryingTypeAsSLType(Varying::Type type) {
        switch (type) {
            case Varying::Type::kFloat:  return SkSLType::kFloat;
            case Varying::Type::kFloat2: return SkSLType::kFloat2;
            case Varying::Type::kFloat3: return SkSLType::kFloat3;
            case Varying::Type::kFloat4: return SkSLType::kFloat4;
            case Varying::Type::kHalf:   return SkSLType::kHalf;
            case Varying::Type::kHalf2:  return SkSLType::kHalf2;
            case Varying::Type::kHalf3:  return SkSLType::kHalf3;
            case Varying::Type::kHalf4:  return SkSLType::kHalf4;
        }
        SkUNREACHABLE;
    }

    static GrVertexAttribType AttrTypeAsVertexAttribType(Attribute::Type type) {
        switch (type) {
            case Attribute::Type::kFloat:        return kFloat_GrVertexAttribType;
            case Attribute::Type::kFloat2:       return kFloat2_GrVertexAttribType;
            case Attribute::Type::kFloat3:       return kFloat3_GrVertexAttribType;
            case Attribute::Type::kFloat4:       return kFloat4_GrVertexAttribType;
            case Attribute::Type::kUByte4_unorm: return kUByte4_norm_GrVertexAttribType;
        }
        SkUNREACHABLE;
    }

    static SkSLType AttrTypeAsSLType(Attribute::Type type) {
        switch (type) {
            case Attribute::Type::kFloat:        return SkSLType::kFloat;
            case Attribute::Type::kFloat2:       return SkSLType::kFloat2;
            case Attribute::Type::kFloat3:       return SkSLType::kFloat3;
            case Attribute::Type::kFloat4:       return SkSLType::kFloat4;
            case Attribute::Type::kUByte4_unorm: return SkSLType::kHalf4;
        }
        SkUNREACHABLE;
    }
};

bool SkValidateCustomMesh(const SkCustomMesh&);

std::unique_ptr<const char[]> SkCopyCustomMeshVB(const SkCustomMesh& cm);

std::unique_ptr<const uint16_t[]> SkCopyCustomMeshIB(const SkCustomMesh& cm);

#endif  // SK_ENABLE_SKSL

#endif
