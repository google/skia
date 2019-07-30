/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottiePriv_DEFINED
#define SkottiePriv_DEFINED

#include "modules/skottie/include/Skottie.h"

#include "include/core/SkFontStyle.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTHash.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/sksg/include/SkSGScene.h"
#include "src/utils/SkUTF.h"

#include <functional>

class SkFontMgr;

namespace skjson {
class ArrayValue;
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

class TransformAdapter2D;
class TransformAdapter3D;

namespace internal {

using AnimatorScope = sksg::AnimatorList;

class AnimationBuilder final : public SkNoncopyable {
public:
    AnimationBuilder(sk_sp<ResourceProvider>, sk_sp<SkFontMgr>, sk_sp<PropertyObserver>,
                     sk_sp<Logger>, sk_sp<MarkerObserver>,
                     Animation::Builder::Stats*, const SkSize& size,
                     float duration, float framerate);

    std::unique_ptr<sksg::Scene> parse(const skjson::ObjectValue&);

    struct FontInfo {
        SkString                  fFamily,
                                  fStyle;
        SkScalar                  fAscentPct;
        sk_sp<SkTypeface>         fTypeface;

        bool matches(const char family[], const char style[]) const;
    };
    const FontInfo* findFont(const SkString& name) const;

    // This is the workhorse for property binding: depending on whether the property is animated,
    // it will either apply immediately or instantiate and attach a keyframe animator.
    template <typename T>
    bool bindProperty(const skjson::Value&,
                      std::function<void(const T&)>&&,
                      const T* default_igore = nullptr) const;

    template <typename T>
    bool bindProperty(const skjson::Value& jv,
                      std::function<void(const T&)>&& apply,
                      const T& default_ignore) const {
        return this->bindProperty(jv, std::move(apply), &default_ignore);
    }

    void log(Logger::Level, const skjson::Value*, const char fmt[], ...) const;

    sk_sp<sksg::Color> attachColor(const skjson::ObjectValue&, const char prop_name[]) const;
    sk_sp<sksg::Transform> attachMatrix2D(const skjson::ObjectValue&, sk_sp<sksg::Transform>) const;
    sk_sp<sksg::Transform> attachMatrix3D(const skjson::ObjectValue&, sk_sp<sksg::Transform>,
                                          sk_sp<TransformAdapter3D> = nullptr,
                                          bool precompose_parent = false) const;
    sk_sp<sksg::RenderNode> attachOpacity(const skjson::ObjectValue&,
                                          sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::Path> attachPath(const skjson::Value&) const;

    bool hasNontrivialBlending() const { return fHasNontrivialBlending; }

    class AutoScope final {
    public:
        explicit AutoScope(const AnimationBuilder* builder) : AutoScope(builder, AnimatorScope()) {}

        AutoScope(const AnimationBuilder* builder, AnimatorScope&& scope)
            : fBuilder(builder)
            , fCurrentScope(std::move(scope))
            , fPrevScope(fBuilder->fCurrentAnimatorScope) {
            fBuilder->fCurrentAnimatorScope = &fCurrentScope;
        }

        AnimatorScope release() {
            fBuilder->fCurrentAnimatorScope = fPrevScope;
            SkDEBUGCODE(fBuilder = nullptr);

            return std::move(fCurrentScope);
        }

        ~AutoScope() { SkASSERT(!fBuilder); }

    private:
        const AnimationBuilder* fBuilder;
        AnimatorScope           fCurrentScope;
        AnimatorScope*          fPrevScope;
    };

    // TODO: delete
    AnimatorScope* currentScope() const { return fCurrentAnimatorScope; }

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

private:
    struct AttachLayerContext;
    struct AttachShapeContext;
    struct ImageAssetInfo;
    struct LayerInfo;

    void parseAssets(const skjson::ArrayValue*);
    void parseFonts (const skjson::ObjectValue* jfonts,
                     const skjson::ArrayValue* jchars);

    void dispatchMarkers(const skjson::ArrayValue*) const;

    sk_sp<sksg::RenderNode> attachComposition(const skjson::ObjectValue&) const;
    sk_sp<sksg::RenderNode> attachLayer(const skjson::ObjectValue*,
                                        AttachLayerContext*) const;

    sk_sp<sksg::RenderNode> attachBlendMode(const skjson::ObjectValue&,
                                            sk_sp<sksg::RenderNode>) const;

    sk_sp<sksg::RenderNode> attachShape(const skjson::ArrayValue*, AttachShapeContext*) const;
    sk_sp<sksg::RenderNode> attachAssetRef(const skjson::ObjectValue&,
        const std::function<sk_sp<sksg::RenderNode>(const skjson::ObjectValue&)>&) const;
    const ImageAssetInfo* loadImageAsset(const skjson::ObjectValue&) const;
    sk_sp<sksg::RenderNode> attachImageAsset(const skjson::ObjectValue&, LayerInfo*) const;

    sk_sp<sksg::RenderNode> attachNestedAnimation(const char* name) const;

    sk_sp<sksg::RenderNode> attachImageLayer  (const skjson::ObjectValue&, LayerInfo*) const;
    sk_sp<sksg::RenderNode> attachNullLayer   (const skjson::ObjectValue&, LayerInfo*) const;
    sk_sp<sksg::RenderNode> attachPrecompLayer(const skjson::ObjectValue&, LayerInfo*) const;
    sk_sp<sksg::RenderNode> attachShapeLayer  (const skjson::ObjectValue&, LayerInfo*) const;
    sk_sp<sksg::RenderNode> attachSolidLayer  (const skjson::ObjectValue&, LayerInfo*) const;
    sk_sp<sksg::RenderNode> attachTextLayer   (const skjson::ObjectValue&, LayerInfo*) const;

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

    sk_sp<ResourceProvider>    fResourceProvider;
    LazyResolveFontMgr         fLazyFontMgr;
    sk_sp<PropertyObserver>    fPropertyObserver;
    sk_sp<Logger>              fLogger;
    sk_sp<MarkerObserver>      fMarkerObserver;
    Animation::Builder::Stats* fStats;
    const SkSize               fSize;
    const float                fDuration,
                               fFrameRate;
    mutable AnimatorScope*     fCurrentAnimatorScope;
    mutable const char*        fPropertyObserverContext;
    mutable bool               fHasNontrivialBlending : 1;

    struct LayerInfo {
        SkSize      fSize;
        const float fInPoint,
                    fOutPoint;
    };

    struct AssetInfo {
        const skjson::ObjectValue* fAsset;
        mutable bool               fIsAttaching; // Used for cycle detection
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

struct AnimationBuilder::AttachLayerContext {
    explicit AttachLayerContext(const skjson::ArrayValue&);
    ~AttachLayerContext();

    struct TransformRec {
        sk_sp<sksg::Transform> fTransformNode;
        AnimatorScope          fTransformScope;
    };

    const skjson::ArrayValue&     fLayerList;
    SkTHashMap<int, TransformRec> fLayerTransformMap;
    sk_sp<sksg::RenderNode>       fCurrentMatte;
    sk_sp<sksg::Transform>        fCameraTransform;

    size_t                        fMotionBlurSamples = 1;
    float                         fMotionBlurAngle   = 0,
                                  fMotionBlurPhase   = 0;

    enum class TransformType { kLayer, kCamera };

    TransformRec attachLayerTransform(const skjson::ObjectValue& jlayer,
                                      const AnimationBuilder* abuilder,
                                      TransformType type = TransformType::kLayer);

    bool hasMotionBlur(const skjson::ObjectValue& jlayer) const;

private:
    sk_sp<sksg::Transform> attachParentLayerTransform(const skjson::ObjectValue& jlayer,
                                                      const AnimationBuilder* abuilder,
                                                      int layer_index);

    sk_sp<sksg::Transform> attachTransformNode(const skjson::ObjectValue& jlayer,
                                               const AnimationBuilder* abuilder,
                                               sk_sp<sksg::Transform> parent_transform,
                                               TransformType type) const;

    TransformRec* attachLayerTransformImpl(const skjson::ObjectValue& jlayer,
                                           const AnimationBuilder* abuilder,
                                           TransformType type, int layer_index);
};

} // namespace internal
} // namespace skottie

#endif // SkottiePriv_DEFINED
