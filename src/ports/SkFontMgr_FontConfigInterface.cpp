/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkFontConfigInterface.h"
#include "include/ports/SkFontMgr_FontConfigInterface.h"
#include "include/private/base/SkMutex.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkTypefaceCache.h"
#include "src/ports/SkFontConfigTypeface.h"
#include <new>

using namespace skia_private;

std::unique_ptr<SkStreamAsset> SkTypeface_FCI::onOpenStream(int* ttcIndex) const {
    *ttcIndex =  this->getIdentity().fTTCIndex;
    return std::unique_ptr<SkStreamAsset>(fFCI->openStream(this->getIdentity()));
}

std::unique_ptr<SkFontData> SkTypeface_FCI::onMakeFontData() const {
    const SkFontConfigInterface::FontIdentity& id = this->getIdentity();
    return std::make_unique<SkFontData>(std::unique_ptr<SkStreamAsset>(fFCI->openStream(id)),
                                        id.fTTCIndex, 0, nullptr, 0, nullptr, 0);
}

void SkTypeface_FCI::onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const {
    SkString name;
    this->getFamilyName(&name);
    desc->setFamilyName(name.c_str());
    desc->setStyle(this->fontStyle());
    desc->setFactoryId(SkTypeface_FreeType::FactoryId);
    *serialize = true;
}

///////////////////////////////////////////////////////////////////////////////

class SkFontStyleSet_FCI : public SkFontStyleSet {
public:
    SkFontStyleSet_FCI() {}

    int count() override { return 0; }
    void getStyle(int index, SkFontStyle*, SkString* style) override { SkASSERT(false); }
    sk_sp<SkTypeface> createTypeface(int index) override { SkASSERT(false); return nullptr; }
    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override { return nullptr; }
};

///////////////////////////////////////////////////////////////////////////////

class SkFontRequestCache {
public:
    struct Request : public SkResourceCache::Key {
    private:
        Request(const char* name, size_t nameLen, const SkFontStyle& style) : fStyle(style) {
            /** Pointer to just after the last field of this class. */
            char* content = const_cast<char*>(SkTAfter<const char>(&this->fStyle));

            // No holes.
            SkASSERT(SkTAddOffset<char>(this, sizeof(SkResourceCache::Key) + keySize) == content);

            // Has a size divisible by size of uint32_t.
            SkASSERT((content - reinterpret_cast<char*>(this)) % sizeof(uint32_t) == 0);

            size_t contentLen = SkAlign4(nameLen);
            sk_careful_memcpy(content, name, nameLen);
            sk_bzero(content + nameLen, contentLen - nameLen);
            this->init(nullptr, 0, keySize + contentLen);
        }
        const SkFontStyle fStyle;
        /** The sum of the sizes of the fields of this class. */
        static const size_t keySize = sizeof(fStyle);

    public:
        static Request* Create(const char* name, const SkFontStyle& style) {
            size_t nameLen = name ? strlen(name) : 0;
            size_t contentLen = SkAlign4(nameLen);
            char* storage = new char[sizeof(Request) + contentLen];
            return new (storage) Request(name, nameLen, style);
        }
        void operator delete(void* storage) {
            delete[] reinterpret_cast<char*>(storage);
        }
    };


private:
    struct Result : public SkResourceCache::Rec {
        Result(Request* request, sk_sp<SkTypeface> typeface)
            : fRequest(request), fFace(std::move(typeface)) {}
        Result(Result&&) = default;
        Result& operator=(Result&&) = default;

        const Key& getKey() const override { return *fRequest; }
        size_t bytesUsed() const override { return fRequest->size() + sizeof(fFace); }
        const char* getCategory() const override { return "request_cache"; }
        SkDiscardableMemory* diagnostic_only_getDiscardable() const override { return nullptr; }

        std::unique_ptr<Request> fRequest;
        sk_sp<SkTypeface> fFace;
    };

    SkResourceCache fCachedResults;

public:
    SkFontRequestCache(size_t maxSize) : fCachedResults(maxSize) {}

    /** Takes ownership of request. It will be deleted when no longer needed. */
    void add(sk_sp<SkTypeface> face, Request* request) {
        fCachedResults.add(new Result(request, std::move(face)));
    }
    /** Does not take ownership of request. */
    sk_sp<SkTypeface> findAndRef(Request* request) {
        sk_sp<SkTypeface> face;
        fCachedResults.find(*request, [](const SkResourceCache::Rec& rec, void* context) -> bool {
            const Result& result = static_cast<const Result&>(rec);
            sk_sp<SkTypeface>* face = static_cast<sk_sp<SkTypeface>*>(context);

            *face = result.fFace;
            return true;
        }, &face);
        return face;
    }
};

///////////////////////////////////////////////////////////////////////////////

static bool find_by_FontIdentity(SkTypeface* cachedTypeface, void* ctx) {
    typedef SkFontConfigInterface::FontIdentity FontIdentity;
    SkTypeface_FCI* cachedFCTypeface = static_cast<SkTypeface_FCI*>(cachedTypeface);
    FontIdentity* identity = static_cast<FontIdentity*>(ctx);

    return cachedFCTypeface->getIdentity() == *identity;
}

///////////////////////////////////////////////////////////////////////////////

class SkFontMgr_FCI : public SkFontMgr {
    sk_sp<SkFontConfigInterface> fFCI;
    SkTypeface_FreeType::Scanner fScanner;

    mutable SkMutex fMutex;
    mutable SkTypefaceCache fTFCache;

    // The value of maxSize here is a compromise between cache hits and cache size.
    // See https://crbug.com/424082#63 for reason for current size.
    static const size_t kMaxSize = 1 << 15;
    mutable SkFontRequestCache fCache;

public:
    SkFontMgr_FCI(sk_sp<SkFontConfigInterface> fci)
        : fFCI(std::move(fci))
        , fCache(kMaxSize)
    {
        SkASSERT_RELEASE(fFCI);
    }

protected:
    int onCountFamilies() const override {
        SK_ABORT("Not implemented.");
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        SK_ABORT("Not implemented.");
    }

    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        SK_ABORT("Not implemented.");
    }

    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        SK_ABORT("Not implemented.");
    }

    sk_sp<SkTypeface> onMatchFamilyStyle(const char requestedFamilyName[],
                                         const SkFontStyle& requestedStyle) const override
    {
        SkAutoMutexExclusive ama(fMutex);

        // Check if this request is already in the request cache.
        using Request = SkFontRequestCache::Request;
        std::unique_ptr<Request> request(Request::Create(requestedFamilyName, requestedStyle));
        sk_sp<SkTypeface> face = fCache.findAndRef(request.get());
        if (face) {
            return sk_sp<SkTypeface>(face);
        }

        SkFontConfigInterface::FontIdentity identity;
        SkString outFamilyName;
        SkFontStyle outStyle;
        if (!fFCI->matchFamilyName(requestedFamilyName, requestedStyle,
                                   &identity, &outFamilyName, &outStyle))
        {
            return nullptr;
        }

        // Check if a typeface with this FontIdentity is already in the FontIdentity cache.
        face = fTFCache.findByProcAndRef(find_by_FontIdentity, &identity);
        if (!face) {
            face.reset(SkTypeface_FCI::Create(fFCI, identity, std::move(outFamilyName), outStyle));
            // Add this FontIdentity to the FontIdentity cache.
            fTFCache.add(face);
        }
        // Add this request to the request cache.
        fCache.add(face, request.release());

        return face;
    }

    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                            const char* bcp47[], int bcp47Count,
                                            SkUnichar character) const override {
        SK_ABORT("Not implemented.");
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        return this->onMakeFromStreamIndex(SkMemoryStream::Make(std::move(data)), ttcIndex);
    }

    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        return this->makeFromStream(std::move(stream),
                                    SkFontArguments().setCollectionIndex(ttcIndex));
    }

    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        const size_t length = stream->getLength();
        if (!length) {
            return nullptr;
        }
        if (length >= 1024 * 1024 * 1024) {
            return nullptr;  // don't accept too large fonts (>= 1GB) for safety.
        }

        return SkTypeface_FreeType::MakeFromStream(std::move(stream), args);
    }

    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path);
        return stream ? this->makeFromStream(std::move(stream), ttcIndex) : nullptr;
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char requestedFamilyName[],
                                           SkFontStyle requestedStyle) const override {
        return this->onMatchFamilyStyle(requestedFamilyName, requestedStyle);
    }
};

SK_API sk_sp<SkFontMgr> SkFontMgr_New_FCI(sk_sp<SkFontConfigInterface> fci) {
    SkASSERT(fci);
    return sk_make_sp<SkFontMgr_FCI>(std::move(fci));
}
