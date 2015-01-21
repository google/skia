
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceKey_DEFINED
#define GrResourceKey_DEFINED

#include "GrTypes.h"
#include "SkTemplates.h"
#include "GrBinHashKey.h"

/**
 * A key used for scratch resources. The key consists of a resource type (subclass) identifier, a
 * hash, a data length, and type-specific data. A Builder object is used to initialize the
 * key contents. The contents must be initialized before the key can be used.
 */
class GrScratchKey {
public:
    /** Uniquely identifies the type of resource that is cached as scratch. */
    typedef uint32_t ResourceType;
    /** Generate a unique ResourceType. */
    static ResourceType GenerateResourceType();

    GrScratchKey() { this->reset(); }
    GrScratchKey(const GrScratchKey& that) { *this = that; }

    /** Reset to an invalid key. */
    void reset() {
        fKey.reset(kMetaDataCnt);
        fKey[kHash_MetaDataIdx] = 0;
        fKey[kTypeAndSize_MetaDataIdx] = kInvalidResourceType;
    }

    bool isValid() const { return kInvalidResourceType != this->resourceType(); }

    ResourceType resourceType() const { return fKey[kTypeAndSize_MetaDataIdx] & 0xffff; }

    uint32_t hash() const { return fKey[kHash_MetaDataIdx]; }

    size_t size() const { return SkToInt(fKey[kTypeAndSize_MetaDataIdx] >> 16); }

    const uint32_t* data() const { return &fKey[kMetaDataCnt]; }

    GrScratchKey& operator=(const GrScratchKey& that) {
        size_t bytes = that.size();
        fKey.reset(SkToInt(bytes / sizeof(uint32_t)));
        memcpy(fKey.get(), that.fKey.get(), bytes);
        return *this;
    }

    bool operator==(const GrScratchKey& that) const {
        return 0 == memcmp(fKey.get(), that.fKey.get(), this->size());
    }
    bool operator!=(const GrScratchKey& that) const { return !(*this == that); }

    /** Used to initialize scratch key. */
    class Builder {
    public:
        Builder(GrScratchKey* key, ResourceType type, int data32Count) : fKey(key) {
            SkASSERT(data32Count >= 0);
            SkASSERT(type != kInvalidResourceType);
            key->fKey.reset(kMetaDataCnt + data32Count);
            SkASSERT(type <= SK_MaxU16);
            int size = (data32Count + kMetaDataCnt) * sizeof(uint32_t);
            SkASSERT(size <= SK_MaxU16);
            key->fKey[kTypeAndSize_MetaDataIdx] = type | (size << 16);
        }

        ~Builder() { this->finish(); }

        void finish();

        uint32_t& operator[](int dataIdx) {
            SkASSERT(fKey);
            SkDEBUGCODE(size_t dataCount = fKey->size() / sizeof(uint32_t) - kMetaDataCnt;)
            SkASSERT(SkToU32(dataIdx) < dataCount);
            return fKey->fKey[kMetaDataCnt + dataIdx];
        }

    private:
        GrScratchKey* fKey;
    };

private:
    enum MetaDataIdx {
        kHash_MetaDataIdx,
        // The resource type and size are packed into a single uint32_t.
        kTypeAndSize_MetaDataIdx,

        kLastMetaDataIdx = kTypeAndSize_MetaDataIdx
    };
    static const uint32_t kInvalidResourceType = 0;
    static const uint32_t kMetaDataCnt = kLastMetaDataIdx + 1;

    friend class TestResource; // For unit test to access kMetaDataCnt.

    // Stencil and textures each require 2 uint32_t values.
    SkAutoSTArray<kMetaDataCnt + 2, uint32_t> fKey;
};

class GrResourceKey {
public:
    /** Flags set by the GrGpuResource subclass. */
    typedef uint8_t ResourceFlags;

    /** Creates a key for resource */
    GrResourceKey(const GrCacheID& id, ResourceFlags flags) {
        this->init(id.getDomain(), id.getKey(), flags);
    };

    GrResourceKey(const GrResourceKey& src) { fKey = src.fKey; }

    GrResourceKey() { fKey.reset(); }

    void reset(const GrCacheID& id, ResourceFlags flags) {
        this->init(id.getDomain(), id.getKey(), flags);
    }

    uint32_t getHash() const { return fKey.getHash(); }

    ResourceFlags getResourceFlags() const {
        return *reinterpret_cast<const ResourceFlags*>(fKey.getData() +
                                                       kResourceFlagsOffset);
    }

    bool operator==(const GrResourceKey& other) const { return fKey == other.fKey; }

    // A key indicating that the resource is not usable as a scratch resource.
    static GrResourceKey& NullScratchKey();

private:
    enum {
        kCacheIDKeyOffset = 0,
        kCacheIDDomainOffset = kCacheIDKeyOffset + sizeof(GrCacheID::Key),
        kResourceFlagsOffset = kCacheIDDomainOffset + sizeof(GrCacheID::Domain),
        kPadOffset = kResourceFlagsOffset + sizeof(ResourceFlags),
        kKeySize = SkAlign4(kPadOffset),
        kPadSize = kKeySize - kPadOffset
    };

    void init(const GrCacheID::Domain domain, const GrCacheID::Key& key, ResourceFlags flags) {
        union {
            uint8_t  fKey8[kKeySize];
            uint32_t fKey32[kKeySize / 4];
        } keyData;

        uint8_t* k = keyData.fKey8;
        memcpy(k + kCacheIDKeyOffset, key.fData8, sizeof(GrCacheID::Key));
        memcpy(k + kCacheIDDomainOffset, &domain, sizeof(GrCacheID::Domain));
        memcpy(k + kResourceFlagsOffset, &flags, sizeof(ResourceFlags));
        memset(k + kPadOffset, 0, kPadSize);
        fKey.setKeyData(keyData.fKey32);
    }
    GrBinHashKey<kKeySize> fKey;
};

#endif
