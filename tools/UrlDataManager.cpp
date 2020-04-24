/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/UrlDataManager.h"

bool operator==(const SkData& a, const SkData& b) {
    return a.equals(&b);
}

UrlDataManager::UrlDataManager(SkString rootUrl) : fRootUrl(rootUrl), fDataId(0) {}

SkString UrlDataManager::addData(SkData* data, const char* contentType) {
    UrlData* urlData = fCache.find(*data);
    if (fCache.find(*data)) {
        SkASSERT(data->equals(urlData->fData.get()));
        return urlData->fUrl;
    }

    urlData = new UrlData;
    urlData->fData.reset(SkRef(data));
    urlData->fContentType.set(contentType);
    urlData->fUrl.appendf("%s/%d", fRootUrl.c_str(), fDataId++);

    fCache.add(urlData);

    SkASSERT(!fUrlLookup.find(urlData->fUrl));
    fUrlLookup.add(urlData);
    return urlData->fUrl;
}

void UrlDataManager::reset() {
    SkTDynamicHash<UrlData, SkData, LookupTrait>::Iter iter(&fCache);
    while (!iter.done()) {
        UrlData* urlData = &(*iter);
        urlData->unref();
        ++iter;
    }

    fCache.rewind();
}
