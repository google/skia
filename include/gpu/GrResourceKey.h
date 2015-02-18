
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

uint32_t GrResourceKeyHash(const uint32_t* data, size_t size);

class GrResourceKey {
public:
    uint32_t hash() const {
        this->validate();
        return fKey[kHash_MetaDataIdx];
    }

    size_t size() const {
        this->validate();
        SkASSERT(this->isValid());
        return this->internalSize();
    }

protected:
    static const uint32_t kInvalidDomain = 0;

    GrResourceKey() { this->reset(); }

    /** Reset to an invalid key. */
    void reset() {
        GR_STATIC_ASSERT((uint16_t)kInvalidDomain == kInvalidDomain);
        fKey.reset(kMetaDataCnt);
        fKey[kHash_MetaDataIdx] = 0;
        fKey[kDomainAndSize_MetaDataIdx] = kInvalidDomain;
    }

    bool operator==(const GrResourceKey& that) const {
        SkASSERT(this->isValid() && that.isValid());
        return 0 == memcmp(fKey.get(), that.fKey.get(), this->size());
    }

    GrResourceKey& operator=(const GrResourceKey& that) {
        SkASSERT(that.isValid());
        if (this != &that) {
            size_t bytes = that.size();
            SkASSERT(SkIsAlign4(bytes));
            fKey.reset(SkToInt(bytes / sizeof(uint32_t)));
            memcpy(fKey.get(), that.fKey.get(), bytes);
            this->validate();
        }
        return *this;
    }

    bool isValid() const { return kInvalidDomain != this->domain(); }

    uint32_t domain() const { return fKey[kDomainAndSize_MetaDataIdx] & 0xffff; }

    /** size of the key data, excluding meta-data (hash, domain, etc).  */
    size_t dataSize() const { return this->size() - 4 * kMetaDataCnt; }
 
    /** ptr to the key data, excluding meta-data (hash, domain, etc).  */
    const uint32_t* data() const {
        this->validate();
        return &fKey[kMetaDataCnt];
    }

    /** Used to initialize a key. */
    class Builder {
    public:
        Builder(GrResourceKey* key, uint32_t domain, int data32Count) : fKey(key) {
            SkASSERT(data32Count >= 0);
            SkASSERT(domain != kInvalidDomain);
            key->fKey.reset(kMetaDataCnt + data32Count);
            int size = (data32Count + kMetaDataCnt) * sizeof(uint32_t);
            SkASSERT(SkToU16(size) == size);
            SkASSERT(SkToU16(domain) == domain);
            key->fKey[kDomainAndSize_MetaDataIdx] = domain | (size << 16);
        }

        ~Builder() { this->finish(); }

        void finish() {
            if (NULL == fKey) {
                return;
            }
            GR_STATIC_ASSERT(0 == kHash_MetaDataIdx);
            uint32_t* hash = &fKey->fKey[kHash_MetaDataIdx];
            *hash = GrResourceKeyHash(hash + 1, fKey->internalSize() - sizeof(uint32_t));
            fKey->validate();
            fKey = NULL;
        }

        uint32_t& operator[](int dataIdx) {
            SkASSERT(fKey);
            SkDEBUGCODE(size_t dataCount = fKey->internalSize() / sizeof(uint32_t) - kMetaDataCnt;)
            SkASSERT(SkToU32(dataIdx) < dataCount);
            return fKey->fKey[kMetaDataCnt + dataIdx];
        }

    private:
        GrResourceKey* fKey;
    };

private:
    enum MetaDataIdx {
        kHash_MetaDataIdx,
        // The key domain and size are packed into a single uint32_t.
        kDomainAndSize_MetaDataIdx,

        kLastMetaDataIdx = kDomainAndSize_MetaDataIdx
    };
    static const uint32_t kMetaDataCnt = kLastMetaDataIdx + 1;

    size_t internalSize() const {
        return fKey[kDomainAndSize_MetaDataIdx] >> 16;
    }

    void validate() const {
        SkASSERT(fKey[kHash_MetaDataIdx] ==
                 GrResourceKeyHash(&fKey[kHash_MetaDataIdx] + 1,
                                   this->internalSize() - sizeof(uint32_t)));
        SkASSERT(SkIsAlign4(this->internalSize()));
    }

    friend class TestResource; // For unit test to access kMetaDataCnt.

    // bmp textures require 4 uint32_t values.
    SkAutoSTMalloc<kMetaDataCnt + 4, uint32_t> fKey;
};

/**
 * A key used for scratch resources. The key consists of a resource type (subclass) identifier, a
 * hash, a data length, and type-specific data. A Builder object is used to initialize the
 * key contents. The contents must be initialized before the key can be used.
 */
class GrScratchKey : public GrResourceKey {
private:
    typedef GrResourceKey INHERITED;

public:
    /** Uniquely identifies the type of resource that is cached as scratch. */
    typedef uint32_t ResourceType;

    /** Generate a unique ResourceType. */
    static ResourceType GenerateResourceType();

    /** Creates an invalid scratch key. It must be initialized using a Builder object before use. */
    GrScratchKey() {}

    GrScratchKey(const GrScratchKey& that) { *this = that; }

    /** reset() returns the key to the invalid state. */
    using INHERITED::reset;

    using INHERITED::isValid;

    ResourceType resourceType() const { return this->domain(); }

    GrScratchKey& operator=(const GrScratchKey& that) {
        this->INHERITED::operator=(that);
        return *this;
    }

    bool operator==(const GrScratchKey& that) const {
        return this->INHERITED::operator==(that);
    }
    bool operator!=(const GrScratchKey& that) const { return !(*this == that); }

    class Builder : public INHERITED::Builder {
    public:
        Builder(GrScratchKey* key, ResourceType type, int data32Count)
            : INHERITED::Builder(key, type, data32Count) {}
    };
};

/**
 * A key used to cache resources based on their content. The key consists of a domain type (use
 * case for the cache), a hash, a data length, and domain-specific data. A Builder object is used to
 * initialize the key contents. The contents must be initialized before the key can be used.
 */
class GrContentKey : public GrResourceKey {
private:
    typedef GrResourceKey INHERITED;

public:
    typedef uint32_t Domain;
    /** Generate a unique Domain of content keys. */
    static Domain GenerateDomain();

    /** Creates an invalid content key. It must be initialized using a Builder object before use. */
    GrContentKey() {}

    GrContentKey(const GrContentKey& that) { *this = that; }

    /** reset() returns the key to the invalid state. */
    using INHERITED::reset;

    using INHERITED::isValid;

    GrContentKey& operator=(const GrContentKey& that) {
        this->INHERITED::operator=(that);
        return *this;
    }

    bool operator==(const GrContentKey& that) const {
        return this->INHERITED::operator==(that);
    }
    bool operator!=(const GrContentKey& that) const { return !(*this == that); }

    class Builder : public INHERITED::Builder {
    public:
        Builder(GrContentKey* key, Domain domain, int data32Count)
            : INHERITED::Builder(key, domain, data32Count) {}

        /** Used to build a key that wraps another key and adds additional data. */
        Builder(GrContentKey* key, const GrContentKey& innerKey, Domain domain,
                int extraData32Cnt)
            : INHERITED::Builder(key, domain, Data32CntForInnerKey(innerKey) + extraData32Cnt) {
            SkASSERT(&innerKey != key);
            // add the inner key to the end of the key so that op[] can be indexed normally.
            uint32_t* innerKeyData = &this->operator[](extraData32Cnt);
            const uint32_t* srcData = innerKey.data();
            (*innerKeyData++) = innerKey.domain();
            memcpy(innerKeyData, srcData, innerKey.dataSize());
        }

    private:
        static int Data32CntForInnerKey(const GrContentKey& innerKey) {
            // key data + domain
            return SkToInt((innerKey.dataSize() >> 2) + 1);
        }
    };
};

// The cache listens for these messages to purge junk resources proactively.
class GrContentKeyInvalidatedMessage {
public:
    explicit GrContentKeyInvalidatedMessage(const GrContentKey& key) : fKey(key) {}
    GrContentKeyInvalidatedMessage(const GrContentKeyInvalidatedMessage& that) : fKey(that.fKey) {}
    GrContentKeyInvalidatedMessage& operator=(const GrContentKeyInvalidatedMessage& that) {
        fKey = that.fKey;
        return *this;
    }
    const GrContentKey& key() const { return fKey; }
private:
    GrContentKey fKey;
};
#endif
