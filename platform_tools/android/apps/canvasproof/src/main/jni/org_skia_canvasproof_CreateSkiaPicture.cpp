/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "org_skia_canvasproof_CreateSkiaPicture.h"
#include "JavaInputStream.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"

/*
 * Class:     org_skia_canvasproof_CreateSkiaPicture
 * Method:    delete
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_skia_canvasproof_CreateSkiaPicture_deleteImpl(
        JNIEnv* env, jclass clazz, jlong ptr) {
    SkSafeUnref(reinterpret_cast<SkPicture*>(ptr));
}

/*
 * Class:     org_skia_canvasproof_CreateSkiaPicture
 * Method:    createImpl
 * Signature: (Ljava/io/InputStream;[B)J
 */
JNIEXPORT jlong JNICALL Java_org_skia_canvasproof_CreateSkiaPicture_createImpl
  (JNIEnv* env, jclass clazz, jobject inputStream, jbyteArray buffer) {
    JavaInputStream stream(env, buffer, inputStream);
    #if 0
    SkAutoTUnref<SkPicture> p(SkPicture::CreateFromStream(&stream));
    if (!p) { return 0; }
    SkPictureRecorder recorder;
    SkRect bounds = p->cullRect();
    SkRTreeFactory bbh;
    recorder.beginRecording(bounds, &bbh)->drawPicture(p);
    return reinterpret_cast<long>(recorder.endRecordingAsPicture());
    #else
    return reinterpret_cast<long>(SkPicture::CreateFromStream(&stream));
    #endif
}
