/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Skottie_DEFINED
#define Skottie_DEFINED

#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "modules/skresources/include/SkResources.h"

#include <memory>

class SkCanvas;
struct SkRect;
class SkStream;

namespace skjson { class ObjectValue; }

namespace sksg {

class InvalidationController;
class Scene;

} // namespace sksg

namespace skottie {

using ImageAsset = skresources::ImageAsset;
using ResourceProvider = skresources::ResourceProvider;

class PropertyObserver;

/**
 * A Logger subclass can be used to receive Animation::Builder parsing errors and warnings.
 */
class SK_API Logger : public SkRefCnt {
public:
    enum class Level {
        kWarning,
        kError,
    };

    virtual void log(Level, const char message[], const char* json = nullptr);
};

/**
 * Interface for receiving AE composition markers at Animation build time.
 */
class SK_API MarkerObserver : public SkRefCnt {
public:
    // t0,t1 are in the Animation::seek() domain.
    virtual void onMarker(const char name[], float t0, float t1) = 0;
};

class SK_API Animation : public SkNVRefCnt<Animation> {
public:

    class Builder final {
    public:
        enum Flags : uint32_t {
            kDeferImageLoading = 0x01, // Normally, all static image frames are resolved at
                                       // load time via ImageAsset::getFrame(0).  With this flag,
                                       // frames are only resolved when needed, at seek() time.
        };

        explicit Builder(uint32_t flags = 0);
        ~Builder();

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
         * Specify a PropertyObserver to receive callbacks during parsing.
         *
         * See SkottieProperty.h for more details.
         *
         */
        Builder& setPropertyObserver(sk_sp<PropertyObserver>);

        /**
         * Register a Logger with this builder.
         */
        Builder& setLogger(sk_sp<Logger>);

        /**
         * Register a MarkerObserver with this builder.
         */
        Builder& setMarkerObserver(sk_sp<MarkerObserver>);

        /**
         * Animation factories.
         */
        sk_sp<Animation> make(SkStream*);
        sk_sp<Animation> make(const char* data, size_t length);
        sk_sp<Animation> makeFromFile(const char path[]);

    private:
        const uint32_t          fFlags;

        sk_sp<ResourceProvider> fResourceProvider;
        sk_sp<SkFontMgr>        fFontMgr;
        sk_sp<PropertyObserver> fPropertyObserver;
        sk_sp<Logger>           fLogger;
        sk_sp<MarkerObserver>   fMarkerObserver;
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

    ~Animation();

    enum RenderFlag : uint32_t {
        // When rendering into a known transparent buffer, clients can pass
        // this flag to avoid some unnecessary compositing overhead for
        // animations using layer blend modes.
        kSkipTopLevelIsolation = 0x01,
    };
    using RenderFlags = uint32_t;

    /**
     * Draws the current animation frame.
     *
     * It is undefined behavior to call render() on a newly created Animation
     * before specifying an initial frame via one of the seek() variants.
     *
     * @param canvas   destination canvas
     * @param dst      optional destination rect
     * @param flags    optional RenderFlags
     */
    void render(SkCanvas* canvas, const SkRect* dst = nullptr) const;
    void render(SkCanvas* canvas, const SkRect* dst, RenderFlags) const;

    /**
     * [Deprecated: use one of the other versions.]
     *
     * Updates the animation state for |t|.
     *
     * @param t   normalized [0..1] frame selector (0 -> first frame, 1 -> final frame)
     * @param ic  optional invalidation controller (dirty region tracking)
     *
     */
    void seek(SkScalar t, sksg::InvalidationController* ic = nullptr) {
        this->seekFrameTime(t * this->duration(), ic);
    }

    /**
     * Update the animation state to match |t|, specified as a frame index
     * i.e. relative to duration() * fps().
     *
     * Fractional values are allowed and meaningful - e.g.
     *
     *   0.0 -> first frame
     *   1.0 -> second frame
     *   0.5 -> halfway between first and second frame
     */
    void seekFrame(double t, sksg::InvalidationController* ic = nullptr);

    /** Update the animation state to match t, specifed in frame time
     *  i.e. relative to duration().
     */
    void seekFrameTime(double t, sksg::InvalidationController* = nullptr);

    /**
     * Returns the animation duration in seconds.
     */
    double duration() const { return fDuration; }

    /**
     * Returns the animation frame rate (frames / second).
     */
    double fps() const { return fFPS; }

    const SkString& version() const { return fVersion; }
    const SkSize&      size() const { return fSize;    }

private:
    enum Flags : uint32_t {
        kRequiresTopLevelIsolation = 1 << 0, // Needs to draw into a layer due to layer blending.
    };

    Animation(std::unique_ptr<sksg::Scene>, SkString ver, const SkSize& size,
              double inPoint, double outPoint, double duration, double fps, uint32_t flags);

    std::unique_ptr<sksg::Scene> fScene;
    const SkString               fVersion;
    const SkSize                 fSize;
    const double                 fInPoint,
                                 fOutPoint,
                                 fDuration,
                                 fFPS;
    const uint32_t               fFlags;

    typedef SkNVRefCnt<Animation> INHERITED;
};

} // namespace skottie

#endif // Skottie_DEFINED
