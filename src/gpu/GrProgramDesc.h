/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramDesc_DEFINED
#define GrProgramDesc_DEFINED

#include "include/private/GrTypesPriv.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTo.h"
#include "src/core/SkOpts.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"

class GrProgramInfo;
class GrShaderCaps;

/** This class describes a program to generate. It also serves as a program cache key */
class GrProgramDesc {
public:
    // Creates an uninitialized key that must be populated by GrGpu::buildProgramDesc()
    GrProgramDesc() {}

    /**
     * Builds a program descriptor. Before the descriptor can be used, the client must call finalize
     * on the filled in GrProgramDesc.
     *
     * @param desc          The built and finalized descriptor
     * @param renderTarget  The target of the draw
     * @param programInfo   Program information need to build the key
     * @param primitiveType Controls whether the shader will output a point size.
     * @param gpu           Pointer to the GrGpu object the program will be used with.
     **/
    static bool Build(GrProgramDesc*, const GrRenderTarget*, const GrProgramInfo&,
                      GrPrimitiveType, GrGpu*);

    // This is strictly an OpenGL call since the other backends have additional data in their
    // keys
    static bool BuildFromData(GrProgramDesc* desc, const void* keyData, size_t keyLength) {
        if (!SkTFitsIn<int>(keyLength)) {
            return false;
        }
        desc->fKey.reset(SkToInt(keyLength));
        memcpy(desc->fKey.begin(), keyData, keyLength);
        return true;
    }

    // Returns this as a uint32_t array to be used as a key in the program cache.
    const uint32_t* asKey() const {
        return reinterpret_cast<const uint32_t*>(fKey.begin());
    }

    // Gets the number of bytes in asKey(). It will be a 4-byte aligned value.
    uint32_t keyLength() const {
        SkASSERT(0 == (fKey.count() % 4));
        return fKey.count();
    }

    GrProgramDesc& operator= (const GrProgramDesc& other) {
        uint32_t keyLength = other.keyLength();
        fKey.reset(SkToInt(keyLength));
        memcpy(fKey.begin(), other.fKey.begin(), keyLength);
        return *this;
    }

    bool operator== (const GrProgramDesc& that) const {
        if (this->keyLength() != that.keyLength()) {
            return false;
        }

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

    // TODO: remove this use of the header
    bool hasPointSize() const { return this->header().fHasPointSize; }

protected:
    struct KeyHeader {
        // Set to uniquely identify any swizzling of the shader's output color(s).
        uint16_t fOutputSwizzle;
        uint8_t fColorFragmentProcessorCnt; // Can be packed into 4 bits if required.
        uint8_t fCoverageFragmentProcessorCnt;
        // Set to uniquely identify the rt's origin, or 0 if the shader does not require this info.
        uint8_t fSurfaceOriginKey : 2;
        uint8_t fProcessorFeatures : 1;
        bool fSnapVerticesToPixelCenters : 1;
        bool fHasPointSize : 1;
        uint8_t fPad : 3;
    };
    GR_STATIC_ASSERT(sizeof(KeyHeader) == 6);

    const KeyHeader& header() const { return *this->atOffset<KeyHeader, kHeaderOffset>(); }

    template<typename T, size_t OFFSET> T* atOffset() {
        return reinterpret_cast<T*>(reinterpret_cast<intptr_t>(fKey.begin()) + OFFSET);
    }

    template<typename T, size_t OFFSET> const T* atOffset() const {
        return reinterpret_cast<const T*>(reinterpret_cast<intptr_t>(fKey.begin()) + OFFSET);
    }

    // The key, stored in fKey, is composed of two parts:
    // 1. Header struct defined above.
    // 2. A Backend specific payload which includes the per-processor keys.
    enum KeyOffsets {
        kHeaderOffset = 0,
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
