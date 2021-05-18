/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

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

} // namespace utils
} // namespace androidkit
