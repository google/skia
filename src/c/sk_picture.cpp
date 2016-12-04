/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPicture.h"

#include "sk_picture.h"

#include "sk_types_priv.h"

sk_picture_recorder_t* sk_picture_recorder_new() {
    return ToPictureRecorder(new SkPictureRecorder);
}

void sk_picture_recorder_delete(sk_picture_recorder_t* crec) {
    delete AsPictureRecorder(crec);
}

sk_canvas_t* sk_picture_recorder_begin_recording(sk_picture_recorder_t* crec,
                                                 const sk_rect_t* cbounds) {
    return ToCanvas(AsPictureRecorder(crec)->beginRecording(AsRect(*cbounds)));
}

sk_picture_t* sk_picture_recorder_end_recording(sk_picture_recorder_t* crec) {
    return ToPicture(AsPictureRecorder(crec)->finishRecordingAsPicture().release());
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

sk_rect_t sk_picture_get_bounds(sk_picture_t* cpic) {
    return ToRect(AsPicture(cpic)->cullRect());
}
