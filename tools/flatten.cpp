/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkForceLinking.h"
#include "SkImageGenerator.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include <stdio.h>

__SK_FORCE_IMAGE_DECODER_LINKING;

#define ASSERTF(cond, fmt, ...) if (!(cond)) { fprintf(stderr, fmt"\n", __VA_ARGS__); exit(1); }

static bool lazy_decode_bitmap(const void* src, size_t size, SkBitmap* dst) {
    SkAutoTUnref<SkData> encoded(SkData::NewWithCopy(src, size));
    return encoded && SkInstallDiscardablePixelRef(encoded, dst);
}

int main(int argc, char** argv) {
    ASSERTF(argc == 3, "usage: %s nested.skp flat.skp", argv[0]);
    const char *nestedPath = argv[1],
               *flatPath   = argv[2];

    // Read nested.skp.
    SkFILEStream stream(nestedPath);
    ASSERTF(stream.isValid(), "Couldn't read %s.", nestedPath);
    SkAutoTUnref<const SkPicture> nested(SkPicture::CreateFromStream(&stream, &lazy_decode_bitmap));
    ASSERTF(nested, "Couldn't parse %s as a picture.", nestedPath);

    // Play it back into a new picture using kPlaybackDrawPicture_RecordFlag.
    SkPictureRecorder recorder;
    uint32_t flags = SkPictureRecorder::kPlaybackDrawPicture_RecordFlag;
    nested->playback(recorder.beginRecording(nested->cullRect(), nullptr, flags));
    SkAutoTUnref<const SkPicture> flat(recorder.endRecordingAsPicture());

    // Write out that flat.skp
    SkFILEWStream wstream(flatPath);
    ASSERTF(wstream.isValid(), "Could not open %s.", flatPath);
    flat->serialize(&wstream);
    wstream.flush();

    return 0;
}
