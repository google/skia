/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Skottie_DEFINED
#define Skottie_DEFINED

#include "SkFontMgr.h"
#include "SkRefCnt.h"
#include "SkSize.h"
#include "SkString.h"
#include "SkTypes.h"

#include <memory>

class SkCanvas;
class SkData;
struct SkRect;
class SkStream;

namespace skjson { class ObjectValue; }
namespace sksg { class Scene;  }

namespace skottie {

class SK_API ResourceProvider : public SkRefCnt {
public:
    ResourceProvider() = default;
    virtual ~ResourceProvider() = default;
    ResourceProvider(const ResourceProvider&) = delete;
    ResourceProvider& operator=(const ResourceProvider&) = delete;

    virtual sk_sp<SkData> load(const char resource_path[], const char resource_name[]) const = 0;
};

class SK_API Animation : public SkRefCnt {
public:

    class Builder final {
    public:
        struct Stats {
            float  fTotalLoadTimeMS  = 0, // Total animation instantiation time.
                   fJsonParseTimeMS  = 0, // Time spent building a JSON DOM.
                   fSceneParseTimeMS = 0; // Time spent constructing the animation scene graph.
            size_t fJsonSize         = 0, // Input JSON size.
                   fAnimatorCount    = 0; // Number of dynamically animated properties.
        };

        /**
         * Returns various animation build stats.
         *
         * @return Stats (see above).
         */
        const Stats& getStats() const { return fStats; }

        /**
         * Specify a loader for external resources (images, etc.).
         */
        Builder& setResourceProvider(sk_sp<ResourceProvider>);

        /**
         * Specify a font manager for loading animation fonts.
         */
        Builder& setFontManager(sk_sp<SkFontMgr>);

        /**
         * Animation factories.
         */
        sk_sp<Animation> make(SkStream*);
        sk_sp<Animation> make(const char* data, size_t length);
        sk_sp<Animation> makeFromFile(const char path[]);

    private:
        sk_sp<ResourceProvider> fResourceProvider;
        sk_sp<SkFontMgr>        fFontMgr;
        Stats                   fStats;
    };

    /**
     * Animation factories.
     *
     * Use the Builder helper above for more options/control.
     */
    static sk_sp<Animation> Make(const char* data, size_t length);
    static sk_sp<Animation> Make(SkStream*);
    static sk_sp<Animation> MakeFromFile(const char path[]);

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
    SkScalar duration() const { return fDuration; }

    const SkString& version() const { return fVersion;   }
    const SkSize&      size() const { return fSize;      }

    void setShowInval(bool show);

private:
    Animation(std::unique_ptr<sksg::Scene>, SkString ver, const SkSize& size,
              SkScalar inPoint, SkScalar outPoint, SkScalar duration);

    std::unique_ptr<sksg::Scene> fScene;
    const SkString               fVersion;
    const SkSize                 fSize;
    const SkScalar               fInPoint,
                                 fOutPoint,
                                 fDuration;

    typedef SkRefCnt INHERITED;
};

} // namespace skottie

#endif // Skottie_DEFINED
