/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieUtils.h"

#include "SkAnimCodecPlayer.h"
#include "SkData.h"
#include "SkCodec.h"
#include "SkImage.h"
#include "SkMakeUnique.h"
#include "SkOSFile.h"
#include "SkOSPath.h"

namespace skottie_utils {

sk_sp<MultiFrameImageAsset> MultiFrameImageAsset::Make(sk_sp<SkData> data) {
    if (auto codec = SkCodec::MakeFromData(std::move(data))) {
        return sk_sp<MultiFrameImageAsset>(
              new MultiFrameImageAsset(skstd::make_unique<SkAnimCodecPlayer>(std::move(codec))));
    }

    return nullptr;
}

MultiFrameImageAsset::MultiFrameImageAsset(std::unique_ptr<SkAnimCodecPlayer> player)
    : fPlayer(std::move(player)) {
    SkASSERT(fPlayer);
}

bool MultiFrameImageAsset::isMultiFrame() {
    return fPlayer->duration() > 0;
}

sk_sp<SkImage> MultiFrameImageAsset::getFrame(float t) {
    fPlayer->seek(static_cast<uint32_t>(t * 1000));
    return fPlayer->getFrame();
}

sk_sp<FileResourceProvider> FileResourceProvider::Make(SkString base_dir) {
    return sk_isdir(base_dir.c_str())
        ? sk_sp<FileResourceProvider>(new FileResourceProvider(std::move(base_dir)))
        : nullptr;
}

FileResourceProvider::FileResourceProvider(SkString base_dir) : fDir(std::move(base_dir)) {}

sk_sp<SkData> FileResourceProvider::load(const char resource_path[],
                                         const char resource_name[]) const {
    const auto full_dir  = SkOSPath::Join(fDir.c_str()    , resource_path),
               full_path = SkOSPath::Join(full_dir.c_str(), resource_name);
    return SkData::MakeFromFileName(full_path.c_str());
}

sk_sp<skottie::ImageAsset> FileResourceProvider::loadImageAsset(const char resource_path[],
                                                                const char resource_name[]) const {
    return MultiFrameImageAsset::Make(this->load(resource_path, resource_name));
}

} // namespace skottie_utils
