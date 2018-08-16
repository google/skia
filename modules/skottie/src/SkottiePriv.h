/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottiePriv_DEFINED
#define SkottiePriv_DEFINED

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
class RenderNode;
} // namespace sksg

namespace skottie {

class ResourceProvider;

namespace internal {

struct AssetInfo {
    const skjson::ObjectValue* fAsset;
    mutable bool               fIsAttaching; // Used for cycle detection
};
using AssetMap   = SkTHashMap<SkString, AssetInfo>;
using AssetCache = SkTHashMap<SkString, sk_sp<sksg::RenderNode>>;

struct FontInfo {
    SkString                  fFamily,
                              fStyle;
    SkScalar                  fAscent;
    sk_sp<SkTypeface>         fTypeface;

    bool matches(const char family[], const char style[]) const;
};
using FontMap = SkTHashMap<SkString, FontInfo>;

struct AttachContext {
    AttachContext makeScoped(sksg::AnimatorList& animators) const {
        return { fResources, fAssets, fFonts, fDuration, fFrameRate, fAssetCache, animators };
    }

    const ResourceProvider& fResources;
    const AssetMap&         fAssets;
    const FontMap&          fFonts;
    const float             fDuration,
                            fFrameRate;
    AssetCache&             fAssetCache;
    sksg::AnimatorList&     fAnimators;
};

void LogJSON(const skjson::Value&, const char[]);

FontMap ParseFonts(const skjson::ObjectValue* jfonts,
                   const skjson::ArrayValue* jchars,
                   const SkFontMgr*);

sk_sp<sksg::RenderNode> AttachTextLayer(const skjson::ObjectValue&, AttachContext*);

} // namespace internal
} // namespace skottie

#endif // SkottiePriv_DEFINED
