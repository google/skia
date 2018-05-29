/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Skottie_DEFINED
#define Skottie_DEFINED

#include "SkRefCnt.h"
#include "SkSize.h"
#include "SkString.h"
#include "SkTypes.h"

#include <memory>

class SkCanvas;
struct SkRect;
class SkStream;

namespace sksg { class Scene;  }

namespace skottie {

namespace json { class ValueRef; }

class SK_API ResourceProvider : public SkNoncopyable {
public:
    virtual ~ResourceProvider() = default;

    virtual std::unique_ptr<SkStream> openStream(const char resource[]) const = 0;
};

class SK_API Animation : public SkRefCnt {
public:
    struct Stats {
        float  fTotalLoadTimeMS,
               fJsonParseTimeMS,
               fSceneParseTimeMS;
        size_t fJsonSize,
               fAnimatorCount;
    };

    static sk_sp<Animation> Make(SkStream*, const ResourceProvider&, Stats* = nullptr);
    static sk_sp<Animation> MakeFromFile(const char path[], const ResourceProvider* = nullptr,
                                         Stats* = nullptr);

    ~Animation() override;

    /**
     * Draws the current animation frame.
     *
     * @param canvas   destination canvas
     * @param dst      optional destination rect
     */
    void render(SkCanvas* canvas, const SkRect* dst = nullptr) const;

    /**
     * Updates the animation state for |t|.
     *
     * @param t   normalized [0..1] frame selector, where 0 == inPoint and 1 == outPoint.
     */
    void seek(SkScalar t);

    /**
     * Returns the animation duration in seconds.
     */
    SkScalar duration() const {
        return (fOutPoint - fInPoint) / fFrameRate;
    }

    const SkString& version() const { return fVersion;   }
    const SkSize&      size() const { return fSize;      }
         SkScalar frameRate() const { return fFrameRate; }
         SkScalar   inPoint() const { return fInPoint;   }
         SkScalar  outPoint() const { return fOutPoint;  }

    void setShowInval(bool show);

private:
    Animation(const ResourceProvider&, SkString ver, const SkSize& size, SkScalar fps,
              const json::ValueRef&, Stats*);

    SkString                     fVersion;
    SkSize                       fSize;
    SkScalar                     fFrameRate,
                                 fInPoint,
                                 fOutPoint;

    std::unique_ptr<sksg::Scene> fScene;

    typedef SkRefCnt INHERITED;
};

} // namespace skottie

#endif // Skottie_DEFINED
