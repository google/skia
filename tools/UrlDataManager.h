/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUrlDataManager_DEFINED
#define SkUrlDataManager_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkString.h"
#include "src/core/SkOpts.h"
#include "src/core/SkTDynamicHash.h"

#include <unordered_map>

/*
 * A simple class which allows clients to add opaque data types, and returns a url where this data
 * will be hosted.  Its up to the owner of this class to actually serve the data.
 */
bool operator==(const SkData& a, const SkData& b);

class UrlDataManager {
public:
    UrlDataManager(SkString rootUrl);
    ~UrlDataManager() { this->reset(); }

    /*
     * Adds a data blob to the cache with a particular content type.  UrlDataManager will hash
     * the blob data to ensure uniqueness
     */
    SkString addData(SkData*, const char* contentType);

    struct UrlData : public SkRefCnt {
        SkString fUrl;
        SkString fContentType;
        sk_sp<SkData> fData;
    };

    /*
     * returns the UrlData object which should be hosted at 'url'
     */
    UrlData* getDataFromUrl(SkString url) {
        return fUrlLookup.find(url);
    }
    void reset();

    // Methods used to identify images differently in wasm debugger for mskp animations.
    // serving is uncessary, as a collection of images with identifiers is already present, we
    // just want to use it when serializing commands.

    /*
     * Construct an index from a list of images
     * (expected to be the list that was loaded from the mskp file)
     * Use only once.
     */
    void indexImages(const std::vector<sk_sp<SkImage>>&);

    /*
     * Reports whether this UDM has an initialized image index (effevitely whether we're in wasm)
     */
    bool hasImageIndex() { return imageMap.size() > 0; }

    /*
     * Return the file id (index of the image in the originally provided list) of an SkImage
     */
    int lookupImage(const SkImage*);

private:
    struct LookupTrait {
        // We use the data as a hash, this is not really optimal but is fine until proven otherwise
        static const SkData& GetKey(const UrlData& data) {
            return *data.fData.get();
        }

        static uint32_t Hash(const SkData& key) {
            return SkOpts::hash(key.bytes(), key.size());
        }
    };

    struct ReverseLookupTrait {
        static const SkString& GetKey(const UrlData& data) {
            return data.fUrl;
        }

        static uint32_t Hash(const SkString& key) {
            return SkOpts::hash(key.c_str(), strlen(key.c_str()));
        }
    };


    SkString fRootUrl;
    SkTDynamicHash<UrlData, SkData, LookupTrait> fCache;
    SkTDynamicHash<UrlData, SkString, ReverseLookupTrait> fUrlLookup;
    uint32_t fDataId;
    std::unordered_map<const SkImage*, int> imageMap;
};

#endif
