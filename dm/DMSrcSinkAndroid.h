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

class HWUISink : public Sink {
public:
    HWUISink() { }

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const SK_OVERRIDE;
    int enclave() const SK_OVERRIDE { return kGPU_Enclave; }
    const char* fileExtension() const SK_OVERRIDE { return "png"; }

private:
    const float kDensity = 1.0f;
    inline float dp(int x) const { return x * kDensity; }
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

}  // namespace DM

#endif  // SK_BUILD_FOR_ANDROID_FRAMEWORK

#endif  // DMSrcSinkAndroid_DEFINED
