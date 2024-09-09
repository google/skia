/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPersistentCacheEntry_DEFINED
#define GrPersistentCacheEntry_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkFourByteTag.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <string>

class SkReadBuffer;
namespace SkSL { struct ProgramSettings; }

// The GrPersistentCache stores opaque blobs, as far as clients are concerned. It's helpful to
// inspect certain kinds of cached data within our tools, so for those cases (GLSL, SPIR-V), we
// put the serialization logic here, to be shared by the backend code and the tool code.
namespace GrPersistentCacheUtils {

struct ShaderMetadata {
    SkSL::ProgramSettings* fSettings = nullptr;
    skia_private::TArray<std::string> fAttributeNames;
    bool fHasSecondaryColorOutput = false;
    sk_sp<SkData> fPlatformData;
};

int GetCurrentVersion();

sk_sp<SkData> PackCachedShaders(SkFourByteTag shaderType,
                                const std::string shaders[],
                                const SkSL::Program::Interface[],
                                int numInterfaces,
                                const ShaderMetadata* meta = nullptr);

SkFourByteTag GetType(SkReadBuffer* reader);

bool UnpackCachedShaders(SkReadBuffer* reader,
                         std::string shaders[],
                         SkSL::Program::Interface[],
                         int numInterfaces,
                         ShaderMetadata* meta = nullptr);

}  // namespace GrPersistentCacheUtils

#endif
