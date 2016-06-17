
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceKey_DEFINED
#define GrResourceKey_DEFINED

#include "../private/SkTemplates.h"
#include "GrTypes.h"
#include "SkData.h"
#include "../private/SkOnce.h"

uint32_t GrResourceKeyHash(const uint32_t* data, size_t size);

/**
 * Base class for all GrGpuResource cache keys. There are two types of cache keys. Refer to the
 * comments for each key type below.
 */
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
        return this->hash() == that.hash() &&
                0 == memcmp(&fKey[kHash_MetaDataIdx + 1],
                            &that.fKey[kHash_MetaDataIdx + 1],
                            this->internalSize() - sizeof(uint32_t));
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

    // bmp textures require 5 uint32_t values.
    SkAutoSTMalloc<kMetaDataCnt + 5, uint32_t> fKey;
};

/**
 * A key used for scratch resources. There are three important rules about scratch keys:
 *        * Multiple resources can share the same scratch key. Therefore resources assigned the same
 *          scratch key should be interchangeable with respect to the code that uses them.
 *        * A resource can have at most one scratch key and it is set at resource creation by the
 *          resource itself.
 *        * When a scratch resource is ref'ed it will not be returned from the
 *          cache for a subsequent cache request until all refs are released. This facilitates using
 *          a scratch key for multiple render-to-texture scenarios. An example is a separable blur:
 *
 *  GrTexture* texture[2];
 *  texture[0] = get_scratch_texture(scratchKey);
 *  texture[1] = get_scratch_texture(scratchKey); // texture[0] is already owned so we will get a
 *                                                // different one for texture[1]
 *  draw_mask(texture[0], path);        // draws path mask to texture[0]
 *  blur_x(texture[0], texture[1]);     // blurs texture[0] in y and stores result in texture[1]
 *  blur_y(texture[1], texture[0]);     // blurs texture[1] in y and stores result in texture[0]
 *  texture[1]->unref();  // texture 1 can now be recycled for the next request with scratchKey
 *  consume_blur(texture[0]);
 *  texture[0]->unref();  // texture 0 can now be recycled for the next request with scratchKey
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
 * A key that allows for exclusive use of a resource for a use case (AKA "domain"). There are three
 * rules governing the use of unique keys:
 *        * Only one resource can have a given unique key at a time. Hence, "unique".
 *        * A resource can have at most one unique key at a time.
 *        * Unlike scratch keys, multiple requests for a unique key will return the same
 *          resource even if the resource already has refs.
 * This key type allows a code path to create cached resources for which it is the exclusive user.
 * The code path creates a domain which it sets on its keys. This guarantees that there are no
 * cross-domain collisions.
 *
 * Unique keys preempt scratch keys. While a resource has a unique key it is inaccessible via its
 * scratch key. It can become scratch again if the unique key is removed.
 */
class GrUniqueKey : public GrResourceKey {
private:
    typedef GrResourceKey INHERITED;

public:
    typedef uint32_t Domain;
    /** Generate a Domain for unique keys. */
    static Domain GenerateDomain();

    /** Creates an invalid unique key. It must be initialized using a Builder object before use. */
    GrUniqueKey() {}

    GrUniqueKey(const GrUniqueKey& that) { *this = that; }

    /** reset() returns the key to the invalid state. */
    using INHERITED::reset;

    using INHERITED::isValid;

    GrUniqueKey& operator=(const GrUniqueKey& that) {
        this->INHERITED::operator=(that);
        this->setCustomData(that.getCustomData());
        return *this;
    }

    bool operator==(const GrUniqueKey& that) const {
        return this->INHERITED::operator==(that);
    }
    bool operator!=(const GrUniqueKey& that) const { return !(*this == that); }

    void setCustomData(const SkData* data) {
        SkSafeRef(data);
        fData.reset(data);
    }
    const SkData* getCustomData() const {
        return fData.get();
    }

    class Builder : public INHERITED::Builder {
    public:
        Builder(GrUniqueKey* key, Domain domain, int data32Count)
            : INHERITED::Builder(key, domain, data32Count) {}

        /** Used to build a key that wraps another key and adds additional data. */
        Builder(GrUniqueKey* key, const GrUniqueKey& innerKey, Domain domain,
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
        static int Data32CntForInnerKey(const GrUniqueKey& innerKey) {
            // key data + domain
            return SkToInt((innerKey.dataSize() >> 2) + 1);
        }
    };

private:
    SkAutoTUnref<const SkData> fData;
};

/**
 * It is common to need a frequently reused GrUniqueKey where the only requirement is that the key
 * is unique. These macros create such a key in a thread safe manner so the key can be truly global
 * and only constructed once.
 */

/** Place outside of function/class definitions. */
#define GR_DECLARE_STATIC_UNIQUE_KEY(name) static SkOnce name##_once

/** Place inside function where the key is used. */
#define GR_DEFINE_STATIC_UNIQUE_KEY(name)                                                       \
    static SkAlignedSTStorage<1, GrUniqueKey> name##_storage;                                   \
    name##_once(gr_init_static_unique_key_once, &name##_storage);                               \
    static const GrUniqueKey& name = *reinterpret_cast<GrUniqueKey*>(name##_storage.get());

static inline void gr_init_static_unique_key_once(SkAlignedSTStorage<1,GrUniqueKey>* keyStorage) {
    GrUniqueKey* key = new (keyStorage->get()) GrUniqueKey;
    GrUniqueKey::Builder builder(key, GrUniqueKey::GenerateDomain(), 0);
}

// The cache listens for these messages to purge junk resources proactively.
class GrUniqueKeyInvalidatedMessage {
public:
    explicit GrUniqueKeyInvalidatedMessage(const GrUniqueKey& key) : fKey(key) {}

    GrUniqueKeyInvalidatedMessage(const GrUniqueKeyInvalidatedMessage& that) : fKey(that.fKey) {}

    GrUniqueKeyInvalidatedMessage& operator=(const GrUniqueKeyInvalidatedMessage& that) {
        fKey = that.fKey;
        return *this;
    }

    const GrUniqueKey& key() const { return fKey; }

private:
    GrUniqueKey fKey;
};
#endif
