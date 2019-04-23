/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPersistentCacheEntry_DEFINED
#define GrPersistentCacheEntry_DEFINED

#include "include/core/SkData.h"
#include "include/private/GrTypesPriv.h"
#include "src/core/SkReader32.h"
#include "src/core/SkWriter32.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLProgram.h"

// The GrPersistentCache stores opaque blobs, as far as clients are concerned. It's helpful to
// inspect certain kinds of cached data within our tools, so for those cases (GLSL, SPIR-V), we
// put the serialization logic here, to be shared by the backend code and the tool code.
namespace GrPersistentCacheUtils {

static inline sk_sp<SkData> PackCachedGLSL(const SkSL::Program::Inputs& inputs,
                                           const SkSL::String glsl[]) {
    SkWriter32 writer;
    writer.writePad(&inputs, sizeof(inputs));
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        writer.writeString(glsl[i].c_str(), glsl[i].size());
    }
    return writer.snapshotAsData();
}

static inline void UnpackCachedGLSL(const SkData* data, SkSL::Program::Inputs* inputs,
                                    SkSL::String glsl[]) {
    SkReader32 reader(data->data(), data->size());
    reader.read(inputs, sizeof(*inputs));
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        size_t stringLen = 0;
        const char* string = reader.readString(&stringLen);
        glsl[i] = SkSL::String(string, stringLen);
    }
}

static inline sk_sp<SkData> PackCachedSPIRV(const SkSL::String shaders[],
                                            const SkSL::Program::Inputs inputs[]) {
    SkWriter32 writer;
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        writer.writeString(shaders[i].c_str(), shaders[i].size());
        writer.writePad(&inputs[i], sizeof(inputs[i]));
    }
    return writer.snapshotAsData();
}

static inline void UnpackCachedSPIRV(const SkData* data, SkSL::String shaders[],
                                     SkSL::Program::Inputs inputs[]) {
    SkReader32 reader(data->data(), data->size());
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        size_t stringLen = 0;
        const char* string = reader.readString(&stringLen);
        shaders[i] = SkSL::String(string, stringLen);
        reader.read(&inputs[i], sizeof(inputs[i]));
    }
}

}

#endif
