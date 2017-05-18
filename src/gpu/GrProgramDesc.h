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
#include "SkOpts.h"
#include "SkTArray.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class GrShaderCaps;
class GrPipeline;
class GrPrimitiveProcessor;

/** This class describes a program to generate. It also serves as a program cache key */
class GrProgramDesc {
public:
    // Creates an uninitialized key that must be populated by GrGpu::buildProgramDesc()
    GrProgramDesc() {}

    /**
    * Builds a program descriptor. Before the descriptor can be used, the client must call finalize
    * on the returned GrProgramDesc.
    *
    * @param GrPrimitiveProcessor The geometry
    * @param hasPointSize Controls whether the shader will output a point size.
    * @param GrPipeline  The optimized drawstate.  The descriptor will represent a program
    *                        which this optstate can use to draw with.  The optstate contains
    *                        general draw information, as well as the specific color, geometry,
    *                        and coverage stages which will be used to generate the GL Program for
    *                        this optstate.
    * @param GrShaderCaps   Capabilities of the shading language.
    * @param GrProgramDesc  The built and finalized descriptor
    **/
    static bool Build(GrProgramDesc*,
                      const GrPrimitiveProcessor&,
                      bool hasPointSize,
                      const GrPipeline&,
                      const GrShaderCaps&);

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

    void setSurfaceOriginKey(int key) {
        KeyHeader* header = this->atOffset<KeyHeader, kHeaderOffset>();
        header->fSurfaceOriginKey = key;
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
        // Set to uniquely identify the sample pattern, or 0 if the shader doesn't use sample
        // locations.
        uint8_t                     fSamplePatternKey;
        // Set to uniquely idenitify any swizzling of the shader's output color(s).
        uint8_t                     fOutputSwizzle;
        uint8_t                     fColorFragmentProcessorCnt : 4;
        uint8_t                     fCoverageFragmentProcessorCnt : 4;
        // Set to uniquely identify the rt's origin, or 0 if the shader does not require this info.
        uint8_t                     fSurfaceOriginKey : 2;
        uint8_t                     fSnapVerticesToPixelCenters : 1;
        uint8_t                     fHasPointSize : 1;
        uint8_t                     fPad : 4;
    };
    GR_STATIC_ASSERT(sizeof(KeyHeader) == 4);

    // This should really only be used internally, base classes should return their own headers
    const KeyHeader& header() const { return *this->atOffset<KeyHeader, kHeaderOffset>(); }

    void finalize() {
        int keyLength = fKey.count();
        SkASSERT(0 == (keyLength % 4));
        *(this->atOffset<uint32_t, GrProgramDesc::kLengthOffset>()) = SkToU32(keyLength);

        uint32_t* checksum = this->atOffset<uint32_t, GrProgramDesc::kChecksumOffset>();
        *checksum = 0;  // We'll hash through these bytes, so make sure they're initialized.
        *checksum = SkOpts::hash(fKey.begin(), keyLength);
    }

protected:
    template<typename T, size_t OFFSET> T* atOffset() {
        return reinterpret_cast<T*>(reinterpret_cast<intptr_t>(fKey.begin()) + OFFSET);
    }

    template<typename T, size_t OFFSET> const T* atOffset() const {
        return reinterpret_cast<const T*>(reinterpret_cast<intptr_t>(fKey.begin()) + OFFSET);
    }

    // The key, stored in fKey, is composed of four parts:
    // 1. uint32_t for total key length.
    // 2. uint32_t for a checksum.
    // 3. Header struct defined above.
    // 4. A Backend specific payload which includes the per-processor keys.
    enum KeyOffsets {
        // Part 1.
        kLengthOffset = 0,
        // Part 2.
        kChecksumOffset = kLengthOffset + sizeof(uint32_t),
        // Part 3.
        kHeaderOffset = kChecksumOffset + sizeof(uint32_t),
        kHeaderSize = SkAlign4(sizeof(KeyHeader)),
        // Part 4.
        // This is the offset into the backenend specific part of the key, which includes
        // per-processor keys.
        kProcessorKeysOffset = kHeaderOffset + kHeaderSize,
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
