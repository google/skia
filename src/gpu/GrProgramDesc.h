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

class GrCaps;
class GrProgramInfo;
class GrRenderTarget;
class GrShaderCaps;

/** This class is used to generate a generic program cache key. The Dawn, Metal and Vulkan
 *  backends derive backend-specific versions which add additional information.
 */
class GrProgramDesc {
public:
    GrProgramDesc(const GrProgramDesc& other) = default;

    bool isValid() const { return !fKey.empty(); }

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
        fInitialKeyLength = other.fInitialKeyLength;
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
        desc->fKey.reset(SkToInt(keyLength));
        memcpy(desc->fKey.begin(), keyData, keyLength);
        return true;
    }

    enum {
        kHeaderSize            = 4,    // "header" in ::Build
        kMaxPreallocProcessors = 8,
        kIntsPerProcessor      = 4,    // This is an overestimate of the average effect key size.
        kPreAllocSize = kHeaderSize +
                        kMaxPreallocProcessors * sizeof(uint32_t) * kIntsPerProcessor,
    };

    SkSTArray<kPreAllocSize, uint8_t, true>& key() { return fKey; }

private:
    SkSTArray<kPreAllocSize, uint8_t, true> fKey;
    uint32_t fInitialKeyLength = 0;
};

#endif
