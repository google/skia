/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkSharingProc.h"

#include "include/codec/SkPngDecoder.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"

namespace {
    sk_sp<SkData> collectNonTextureImagesProc(SkImage* img, void* ctx) {
        SkSharingSerialContext* context = reinterpret_cast<SkSharingSerialContext*>(ctx);
        uint32_t originalId = img->uniqueID();
        sk_sp<SkImage>* imageInMap = context->fNonTexMap.find(originalId);
        if (!imageInMap) {
            context->fNonTexMap[originalId] = img->makeRasterImage(context->fDirectContext);
        }

        // This function implements a proc that is more generally for serialization, but
        // we really only want to build our map. The output of this function is ignored.
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

void SkSharingSerialContext::setDirectContext(GrDirectContext* ctx) {
    fDirectContext = ctx;
}

sk_sp<SkData> SkSharingSerialContext::serializeImage(SkImage* img, void* ctx) {
    SkSharingSerialContext* context = reinterpret_cast<SkSharingSerialContext*>(ctx);
    uint32_t id = img->uniqueID(); // get this process's id for the image. these are not hashes.
    // find out if we have already serialized this, and if so, what its in-file id is.
    int* fid = context->fImageMap.find(id);
    if (!fid) {
        // encode the image or it's non-texture replacement if one was collected
        sk_sp<SkImage>* replacementImage = context->fNonTexMap.find(id);
        if (replacementImage) {
            img = replacementImage->get();
        }
        auto data = SkPngEncoder::Encode(context->fDirectContext, img, {});
        if (!data) {
            // If encoding fails, we must return something. If we return null then SkWriteBuffer's
            // serialize_image which calls this proc will continue to try writing to the mskp file.
            SkBitmap bm;
            bm.allocPixels(SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
            SkCanvas canvas = SkCanvas(bm);
            canvas.clear(SK_ColorMAGENTA);
            data = SkPngEncoder::Encode(context->fDirectContext, bm.asImage().get(), {});
        }
        context->fImageMap[id] = context->fImageMap.count(); // Next in-file id
        return data;
    }
    // if present, return only the in-file id we registered the first time we serialized it.
    return SkData::MakeWithCopy(fid, sizeof(*fid));
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
            SkDebugf("Cannot deserialize using id, We do not have the data for image %u.\n", fid);
            return nullptr;
        }
        return context->fImages[fid];
    }
    // Otherwise, the data is an image, deserialise it, store it in our map at its fid.
    // TODO(nifong): make DeserialProcs accept sk_sp<SkData> so we don't have to copy this.
    sk_sp<SkData> dataView = SkData::MakeWithCopy(data, length);
    auto codec = SkPngDecoder::Decode(std::move(dataView), nullptr);
    if (!codec) {
        SkDebugf("Cannot deserialize image - might not be a PNG.\n");
        return nullptr;
    }
    SkImageInfo info = codec->getInfo().makeAlphaType(kPremul_SkAlphaType);
    auto [image, result] = codec->getImage(info);
    if (result != SkCodec::Result::kSuccess) {
        SkDebugf("Error decoding image %d.\n", result);
        // Might have partially decoded.
    }
    context->fImages.push_back(image);
    return image;
}
