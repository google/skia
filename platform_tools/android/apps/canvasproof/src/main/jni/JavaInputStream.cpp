/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "JavaInputStream.h"

JavaInputStream::JavaInputStream(
        JNIEnv* env, jbyteArray javaBuffer, jobject inputStream)
    : fEnv(env)
    , fStartIndex(0)
    , fEndIndex(0) {
    SkASSERT(inputStream);
    SkASSERT(javaBuffer);
    fInputStream = inputStream;
    fJavaBuffer = javaBuffer;
    fInputStreamClass = env->FindClass("java/io/InputStream");
    SkASSERT(fInputStreamClass);
    fReadMethodID = env->GetMethodID(fInputStreamClass, "read", "([B)I");
    SkASSERT(fReadMethodID);
}

bool JavaInputStream::isAtEnd() const { return fStartIndex == fEndIndex; }

size_t JavaInputStream::read(void* voidBuffer, size_t size) {
    size_t totalRead = 0;
    char* buffer = static_cast<char*>(voidBuffer);  // may be NULL;
    while (size) {
        // make sure the cache has at least one byte or is done.
        if (fStartIndex == fEndIndex) {
            jint count =
                fEnv->CallIntMethod(fInputStream, fReadMethodID, fJavaBuffer);
            if (fEnv->ExceptionCheck()) {
                fEnv->ExceptionDescribe();
                fEnv->ExceptionClear();
                SkDebugf("---- java.io.InputStream::read() threw an exception\n");
                return 0;
            }
            fStartIndex = 0;
            fEndIndex = count;
            if (this->isAtEnd()) {
                return totalRead;  // No more to read.
            }
        }
        SkASSERT(fEndIndex > fStartIndex);
        size_t length = SkTMin(SkToSizeT(fEndIndex - fStartIndex), size);
        if (buffer && length) {
            jbyte* bufferElements
                = fEnv->GetByteArrayElements(fJavaBuffer, NULL);
            memcpy(buffer, &bufferElements[fStartIndex], length);
            buffer += length;
            fEnv->ReleaseByteArrayElements(fJavaBuffer, bufferElements, 0);
        }
        totalRead += length;
        size -= length;
        fStartIndex += length;
    }
    return totalRead;
}
