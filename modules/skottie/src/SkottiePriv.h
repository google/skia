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
#include "SkSGScene.h"
#include "SkString.h"
#include "SkTHash.h"
#include "SkTypeface.h"
#include "SkUTF.h"

#define LOG SkDebugf

class SkFontMgr;

namespace skjson {
class ArrayValue;
class ObjectValue;
class Value;
} // namespace skjson

namespace sksg {
class Color;
class Matrix;
class Path;
class RenderNode;
} // namespace sksg

namespace skottie {

namespace internal {

void LogJSON(const skjson::Value&, const char[]);

using AnimatorScope = sksg::AnimatorList;

class AnimationBuilder final : public SkNoncopyable {
public:
    AnimationBuilder(sk_sp<ResourceProvider>, sk_sp<SkFontMgr>, Animation::Builder::Stats*,
                    float duration, float framerate);

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

    sk_sp<sksg::Color> attachColor(const skjson::ObjectValue&, AnimatorScope*,
                                   const char prop_name[]) const;
    sk_sp<sksg::Matrix> attachMatrix(const skjson::ObjectValue&, AnimatorScope*,
                                     sk_sp<sksg::Matrix>) const;
    sk_sp<sksg::RenderNode> attachOpacity(const skjson::ObjectValue&, AnimatorScope*,
                                      sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::Path> attachPath(const skjson::Value&, AnimatorScope*) const;

private:
    struct AttachLayerContext;

    void parseAssets(const skjson::ArrayValue*);
    void parseFonts (const skjson::ObjectValue* jfonts,
                     const skjson::ArrayValue* jchars);

    sk_sp<sksg::RenderNode> attachComposition(const skjson::ObjectValue&, AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachLayer(const skjson::ObjectValue*, AttachLayerContext*) const;
    sk_sp<sksg::RenderNode> attachLayerEffects(const skjson::ArrayValue& jeffects, AnimatorScope*,
                                               sk_sp<sksg::RenderNode>) const;

    sk_sp<sksg::RenderNode> attachAssetRef(const skjson::ObjectValue&, AnimatorScope*,
        sk_sp<sksg::RenderNode>(AnimationBuilder::*)(const skjson::ObjectValue&,
                                                     AnimatorScope* ctx) const) const;
    sk_sp<sksg::RenderNode> attachImageAsset(const skjson::ObjectValue&, AnimatorScope*) const;

    sk_sp<sksg::RenderNode> attachNestedAnimation(const char* name, AnimatorScope* ascope) const;

    sk_sp<sksg::RenderNode> attachImageLayer  (const skjson::ObjectValue&, AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachNullLayer   (const skjson::ObjectValue&, AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachPrecompLayer(const skjson::ObjectValue&, AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachShapeLayer  (const skjson::ObjectValue&, AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachSolidLayer  (const skjson::ObjectValue&, AnimatorScope*) const;
    sk_sp<sksg::RenderNode> attachTextLayer   (const skjson::ObjectValue&, AnimatorScope*) const;

    sk_sp<ResourceProvider>    fResourceProvider;
    sk_sp<SkFontMgr>           fFontMgr;
    Animation::Builder::Stats* fStats;
    const float                fDuration,
                               fFrameRate;

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

    // TODO: consolidate these two?
    using AssetMap   = SkTHashMap<SkString, AssetInfo>;
    using AssetCache = SkTHashMap<SkString, sk_sp<sksg::RenderNode>>;
    using FontMap    = SkTHashMap<SkString, FontInfo>;

    AssetMap           fAssets;
    FontMap            fFonts;
    mutable AssetCache fAssetCache;

    using INHERITED = SkNoncopyable;
};

} // namespace internal
} // namespace skottie

#endif // SkottiePriv_DEFINED
