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
    AnimationBuilder(const ResourceProvider&, sk_sp<SkFontMgr>, Animation::Builder::Stats*,
                    float duration, float framerate);

    std::unique_ptr<sksg::Scene> parse(const skjson::ObjectValue&);

private:
    struct AttachLayerContext;

    void parseAssets(const skjson::ArrayValue*);
    void parseFonts (const skjson::ObjectValue* jfonts,
                     const skjson::ArrayValue* jchars);

    sk_sp<sksg::RenderNode> attachComposition(const skjson::ObjectValue&, AnimatorScope*);
    sk_sp<sksg::RenderNode> attachLayer(const skjson::ObjectValue*, AttachLayerContext*);
    sk_sp<sksg::RenderNode> attachLayerEffects(const skjson::ArrayValue& jeffects, AnimatorScope*,
                                               sk_sp<sksg::RenderNode>);

    sk_sp<sksg::RenderNode> attachAssetRef(const skjson::ObjectValue&, AnimatorScope*,
        sk_sp<sksg::RenderNode>(AnimationBuilder::*)(const skjson::ObjectValue&,
                                                     AnimatorScope* ctx));
    sk_sp<sksg::RenderNode> attachImageAsset(const skjson::ObjectValue&, AnimatorScope*);

    sk_sp<sksg::RenderNode> attachNestedAnimation(const char* name, AnimatorScope* ascope);

    sk_sp<sksg::RenderNode> attachImageLayer  (const skjson::ObjectValue&, AnimatorScope*);
    sk_sp<sksg::RenderNode> attachNullLayer   (const skjson::ObjectValue&, AnimatorScope*);
    sk_sp<sksg::RenderNode> attachPrecompLayer(const skjson::ObjectValue&, AnimatorScope*);
    sk_sp<sksg::RenderNode> attachShapeLayer  (const skjson::ObjectValue&, AnimatorScope*);
    sk_sp<sksg::RenderNode> attachSolidLayer  (const skjson::ObjectValue&, AnimatorScope*);
    sk_sp<sksg::RenderNode> attachTextLayer   (const skjson::ObjectValue&, AnimatorScope*);

    const ResourceProvider&    fResourceProvider;
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

    AssetMap   fAssets;
    AssetCache fAssetCache;
    FontMap    fFonts;

    using INHERITED = SkNoncopyable;
};

// Shared helpers
sk_sp<sksg::Color> AttachColor(const skjson::ObjectValue&, AnimatorScope*, const char prop_name[]);
sk_sp<sksg::Path> AttachPath(const skjson::Value&, AnimatorScope*);
sk_sp<sksg::Matrix> AttachMatrix(const skjson::ObjectValue&, AnimatorScope*, sk_sp<sksg::Matrix>);
sk_sp<sksg::RenderNode> AttachOpacity(const skjson::ObjectValue&, AnimatorScope*,
                                      sk_sp<sksg::RenderNode>);

} // namespace internal
} // namespace skottie

#endif // SkottiePriv_DEFINED
