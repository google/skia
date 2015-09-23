/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef JavaInputStream_DEFINED
#define JavaInputStream_DEFINED

#include <jni.h>
#include "SkStream.h"

class JavaInputStream : public SkStream {
public:
    JavaInputStream(JNIEnv*, jbyteArray javaBuffer, jobject javaIoInputStream);
    bool isAtEnd() const override;
    size_t read(void*, size_t) override;
private:
    JNIEnv* fEnv;
    jobject fInputStream;
    jbyteArray fJavaBuffer;
    jclass fInputStreamClass;
    jmethodID fReadMethodID;
    jint fStartIndex;
    jint fEndIndex;
};

#endif  // JavaInputStream_DEFINED
