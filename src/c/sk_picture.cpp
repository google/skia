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
#include "include/core/SkShader.h"

#include "include/c/sk_picture.h"

#include "src/c/sk_types_priv.h"

sk_picture_recorder_t* sk_picture_recorder_new(void) {
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

sk_shader_t* sk_picture_make_shader(sk_picture_t* src, sk_shader_tilemode_t tmx, sk_shader_tilemode_t tmy, const sk_matrix_t* localMatrix, const sk_rect_t* tile) {
    SkMatrix m;
    if (localMatrix) {
        m = AsMatrix(localMatrix);
    }
    return ToShader(AsPicture(src)->makeShader((SkTileMode)tmx, (SkTileMode)tmy, localMatrix ? &m : nullptr, AsRect(tile)).release());
}

sk_data_t* sk_picture_serialize_to_data(const sk_picture_t* picture) {
    return ToData(AsPicture(picture)->serialize().release());
}

void sk_picture_serialize_to_stream(const sk_picture_t* picture, sk_wstream_t* stream) {
    AsPicture(picture)->serialize(AsWStream(stream));
}

sk_picture_t* sk_picture_deserialize_from_stream(sk_stream_t* stream) {
    return ToPicture(SkPicture::MakeFromStream(AsStream(stream)).release());
}

sk_picture_t* sk_picture_deserialize_from_data(sk_data_t* data) {
    return ToPicture(SkPicture::MakeFromData(AsData(data)).release());
}

sk_picture_t* sk_picture_deserialize_from_memory(void* buffer, size_t length) {
    return ToPicture(SkPicture::MakeFromData(buffer, length).release());
}
