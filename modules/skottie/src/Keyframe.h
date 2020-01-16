/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieKeyframe_DEFINED
#define SkottieKeyframe_DEFINED

#include "include/core/SkCubicMap.h"
#include "include/private/SkNoncopyable.h"

#include <vector>

class SkCubicMap;

namespace skjson {

class ArrayValue;
class Value;

} // namespace skjson

namespace skottie {
namespace internal {

class AnimationBuilder;

class ValueStoreProvider {
public:
    virtual ~ValueStoreProvider() = default;

    // Parse and store the value.  Returns an index or -1 on failure.
    virtual int parseValue(const AnimationBuilder&, const skjson::Value&) = 0;
};

class KeyframeStore final : SkNoncopyable {
public:
    struct KeyframeRec {
        float t0, t1;
        int   vidx0, vidx1, // v0/v1 indices
              cmidx;        // cubic map index

        bool contains(float t) const { return t0 <= t && t <= t1; }
        bool isConstant() const { return vidx0 == vidx1; }
        bool isValid() const {
            SkASSERT(t0 <= t1);
            // Constant frames don't need/use t1 and vidx1.
            return t0 < t1 || this->isConstant();
        }
    };

    const KeyframeRec& frame(float t) const {
        if (!fCachedRec || !fCachedRec->contains(t)) {
            fCachedRec = this->findFrame(t);
        }

        return *fCachedRec;
    }

    void parseKeyFrames(const AnimationBuilder&, const skjson::ArrayValue&, ValueStoreProvider*);

    bool isEmpty() const { return fRecs.empty(); }

    float remapForKeyframe(const KeyframeRec& kf, float t) const {
        SkASSERT(kf.isValid());
        SkASSERT(!kf.isConstant());
        SkASSERT(t > kf.t0 && t < kf.t1);

        // remap relative to the keyframe range
        t = (t - kf.t0) / (kf.t1 - kf.t0);

        // run through cubic mapper, when present
        if (kf.cmidx >= 0) {
            t = fCubicMaps[static_cast<size_t>(kf.cmidx)].computeYFromX(t);
        }

        return t;
    }

private:
    const KeyframeRec* findFrame(float) const;

    std::vector<KeyframeRec>   fRecs;
    std::vector<SkCubicMap>    fCubicMaps;

    mutable const KeyframeRec* fCachedRec = nullptr;
};

} // namespace internal
} // namespace skottie

#endif // SkottieKeyframe_DEFINED
