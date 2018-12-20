/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkSamplerYcbcrConverison_DEFINED
#define GrVkSamplerYcbcrConverison_DEFINED

#include "GrVkResource.h"

#include "SkOpts.h"
#include "vk/GrVkTypes.h"

class GrVkGpu;

class GrVkSamplerYcbcrConversion : public GrVkResource {
public:
    static GrVkSamplerYcbcrConversion* Create(const GrVkGpu* gpu, const GrVkYcbcrConversionInfo&);

    VkSamplerYcbcrConversion ycbcrConversion() const { return fYcbcrConversion; }

    struct Key {
        Key() : fExternalFormat(0), fConversionKey(0) {}
        Key(uint64_t externalFormat, uint8_t conversionKey) {
            memset(this, 0, sizeof(Key));
            fExternalFormat = externalFormat;
            fConversionKey = conversionKey;
        }

        uint64_t fExternalFormat;
        uint8_t  fConversionKey;

        bool operator==(const Key& that) const {
            return this->fExternalFormat == that.fExternalFormat &&
                   this->fConversionKey == that.fConversionKey;
        }
    };

    // Helpers for hashing GrVkSamplerYcbcrConversion
    static Key GenerateKey(const GrVkYcbcrConversionInfo& ycbcrInfo);

    static const Key& GetKey(const GrVkSamplerYcbcrConversion& ycbcrConversion) {
        return ycbcrConversion.fKey;
    }
    static uint32_t Hash(const Key& key) {
        return SkOpts::hash(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
    }

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkSamplerYcbcrConversion: %d (%d refs)\n", fYcbcrConversion, this->getRefCnt());
    }
#endif

private:
    GrVkSamplerYcbcrConversion(VkSamplerYcbcrConversion ycbcrConversion, Key key)
            : INHERITED()
            , fYcbcrConversion(ycbcrConversion)
            , fKey(key) {}

    void freeGPUData(GrVkGpu* gpu) const override;

    VkSamplerYcbcrConversion fYcbcrConversion;
    Key                      fKey;

    typedef GrVkResource INHERITED;
};

#endif

