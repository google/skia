/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include <string>
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

// RAII helper for jstring -> u16String conversions
class U16String {
public:
    U16String(JNIEnv* env, const jstring& jstr)
        : fEnv(env)
        , fJString(jstr)
        , fU16String(env->GetStringChars(jstr, nullptr))
    {}

    ~U16String() {
        fEnv->ReleaseStringChars(fJString, fU16String);
    }

    operator const char16_t*() const { return reinterpret_cast<const char16_t*>(fU16String); }

private:
    JNIEnv*        fEnv;
    const jstring& fJString;
    const jchar*   fU16String;


    U16String(U16String&&) = delete;
    U16String(const U16String&) = delete;
    U16String& operator=(U16String&&) = delete;
    U16String& operator=(const U16String&) = delete;
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
