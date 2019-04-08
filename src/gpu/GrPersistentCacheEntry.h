/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPersistentCacheEntry_DEFINED
#define GrPersistentCacheEntry_DEFINED

#include "GrTypesPriv.h"
#include "SkData.h"
#include "ir/SkSLProgram.h"
#include "SkSLString.h"
#include "SkTArray.h"
#include "SkWriter32.h"

/** The GrContext persistent cache stores opaque blobs (as SkData) from the client's perspective.
    Internally, the data is actually an instance of this class, which allows some introspection. */
class GrPersistentCacheBlob {
public:
    enum EntryType {
        kInputs_Type,
        kGLBinary_Type,
        kGLBinaryFormat_Type,
        kGLSL_Type,
        kSPIRV_Type,
        kVKPipeline_Type,
    };

    struct Entry {
        EntryType    fEntryType;
        GrShaderType fShaderType;
        union {
            struct {
                uint32_t fOffset;
                uint32_t fSize;
            } fOffsetAndSize;

            SkSL::Program::Inputs fInputs;

            GrGLenum fBinaryFormat;
        };
    };

    GrPersistentCacheBlob(sk_sp<SkData> data)
            : fData(std::move(data))
            , fNumEntries(0)
            , fEntries(nullptr)
            , fBuffer(nullptr) {
        const uint8_t* buffer = fData->bytes();
        if (fData->size() < sizeof(fNumEntries)) {
            return;
        }
        memcpy(&fNumEntries, buffer, sizeof(fNumEntries));
        buffer += sizeof(fNumEntries);

        if (fData->size() < sizeof(fNumEntries) + fNumEntries * sizeof(Entry)) {
            fNumEntries = 0;
            return;
        }
        fEntries = (const Entry*)buffer;
        fBuffer = buffer + fNumEntries * sizeof(Entry);
    }

    bool get(GrShaderType shaderType, SkSL::Program::Inputs* inputs) const {
        SkASSERT(shaderType >= 0 && shaderType < kGrShaderTypeCount);
        if (auto entry = this->find(kInputs_Type, shaderType)) {
            *inputs = entry->fInputs;
            return true;
        }
        return false;
    }

    bool get(GrGLenum* binaryFormat) const {
        if (auto entry = this->find(kGLBinaryFormat_Type)) {
            *binaryFormat = entry->fBinaryFormat;
            return true;
        }
        return false;
    }

    bool get(EntryType entryType, GrShaderType shaderType, SkSL::String* string) const {
        SkASSERT(kGLSL_Type == entryType || kSPIRV_Type == entryType);
        SkASSERT(shaderType >= 0 && shaderType < kGrShaderTypeCount);
        if (auto entry = this->find(entryType, shaderType)) {
            *string = SkSL::String((const char*)(fBuffer + entry->fOffsetAndSize.fOffset),
                                   entry->fOffsetAndSize.fSize);
            return true;
        }
        return false;
    }

    const void* get(EntryType entryType, size_t* size) const {
        SkASSERT(kGLBinary_Type == entryType || kVKPipeline_Type == entryType);
        if (auto entry = this->find(entryType)) {
            *size = entry->fOffsetAndSize.fSize;
            return fBuffer + entry->fOffsetAndSize.fOffset;
        }
        return nullptr;
    }

    /** Assemble a collection of entries into a blob with a small header. */
    class Builder {
    public:
        Builder() {}

        void add(GrShaderType shaderType, const SkSL::Program::Inputs& inputs) {
            SkASSERT(shaderType >= 0 && shaderType < kGrShaderTypeCount);
            Entry& entry(fEntries.push_back());
            entry.fEntryType = kInputs_Type;
            entry.fShaderType = shaderType;
            entry.fInputs = inputs;
        }

        void add(GrGLenum binaryFormat) {
            Entry& entry(fEntries.push_back());
            entry.fEntryType = kGLBinaryFormat_Type;
            entry.fShaderType = kFragment_GrShaderType; // Irrelevant/ignored. Move into union?
            entry.fBinaryFormat = binaryFormat;
        }

        void add(EntryType entryType, GrShaderType shaderType, const SkSL::String& string) {
            SkASSERT(kGLSL_Type == entryType || kSPIRV_Type == entryType);
            SkASSERT(shaderType >= 0 && shaderType < kGrShaderTypeCount);
            Entry& entry(fEntries.push_back());
            entry.fEntryType = entryType;
            entry.fShaderType = shaderType;
            entry.fOffsetAndSize.fOffset = SkToU32(fWriter.bytesWritten());
            entry.fOffsetAndSize.fSize = SkToU32(string.size());
            fWriter.writeString(string.c_str(), string.size());
        }

        void* reserve(EntryType entryType, size_t size) {
            SkASSERT(kGLBinary_Type == entryType || kVKPipeline_Type == entryType);
            Entry& entry(fEntries.push_back());
            entry.fEntryType = entryType;
            entry.fShaderType = kFragment_GrShaderType; // Irrelevant/ignored. Move into union?
            entry.fOffsetAndSize.fOffset = SkToU32(fWriter.bytesWritten());
            entry.fOffsetAndSize.fSize = SkToU32(size);
            return fWriter.reservePad(size);
        }

        sk_sp<SkData> snapshot() {
            // TODO: Include a magic/version number as well?
            uint32_t numEntries = SkToU32(fEntries.size());
            size_t headerSize = numEntries * sizeof(Entry);
            size_t size = sizeof(numEntries) + headerSize + fWriter.bytesWritten();

            sk_sp<SkData> data = SkData::MakeUninitialized(size);
            uint8_t* buffer = (uint8_t*)data->writable_data();
            memcpy(buffer, &numEntries, sizeof(numEntries));
            memcpy(buffer + sizeof(numEntries), fEntries.begin(), headerSize);
            fWriter.flatten(buffer + sizeof(numEntries) + headerSize);
            return data;
        }

    private:
        SkSTArray<8, Entry, true> fEntries;
        SkWriter32 fWriter;
    };

private:
    const Entry* find(EntryType entryType, GrShaderType shaderType = kFragment_GrShaderType) const {
        for (uint32_t i = 0; i < fNumEntries; ++i) {
            if (entryType == fEntries[i].fEntryType && shaderType == fEntries[i].fShaderType) {
                return fEntries + i;
            }
        }
        return nullptr;
    }

    sk_sp<SkData> fData;

    uint32_t fNumEntries;
    const Entry* fEntries;
    const uint8_t* fBuffer;
};

#endif
