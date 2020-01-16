/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Keyframe.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"

namespace skottie {
namespace internal {

void KeyframeStore::parseKeyFrames(const AnimationBuilder& abuilder,
                                   const skjson::ArrayValue& jframes,
                                   ValueStoreProvider* value_store) {
    // Logically, a keyframe is defined as a (t0, t1, v0, v1) tuple: a given value
    // is interpolated in the [v0..v1] interval over the [t0..t1] time span.
    //
    // There are three interestingly-different keyframe formats handled here.
    //
    // 1) Legacy keyframe format
    //
    //      - normal keyframes specify t0 ("t"), v0 ("s") and v1 ("e")
    //      - last frame only specifies a t0
    //      - t1[frame] == t0[frame + 1]
    //      - the last entry (where we cannot determine t1) is ignored
    //
    // 2) Regular (new) keyframe format
    //
    //      - all keyframes specify t0 ("t") and v0 ("s")
    //      - t1[frame] == t0[frame + 1]
    //      - v1[frame] == v0[frame + 1]
    //      - the last entry (where we cannot determine t1/v1) is ignored
    //
    // 3) Text value keyframe format
    //
    //      - similar to case #2, all keyframes specify t0 & v0
    //      - unlike case #2, all keyframes are assumed to be constant (v1 == v0),
    //        and the last frame is not discarded (its t1 is assumed -> inf)
    //

    SkPoint prev_c0 = { 0, 0 },
            prev_c1 = prev_c0;

    fRecs.reserve(SkTMax<size_t>(jframes.size(), 1) - 1);

    for (const skjson::ObjectValue* jframe : jframes) {
        if (!jframe) continue;

        float t0;
        if (!Parse<float>((*jframe)["t"], &t0))
            continue;

        const auto v0_idx = value_store->parseValue(abuilder, (*jframe)["s"]),
                   v1_idx = value_store->parseValue(abuilder, (*jframe)["e"]);

        if (!fRecs.empty()) {
            if (fRecs.back().t1 >= t0) {
                abuilder.log(Logger::Level::kWarning, nullptr,
                             "Ignoring out-of-order key frame (t:%f < t:%f).",
                             t0, fRecs.back().t1);
                continue;
            }

            // Back-fill t1 and v1 (if needed).
            auto& prev = fRecs.back();
            prev.t1 = t0;

            // Previous keyframe did not specify an end value (case #2, #3).
            if (prev.vidx1 < 0) {
                // If this frame has no v0, we're in case #3 (constant text value),
                // otherwise case #2 (v0 for current frame is the same as prev frame v1).
                prev.vidx1 = v0_idx < 0 ? prev.vidx0 : v0_idx;
            }
        }

        // Start value 's' is required.
        if (v0_idx < 0)
            continue;

        if ((v1_idx < 0) && ParseDefault((*jframe)["h"], false)) {
            // Constant keyframe ("h": true).
            fRecs.push_back({t0, t0, v0_idx, v0_idx, -1 });
            continue;
        }

        const auto cubic_mapper_index = [&]() -> int {
            // Do we have non-linear control points?
            SkPoint c0, c1;
            if (!Parse((*jframe)["o"], &c0) ||
                !Parse((*jframe)["i"], &c1) ||
                SkCubicMap::IsLinear(c0, c1)) {
                // No need for a cubic mapper.
                return -1;
            }

            // De-dupe sequential cubic mappers.
            if (c0 != prev_c0 || c1 != prev_c1) {
                fCubicMaps.emplace_back(c0, c1);
                prev_c0 = c0;
                prev_c1 = c1;
            }

            SkASSERT(!fCubicMaps.empty());
            return SkToInt(fCubicMaps.size()) - 1;
        };

        fRecs.push_back({t0, t0, v0_idx, v1_idx, cubic_mapper_index()});
    }

    if (!fRecs.empty()) {
        auto& last = fRecs.back();

        // If the last entry has only a v0, we're in case #3 - make it a constant frame.
        if (last.vidx0 >= 0 && last.vidx1 < 0) {
            last.vidx1 = last.vidx0;
            last.t1 = last.t0;
        }

        // If we couldn't determine a valid t1 for the last frame, discard it
        // (most likely the last frame entry for all 3 cases).
        if (!last.isValid()) {
            fRecs.pop_back();
        }
    }

    fRecs.shrink_to_fit();
    fCubicMaps.shrink_to_fit();

    SkASSERT(fRecs.empty() || fRecs.back().isValid());
}

const KeyframeStore::KeyframeRec* KeyframeStore::findFrame(float t) const {
    SkASSERT(!fRecs.empty());

    auto f0 = &fRecs.front(),
         f1 = &fRecs.back();

    SkASSERT(f0->isValid());
    SkASSERT(f1->isValid());

    if (t < f0->t0) {
        return f0;
    }

    if (t > f1->t1) {
        return f1;
    }

    while (f0 != f1) {
        SkASSERT(f0 < f1);
        SkASSERT(t >= f0->t0 && t <= f1->t1);

        const auto f = f0 + (f1 - f0) / 2;
        SkASSERT(f->isValid());

        if (t > f->t1) {
            f0 = f + 1;
        } else {
            f1 = f;
        }
    }

    SkASSERT(f0 == f1);
    SkASSERT(f0->contains(t));

    return f0;
}

//float KeyframeStore::remapForFrame(const KeyframeRec& rec, float t) const {
//    SkASSERT(rec.isValid());
//    SkASSERT(!rec.isConstant());
//    SkASSERT(t > rec.t0 && t < rec.t1);

//    auto rel_t = (t - rec.t0) / (rec.t1 - rec.t0);

//    return rec.cmidx < 0
//        ? rel_t
//        : fCubicMaps[static_cast<size_t>(rec.cmidx)].computeYFromX(rel_t);
//}

} // namespace internal
} // namespace skottie
