/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrPersistentCacheUtils.h"

#include "include/private/SkSLString.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

namespace GrPersistentCacheUtils {

static constexpr int kCurrentVersion = 8;

int GetCurrentVersion() {
    // The persistent cache stores a copy of the SkSL::Program::Inputs struct. If you alter the
    // Program::Inputs struct in any way, you must increment kCurrentVersion to invalidate the
    // outdated persistent cache files. The KnownSkSLProgramInputs struct must also be updated to
    // match the new contents of Program::Inputs.
    struct KnownSkSLProgramInputs { bool useRTFlipUniform; };
    static_assert(sizeof(SkSL::Program::Inputs) == sizeof(KnownSkSLProgramInputs));

    return kCurrentVersion;
}

sk_sp<SkData> PackCachedShaders(SkFourByteTag shaderType,
                                const SkSL::String shaders[],
                                const SkSL::Program::Inputs inputs[],
                                int numInputs,
                                const ShaderMetadata* meta) {
    // For consistency (so tools can blindly pack and unpack cached shaders), we always write
    // kGrShaderTypeCount inputs. If the backend gives us fewer, we just replicate the last one.
    SkASSERT(numInputs >= 1 && numInputs <= kGrShaderTypeCount);

    SkBinaryWriteBuffer writer;
    writer.writeInt(kCurrentVersion);
    writer.writeUInt(shaderType);
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        writer.writeByteArray(shaders[i].c_str(), shaders[i].size());
        writer.writePad32(&inputs[std::min(i, numInputs - 1)], sizeof(SkSL::Program::Inputs));
    }
    writer.writeBool(SkToBool(meta));
    if (meta) {
        writer.writeBool(SkToBool(meta->fSettings));
        if (meta->fSettings) {
            writer.writeBool(meta->fSettings->fFragColorIsInOut);
            writer.writeBool(meta->fSettings->fForceHighPrecision);
            writer.writeBool(meta->fSettings->fUsePushConstants);
        }

        writer.writeInt(meta->fAttributeNames.count());
        for (const auto& attr : meta->fAttributeNames) {
            writer.writeByteArray(attr.c_str(), attr.size());
        }

        writer.writeBool(meta->fHasCustomColorOutput);
        writer.writeBool(meta->fHasSecondaryColorOutput);

        if (meta->fPlatformData) {
            writer.writeByteArray(meta->fPlatformData->data(), meta->fPlatformData->size());
        }
    }
    return writer.snapshotAsData();
}

SkFourByteTag GetType(SkReadBuffer* reader) {
    constexpr SkFourByteTag kInvalidTag = ~0;
    int version           = reader->readInt();
    SkFourByteTag typeTag = reader->readUInt();
    return reader->validate(version == kCurrentVersion) ? typeTag : kInvalidTag;
}

bool UnpackCachedShaders(SkReadBuffer* reader,
                         SkSL::String shaders[],
                         SkSL::Program::Inputs inputs[],
                         int numInputs,
                         ShaderMetadata* meta) {
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        size_t shaderLen = 0;
        const char* shaderBuf = static_cast<const char*>(reader->skipByteArray(&shaderLen));
        if (shaderBuf) {
            shaders[i].assign(shaderBuf, shaderLen);
        }

        // GL, for example, only wants one set of Inputs
        if (i < numInputs) {
            reader->readPad32(&inputs[i], sizeof(inputs[i]));
        } else {
            reader->skip(sizeof(SkSL::Program::Inputs));
        }
    }
    if (reader->readBool() && meta) {
        SkASSERT(meta->fSettings != nullptr);

        if (reader->readBool()) {
            meta->fSettings->fFragColorIsInOut   = reader->readBool();
            meta->fSettings->fForceHighPrecision = reader->readBool();
            meta->fSettings->fUsePushConstants   = reader->readBool();
        }

        meta->fAttributeNames.resize(reader->readInt());
        for (auto& attr : meta->fAttributeNames) {
            size_t attrLen = 0;
            const char* attrName = static_cast<const char*>(reader->skipByteArray(&attrLen));
            if (attrName) {
                attr.assign(attrName, attrLen);
            }
        }

        meta->fHasCustomColorOutput    = reader->readBool();
        meta->fHasSecondaryColorOutput = reader->readBool();

        // a given platform will be responsible for reading its data
    }

    if (!reader->isValid()) {
        for (int i = 0; i < kGrShaderTypeCount; ++i) {
            shaders[i].clear();
        }
    }
    return reader->isValid();
}

}  // namespace GrPersistentCacheUtils
