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
class GrShaderCaps;

class GrProcessorKeyBuilder {
public:
    GrProcessorKeyBuilder() = default;
    GrProcessorKeyBuilder(const GrProcessorKeyBuilder& other) = default;

    void reset() { *this = GrProcessorKeyBuilder{}; }

    void addBits(uint32_t numBits, uint32_t val, const char* label) {
        SkASSERT(numBits > 0 && numBits <= 32);
        SkASSERT(numBits == 32 || (val < (1u << numBits)));

        SkDEBUGCODE(fDescription.appendf("%s: %u\n", label, val);)

        fCurValue |= (val << fBitsUsed);
        fBitsUsed += numBits;

        if (fBitsUsed >= 32) {
            // Overflow, start a new working value
            fData.push_back(fCurValue);
            uint32_t excess = fBitsUsed - 32;
            fCurValue = excess ? (val >> (numBits - excess)) : 0;
            fBitsUsed = excess;
        }

        SkASSERT(fCurValue < (1u << fBitsUsed));
    }

    void addBytes(uint32_t numBytes, const void* data, const char* label) {
        // TODO: Make this smarter/faster?
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

    template <typename StringFunc>
    void appendComment(StringFunc&& sf) {
        #ifdef SK_DEBUG
            fDescription.append(sf());
            fDescription.append("\n");
        #endif
    }

    // Introduces a word-boundary in the key. Must be called before using the key with any cache,
    // but can also be called to create a break between generic data and backend-specific data.
    void flush() {
        if (fBitsUsed) {
            fData.push_back(fCurValue);
            fCurValue = 0;
            fBitsUsed = 0;
        }
    }

    bool empty() const { return fData.empty() && !fBitsUsed; }

    const uint32_t* data() const {
        SkASSERT(fBitsUsed == 0);  // flush() must be called when construction is complete
        return fData.begin();
    }

    size_t size() const {
        return (fData.count() + (fBitsUsed ? 1 : 0)) * sizeof(uint32_t);
    }

    GrProcessorKeyBuilder& operator=(const GrProcessorKeyBuilder& other) = default;

    bool operator==(const GrProcessorKeyBuilder& that) const {
        return fBitsUsed == that.fBitsUsed &&
               fCurValue == that.fCurValue &&
               fData == that.fData;
    }

    bool operator!= (const GrProcessorKeyBuilder& other) const {
        return !(*this == other);
    }

    void setData(const void* data, size_t length) {
        SkASSERT(SkIsAlign4(length));
        fData.reset(length / 4);
        memcpy(fData.begin(), data, length);
    }

    SkString description() const {
        #ifdef SK_DEBUG
            return fDescription;
        #else
            return SkString{};
        #endif
    }

private:
    enum {
        kHeaderSize            = 1,    // "header" in ::Build
        kMaxPreallocProcessors = 8,
        kIntsPerProcessor      = 4,    // This is an overestimate of the average effect key size.
        kPreAllocSize = kHeaderSize +
                        kMaxPreallocProcessors * kIntsPerProcessor,
    };

    SkSTArray<kPreAllocSize, uint32_t, true> fData;
    uint32_t fCurValue = 0;
    uint32_t fBitsUsed = 0;  // ... in current value

    SkDEBUGCODE(SkString fDescription;)
};

/** This class is used to generate a generic program cache key. The Dawn, Metal and Vulkan
 *  backends derive backend-specific versions which add additional information.
 */
class GrProgramDesc {
public:
    GrProgramDesc(const GrProgramDesc& other) = default;

    bool isValid() const { return !fKey.empty(); }

    // Returns this as a uint32_t array to be used as a key in the program cache.
    const uint32_t* asKey() const {
        return fKey.data();
    }

    // Gets the number of bytes in asKey(). It will be a 4-byte aligned value.
    uint32_t keyLength() const {
        SkASSERT(0 == (fKey.size() % 4));
        return fKey.size();
    }

    SkString description() const { return fKey.description(); }

    GrProgramDesc& operator= (const GrProgramDesc& other) = default;

    bool operator== (const GrProgramDesc& that) const {
        return this->fKey == that.fKey;
    }

    bool operator!= (const GrProgramDesc& other) const {
        return !(*this == other);
    }

    uint32_t initialKeyLength() const { return fInitialKeyLength; }

protected:
    friend class GrDawnCaps;
    friend class GrD3DCaps;
    friend class GrGLCaps;
    friend class GrMockCaps;
    friend class GrMtlCaps;
    friend class GrVkCaps;

    friend class GrGLGpu; // for ProgramCache to access BuildFromData

    // Creates an uninitialized key that must be populated by Build
    GrProgramDesc() {}

    /**
     * Builds a program descriptor.
     *
     * @param desc          The built descriptor
     * @param renderTarget  The target of the draw
     * @param programInfo   Program information need to build the key
     * @param caps          the caps
     **/
    static bool Build(GrProgramDesc*, GrRenderTarget*, const GrProgramInfo&, const GrCaps&);

    // This is strictly an OpenGL call since the other backends have additional data in their keys.
    static bool BuildFromData(GrProgramDesc* desc, const void* keyData, size_t keyLength) {
        if (!SkTFitsIn<int>(keyLength)) {
            return false;
        }
        desc->fKey.setData(keyData, keyLength);
        return true;
    }

    GrProcessorKeyBuilder* key() { return &fKey; }

private:
    GrProcessorKeyBuilder fKey;
    uint32_t fInitialKeyLength = 0;
};

#endif
