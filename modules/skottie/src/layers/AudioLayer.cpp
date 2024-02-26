/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/utils/SkJSON.h"

#include <utility>

namespace skottie::internal {

namespace {

class ForwardingPlaybackController final : public Animator {
public:
    ForwardingPlaybackController(sk_sp<skresources::ExternalTrackAsset> track,
                                 float in_point,
                                 float out_point,
                                 float fps )
        : fTrack(std::move(track))
        , fInPoint(in_point)
        , fOutPoint(out_point)
        , fFps(fps) {}

private:
    StateChanged onSeek(float t) override {
        // Adjust t relative to the track time (s).
        if (t < fInPoint || t > fOutPoint) {
            t = -1;
        } else {
            t = (t - fInPoint) / fFps;
        }

        fTrack->seek(t);

        // does not interact with the render tree.
        return false;
    }

    const sk_sp<skresources::ExternalTrackAsset> fTrack;
    const float                                  fInPoint,
                                                 fOutPoint,
                                                 fFps;
};

} // namespace

sk_sp<sksg::RenderNode> AnimationBuilder::attachAudioLayer(const skjson::ObjectValue& jlayer,
                                                           LayerInfo* layer_info) const {
    const ScopedAssetRef audio_asset(this, jlayer);

    if (audio_asset) {
        const auto& jaudio = *audio_asset;
        const skjson::StringValue* name = jaudio["p"];
        const skjson::StringValue* path = jaudio["u"];
        const skjson::StringValue* id   = jaudio["id"];

        if (name && path && id) {
            auto track = fResourceProvider->loadAudioAsset(path->begin(),
                                                           name->begin(),
                                                           id  ->begin());
            if (track) {
                fCurrentAnimatorScope->push_back(
                    sk_make_sp<ForwardingPlaybackController>(std::move(track),
                                                             layer_info->fInPoint,
                                                             layer_info->fOutPoint,
                                                             fFrameRate));
            } else {
                this->log(Logger::Level::kWarning, nullptr,
                          "Could not load audio asset '%s'.", name->begin());
            }
        }
    }

    // no render node, playback is controlled from the Animator tree.
    return nullptr;
}

} // namespace skottie::internal
