/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieUtils_DEFINED
#define SkottieUtils_DEFINED

#include "Skottie.h"
#include "SkString.h"

#include <memory>

class SkAnimCodecPlayer;
class SkData;
class SkImage;

namespace skottie_utils {

class MultiFrameImageAsset final : public skottie::ImageAsset {
public:
    static sk_sp<MultiFrameImageAsset> Make(sk_sp<SkData>);

    bool isMultiFrame() override;

    sk_sp<SkImage> getFrame(float t) override;

private:
    explicit MultiFrameImageAsset(std::unique_ptr<SkAnimCodecPlayer>);

    std::unique_ptr<SkAnimCodecPlayer> fPlayer;

    using INHERITED = skottie::ImageAsset;
};

class FileResourceProvider final : public skottie::ResourceProvider {
public:
    static sk_sp<FileResourceProvider> Make(SkString base_dir);

    sk_sp<SkData> load(const char resource_path[], const char resource_name[]) const override;

    sk_sp<skottie::ImageAsset> loadImageAsset(const char[], const char []) const override;

private:
    explicit FileResourceProvider(SkString);

    const SkString fDir;

    using INHERITED = skottie::ResourceProvider;
};

} // namespace skottie_utils

#endif // SkottieUtils_DEFINED
