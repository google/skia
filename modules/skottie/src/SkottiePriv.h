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
#include "modules/skottie/src/animator/Animator.h"
#include "modules/sksg/include/SkSGScene.h"
#include "src/utils/SkUTF.h"

#include <functional>
#include <vector>

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
namespace internal {

// Close-enough to AE.
static constexpr float kBlurSizeToSigma = 0.3f;

class TextAdapter;
class TransformAdapter2D;
class TransformAdapter3D;

using AnimatorScope = std::vector<sk_sp<Animator>>;

class AnimationBuilder final : public SkNoncopyable {
public:
    AnimationBuilder(sk_sp<ResourceProvider>, sk_sp<SkFontMgr>, sk_sp<PropertyObserver>,
                     sk_sp<Logger>, sk_sp<MarkerObserver>,
                     Animation::Builder::Stats*, const SkSize& comp_size,
                     float duration, float framerate, uint32_t flags);

    struct AnimationInfo {
        std::unique_ptr<sksg::Scene> fScene;
        AnimatorScope                fAnimators;
    };

    AnimationInfo parse(const skjson::ObjectValue&);

    struct FontInfo {
        SkString                  fFamily,
                                  fStyle;
        SkScalar                  fAscentPct;
        sk_sp<SkTypeface>         fTypeface;

        bool matches(const char family[], const char style[]) const;
    };
    const FontInfo* findFont(const SkString& name) const;

    void log(Logger::Level, const skjson::Value*, const char fmt[], ...) const;

    sk_sp<sksg::Transform> attachMatrix2D(const skjson::ObjectValue&, sk_sp<sksg::Transform>) const;
    sk_sp<sksg::Transform> attachMatrix3D(const skjson::ObjectValue&, sk_sp<sksg::Transform>) const;

    sk_sp<sksg::Transform> attachCamera(const skjson::ObjectValue& jlayer,
                                        const skjson::ObjectValue& jtransform,
                                        sk_sp<sksg::Transform>,
                                        const SkSize&) const;

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

    template <typename T>
    void attachDiscardableAdapter(sk_sp<T> adapter) const {
        if (adapter->isStatic()) {
            // Fire off a synthetic tick to force a single SG sync before discarding.
            adapter->seek(0);
        } else {
            fCurrentAnimatorScope->push_back(std::move(adapter));
        }
    }

    template <typename T, typename... Args>
    auto attachDiscardableAdapter(Args&&... args) const ->
        typename std::decay<decltype(T::Make(std::forward<Args>(args)...)->node())>::type
    {
        using NodeType =
        typename std::decay<decltype(T::Make(std::forward<Args>(args)...)->node())>::type;

        NodeType node;
        if (auto adapter = T::Make(std::forward<Args>(args)...)) {
            node = adapter->node();
            this->attachDiscardableAdapter(std::move(adapter));
        }
        return node;
    }

    class AutoPropertyTracker {
    public:
        AutoPropertyTracker(const AnimationBuilder* builder, const skjson::ObjectValue& obj)
            : fBuilder(builder)
            , fPrevContext(builder->fPropertyObserverContext) {
            if (fBuilder->fPropertyObserver) {
                auto observer = builder->fPropertyObserver.get();
                this->updateContext(observer, obj);
                observer->onEnterNode(fBuilder->fPropertyObserverContext);
            }
        }

        ~AutoPropertyTracker() {
            if (fBuilder->fPropertyObserver) {
                fBuilder->fPropertyObserver->onLeavingNode(fBuilder->fPropertyObserverContext);
                fBuilder->fPropertyObserverContext = fPrevContext;
            }
        }
    private:
        void updateContext(PropertyObserver*, const skjson::ObjectValue&);

        const AnimationBuilder* fBuilder;
        const char*             fPrevContext;
    };

    bool dispatchColorProperty(const sk_sp<sksg::Color>&) const;
    bool dispatchOpacityProperty(const sk_sp<sksg::OpacityEffect>&) const;
    bool dispatchTextProperty(const sk_sp<TextAdapter>&) const;
    bool dispatchTransformProperty(const sk_sp<TransformAdapter2D>&) const;

private:
    friend class CompositionBuilder;
    friend class LayerBuilder;

    struct AttachLayerContext;
    struct AttachShapeContext;
    struct ImageAssetInfo;
    struct LayerInfo;

    void parseAssets(const skjson::ArrayValue*);
    void parseFonts (const skjson::ObjectValue* jfonts,
                     const skjson::ArrayValue* jchars);

    void dispatchMarkers(const skjson::ArrayValue*) const;

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
    const SkSize               fCompSize;
    const float                fDuration,
                               fFrameRate;
    const uint32_t             fFlags;
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

} // namespace internal
} // namespace skottie

#endif // SkottiePriv_DEFINED
