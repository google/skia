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

static inline sk_sp<SkData> PackCachedShaders(SkFourByteTag shaderType,
                                              const SkSL::String shaders[],
                                              const SkSL::Program::Inputs inputs[],
                                              int numInputs) {
    // For consistency (so tools can blindly pack and unpack cached shaders), we always write
    // kGrShaderTypeCount inputs. If the backend gives us fewer, we just replicate the last one.
    SkASSERT(numInputs >= 1 && numInputs <= kGrShaderTypeCount);

    SkWriter32 writer;
    writer.write32(shaderType);
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        writer.writeString(shaders[i].c_str(), shaders[i].size());
        writer.writePad(&inputs[SkTMin(i, numInputs - 1)], sizeof(SkSL::Program::Inputs));
    }
    return writer.snapshotAsData();
}

static inline SkFourByteTag UnpackCachedShaders(const SkData* data,
                                                SkSL::String shaders[],
                                                SkSL::Program::Inputs inputs[],
                                                int numInputs) {
    SkReader32 reader(data->data(), data->size());
    SkFourByteTag shaderType = reader.readU32();
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        size_t stringLen = 0;
        const char* string = reader.readString(&stringLen);
        shaders[i] = SkSL::String(string, stringLen);

        // GL, for example, only wants one set of Inputs
        if (i < numInputs) {
            reader.read(&inputs[i], sizeof(inputs[i]));
        } else {
            reader.skip(sizeof(SkSL::Program::Inputs));
        }
    }
    return shaderType;
}

}

#endif
