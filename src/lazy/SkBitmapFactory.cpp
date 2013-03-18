/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapFactory.h"

#include "SkBitmap.h"
#include "SkData.h"
#include "SkImageCache.h"
#include "SkImagePriv.h"
#include "SkLazyPixelRef.h"

SkBitmapFactory::SkBitmapFactory(SkBitmapFactory::DecodeProc proc)
    : fDecodeProc(proc)
    , fImageCache(NULL)
    , fCacheSelector(NULL) {
    SkASSERT(fDecodeProc != NULL);
}

SkBitmapFactory::~SkBitmapFactory() {
    SkSafeUnref(fImageCache);
    SkSafeUnref(fCacheSelector);
}

void SkBitmapFactory::setImageCache(SkImageCache *cache) {
    SkRefCnt_SafeAssign(fImageCache, cache);
    if (cache != NULL) {
        SkSafeUnref(fCacheSelector);
        fCacheSelector = NULL;
    }
}

void SkBitmapFactory::setCacheSelector(CacheSelector* selector) {
    SkRefCnt_SafeAssign(fCacheSelector, selector);
    if (selector != NULL) {
        SkSafeUnref(fImageCache);
        fImageCache = NULL;
    }
}

bool SkBitmapFactory::installPixelRef(SkData* data, SkBitmap* dst) {
    if (NULL == data || 0 == data->size() || dst == NULL) {
        return false;
    }

    SkImage::Info info;
    if (!fDecodeProc(data->data(), data->size(), &info, NULL)) {
        return false;
    }

    bool isOpaque = false;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);

    Target target;
    // FIMXE: There will be a problem if this rowbytes is calculated differently from
    // in SkLazyPixelRef.
    target.fRowBytes = SkImageMinRowBytes(info);

    dst->setConfig(config, info.fWidth, info.fHeight, target.fRowBytes);
    dst->setIsOpaque(isOpaque);

    // fImageCache and fCacheSelector are mutually exclusive.
    SkASSERT(NULL == fImageCache || NULL == fCacheSelector);

    SkImageCache* cache = NULL == fCacheSelector ? fImageCache : fCacheSelector->selectCache(info);

    if (cache != NULL) {
        // Now set a new LazyPixelRef on dst.
        SkAutoTUnref<SkLazyPixelRef> lazyRef(SkNEW_ARGS(SkLazyPixelRef,
                                                        (data, fDecodeProc, cache)));
        dst->setPixelRef(lazyRef);
        return true;
    } else {
        dst->allocPixels();
        target.fAddr = dst->getPixels();
        return fDecodeProc(data->data(), data->size(), &info, &target);
    }
}
