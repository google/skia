/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CODEGENTYPES
#define SKSL_CODEGENTYPES

namespace SkSL {

enum class PrettyPrint : bool {
    kNo = false,
    kYes = true,
};

namespace spirv {

enum ReservedId {
    // Zero is not a valid ID in SPIR-V.
    kIdInvalid = 0,

    // The following are IDs that are fixed in every SPIR-V, if present. If the backend needs to
    // transform the SPIR-V, it doesn't have to figure them out. See the transformations in
    // VulkanSpirvTransforms.cpp for where and why each of these IDs might be used.

    // %kIdTypeInt = OpTypeInt 32 1
    kIdTypeInt,
    // %kIdTypePointerInputInt = OpTypePointer Input %kIdTypeInt
    kIdTypePointerInputInt,

    // Input-attachment-related IDs. There can only be one input attachment, so these are unique.
    // %kIdTypeImageSubpassData = OpTypeImage %type SubpassData 0 0 0 2 Unknown
    kIdTypeImageSubpassData,
    // %kIdVariableImageSubpassData = OpVariable %pointer_to_kIdTypeImageSubpassData UniformConstant
    kIdVariableImageSubpassData,

    // All other SPIR-V IDs start from this.
    kIdFirstUnreserved,
};

}  // namespace spirv

}  // namespace SkSL

#endif
