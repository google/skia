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
                                              int numInputs,
                                              const SkSL::Program::Settings* settings) {
    // For consistency (so tools can blindly pack and unpack cached shaders), we always write
    // kGrShaderTypeCount inputs. If the backend gives us fewer, we just replicate the last one.
    SkASSERT(numInputs >= 1 && numInputs <= kGrShaderTypeCount);

    SkWriter32 writer;
    writer.write32(shaderType);
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        writer.writeString(shaders[i].c_str(), shaders[i].size());
        writer.writePad(&inputs[SkTMin(i, numInputs - 1)], sizeof(SkSL::Program::Inputs));
    }
    writer.writeBool(SkToBool(settings));
    if (settings) {
        writer.writeBool(settings->fFlipY);
        writer.writeBool(settings->fFragColorIsInOut);
        writer.writeBool(settings->fForceHighPrecision);
    }
    return writer.snapshotAsData();
}

static inline void UnpackCachedShaders(SkReader32* reader,
                                       SkSL::String shaders[],
                                       SkSL::Program::Inputs inputs[],
                                       int numInputs,
                                       SkSL::Program::Settings* settings = nullptr) {
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        size_t stringLen = 0;
        const char* string = reader->readString(&stringLen);
        shaders[i] = SkSL::String(string, stringLen);

        // GL, for example, only wants one set of Inputs
        if (i < numInputs) {
            reader->read(&inputs[i], sizeof(inputs[i]));
        } else {
            reader->skip(sizeof(SkSL::Program::Inputs));
        }
    }
    if (reader->readBool() && settings) {
        settings->fFlipY = reader->readBool();
        settings->fFragColorIsInOut = reader->readBool();
        settings->fForceHighPrecision = reader->readBool();
    }
}

}

#endif
