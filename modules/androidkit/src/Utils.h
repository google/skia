/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkBlendMode.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTileMode.h"

namespace androidkit {
namespace utils {

// RAII helper for jstring -> cstring conversions
class CString {
public:
    CString(JNIEnv* env, const jstring& jstr)
        : fEnv(env)
        , fJString(jstr)
        , fCString(env->GetStringUTFChars(jstr, nullptr))
    {}

    ~CString() {
        fEnv->ReleaseStringUTFChars(fJString, fCString);
    }

    operator const char*() const { return fCString; }

private:
    JNIEnv*        fEnv;
    const jstring& fJString;
    const char*    fCString;


    CString(CString&&) = delete;
    CString(const CString&) = delete;
    CString& operator=(CString&&) = delete;
    CString& operator=(const CString&) = delete;
};

// RAII helper for float array access
class CFloats {
public:
    CFloats(JNIEnv* env, const jfloatArray& jfloats)
        : fEnv(env)
        , fJFloats(jfloats)
        , fCFloats(env->GetFloatArrayElements(jfloats, nullptr))
    {}

    ~CFloats() {
        fEnv->ReleaseFloatArrayElements(fJFloats, fCFloats, 0);
    }

    operator const float*() const { return fCFloats; }

private:
    JNIEnv*            fEnv;
    const jfloatArray& fJFloats;
    float*             fCFloats;


    CFloats(CFloats&&) = delete;
    CFloats(const CFloats&) = delete;
    CFloats& operator=(CFloats&&) = delete;
    CFloats& operator=(const CFloats&) = delete;
};

SkSamplingOptions SamplingOptions(jint, jfloat, jfloat);
SkTileMode TileMode(jint);
SkBlendMode BlendMode(jint);

} // namespace utils
} // namespace androidkit
