/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMaskCache.h"

#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkMask.h"
#include "src/core/SkResourceCache.h"

#include <cstddef>
#include <cstdint>

class SkDiscardableMemory;
enum SkBlurStyle : int;

#define CHECK_LOCAL(localCache, localName, globalName, ...) \
    ((localCache) ? localCache->localName(__VA_ARGS__) : SkResourceCache::globalName(__VA_ARGS__))

struct MaskValue {
    SkMask          fMask;
    SkCachedData*   fData;
};

namespace {
static unsigned gRRectBlurKeyNamespaceLabel;

struct RRectBlurKey : public SkResourceCache::Key {
public:
    RRectBlurKey(SkScalar sigma, const SkRRect& rrect, SkBlurStyle style)
        : fSigma(sigma)
        , fStyle(style)
        , fRRect(rrect)
    {
        this->init(&gRRectBlurKeyNamespaceLabel, 0,
                   sizeof(fSigma) + sizeof(fStyle) + sizeof(fRRect));
    }

    SkScalar   fSigma;
    int32_t    fStyle;
    SkRRect    fRRect;
};

struct RRectBlurRec : public SkResourceCache::Rec {
    RRectBlurRec(RRectBlurKey key, const SkMask& mask, SkCachedData* data)
        : fKey(key), fValue({{nullptr, mask.fBounds, mask.fRowBytes, mask.fFormat}, data})
    {
        fValue.fData->attachToCacheAndRef();
    }
    ~RRectBlurRec() override {
        fValue.fData->detachFromCacheAndUnref();
    }

    RRectBlurKey   fKey;
    MaskValue      fValue;

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(*this) + fValue.fData->size(); }
    const char* getCategory() const override { return "rrect-blur"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fValue.fData->diagnostic_only_getDiscardable();
    }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* contextData) {
        const RRectBlurRec& rec = static_cast<const RRectBlurRec&>(baseRec);
        SkTLazy<MaskValue>* result = (SkTLazy<MaskValue>*)contextData;

        SkCachedData* tmpData = rec.fValue.fData;
        tmpData->ref();
        if (nullptr == tmpData->data()) {
            tmpData->unref();
            return false;
        }
        result->init(rec.fValue);
        return true;
    }
};
} // namespace

SkCachedData* SkMaskCache::FindAndRef(SkScalar sigma, SkBlurStyle style,
                                      const SkRRect& rrect, SkTLazy<SkMask>* mask,
                                      SkResourceCache* localCache) {
    SkTLazy<MaskValue> result;
    RRectBlurKey key(sigma, rrect, style);
    if (!CHECK_LOCAL(localCache, find, Find, key, RRectBlurRec::Visitor, &result)) {
        return nullptr;
    }

    mask->init(static_cast<const uint8_t*>(result->fData->data()),
               result->fMask.fBounds, result->fMask.fRowBytes, result->fMask.fFormat);
    return result->fData;
}

void SkMaskCache::Add(SkScalar sigma, SkBlurStyle style,
                      const SkRRect& rrect, const SkMask& mask, SkCachedData* data,
                      SkResourceCache* localCache) {
    RRectBlurKey key(sigma, rrect, style);
    return CHECK_LOCAL(localCache, add, Add, new RRectBlurRec(key, mask, data));
}

//////////////////////////////////////////////////////////////////////////////////////////

namespace {
static unsigned gRectsBlurKeyNamespaceLabel;

struct RectsBlurKey : public SkResourceCache::Key {
public:
    RectsBlurKey(SkScalar sigma, SkBlurStyle style, const SkRect rects[], int count)
        : fSigma(sigma)
        , fStyle(style)
    {
        SkASSERT(1 == count || 2 == count);
        SkIRect ir;
        rects[0].roundOut(&ir);
        fSizes[0] = SkSize{rects[0].width(), rects[0].height()};
        if (2 == count) {
            fSizes[1] = SkSize{rects[1].width(), rects[1].height()};
            fSizes[2] = SkSize{rects[0].x() - rects[1].x(), rects[0].y() - rects[1].y()};
        } else {
            fSizes[1] = SkSize{0, 0};
            fSizes[2] = SkSize{0, 0};
        }
        fSizes[3] = SkSize{rects[0].x() - ir.x(), rects[0].y() - ir.y()};

        this->init(&gRectsBlurKeyNamespaceLabel, 0,
                   sizeof(fSigma) + sizeof(fStyle) + sizeof(fSizes));
    }

    SkScalar    fSigma;
    int32_t     fStyle;
    SkSize      fSizes[4];
};

struct RectsBlurRec : public SkResourceCache::Rec {
    RectsBlurRec(RectsBlurKey key, const SkMask& mask, SkCachedData* data)
        : fKey(key), fValue({{nullptr, mask.fBounds, mask.fRowBytes, mask.fFormat}, data})
    {
        fValue.fData->attachToCacheAndRef();
    }
    ~RectsBlurRec() override {
        fValue.fData->detachFromCacheAndUnref();
    }

    RectsBlurKey   fKey;
    MaskValue      fValue;

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(*this) + fValue.fData->size(); }
    const char* getCategory() const override { return "rects-blur"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fValue.fData->diagnostic_only_getDiscardable();
    }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* contextData) {
        const RectsBlurRec& rec = static_cast<const RectsBlurRec&>(baseRec);
        SkTLazy<MaskValue>* result = static_cast<SkTLazy<MaskValue>*>(contextData);

        SkCachedData* tmpData = rec.fValue.fData;
        tmpData->ref();
        if (nullptr == tmpData->data()) {
            tmpData->unref();
            return false;
        }
        result->init(rec.fValue);
        return true;
    }
};
} // namespace

SkCachedData* SkMaskCache::FindAndRef(SkScalar sigma, SkBlurStyle style,
                                      const SkRect rects[], int count, SkTLazy<SkMask>* mask,
                                      SkResourceCache* localCache) {
    SkTLazy<MaskValue> result;
    RectsBlurKey key(sigma, style, rects, count);
    if (!CHECK_LOCAL(localCache, find, Find, key, RectsBlurRec::Visitor, &result)) {
        return nullptr;
    }

    mask->init(static_cast<const uint8_t*>(result->fData->data()),
               result->fMask.fBounds, result->fMask.fRowBytes, result->fMask.fFormat);
    return result->fData;
}

void SkMaskCache::Add(SkScalar sigma, SkBlurStyle style,
                      const SkRect rects[], int count, const SkMask& mask, SkCachedData* data,
                      SkResourceCache* localCache) {
    RectsBlurKey key(sigma, style, rects, count);
    return CHECK_LOCAL(localCache, add, Add, new RectsBlurRec(key, mask, data));
}
