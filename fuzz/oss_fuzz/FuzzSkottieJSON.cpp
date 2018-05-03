/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "Skottie.h"
#include "SkStream.h"

void FuzzSkottieJSON(sk_sp<SkData> bytes) {
    // Always returns nullptr to any resource
    class EmptyResourceProvider final : public skottie::ResourceProvider {
    public:
        std::unique_ptr<SkStream> openStream(const char resource[]) const override {
            return nullptr;
        }
    };
    SkMemoryStream stream(bytes);
    EmptyResourceProvider erp;
    auto animation = skottie::Animation::Make(&stream, erp);
    if (!animation) {
        return;
    }
    animation->animationTick(1337); // A "nothing up my sleeve" number
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSkottieJSON(bytes);
    return 0;
}
#endif
