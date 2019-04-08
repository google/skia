/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPersistentCacheEntry_DEFINED
#define GrPersistentCacheEntry_DEFINED

#include "GrTypesPriv.h"
#include "SkReader32.h"
#include "ir/SkSLProgram.h"
#include "SkSLString.h"
#include "SkWriter32.h"

// The GrPersistentCache stores opaque blobs, as far as clients are concerned. It's helpful to
// inspect certain kinds of cached data within our tools, so for those cases (GLSL, SPIR-V), we
// put the serialization logic here, to be shared by the backend code and the tool code.
namespace GrPersistentCacheUtils {

static inline void PackCachedGLSL(SkWriter32& writer, const SkSL::Program::Inputs& inputs,
                                  const SkSL::String glsl[]) {
    writer.writePad(&inputs, sizeof(inputs));
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        writer.writeString(glsl[i].c_str(), glsl[i].size());
    }
}

static inline void UnpackCachedGLSL(SkReader32& reader, SkSL::Program::Inputs* inputs,
                                    SkSL::String glsl[]) {
    reader.read(inputs, sizeof(*inputs));
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        size_t stringLen = 0;
        const char* string = reader.readString(&stringLen);
        glsl[i] = SkSL::String(string, stringLen);
    }
}

static inline void PackCachedSPIRV(SkWriter32& writer, const SkSL::String shaders[],
                                   const SkSL::Program::Inputs inputs[]) {
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        writer.writeString(shaders[i].c_str(), shaders[i].size());
        writer.writePad(&inputs[i], sizeof(inputs[i]));
    }
}

static inline void UnpackCachedSPIRV(SkReader32& reader, SkSL::String shaders[],
                                     SkSL::Program::Inputs inputs[]) {
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        size_t stringLen = 0;
        const char* string = reader.readString(&stringLen);
        shaders[i] = SkSL::String(string, stringLen);
        reader.read(&inputs[i], sizeof(inputs[i]));
    }
}

}

#endif
