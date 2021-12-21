/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ResourceKey_DEFINED
#define skgpu_ResourceKey_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkString.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"

#include <new>

class TestResource;

namespace skgpu {

uint32_t ResourceKeyHash(const uint32_t* data, size_t size);

/**
 * Base class for all gpu Resource cache keys. There are two types of cache keys. Refer to the
 * comments for each key type below.
 */
class ResourceKey {
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

    /** Used to initialize a key. */
    class Builder {
    public:
        ~Builder() { this->finish(); }

        void finish() {
            if (nullptr == fKey) {
                return;
            }
            uint32_t* hash = &fKey->fKey[kHash_MetaDataIdx];
            *hash = ResourceKeyHash(hash + 1, fKey->internalSize() - sizeof(uint32_t));
            fKey->validate();
            fKey = nullptr;
        }

        uint32_t& operator[](int dataIdx) {
            SkASSERT(fKey);
            SkDEBUGCODE(size_t dataCount = fKey->internalSize() / sizeof(uint32_t) - kMetaDataCnt;)
                    SkASSERT(SkToU32(dataIdx) < dataCount);
            return fKey->fKey[(int)kMetaDataCnt + dataIdx];
        }

    protected:
        Builder(ResourceKey* key, uint32_t domain, int data32Count) : fKey(key) {
            size_t count = SkToSizeT(data32Count);
            SkASSERT(domain != kInvalidDomain);
            key->fKey.reset(kMetaDataCnt + count);
            size_t size = (count + kMetaDataCnt) * sizeof(uint32_t);
            SkASSERT(SkToU16(size) == size);
            SkASSERT(SkToU16(domain) == domain);
            key->fKey[kDomainAndSize_MetaDataIdx] = domain | (size << 16);
        }

    private:
        ResourceKey* fKey;
    };

protected:
    static const uint32_t kInvalidDomain = 0;

    ResourceKey() { this->reset(); }

    /** Reset to an invalid key. */
    void reset() {
        fKey.reset(kMetaDataCnt);
        fKey[kHash_MetaDataIdx] = 0;
        fKey[kDomainAndSize_MetaDataIdx] = kInvalidDomain;
    }

    bool operator==(const ResourceKey& that) const {
        // Both keys should be sized to at least contain the meta data. The metadata contains each
        // key's length. So the second memcmp should only run if the keys have the same length.
        return 0 == memcmp(fKey.get(), that.fKey.get(), kMetaDataCnt*sizeof(uint32_t)) &&
               0 == memcmp(&fKey[kMetaDataCnt], &that.fKey[kMetaDataCnt], this->dataSize());
    }

    ResourceKey& operator=(const ResourceKey& that) {
        if (this != &that) {
            if (!that.isValid()) {
                this->reset();
            } else {
                size_t bytes = that.size();
                SkASSERT(SkIsAlign4(bytes));
                fKey.reset(bytes / sizeof(uint32_t));
                memcpy(fKey.get(), that.fKey.get(), bytes);
                this->validate();
            }
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

#ifdef SK_DEBUG
    void dump() const {
        if (!this->isValid()) {
            SkDebugf("Invalid Key\n");
        } else {
            SkDebugf("hash: %d ", this->hash());
            SkDebugf("domain: %d ", this->domain());
            SkDebugf("size: %zuB ", this->internalSize());
            size_t dataCount = this->internalSize() / sizeof(uint32_t) - kMetaDataCnt;
            for (size_t i = 0; i < dataCount; ++i) {
                SkDebugf("%d ", fKey[SkTo<int>(kMetaDataCnt+i)]);
            }
            SkDebugf("\n");
        }
    }
#endif

private:
    enum MetaDataIdx {
        kHash_MetaDataIdx,
        // The key domain and size are packed into a single uint32_t.
        kDomainAndSize_MetaDataIdx,

        kLastMetaDataIdx = kDomainAndSize_MetaDataIdx
    };
    static const uint32_t kMetaDataCnt = kLastMetaDataIdx + 1;

    size_t internalSize() const { return fKey[kDomainAndSize_MetaDataIdx] >> 16; }

    void validate() const {
        SkASSERT(this->isValid());
        SkASSERT(fKey[kHash_MetaDataIdx] ==
                 ResourceKeyHash(&fKey[kHash_MetaDataIdx] + 1,
                                 this->internalSize() - sizeof(uint32_t)));
        SkASSERT(SkIsAlign4(this->internalSize()));
    }

    friend class ::TestResource;  // For unit test to access kMetaDataCnt.

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
class ScratchKey : public ResourceKey {
public:
    /** Uniquely identifies the type of resource that is cached as scratch. */
    typedef uint32_t ResourceType;

    /** Generate a unique ResourceType. */
    static ResourceType GenerateResourceType();

    /** Creates an invalid scratch key. It must be initialized using a Builder object before use. */
    ScratchKey() {}

    ScratchKey(const ScratchKey& that) { *this = that; }

    /** reset() returns the key to the invalid state. */
    using ResourceKey::reset;

    using ResourceKey::isValid;

    ResourceType resourceType() const { return this->domain(); }

    ScratchKey& operator=(const ScratchKey& that) {
        this->ResourceKey::operator=(that);
        return *this;
    }

    bool operator==(const ScratchKey& that) const { return this->ResourceKey::operator==(that); }
    bool operator!=(const ScratchKey& that) const { return !(*this == that); }

    class Builder : public ResourceKey::Builder {
    public:
        Builder(ScratchKey* key, ResourceType type, int data32Count)
                : ResourceKey::Builder(key, type, data32Count) {}
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
class UniqueKey : public ResourceKey {
public:
    typedef uint32_t Domain;
    /** Generate a Domain for unique keys. */
    static Domain GenerateDomain();

    /** Creates an invalid unique key. It must be initialized using a Builder object before use. */
    UniqueKey() : fTag(nullptr) {}

    UniqueKey(const UniqueKey& that) { *this = that; }

    /** reset() returns the key to the invalid state. */
    using ResourceKey::reset;

    using ResourceKey::isValid;

    UniqueKey& operator=(const UniqueKey& that) {
        this->ResourceKey::operator=(that);
        this->setCustomData(sk_ref_sp(that.getCustomData()));
        fTag = that.fTag;
        return *this;
    }

    bool operator==(const UniqueKey& that) const { return this->ResourceKey::operator==(that); }
    bool operator!=(const UniqueKey& that) const { return !(*this == that); }

    void setCustomData(sk_sp<SkData> data) { fData = std::move(data); }
    SkData* getCustomData() const { return fData.get(); }
    sk_sp<SkData> refCustomData() const { return fData; }

    const char* tag() const { return fTag; }

#ifdef SK_DEBUG
    void dump(const char* label) const {
        SkDebugf("%s tag: %s\n", label, fTag ? fTag : "None");
        this->ResourceKey::dump();
    }
#endif

    class Builder : public ResourceKey::Builder {
    public:
        Builder(UniqueKey* key, Domain type, int data32Count, const char* tag = nullptr)
                : ResourceKey::Builder(key, type, data32Count) {
            key->fTag = tag;
        }

        /** Used to build a key that wraps another key and adds additional data. */
        Builder(UniqueKey* key, const UniqueKey& innerKey, Domain domain, int extraData32Cnt,
                const char* tag = nullptr)
                : ResourceKey::Builder(key,
                                       domain,
                                       Data32CntForInnerKey(innerKey) + extraData32Cnt) {
            SkASSERT(&innerKey != key);
            // add the inner key to the end of the key so that op[] can be indexed normally.
            uint32_t* innerKeyData = &this->operator[](extraData32Cnt);
            const uint32_t* srcData = innerKey.data();
            (*innerKeyData++) = innerKey.domain();
            memcpy(innerKeyData, srcData, innerKey.dataSize());
            key->fTag = tag;
        }

    private:
        static int Data32CntForInnerKey(const UniqueKey& innerKey) {
            // key data + domain
            return SkToInt((innerKey.dataSize() >> 2) + 1);
        }
    };

private:
    sk_sp<SkData> fData;
    const char* fTag;
};

/**
 * It is common to need a frequently reused UniqueKey where the only requirement is that the key
 * is unique. These macros create such a key in a thread safe manner so the key can be truly global
 * and only constructed once.
 */

/** Place outside of function/class definitions. */
#define SKGPU_DECLARE_STATIC_UNIQUE_KEY(name) static SkOnce name##_once

/** Place inside function where the key is used. */
#define SKGPU_DEFINE_STATIC_UNIQUE_KEY(name)                                \
    static SkAlignedSTStorage<1, skgpu::UniqueKey> name##_storage;          \
    name##_once(skgpu::skgpu_init_static_unique_key_once, &name##_storage); \
    static const skgpu::UniqueKey& name =                                   \
        *reinterpret_cast<skgpu::UniqueKey*>(name##_storage.get())

static inline void skgpu_init_static_unique_key_once(SkAlignedSTStorage<1, UniqueKey>* keyStorage) {
    UniqueKey* key = new (keyStorage->get()) UniqueKey;
    UniqueKey::Builder builder(key, UniqueKey::GenerateDomain(), 0);
}

// The cache listens for these messages to purge junk resources proactively.
class UniqueKeyInvalidatedMessage {
public:
    UniqueKeyInvalidatedMessage() = default;
    UniqueKeyInvalidatedMessage(const UniqueKey& key,
                                uint32_t contextUniqueID,
                                bool inThreadSafeCache = false)
            : fKey(key), fContextID(contextUniqueID), fInThreadSafeCache(inThreadSafeCache) {
        SkASSERT(SK_InvalidUniqueID != contextUniqueID);
    }

    UniqueKeyInvalidatedMessage(const UniqueKeyInvalidatedMessage&) = default;

    UniqueKeyInvalidatedMessage& operator=(const UniqueKeyInvalidatedMessage&) = default;

    const UniqueKey& key() const { return fKey; }
    uint32_t contextID() const { return fContextID; }
    bool inThreadSafeCache() const { return fInThreadSafeCache; }

private:
    UniqueKey fKey;
    uint32_t fContextID = SK_InvalidUniqueID;
    bool fInThreadSafeCache = false;
};

static inline bool SkShouldPostMessageToBus(const UniqueKeyInvalidatedMessage& msg,
                                            uint32_t msgBusUniqueID) {
    return msg.contextID() == msgBusUniqueID;
}

} // namespace skgpu

#endif // skgpu_ResourceKey_DEFINED
