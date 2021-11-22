/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramDesc_DEFINED
#define GrProgramDesc_DEFINED

#include "include/core/SkString.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTo.h"

#include <limits.h>

class GrCaps;
class GrProgramInfo;
class GrRenderTarget;

class GrProcessorKeyBuilder {
public:
    GrProcessorKeyBuilder(SkTArray<uint32_t, true>* data) : fData(data) {}

    virtual ~GrProcessorKeyBuilder() {
        // Ensure that flush was called before we went out of scope
        SkASSERT(fBitsUsed == 0);
    }

    virtual void addBits(uint32_t numBits, uint32_t val, const char* label) {
        SkASSERT(numBits > 0 && numBits <= 32);
        SkASSERT(numBits == 32 || (val < (1u << numBits)));

        fCurValue |= (val << fBitsUsed);
        fBitsUsed += numBits;

        if (fBitsUsed >= 32) {
            // Overflow, start a new working value
            fData->push_back(fCurValue);
            uint32_t excess = fBitsUsed - 32;
            fCurValue = excess ? (val >> (numBits - excess)) : 0;
            fBitsUsed = excess;
        }

        SkASSERT(fCurValue < (1u << fBitsUsed));
    }

    void addBytes(uint32_t numBytes, const void* data, const char* label) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
        for (; numBytes --> 0; bytes++) {
            this->addBits(8, *bytes, label);
        }
    }

    void addBool(bool b, const char* label) {
        this->addBits(1, b, label);
    }

    void add32(uint32_t v, const char* label = "unknown") {
        this->addBits(32, v, label);
    }

    virtual void appendComment(const char* comment) {}

    // Introduces a word-boundary in the key. Must be called before using the key with any cache,
    // but can also be called to create a break between generic data and backend-specific data.
    void flush() {
        if (fBitsUsed) {
            fData->push_back(fCurValue);
            fCurValue = 0;
            fBitsUsed = 0;
        }
    }

private:
    SkTArray<uint32_t, true>* fData;
    uint32_t fCurValue = 0;
    uint32_t fBitsUsed = 0;  // ... in current value
};

class GrProcessorStringKeyBuilder : public GrProcessorKeyBuilder {
public:
    GrProcessorStringKeyBuilder(SkTArray<uint32_t, true>* data) : INHERITED(data) {}

    void addBits(uint32_t numBits, uint32_t val, const char* label) override {
        INHERITED::addBits(numBits, val, label);
        fDescription.appendf("%s: %u\n", label, val);
    }

    void appendComment(const char* comment) override {
        fDescription.appendf("%s\n", comment);
    }

    SkString description() const { return fDescription; }

private:
    using INHERITED = GrProcessorKeyBuilder;
    SkString fDescription;
};

/** This class is used to generate a generic program cache key. The Dawn, Metal and Vulkan
 *  backends derive backend-specific versions which add additional information.
 */
class GrProgramDesc {
public:
    GrProgramDesc(const GrProgramDesc& other) = default;
    GrProgramDesc& operator=(const GrProgramDesc &other) = default;

    bool isValid() const { return !fKey.empty(); }
    void reset() { *this = GrProgramDesc{}; }

    // Returns this as a uint32_t array to be used as a key in the program cache.
    const uint32_t* asKey() const {
        return fKey.data();
    }

    // Gets the number of bytes in asKey(). It will be a 4-byte aligned value.
    uint32_t keyLength() const {
        return fKey.size() * sizeof(uint32_t);
    }

    bool operator== (const GrProgramDesc& that) const {
        return this->fKey == that.fKey;
    }

    bool operator!= (const GrProgramDesc& other) const {
        return !(*this == other);
    }

    uint32_t initialKeyLength() const { return fInitialKeyLength; }

    // TODO(skia:11372): Incorporate this into caps interface (part of makeDesc, or a parallel
    // function), so other backends can include their information in the description.
    static SkString Describe(const GrProgramInfo&, const GrCaps&);

protected:
    friend class GrDawnCaps;
    friend class GrD3DCaps;
    friend class GrGLCaps;
    friend class GrMockCaps;
    friend class GrMtlCaps;
    friend class GrVkCaps;

    friend class GrGLGpu; // for ProgramCache to access BuildFromData
    friend class GrMtlResourceProvider; // for PipelineStateCache to access BuildFromData

    // Creates an uninitialized key that must be populated by Build
    GrProgramDesc() {}

    /**
     * Builds a program descriptor.
     *
     * @param desc          The built descriptor
     * @param programInfo   Program information need to build the key
     * @param caps          the caps
     **/
    static void Build(GrProgramDesc*, const GrProgramInfo&, const GrCaps&);

    // This is strictly an OpenGL call since the other backends have additional data in their keys.
    static bool BuildFromData(GrProgramDesc* desc, const void* keyData, size_t keyLength) {
        if (!SkTFitsIn<int>(keyLength) || !SkIsAlign4(keyLength)) {
            return false;
        }
        desc->fKey.reset(keyLength / 4);
        memcpy(desc->fKey.begin(), keyData, keyLength);
        return true;
    }

    enum {
        kHeaderSize            = 1,    // "header" in ::Build
        kMaxPreallocProcessors = 8,
        kIntsPerProcessor      = 4,    // This is an overestimate of the average effect key size.
        kPreAllocSize = kHeaderSize +
                        kMaxPreallocProcessors * kIntsPerProcessor,
    };

    using KeyType = SkSTArray<kPreAllocSize, uint32_t, true>;

    KeyType* key() { return &fKey; }

private:
    SkSTArray<kPreAllocSize, uint32_t, true> fKey;
    uint32_t fInitialKeyLength = 0;
};

#endif
