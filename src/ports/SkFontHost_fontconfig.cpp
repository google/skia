/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface.h"
#include "SkFontConfigTypeface.h"
#include "SkFontDescriptor.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"
#include "SkResourceCache.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SK_DECLARE_STATIC_MUTEX(gFontConfigInterfaceMutex);
static SkFontConfigInterface* gFontConfigInterface;

SkFontConfigInterface* SkFontConfigInterface::RefGlobal() {
    SkAutoMutexAcquire ac(gFontConfigInterfaceMutex);

    return SkSafeRef(gFontConfigInterface);
}

SkFontConfigInterface* SkFontConfigInterface::SetGlobal(SkFontConfigInterface* fc) {
    SkAutoMutexAcquire ac(gFontConfigInterfaceMutex);

    SkRefCnt_SafeAssign(gFontConfigInterface, fc);
    return fc;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// convenience function to create the direct interface if none is installed.
extern SkFontConfigInterface* SkCreateDirectFontConfigInterface();

static SkFontConfigInterface* RefFCI() {
    for (;;) {
        SkFontConfigInterface* fci = SkFontConfigInterface::RefGlobal();
        if (fci) {
            return fci;
        }
        fci = SkFontConfigInterface::GetSingletonDirectInterface(&gFontConfigInterfaceMutex);
        SkFontConfigInterface::SetGlobal(fci);
    }
}

// export this to SkFontMgr_fontconfig.cpp until this file just goes away.
SkFontConfigInterface* SkFontHost_fontconfig_ref_global();
SkFontConfigInterface* SkFontHost_fontconfig_ref_global() {
    return RefFCI();
}

///////////////////////////////////////////////////////////////////////////////

static bool find_by_FontIdentity(SkTypeface* cachedTypeface, const SkFontStyle&, void* ctx) {
    typedef SkFontConfigInterface::FontIdentity FontIdentity;
    FontConfigTypeface* cachedFCTypeface = static_cast<FontConfigTypeface*>(cachedTypeface);
    FontIdentity* identity = static_cast<FontIdentity*>(ctx);

    return cachedFCTypeface->getIdentity() == *identity;
}

SK_DECLARE_STATIC_MUTEX(gSkFontHostRequestCacheMutex);
class SkFontHostRequestCache {

    // The value of maxSize here is a compromise between cache hits and cache size.
    static const size_t gMaxSize = 1 << 12;

    static SkFontHostRequestCache& Get() {
        gSkFontHostRequestCacheMutex.assertHeld();
        static SkFontHostRequestCache gCache(gMaxSize);
        return gCache;
    }

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
            this->init(&gSkFontHostRequestCacheMutex, 0, keySize + contentLen);
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
    SkFontHostRequestCache(size_t maxSize) : fCachedResults(maxSize) {}

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

    /** Takes ownership of request. It will be deleted when no longer needed. */
    static void Add(SkTypeface* face, Request* request) {
        SkAutoMutexAcquire ama(gSkFontHostRequestCacheMutex);
        Get().add(face, request);
    }

    /** Does not take ownership of request. */
    static SkTypeface* FindAndRef(Request* request) {
        SkAutoMutexAcquire ama(gSkFontHostRequestCacheMutex);
        return Get().findAndRef(request);
    }
};

SkTypeface* FontConfigTypeface::LegacyCreateTypeface(const char requestedFamilyName[],
                                                     SkTypeface::Style requestedOldStyle)
{
    SkAutoTUnref<SkFontConfigInterface> fci(RefFCI());
    if (nullptr == fci.get()) {
        return nullptr;
    }

    // Check if this request is already in the request cache.
    using Request = SkFontHostRequestCache::Request;
    SkFontStyle requestedStyle(requestedOldStyle);
    SkAutoTDelete<Request> request(Request::Create(requestedFamilyName, requestedStyle));
    SkTypeface* face = SkFontHostRequestCache::FindAndRef(request);
    if (face) {
        return face;
    }

    SkFontConfigInterface::FontIdentity identity;
    SkString outFamilyName;
    SkTypeface::Style outOldStyle;
    if (!fci->matchFamilyName(requestedFamilyName, requestedOldStyle,
                              &identity, &outFamilyName, &outOldStyle))
    {
        return nullptr;
    }

    // Check if a typeface with this FontIdentity is already in the FontIdentity cache.
    face = SkTypefaceCache::FindByProcAndRef(find_by_FontIdentity, &identity);
    if (!face) {
        face = FontConfigTypeface::Create(SkFontStyle(outOldStyle), identity, outFamilyName);
        // Add this FontIdentity to the FontIdentity cache.
        SkTypefaceCache::Add(face, SkFontStyle(outOldStyle));
    }
    // Add this request to the request cache.
    SkFontHostRequestCache::Add(face, request.release());

    return face;
}

///////////////////////////////////////////////////////////////////////////////

SkStreamAsset* FontConfigTypeface::onOpenStream(int* ttcIndex) const {
    SkStreamAsset* stream = this->getLocalStream();
    if (stream) {
        // TODO: should have been provided by CreateFromStream()
        *ttcIndex = 0;
        return stream->duplicate();
    }

    SkAutoTUnref<SkFontConfigInterface> fci(RefFCI());
    if (nullptr == fci.get()) {
        return nullptr;
    }

    *ttcIndex = this->getIdentity().fTTCIndex;
    return fci->openStream(this->getIdentity());
}

void FontConfigTypeface::onGetFamilyName(SkString* familyName) const {
    *familyName = fFamilyName;
}

void FontConfigTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                             bool* isLocalStream) const {
    SkString name;
    this->getFamilyName(&name);
    desc->setFamilyName(name.c_str());
    *isLocalStream = SkToBool(this->getLocalStream());
}
