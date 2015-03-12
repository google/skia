/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DMSrcSinkAndroid_DEFINED
#define DMSrcSinkAndroid_DEFINED

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK

#include "DMSrcSink.h"

namespace DM {

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Draws to the Android Framework's HWUI API.

class HWUISink : public Sink {
public:
    HWUISink() { }

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const SK_OVERRIDE;
    int enclave() const SK_OVERRIDE { return kGPU_Enclave; }
    const char* fileExtension() const SK_OVERRIDE { return "png"; }
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Trims draw commands to only include those supported by the Android Framework's HWUI API.

class ViaAndroidSDK : public Sink {
public:
    explicit ViaAndroidSDK(Sink*);

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const SK_OVERRIDE;
    int enclave() const SK_OVERRIDE { return fSink->enclave(); }
    const char* fileExtension() const SK_OVERRIDE { return fSink->fileExtension(); }

private:
    SkAutoTDelete<Sink> fSink;
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

}  // namespace DM

#endif  // SK_BUILD_FOR_ANDROID_FRAMEWORK

#endif  // DMSrcSinkAndroid_DEFINED
