/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramDesc_DEFINED
#define GrProgramDesc_DEFINED

#include "GrColor.h"
#include "GrTypesPriv.h"
#include "SkChecksum.h"

/** This class describes a program to generate. It also serves as a program cache key. Very little
    of this is GL-specific. The GL-specific parts could be factored out into a subclass. */
class GrProgramDesc {
public:
    // Creates an uninitialized key that must be populated by GrGpu::buildProgramDesc()
    GrProgramDesc() {}

    // Returns this as a uint32_t array to be used as a key in the program cache.
    const uint32_t* asKey() const {
        return reinterpret_cast<const uint32_t*>(fKey.begin());
    }

    // Gets the number of bytes in asKey(). It will be a 4-byte aligned value. When comparing two
    // keys the size of either key can be used with memcmp() since the lengths themselves begin the
    // keys and thus the memcmp will exit early if the keys are of different lengths.
    uint32_t keyLength() const { return *this->atOffset<uint32_t, kLengthOffset>(); }

    // Gets the a checksum of the key. Can be used as a hash value for a fast lookup in a cache.
    uint32_t getChecksum() const { return *this->atOffset<uint32_t, kChecksumOffset>(); }

    GrProgramDesc& operator= (const GrProgramDesc& other) {
        uint32_t keyLength = other.keyLength();
        fKey.reset(SkToInt(keyLength));
        memcpy(fKey.begin(), other.fKey.begin(), keyLength);
        return *this;
    }

    bool operator== (const GrProgramDesc& that) const {
        SkASSERT(SkIsAlign4(this->keyLength()));
        int l = this->keyLength() >> 2;
        const uint32_t* aKey = this->asKey();
        const uint32_t* bKey = that.asKey();
        for (int i = 0; i < l; ++i) {
            if (aKey[i] != bKey[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!= (const GrProgramDesc& other) const {
        return !(*this == other);
    }

    static bool Less(const GrProgramDesc& a, const GrProgramDesc& b) {
        SkASSERT(SkIsAlign4(a.keyLength()));
        int l = a.keyLength() >> 2;
        const uint32_t* aKey = a.asKey();
        const uint32_t* bKey = b.asKey();
        for (int i = 0; i < l; ++i) {
            if (aKey[i] != bKey[i]) {
                return aKey[i] < bKey[i] ? true : false;
            }
        }
        return false;
    }

    struct KeyHeader {
        uint8_t                     fFragPosKey;   // set by GrGLShaderBuilder if there are
                                                   // effects that read the fragment position.
                                                   // Otherwise, 0.
        uint8_t                     fSnapVerticesToPixelCenters;
        int8_t                      fColorEffectCnt;
        int8_t                      fCoverageEffectCnt;
        uint8_t                     fIgnoresCoverage;
    };
    GR_STATIC_ASSERT(sizeof(KeyHeader) == 5);

    int numColorEffects() const {
        return this->header().fColorEffectCnt;
    }

    int numCoverageEffects() const {
        return this->header().fCoverageEffectCnt;
    }

    int numTotalEffects() const { return this->numColorEffects() + this->numCoverageEffects(); }

    // This should really only be used internally, base classes should return their own headers
    const KeyHeader& header() const { return *this->atOffset<KeyHeader, kHeaderOffset>(); }

protected:
    template<typename T, size_t OFFSET> T* atOffset() {
        return reinterpret_cast<T*>(reinterpret_cast<intptr_t>(fKey.begin()) + OFFSET);
    }

    template<typename T, size_t OFFSET> const T* atOffset() const {
        return reinterpret_cast<const T*>(reinterpret_cast<intptr_t>(fKey.begin()) + OFFSET);
    }

    void finalize() {
        int keyLength = fKey.count();
        SkASSERT(0 == (keyLength % 4));
        *(this->atOffset<uint32_t, GrProgramDesc::kLengthOffset>()) = SkToU32(keyLength);

        uint32_t* checksum = this->atOffset<uint32_t, GrProgramDesc::kChecksumOffset>();
        *checksum = 0;  // We'll hash through these bytes, so make sure they're initialized.
        *checksum = SkChecksum::Murmur3(fKey.begin(), keyLength);
    }

    // The key, stored in fKey, is composed of four parts:
    // 1. uint32_t for total key length.
    // 2. uint32_t for a checksum.
    // 3. Header struct defined above.  Also room for extensions to the header
    // 4. A Backend specific payload.  Room is preallocated for this
    enum KeyOffsets {
        // Part 1.
        kLengthOffset = 0,
        // Part 2.
        kChecksumOffset = kLengthOffset + sizeof(uint32_t),
        // Part 3.
        kHeaderOffset = kChecksumOffset + sizeof(uint32_t),
        kHeaderSize = SkAlign4(2 * sizeof(KeyHeader)),
    };

    enum {
        kMaxPreallocProcessors = 8,
        kIntsPerProcessor      = 4,    // This is an overestimate of the average effect key size.
        kPreAllocSize = kHeaderOffset + kHeaderSize +
                        kMaxPreallocProcessors * sizeof(uint32_t) * kIntsPerProcessor,
    };

    SkSTArray<kPreAllocSize, uint8_t, true>& key() { return fKey; }
    const SkSTArray<kPreAllocSize, uint8_t, true>& key() const { return fKey; }

private:
    SkSTArray<kPreAllocSize, uint8_t, true> fKey;
};

#endif
