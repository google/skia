/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottiePriv_DEFINED
#define SkottiePriv_DEFINED

#include "Skottie.h"

#include "SkFontStyle.h"
#include "SkottieProperty.h"
#include "SkSGScene.h"
#include "SkString.h"
#include "SkTHash.h"
#include "SkTypeface.h"
#include "SkUTF.h"

#include <functional>

class SkFontMgr;

namespace skjson {
class ArrayValue;
class NumberValue;
class ObjectValue;
class Value;
} // namespace skjson

namespace sksg {
class Color;
class Path;
class RenderNode;
class Transform;
} // namespace sksg

namespace skottie {

namespace internal {

using AnimatorScope = sksg::AnimatorList;

class AnimationBuilder final : public SkNoncopyable {
public:
    AnimationBuilder(sk_sp<ResourceProvider>, sk_sp<SkFontMgr>, sk_sp<PropertyObserver>,
                     sk_sp<Logger>, sk_sp<MarkerObserver>,
                     Animation::Builder::Stats*, float duration, float framerate);

    std::unique_ptr<sksg::Scene> parse(const skjson::ObjectValue&);

    sk_sp<SkTypeface> findFont(const SkString& name) const;

    // This is the workhorse for property binding: depending on whether the property is animated,
    // it will either apply immediately or instantiate and attach a keyframe animator.
    template <typename T>
    bool bindProperty(const skjson::Value&,
                      AnimatorScope*,
                      std::function<void(const T&)>&&,
                      const T* default_igore = nullptr) const;

    template <typename T>
    bool bindProperty(const skjson::Value& jv,
                      AnimatorScope* ascope,
                      std::function<void(const T&)>&& apply,
                      const T& default_ignore) const {
        return this->bindProperty(jv, ascope, std::move(apply), &default_ignore);
    }

    void log(Logger::Level, const skjson::Value*, const char fmt[], ...) const;

    sk_sp<sksg::Color> attachColor(const skjson::ObjectValue&, AnimatorScope*,
                                   const char prop_name[]) const;
    sk_sp<sksg::Transform> attachMatrix2D(const skjson::ObjectValue&, AnimatorScope*,
                                          sk_sp<sksg::Transform>) const;
    sk_sp<sksg::Transform> attachMatrix3D(const skjson::ObjectValue&, AnimatorScope*,
                                          sk_sp<sksg::Transform>) const;
    sk_sp<sksg::RenderNode> attachOpacity(const skjson::ObjectValue&, AnimatorScope*,
                                      sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::Path> attachPath(const skjson::Value&, AnimatorScope*) const;

    bool hasNontrivialBlending() const { return fHasNontrivialBlending; }

private:
    struct AttachLayerContext;
    struct AttachShapeContext;
    struct ImageAssetInfo;
    struct LayerInfo;

    void parseAssets(const skjson::ArrayValue*);
    void parseFonts (const skjson::ObjectValue* jfonts,
                     const skjson::ArrayValue* jchars);

    void dispatchMarkers(const skjson::ArrayValue*) const;

    sk_sp<sksg::RenderNode> attachComposition(const skjson::ObjectValue&, AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachLayer(const skjson::ObjectValue*, AttachLayerContext*) const;
    sk_sp<sksg::RenderNode> attachLayerEffects(const skjson::ArrayValue& jeffects, AnimatorScope*,
                                               sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachBlendMode(const skjson::NumberValue& jbm,
                                            sk_sp<sksg::RenderNode>) const;


    sk_sp<sksg::RenderNode> attachShape(const skjson::ArrayValue*, AttachShapeContext*) const;
    sk_sp<sksg::RenderNode> attachAssetRef(const skjson::ObjectValue&, AnimatorScope*,
        const std::function<sk_sp<sksg::RenderNode>(const skjson::ObjectValue&,
                                                    AnimatorScope* ctx)>&) const;
    const ImageAssetInfo* loadImageAsset(const skjson::ObjectValue&) const;
    sk_sp<sksg::RenderNode> attachImageAsset(const skjson::ObjectValue&, const LayerInfo&,
                                             AnimatorScope*) const;

    sk_sp<sksg::RenderNode> attachNestedAnimation(const char* name, AnimatorScope* ascope) const;

    sk_sp<sksg::RenderNode> attachImageLayer  (const skjson::ObjectValue&, const LayerInfo&,
                                               AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachNullLayer   (const skjson::ObjectValue&, const LayerInfo&,
                                               AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachPrecompLayer(const skjson::ObjectValue&, const LayerInfo&,
                                               AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachShapeLayer  (const skjson::ObjectValue&, const LayerInfo&,
                                               AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachSolidLayer  (const skjson::ObjectValue&, const LayerInfo&,
                                               AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachTextLayer   (const skjson::ObjectValue&, const LayerInfo&,
                                               AnimatorScope*) const;

    bool dispatchColorProperty(const sk_sp<sksg::Color>&) const;
    bool dispatchOpacityProperty(const sk_sp<sksg::OpacityEffect>&) const;
    bool dispatchTransformProperty(const sk_sp<TransformAdapter2D>&) const;

    // Delay resolving the fontmgr until it is actually needed.
    struct LazyResolveFontMgr {
        LazyResolveFontMgr(sk_sp<SkFontMgr> fontMgr) : fFontMgr(std::move(fontMgr)) {}

        const sk_sp<SkFontMgr>& get() {
            if (!fFontMgr) {
                fFontMgr = SkFontMgr::RefDefault();
                SkASSERT(fFontMgr);
            }
            return fFontMgr;
        }

        const sk_sp<SkFontMgr>& getMaybeNull() const { return fFontMgr; }

    private:
        sk_sp<SkFontMgr> fFontMgr;
    };

    class AutoPropertyTracker {
    public:
        AutoPropertyTracker(const AnimationBuilder* builder, const skjson::ObjectValue& obj)
            : fBuilder(builder)
            , fPrevContext(builder->fPropertyObserverContext) {
            if (fBuilder->fPropertyObserver) {
                this->updateContext(builder->fPropertyObserver.get(), obj);
            }
        }

        ~AutoPropertyTracker() {
            if (fBuilder->fPropertyObserver) {
                fBuilder->fPropertyObserverContext = fPrevContext;
            }
        }
    private:
        void updateContext(PropertyObserver*, const skjson::ObjectValue&);

        const AnimationBuilder* fBuilder;
        const char*             fPrevContext;
    };

    sk_sp<ResourceProvider>    fResourceProvider;
    LazyResolveFontMgr         fLazyFontMgr;
    sk_sp<PropertyObserver>    fPropertyObserver;
    sk_sp<Logger>              fLogger;
    sk_sp<MarkerObserver>      fMarkerObserver;
    Animation::Builder::Stats* fStats;
    const float                fDuration,
                               fFrameRate;
    mutable const char*        fPropertyObserverContext;
    mutable bool               fHasNontrivialBlending : 1;


    struct LayerInfo {
        float fInPoint,
              fOutPoint;
    };

    struct AssetInfo {
        const skjson::ObjectValue* fAsset;
        mutable bool               fIsAttaching; // Used for cycle detection
    };

    struct FontInfo {
        SkString                  fFamily,
                                  fStyle;
        SkScalar                  fAscent;
        sk_sp<SkTypeface>         fTypeface;

        bool matches(const char family[], const char style[]) const;
    };

    struct ImageAssetInfo {
        sk_sp<ImageAsset> fAsset;
        SkISize           fSize;
    };

    SkTHashMap<SkString, AssetInfo>              fAssets;
    SkTHashMap<SkString, FontInfo>               fFonts;
    mutable SkTHashMap<SkString, ImageAssetInfo> fImageAssetCache;

    using INHERITED = SkNoncopyable;
};

} // namespace internal
} // namespace skottie

#endif // SkottiePriv_DEFINED
