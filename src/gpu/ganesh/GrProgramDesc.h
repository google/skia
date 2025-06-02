/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramDesc_DEFINED
#define GrProgramDesc_DEFINED

#include "include/core/SkString.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"

#include <cstdint>
#include <cstring>

class GrCaps;
class GrProgramInfo;

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
        return SkToU32(fKey.size() * sizeof(uint32_t));
    }

    bool operator== (const GrProgramDesc& that) const {
        return this->fKey == that.fKey;
    }

    bool operator!= (const GrProgramDesc& other) const {
        return !(*this == other);
    }

    uint32_t initialKeyLength() const { return fInitialKeyLength; }

    // TODO(skbug.com/40042745): Incorporate this into caps interface (part of makeDesc, or a parallel
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
        desc->fKey.reset(SkToInt(keyLength / 4));
        memcpy(desc->fKey.begin(), keyData, keyLength);
        return true;
    }

    static constexpr size_t kHeaderSize            = 1;    // "header" in ::Build
    static constexpr size_t kMaxPreallocProcessors = 8;
    // This is an overestimate of the average effect key size.
    static constexpr size_t kIntsPerProcessor      = 4;
    static constexpr size_t kPreAllocSize          =
            kHeaderSize + kMaxPreallocProcessors * kIntsPerProcessor;

    using KeyType = skia_private::STArray<kPreAllocSize, uint32_t, true>;

    KeyType* key() { return &fKey; }

private:
    skia_private::STArray<kPreAllocSize, uint32_t, true> fKey;
    uint32_t fInitialKeyLength = 0;
};

#endif
