/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Need to include SkTypes first, so that SK_BUILD_FOR_ANDROID is defined.
#include "SkTypes.h"
#ifdef SK_BUILD_FOR_ANDROID
#include "SkPicturePlayback.h"
#endif
#include "SkPictureRecorder.h"

SkCanvas* SkPictureRecorder::beginRecording(int width, int height,
                                            SkBBHFactory* bbhFactory /* = NULL */,
                                            uint32_t recordFlags /* = 0 */) {
    fPicture.reset(SkNEW(SkPicture));
    return fPicture->beginRecording(width, height, bbhFactory, recordFlags);
}

#ifdef SK_BUILD_FOR_ANDROID
void SkPictureRecorder::partialReplay(SkCanvas* canvas) {
    if (NULL == fPicture.get() || NULL == canvas) {
        // Not recording or nothing to replay into
        return;
    }

    SkASSERT(NULL != fPicture->fRecord);

    SkAutoTDelete<SkPicturePlayback> playback(SkPicture::FakeEndRecording(fPicture,
                                                                          *fPicture->fRecord,
                                                                          false));
    playback->draw(*canvas, NULL);
}
#endif
