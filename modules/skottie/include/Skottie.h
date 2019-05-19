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

#include <memory>

class SkCanvas;
class SkData;
class SkImage;
struct SkRect;
class SkStream;

namespace skjson { class ObjectValue; }

namespace sksg { class Scene;  }

namespace skottie {

class PropertyObserver;

/**
 * Image asset proxy interface.
 */
class SK_API ImageAsset : public SkRefCnt {
public:
    /**
     * Returns true if the image asset is animated.
     */
    virtual bool isMultiFrame() = 0;

    /**
     * Returns the SkImage for a given frame.
     *
     * If the image asset is static, getImage() is only called once, at animation load time.
     * Otherwise, this gets invoked every time the animation time is adjusted (on every seek).
     *
     * Embedders should cache and serve the same SkImage whenever possible, for efficiency.
     *
     * @param t   Frame time code, in seconds, relative to the image layer timeline origin
     *            (in-point).
     */
    virtual sk_sp<SkImage> getFrame(float t) = 0;
};

/**
 * ResourceProvider allows Skottie embedders to control loading of external
 * Skottie resources -- e.g. images, fonts, nested animations.
 */
class SK_API ResourceProvider : public SkRefCnt {
public:
    /**
     * Load a generic resource (currently only nested animations) specified by |path| + |name|,
     * and return as an SkData.
     */
    virtual sk_sp<SkData> load(const char resource_path[],
                               const char resource_name[]) const;

    /**
     * Load an image asset specified by |path| + |name|, and returns the corresponding
     * ImageAsset proxy.
     */
    virtual sk_sp<ImageAsset> loadImageAsset(const char resource_path[],
                                             const char resource_name[]) const;

    /**
     * Load an external font and return as SkData.
     *
     * @param name  font name    ("fName" Lottie property)
     * @param url   web font URL ("fPath" Lottie property)
     *
     * -- Note --
     *
     *   This mechanism assumes monolithic fonts (single data blob).  Some web font providers may
     *   serve multiple font blobs, segmented for various unicode ranges, depending on user agent
     *   capabilities (woff, woff2).  In that case, the embedder would need to advertise no user
     *   agent capabilities when fetching the URL, in order to receive full font data.
     */
    virtual sk_sp<SkData> loadFont(const char name[], const char url[]) const;
};

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
        Builder();
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
     * @param canvas   destination canvas
     * @param dst      optional destination rect
     * @param flags    optional RenderFlags
     */
    void render(SkCanvas* canvas, const SkRect* dst = nullptr) const;
    void render(SkCanvas* canvas, const SkRect* dst, RenderFlags) const;

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
    enum Flags : uint32_t {
        kRequiresTopLevelIsolation = 1 << 0, // Needs to draw into a layer due to layer blending.
    };

    Animation(std::unique_ptr<sksg::Scene>, SkString ver, const SkSize& size,
              SkScalar inPoint, SkScalar outPoint, SkScalar duration, uint32_t flags = 0);

    std::unique_ptr<sksg::Scene> fScene;
    const SkString               fVersion;
    const SkSize                 fSize;
    const SkScalar               fInPoint,
                                 fOutPoint,
                                 fDuration;
    const uint32_t               fFlags;

    typedef SkNVRefCnt<Animation> INHERITED;
};

} // namespace skottie

#endif // Skottie_DEFINED
