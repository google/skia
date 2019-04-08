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

// Pack/unpack functions, to be shared by backend and debugging utils?
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

}

#endif
