/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSharingProc.h"

#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkSerialProcs.h"

namespace sharingproc {
namespace  {

sk_sp<SkData> SerializeImage(SkImage* img, void* ctx) {
    SharingSerialContext* context = reinterpret_cast<SharingSerialContext*>(ctx);
    uint32_t id = img->uniqueID(); // get this process's id for the image. these are not hashes.
    // find out if we have already serialized this, and if so, what it's in-file id is.
    auto iter = context->imageMap.find(id);
    if (iter == context->imageMap.end()) {
        // When not present, add it's id to the map and return it's usual serialized form.
        context->imageMap[id] = context->fileImageCounter;
        context->fileImageCounter++;
        return nullptr; // nullptr cases skia's default serialization behavior to be used.
    }
    uint32_t fid = context->imageMap[id];
    // if present, return only the in-file id we registered the first time we serialized it.
    return SkData::MakeWithCopy(&fid, sizeof(fid));
}

sk_sp<SkImage> DeserializeImage(const void* data, size_t length, void* ctx) {
    SharingDeserialContext* context = reinterpret_cast<SharingDeserialContext*>(ctx);
    uint32_t fid;
    // If the data is an image fid, look up an already deserialized image from our map
    if (length == sizeof(fid)) {
        memcpy(&fid, data, sizeof(fid));
        auto iter = context->imageMap.find(fid);
        if (iter == context->imageMap.end() || !iter->second) {
            SkDebugf("We do not have the data for image %d.\n", fid);
            return nullptr;
        }
        return iter->second;
    }
    // Otherwise, the data is an image, deserialise it, store it in our map at it's fid.
    // TODO(nifong): make DeserialProcs accept sk_sp<SkData> so we don't have to copy this.
    sk_sp<SkData> dataView = SkData::MakeWithCopy(data, length);
    const sk_sp<SkImage> image = SkImage::MakeFromEncoded(std::move(dataView));
    // When loading a new image, it's in-file id is the number of images loaded before it.
    fid = context->fileImageCounter;
    context->fileImageCounter++;
    context->imageMap[fid] = image;
    return image;
}

} // namespace

SkSerialProcs SharingSerializationProcs(SharingSerialContext* ctx) {
  SkSerialProcs procs;
  procs.fImageProc = SerializeImage;
  procs.fImageCtx = ctx;
  return procs;
}

SkDeserialProcs SharingDeserializationProcs(SharingDeserialContext* ctx) {
  SkDeserialProcs procs;
  procs.fImageProc = DeserializeImage;
  procs.fImageCtx = ctx;
  return procs;
}

} // namespace sharingproc
