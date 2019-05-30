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
    // Used as a set
    std::map<uint32_t, bool> imageMap;
};
struct SharingDeserialContext {
    std::map<uint32_t, sk_sp<SkImage>> imageMap;
};
// A serial proc that shares images between subpictures
SkSerialProcs SharingSerializationProcs(SharingSerialContext* ctx);
SkDeserialProcs SharingDeserializationProcs(SharingDeserialContext* ctx);

} // namespace sharingproc
#endif
