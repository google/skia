/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPersistentCacheEntry_DEFINED
#define GrPersistentCacheEntry_DEFINED

#include "include/core/SkData.h"
#include "include/private/GrTypesPriv.h"
#include "src/sksl/ir/SkSLProgram.h"

class SkReadBuffer;

// The GrPersistentCache stores opaque blobs, as far as clients are concerned. It's helpful to
// inspect certain kinds of cached data within our tools, so for those cases (GLSL, SPIR-V), we
// put the serialization logic here, to be shared by the backend code and the tool code.
namespace GrPersistentCacheUtils {

struct ShaderMetadata {
    SkSL::Program::Settings* fSettings = nullptr;
    SkTArray<SkSL::String> fAttributeNames;
    bool fHasCustomColorOutput = false;
    bool fHasSecondaryColorOutput = false;
    sk_sp<SkData> fPlatformData;
};

int GetCurrentVersion();

sk_sp<SkData> PackCachedShaders(SkFourByteTag shaderType,
                                const SkSL::String shaders[],
                                const SkSL::Program::Inputs inputs[],
                                int numInputs,
                                const ShaderMetadata* meta = nullptr);

SkFourByteTag GetType(SkReadBuffer* reader);

bool UnpackCachedShaders(SkReadBuffer* reader,
                         SkSL::String shaders[],
                         SkSL::Program::Inputs inputs[],
                         int numInputs,
                         ShaderMetadata* meta = nullptr);

}  // namespace GrPersistentCacheUtils

#endif
