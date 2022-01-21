/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniformManager_DEFINED
#define skgpu_UniformManager_DEFINED

#include "include/core/SkSpan.h"
#include "src/core/SkSLTypeShared.h"

class SkUniform;

namespace skgpu {

enum class Layout {
    kStd140,
    kStd430,
    kMetal, /** This is our own self-imposed layout we use for Metal. */
};

class UniformManager {
public:
    UniformManager(Layout layout);

    /*
     * Use the uniform 'definitions' to write the data in 'srcs' into 'dst' (if it is non-null).
     * If non-null, 'offsets' is filled in with the offset of each uniform w/in 'dst'. The
     * number of bytes that was written (or would've been written) to 'dst' is returned.
     * In practice one should call:
     *   auto bytes = writeUniforms(definitions, nullptr, nullptr, nullptr);
     *   // allocate dst and offsets memory
     *   writeUniforms(definitions, src, offsets, dst);
     */
    uint32_t writeUniforms(SkSpan<const SkUniform> definitions,
                           const void** srcs,
                           uint32_t* offsets,
                           void *dst);

private:
    SkSLType getUniformTypeForLayout(SkSLType type);

    Layout fLayout;
};

} // namespace skgpu

#endif // skgpu_UniformManager_DEFINED
