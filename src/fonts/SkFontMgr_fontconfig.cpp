/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface.h"
#include "SkFontConfigTypeface.h"
#include "SkFontDescriptor.h"
#include "SkFontMgr.h"
#include "SkFontStyle.h"
#include "SkMutex.h"
#include "SkString.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"
#include "SkResourceCache.h"

SkStreamAsset* SkTypeface_FCI::onOpenStream(int* ttcIndex) const {
    *ttcIndex =  this->getIdentity().fTTCIndex;

    SkStreamAsset* stream = this->getLocalStream();
    if (stream) {
        return stream->duplicate();
    }

    return fFCI->openStream(this->getIdentity());
}

void SkTypeface_FCI::onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocalStream) const {
    SkString name;
    this->getFamilyName(&name);
    desc->setFamilyName(name.c_str());
    *isLocalStream = SkToBool(this->getLocalStream());
}

///////////////////////////////////////////////////////////////////////////////

class SkFontStyleSet_FCI : public SkFontStyleSet {
public:
    SkFontStyleSet_FCI() {}

    int count() override { return 0; }
    void getStyle(int index, SkFontStyle*, SkString* style) override { SkASSERT(false); }
    SkTypeface* createTypeface(int index) override { SkASSERT(false); return nullptr; }
    SkTypeface* matchStyle(const SkFontStyle& pattern) override { return nullptr; }
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
        Result(Request* request, SkTypeface* typeface)
            : fRequest(request)
            , fFace(SkSafeRef(typeface)) {}
        Result(Result&&) = default;
        Result& operator=(Result&&) = default;

        const Key& getKey() const override { return *fRequest; }
        size_t bytesUsed() const override { return fRequest->size() + sizeof(fFace); }
        const char* getCategory() const override { return "request_cache"; }
        SkDiscardableMemory* diagnostic_only_getDiscardable() const override { return nullptr; }

        SkAutoTDelete<Request> fRequest;
        SkAutoTUnref<SkTypeface> fFace;
    };

    SkResourceCache fCachedResults;

public:
    SkFontRequestCache(size_t maxSize) : fCachedResults(maxSize) {}

    /** Takes ownership of request. It will be deleted when no longer needed. */
    void add(SkTypeface* face, Request* request) {
        fCachedResults.add(new Result(request, face));
    }
    /** Does not take ownership of request. */
    SkTypeface* findAndRef(Request* request) {
        SkTypeface* face = nullptr;
        fCachedResults.find(*request, [](const SkResourceCache::Rec& rec, void* context) -> bool {
            const Result& result = static_cast<const Result&>(rec);
            SkTypeface** face = static_cast<SkTypeface**>(context);

            *face = result.fFace;
            return true;
        }, &face);
        return SkSafeRef(face);
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
    SkAutoTUnref<SkFontConfigInterface> fFCI;
    SkAutoTUnref<SkDataTable> fFamilyNames;
    SkTypeface_FreeType::Scanner fScanner;

    mutable SkMutex fMutex;
    mutable SkTypefaceCache fTFCache;

    // The value of maxSize here is a compromise between cache hits and cache size.
    // See https://crbug.com/424082#63 for reason for current size.
    static const size_t kMaxSize = 1 << 15;
    mutable SkFontRequestCache fCache;

public:
    SkFontMgr_FCI(SkFontConfigInterface* fci)
        : fFCI(fci)
        , fFamilyNames(fFCI->getFamilyNames())
        , fCache(kMaxSize)
    {}

protected:
    int onCountFamilies() const override {
        return fFamilyNames->count();
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        familyName->set(fFamilyNames->atStr(index));
    }

    SkFontStyleSet* onCreateStyleSet(int index) const override {
        return this->onMatchFamily(fFamilyNames->atStr(index));
    }

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
        return new SkFontStyleSet_FCI();
    }

    SkTypeface* onMatchFamilyStyle(const char familyName[],
                                   const SkFontStyle&) const override { return nullptr; }
    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                            const char* bcp47[], int bcp47Count,
                                            SkUnichar character) const override {
        return nullptr;
    }
    SkTypeface* onMatchFaceStyle(const SkTypeface*,
                                 const SkFontStyle&) const override { return nullptr; }

    SkTypeface* onCreateFromData(SkData*, int ttcIndex) const override { return nullptr; }

    SkTypeface* onCreateFromStream(SkStreamAsset* bareStream, int ttcIndex) const override {
        SkAutoTDelete<SkStreamAsset> stream(bareStream);
        const size_t length = stream->getLength();
        if (!length) {
            return nullptr;
        }
        if (length >= 1024 * 1024 * 1024) {
            return nullptr;  // don't accept too large fonts (>= 1GB) for safety.
        }

        // TODO should the caller give us the style or should we get it from freetype?
        SkFontStyle style;
        bool isFixedWidth = false;
        if (!fScanner.scanFont(stream, 0, nullptr, &style, &isFixedWidth, nullptr)) {
            return nullptr;
        }

        return SkTypeface_FCI::Create(style, isFixedWidth, stream.release(), ttcIndex);
    }

    SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const override {
        SkAutoTDelete<SkStreamAsset> stream(SkStream::NewFromFile(path));
        return stream.get() ? this->createFromStream(stream.release(), ttcIndex) : nullptr;
    }

    SkTypeface* onLegacyCreateTypeface(const char requestedFamilyName[],
                                       SkFontStyle requestedStyle) const override
    {
        SkAutoMutexAcquire ama(fMutex);

        // Check if this request is already in the request cache.
        using Request = SkFontRequestCache::Request;
        SkAutoTDelete<Request> request(Request::Create(requestedFamilyName, requestedStyle));
        SkTypeface* face = fCache.findAndRef(request);
        if (face) {
            return face;
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
            face = SkTypeface_FCI::Create(fFCI, identity, outFamilyName, outStyle);
            // Add this FontIdentity to the FontIdentity cache.
            fTFCache.add(face);
        }
        // Add this request to the request cache.
        fCache.add(face, request.release());

        return face;
    }
};

SK_API SkFontMgr* SkFontMgr_New_FCI(SkFontConfigInterface* fci) {
    SkASSERT(fci);
    return new SkFontMgr_FCI(fci);
}
