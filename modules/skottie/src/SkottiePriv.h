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
#include "include/utils/SkCustomTypeface.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/sksg/include/SkSGScene.h"
#include "src/utils/SkUTF.h"

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
                     sk_sp<Logger>, sk_sp<MarkerObserver>, sk_sp<PrecompInterceptor>,
                     sk_sp<ExpressionManager>,
                     Animation::Builder::Stats*, const SkSize& comp_size,
                     float duration, float framerate, uint32_t flags);

    struct AnimationInfo {
        std::unique_ptr<sksg::Scene> fScene;
        AnimatorScope                fAnimators;
    };

    AnimationInfo parse(const skjson::ObjectValue&);

    struct FontInfo {
        SkString                fFamily,
                                fStyle,
                                fPath;
        SkScalar                fAscentPct;
        sk_sp<SkTypeface>       fTypeface;
        SkCustomTypefaceBuilder fCustomBuilder;

        bool matches(const char family[], const char style[]) const;
    };
    const FontInfo* findFont(const SkString& name) const;

    void log(Logger::Level, const skjson::Value*, const char fmt[], ...) const;

    sk_sp<sksg::Transform> attachMatrix2D(const skjson::ObjectValue&, sk_sp<sksg::Transform>,
                                          bool auto_orient = false) const;
    sk_sp<sksg::Transform> attachMatrix3D(const skjson::ObjectValue&, sk_sp<sksg::Transform>,
                                          bool auto_orient = false) const;

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
        AutoPropertyTracker(const AnimationBuilder* builder, const skjson::ObjectValue& obj, const PropertyObserver::NodeType node_type)
            : fBuilder(builder)
            , fPrevContext(builder->fPropertyObserverContext), fNodeType(node_type) {
            if (fBuilder->fPropertyObserver) {
                auto observer = builder->fPropertyObserver.get();
                this->updateContext(observer, obj);
                observer->onEnterNode(fBuilder->fPropertyObserverContext, fNodeType);
            }
        }

        ~AutoPropertyTracker() {
            if (fBuilder->fPropertyObserver) {
                fBuilder->fPropertyObserver->onLeavingNode(fBuilder->fPropertyObserverContext, fNodeType);
                fBuilder->fPropertyObserverContext = fPrevContext;
            }
        }
    private:
        void updateContext(PropertyObserver*, const skjson::ObjectValue&);

        const AnimationBuilder* fBuilder;
        const char*             fPrevContext;
        const PropertyObserver::NodeType fNodeType;
    };

    bool dispatchColorProperty(const sk_sp<sksg::Color>&) const;
    bool dispatchOpacityProperty(const sk_sp<sksg::OpacityEffect>&) const;
    bool dispatchTextProperty(const sk_sp<TextAdapter>&) const;
    bool dispatchTransformProperty(const sk_sp<TransformAdapter2D>&) const;

    sk_sp<ExpressionManager> expression_manager() const;

private:
    friend class CompositionBuilder;
    friend class LayerBuilder;

    struct AttachLayerContext;
    struct AttachShapeContext;
    struct FootageAssetInfo;
    struct LayerInfo;

    void parseAssets(const skjson::ArrayValue*);
    void parseFonts (const skjson::ObjectValue* jfonts,
                     const skjson::ArrayValue* jchars);

    // Return true iff all fonts were resolved.
    bool resolveNativeTypefaces();
    bool resolveEmbeddedTypefaces(const skjson::ArrayValue& jchars);

    void dispatchMarkers(const skjson::ArrayValue*) const;

    sk_sp<sksg::RenderNode> attachBlendMode(const skjson::ObjectValue&,
                                            sk_sp<sksg::RenderNode>) const;

    sk_sp<sksg::RenderNode> attachShape(const skjson::ArrayValue*, AttachShapeContext*,
                                        bool suppress_draws = false) const;
    const FootageAssetInfo* loadFootageAsset(const skjson::ObjectValue&) const;
    sk_sp<sksg::RenderNode> attachFootageAsset(const skjson::ObjectValue&, LayerInfo*) const;

    sk_sp<sksg::RenderNode> attachExternalPrecompLayer(const skjson::ObjectValue&,
                                                       const LayerInfo&) const;

    sk_sp<sksg::RenderNode> attachFootageLayer(const skjson::ObjectValue&, LayerInfo*) const;
    sk_sp<sksg::RenderNode> attachNullLayer   (const skjson::ObjectValue&, LayerInfo*) const;
    sk_sp<sksg::RenderNode> attachPrecompLayer(const skjson::ObjectValue&, LayerInfo*) const;
    sk_sp<sksg::RenderNode> attachShapeLayer  (const skjson::ObjectValue&, LayerInfo*) const;
    sk_sp<sksg::RenderNode> attachSolidLayer  (const skjson::ObjectValue&, LayerInfo*) const;
    sk_sp<sksg::RenderNode> attachTextLayer   (const skjson::ObjectValue&, LayerInfo*) const;
    sk_sp<sksg::RenderNode> attachAudioLayer  (const skjson::ObjectValue&, LayerInfo*) const;

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
    sk_sp<PrecompInterceptor>  fPrecompInterceptor;
    sk_sp<ExpressionManager>   fExpressionManager;
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

    struct FootageAssetInfo {
        sk_sp<ImageAsset> fAsset;
        SkISize           fSize;
    };

    class ScopedAssetRef {
    public:
        ScopedAssetRef(const AnimationBuilder* abuilder, const skjson::ObjectValue& jlayer);

        ~ScopedAssetRef() {
            if (fInfo) {
                fInfo->fIsAttaching = false;
            }
        }

        operator bool() const { return !!fInfo; }

        const skjson::ObjectValue& operator*() const { return *fInfo->fAsset; }

    private:
        const AssetInfo* fInfo = nullptr;
    };

    SkTHashMap<SkString, AssetInfo>                fAssets;
    SkTHashMap<SkString, FontInfo>                 fFonts;
    mutable SkTHashMap<SkString, FootageAssetInfo> fImageAssetCache;

    using INHERITED = SkNoncopyable;
};

} // namespace internal
} // namespace skottie

#endif // SkottiePriv_DEFINED
