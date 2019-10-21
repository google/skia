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

struct ShaderMetadata {
    SkSL::Program::Settings* fSettings = nullptr;
    SkTArray<SkSL::String> fAttributeNames;
    bool fHasCustomColorOutput = false;
    bool fHasSecondaryColorOutput = false;
};

static inline sk_sp<SkData> PackCachedShaders(SkFourByteTag shaderType,
                                              const SkSL::String shaders[],
                                              const SkSL::Program::Inputs inputs[],
                                              int numInputs,
                                              const ShaderMetadata* meta = nullptr) {
    // For consistency (so tools can blindly pack and unpack cached shaders), we always write
    // kGrShaderTypeCount inputs. If the backend gives us fewer, we just replicate the last one.
    SkASSERT(numInputs >= 1 && numInputs <= kGrShaderTypeCount);

    SkWriter32 writer;
    writer.write32(shaderType);
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        writer.writeString(shaders[i].c_str(), shaders[i].size());
        writer.writePad(&inputs[SkTMin(i, numInputs - 1)], sizeof(SkSL::Program::Inputs));
    }
    writer.writeBool(SkToBool(meta));
    if (meta) {
        writer.writeBool(SkToBool(meta->fSettings));
        if (meta->fSettings) {
            writer.writeBool(meta->fSettings->fFlipY);
            writer.writeBool(meta->fSettings->fFragColorIsInOut);
            writer.writeBool(meta->fSettings->fForceHighPrecision);
        }

        writer.writeInt(meta->fAttributeNames.count());
        for (const auto& attr : meta->fAttributeNames) {
            writer.writeString(attr.c_str(), attr.size());
        }

        writer.writeBool(meta->fHasCustomColorOutput);
        writer.writeBool(meta->fHasSecondaryColorOutput);
    }
    return writer.snapshotAsData();
}

static inline void UnpackCachedShaders(SkReader32* reader,
                                       SkSL::String shaders[],
                                       SkSL::Program::Inputs inputs[],
                                       int numInputs,
                                       ShaderMetadata* meta = nullptr) {
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
    if (reader->readBool() && meta) {
        SkASSERT(meta->fSettings != nullptr);

        if (reader->readBool()) {
            meta->fSettings->fFlipY              = reader->readBool();
            meta->fSettings->fFragColorIsInOut   = reader->readBool();
            meta->fSettings->fForceHighPrecision = reader->readBool();
        }

        meta->fAttributeNames.resize(reader->readInt());
        for (int i = 0; i < meta->fAttributeNames.count(); ++i) {
            size_t stringLen = 0;
            const char* string = reader->readString(&stringLen);
            meta->fAttributeNames[i] = SkSL::String(string, stringLen);
        }

        meta->fHasCustomColorOutput    = reader->readBool();
        meta->fHasSecondaryColorOutput = reader->readBool();
    }
}

}

#endif
