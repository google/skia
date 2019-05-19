/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkPictureImageFilter.h"
#include "src/core/SkPictureData.h"
#include "src/core/SkPicturePlayback.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkRecordedDrawable.h"
#include "src/shaders/SkPictureShader.h"

SkPicture::SkPicture() : fUniqueID(0) {}


sk_sp<SkPicture> SkPicturePriv::MakeFromBuffer(SkReadBuffer& buffer) {
    return nullptr;
}

void SkPicturePriv::Flatten(const sk_sp<const SkPicture> picture, SkWriteBuffer& buffer) {}


SkPictureRecorder::SkPictureRecorder() {}

SkPictureRecorder::~SkPictureRecorder() {}

SkCanvas* SkPictureRecorder::beginRecording(const SkRect& userCullRect,
                                            SkBBHFactory* bbhFactory /* = nullptr */,
                                            uint32_t recordFlags /* = 0 */) {
    return nullptr;
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    return nullptr;
}

sk_sp<SkPicture> SkPictureRecorder::finishRecordingAsPicture(uint32_t finishFlags) {
    return nullptr;
}

sk_sp<SkPicture> SkPictureRecorder::finishRecordingAsPictureWithCull(const SkRect& cullRect,
                                                                     uint32_t finishFlags) {
    return nullptr;
}

void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {}

sk_sp<SkDrawable> SkPictureRecorder::finishRecordingAsDrawable(uint32_t finishFlags) {
    return nullptr;
}


SkPictureData* SkPictureData::CreateFromStream(SkStream* stream,
                                               const SkPictInfo& info,
                                               const SkDeserialProcs& procs,
                                               SkTypefacePlayback* topLevelTFPlayback) {
    return nullptr;
}

SkPictureData* SkPictureData::CreateFromBuffer(SkReadBuffer& buffer,
                                               const SkPictInfo& info) {
    return nullptr;
}


sk_sp<SkFlattenable> SkPictureShader::CreateProc(SkReadBuffer& buffer) {
    return nullptr;
}

void SkPictureShader::flatten(SkWriteBuffer& buffer) const {}


void SkPicturePlayback::draw(SkCanvas* canvas,
                             SkPicture::AbortCallback* callback,
                             SkReadBuffer* buffer) {}


std::unique_ptr<SkImageGenerator>
SkImageGenerator::MakeFromPicture(const SkISize& size, sk_sp<SkPicture> picture,
                                  const SkMatrix* matrix, const SkPaint* paint,
                                  SkImage::BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace) {
    return nullptr;
}

void SkRecordedDrawable::flatten(SkWriteBuffer& buffer) const {}

sk_sp<SkFlattenable> SkRecordedDrawable::CreateProc(SkReadBuffer& buffer) {
    return nullptr;
}


sk_sp<SkFlattenable> SkPictureImageFilter::CreateProc(SkReadBuffer& buffer) {
    return nullptr;
}
