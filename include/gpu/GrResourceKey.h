
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceKey_DEFINED
#define GrResourceKey_DEFINED

#include "GrTypes.h"
#include "GrBinHashKey.h"

class GrResourceKey {
public:
    static GrCacheID::Domain ScratchDomain() {
        static const GrCacheID::Domain gDomain = GrCacheID::GenerateDomain();
        return gDomain;
    }

    /** Uniquely identifies the GrGpuResource subclass in the key to avoid collisions
        across resource types. */
    typedef uint8_t ResourceType;

    /** Flags set by the GrGpuResource subclass. */
    typedef uint8_t ResourceFlags;

    /** Generate a unique ResourceType */
    static ResourceType GenerateResourceType();

    /** Creates a key for resource */
    GrResourceKey(const GrCacheID& id, ResourceType type, ResourceFlags flags) {
        this->init(id.getDomain(), id.getKey(), type, flags);
    };

    GrResourceKey(const GrResourceKey& src) { fKey = src.fKey; }

    GrResourceKey() { fKey.reset(); }

    void reset(const GrCacheID& id, ResourceType type, ResourceFlags flags) {
        this->init(id.getDomain(), id.getKey(), type, flags);
    }

    uint32_t getHash() const { return fKey.getHash(); }

    bool isScratch() const {
        return ScratchDomain() ==
            *reinterpret_cast<const GrCacheID::Domain*>(fKey.getData() +
                                                        kCacheIDDomainOffset);
    }

    ResourceType getResourceType() const {
        return *reinterpret_cast<const ResourceType*>(fKey.getData() +
                                                      kResourceTypeOffset);
    }

    ResourceFlags getResourceFlags() const {
        return *reinterpret_cast<const ResourceFlags*>(fKey.getData() +
                                                       kResourceFlagsOffset);
    }

    bool operator==(const GrResourceKey& other) const { return fKey == other.fKey; }

    // A key indicating that the resource is not usable as a scratch resource.
    static GrResourceKey& NullScratchKey() {
        static const GrCacheID::Key kBogusKey = { { {0} } };
        static GrCacheID kBogusID(ScratchDomain(), kBogusKey);
        static GrResourceKey kNullScratchKey(kBogusID, NoneResourceType(), 0);
        return kNullScratchKey;
    }

    bool isNullScratch() const {
        return this->isScratch() && NoneResourceType() == this->getResourceType();
    }

private:
    enum {
        kCacheIDKeyOffset = 0,
        kCacheIDDomainOffset = kCacheIDKeyOffset + sizeof(GrCacheID::Key),
        kResourceTypeOffset = kCacheIDDomainOffset + sizeof(GrCacheID::Domain),
        kResourceFlagsOffset = kResourceTypeOffset + sizeof(ResourceType),
        kPadOffset = kResourceFlagsOffset + sizeof(ResourceFlags),
        kKeySize = SkAlign4(kPadOffset),
        kPadSize = kKeySize - kPadOffset
    };

    static ResourceType NoneResourceType() {
        static const ResourceType gNoneResourceType = GenerateResourceType();
        return gNoneResourceType;
    }

    void init(const GrCacheID::Domain domain,
              const GrCacheID::Key& key,
              ResourceType type,
              ResourceFlags flags) {
        union {
            uint8_t  fKey8[kKeySize];
            uint32_t fKey32[kKeySize / 4];
        } keyData;

        uint8_t* k = keyData.fKey8;
        memcpy(k + kCacheIDKeyOffset, key.fData8, sizeof(GrCacheID::Key));
        memcpy(k + kCacheIDDomainOffset, &domain, sizeof(GrCacheID::Domain));
        memcpy(k + kResourceTypeOffset, &type, sizeof(ResourceType));
        memcpy(k + kResourceFlagsOffset, &flags, sizeof(ResourceFlags));
        memset(k + kPadOffset, 0, kPadSize);
        fKey.setKeyData(keyData.fKey32);
    }
    GrBinHashKey<kKeySize> fKey;
};

#endif
