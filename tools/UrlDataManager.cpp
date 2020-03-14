/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/UrlDataManager.h"

#include <unordered_map>

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
    fCache.foreach([&](UrlData* urlData) {
        urlData->unref();
    });
    fCache.rewind();
}

void UrlDataManager::indexImages(const std::vector<sk_sp<SkImage>>& images) {
  SkASSERT(imageMap.size() == 0); // this method meant only for initialization once.
  for (size_t i = 0; i < images.size(); ++i) {
    imageMap.insert({images[i].get(), i});
  }
}

int UrlDataManager::lookupImage(const SkImage* im) {
  auto search = imageMap.find(im);
  if (search != imageMap.end()) {
    return search->second;
  } else {
      // -1 signals the pointer to this image wasn't in the original list.
      // Maybe it was synthesized after file load? If so, you shouldn't be looking it up here.
      return -1;
  }
}
