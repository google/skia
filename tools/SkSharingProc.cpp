/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkSharingProc.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkSerialProcs.h"

namespace {
    sk_sp<SkData> collectNonTextureImagesProc(SkImage* img, void* ctx) {
        SkSharingSerialContext* context = reinterpret_cast<SkSharingSerialContext*>(ctx);
        uint32_t originalId = img->uniqueID();
        auto it = context->fNonTexMap.find(originalId);
        if (it == context->fNonTexMap.end()) {
            context->fNonTexMap[originalId] = img->makeNonTextureImage();
        }
        return SkData::MakeEmpty();
    }
}

void SkSharingSerialContext::collectNonTextureImagesFromPicture(
    const SkPicture* pic, SkSharingSerialContext* sharingCtx) {
    SkSerialProcs tempProc;
    tempProc.fImageCtx = sharingCtx;
    tempProc.fImageProc = collectNonTextureImagesProc;
    SkNullWStream ns;
    pic->serialize(&ns, &tempProc);
}

sk_sp<SkData> SkSharingSerialContext::serializeImage(SkImage* img, void* ctx) {
    SkSharingSerialContext* context = reinterpret_cast<SkSharingSerialContext*>(ctx);
    uint32_t id = img->uniqueID(); // get this process's id for the image. these are not hashes.
    // find out if we have already serialized this, and if so, what its in-file id is.
    auto iter = context->fImageMap.find(id);
    if (iter == context->fImageMap.end()) {
        // When not present, add its id to the map and return its usual serialized form.
        context->fImageMap[id] = context->fImageMap.size(); // Next in-file id
        // encode the image or it's non-texture replacement if one was collected
        auto iter2 = context->fNonTexMap.find(id);
        if (iter2 != context->fNonTexMap.end()) {
            img = iter2->second.get();
        }
        return img->encodeToData();
    }
    uint32_t fid = context->fImageMap[id];
    // if present, return only the in-file id we registered the first time we serialized it.
    return SkData::MakeWithCopy(&fid, sizeof(fid));
}

sk_sp<SkImage> SkSharingDeserialContext::deserializeImage(
  const void* data, size_t length, void* ctx) {
    if (!data || !length || !ctx) {
        SkDebugf("SkSharingDeserialContext::deserializeImage arguments invalid %p %zu %p.\n",
            data, length, ctx);
        // Return something so the rest of the debugger can proceed.
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::MakeN32Premul(1, 1));
        return bm.asImage();
    }
    SkSharingDeserialContext* context = reinterpret_cast<SkSharingDeserialContext*>(ctx);
    uint32_t fid;
    // If the data is an image fid, look up an already deserialized image from our map
    if (length == sizeof(fid)) {
        memcpy(&fid, data, sizeof(fid));
        if (fid >= context->fImages.size()) {
            SkDebugf("Cannot deserialize using id, We do not have the data for image %d.\n", fid);
            return nullptr;
        }
        return context->fImages[fid];
    }
    // Otherwise, the data is an image, deserialise it, store it in our map at its fid.
    // TODO(nifong): make DeserialProcs accept sk_sp<SkData> so we don't have to copy this.
    sk_sp<SkData> dataView = SkData::MakeWithCopy(data, length);
    const sk_sp<SkImage> image = SkImage::MakeFromEncoded(std::move(dataView));
    context->fImages.push_back(image);
    return image;
}
