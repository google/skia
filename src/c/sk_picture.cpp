/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDrawable.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"

#include "include/c/sk_picture.h"

#include "src/c/sk_types_priv.h"

sk_picture_recorder_t* sk_picture_recorder_new() {
    return ToPictureRecorder(new SkPictureRecorder);
}

void sk_picture_recorder_delete(sk_picture_recorder_t* crec) {
    delete AsPictureRecorder(crec);
}

sk_canvas_t* sk_picture_recorder_begin_recording(sk_picture_recorder_t* crec, const sk_rect_t* cbounds) {
    return ToCanvas(AsPictureRecorder(crec)->beginRecording(*AsRect(cbounds)));
}

sk_picture_t* sk_picture_recorder_end_recording(sk_picture_recorder_t* crec) {
    return ToPicture(AsPictureRecorder(crec)->finishRecordingAsPicture().release());
}

sk_drawable_t* sk_picture_recorder_end_recording_as_drawable(sk_picture_recorder_t* crec) {
    return ToDrawable(AsPictureRecorder(crec)->finishRecordingAsDrawable().release());
}

sk_canvas_t* sk_picture_get_recording_canvas(sk_picture_recorder_t* crec) {
    return ToCanvas(AsPictureRecorder(crec)->getRecordingCanvas());
}

void sk_picture_ref(sk_picture_t* cpic) {
    SkSafeRef(AsPicture(cpic));
}

void sk_picture_unref(sk_picture_t* cpic) {
    SkSafeUnref(AsPicture(cpic));
}

uint32_t sk_picture_get_unique_id(sk_picture_t* cpic) {
    return AsPicture(cpic)->uniqueID();
}

void sk_picture_get_cull_rect(sk_picture_t* cpic, sk_rect_t* crect) {
    *crect = ToRect(AsPicture(cpic)->cullRect());
}
