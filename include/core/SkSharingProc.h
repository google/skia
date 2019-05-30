/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSharingProc_DEFINED
#define SkSharingProc_DEFINED

#include <map>

#include "SkImage.h"
#include "SkSerialProcs.h"

namespace sharingproc {

struct SharingSerialContext {
    // A map from the ids from SkImage::uniqueID() to ids used within the file
    std::map<uint32_t, uint32_t> imageMap;
    // A counter incremented to assign new ids to images in the file.
    // Used as values in the map above.
    uint32_t fileImageCounter = 0;
};
struct SharingDeserialContext {
    // A map from in-file ids to images.
    std::map<uint32_t, sk_sp<SkImage>> imageMap;
    // A counter incremented whenever full new image is loaded.
    uint32_t fileImageCounter = 0;
};
// A serial proc that shares images between subpictures
SkSerialProcs SharingSerializationProcs(SharingSerialContext* ctx);
SkDeserialProcs SharingDeserializationProcs(SharingDeserialContext* ctx);

} // namespace sharingproc
#endif
