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
class SkData;
class SkFontMgr;
struct SkRect;
class SkStream;

namespace skjson { class ObjectValue; }
namespace sksg { class Scene;  }

namespace skottie {

class SK_API ResourceProvider {
public:
    ResourceProvider() = default;
    virtual ~ResourceProvider() = default;
    ResourceProvider(const ResourceProvider&) = delete;
    ResourceProvider& operator=(const ResourceProvider&) = delete;

    virtual sk_sp<SkData> load(const char resource_path[], const char resource_name[]) const = 0;
};

class SK_API Animation : public SkRefCnt {
public:
    // Animation stats, filled by the factory.
    struct Stats {
        float  fTotalLoadTimeMS  = 0,
               fJsonParseTimeMS  = 0,
               fSceneParseTimeMS = 0;
        size_t fJsonSize         = 0u,
               fAnimatorCount    = 0u;
    };

    // Factory args.
    struct Rec {
        // Optional loader for external resources (images, etc.)
        const ResourceProvider* fProvider = nullptr;
        // Optional font manager.
        const SkFontMgr*        fFontMgr  = nullptr;

        // Optional stats out-param.
        Stats*                  fStats    = nullptr;
    };

    static sk_sp<Animation> Make(const char* data, size_t length, const Rec& rec);
    static sk_sp<Animation> Make(SkStream*, const Rec&);
    static sk_sp<Animation> MakeFromFile(const char path[], const Rec&);

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
     * @param t   normalized [0..1] frame selector (0 -> first frame, 1 -> final frame)
     *
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

    void setShowInval(bool show);

private:
    Animation(SkString ver, const SkSize& size, SkScalar fps,
              SkScalar in, SkScalar out, const skjson::ObjectValue&, const Rec&);

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
