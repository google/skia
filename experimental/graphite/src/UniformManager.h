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

enum class CType : unsigned;

enum class Layout {
    kStd140,
    kStd430,
    kMetal, /** This is our own self-imposed layout we use for Metal. */
};

class UniformManager {
public:
    UniformManager(Layout layout);

    void reset();
#ifdef SK_DEBUG
    void checkReset() const;
#endif

    /*
     * Use the uniform 'definitions' to write the data in 'srcs' into 'dst' (if it is non-null).
     * The number of bytes that was written (or would've been written) to 'dst' is returned.
     * In practice one should call:
     *   auto bytes = writeUniforms(definitions, nullptr, nullptr);
     *   // allocate dst memory
     *   writeUniforms(definitions, src, dst);
     */
    uint32_t writeUniforms(SkSpan<const SkUniform> definitions,
                           const void** srcs,
                           char *dst);

private:
    SkSLType getUniformTypeForLayout(SkSLType type);


    using WriteUniformFn = uint32_t(*)(SkSLType type,
                                       CType ctype,
                                       void *dest,
                                       int n,
                                       const void *src);

    WriteUniformFn fWriteUniform;
    Layout fLayout;  // TODO: eventually 'fLayout' will not need to be stored
#ifdef SK_DEBUG
    uint32_t fCurUBOOffset;
    uint32_t fCurUBOMaxAlignment;
#endif // SK_DEBUG
    uint32_t fOffset;
};

} // namespace skgpu

#endif // skgpu_UniformManager_DEFINED
