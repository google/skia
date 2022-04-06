/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniformManager_DEFINED
#define skgpu_UniformManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkSLTypeShared.h"

class SkUniform;
class SkUniformData;

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

    sk_sp<SkUniformData> createUniformData();
    int size() const { return fStorage.count(); }

    void reset();
#ifdef SK_DEBUG
    void checkReset() const;
    void checkExpected(SkSLType type, unsigned int count);
#endif

    /*
     * Use the uniform 'definitions' to write the data in 'srcs' into internally allocated memory.
     */
    void writeUniforms(SkSpan<const SkUniform> definitions, const void** srcs);

private:
    SkSLType getUniformTypeForLayout(SkSLType type);
    void write(SkSLType type, unsigned int count, const void* src);

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

    SkTDArray<char> fStorage;
};

} // namespace skgpu

#endif // skgpu_UniformManager_DEFINED
