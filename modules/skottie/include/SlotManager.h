/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SlotManager_DEFINED
#define SlotManager_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "modules/skottie/src/SkottieValue.h"
#include "src/core/SkTHash.h"

#include <optional>

namespace skjson {
class ObjectValue;
}

namespace skresources {
class ImageAsset;
}

namespace sksg {
class Node;
}
namespace skottie {

struct TextPropertyValue;

namespace internal {
class AnimationBuilder;
class SceneGraphRevalidator;
class AnimatablePropertyContainer;
class TextAdapter;
} // namespace internal

using namespace skia_private;

class SK_API SlotManager final : public SkRefCnt {

public:
    using SlotID = SkString;

    SlotManager(sk_sp<skottie::internal::SceneGraphRevalidator>);
    ~SlotManager() override;

    bool setColorSlot(const SlotID&, SkColor);
    bool setImageSlot(const SlotID&, const sk_sp<skresources::ImageAsset>&);
    bool setScalarSlot(const SlotID&, float);
    bool setVec2Slot(const SlotID&, SkV2);
    bool setTextSlot(const SlotID&, const TextPropertyValue&);

    std::optional<SkColor>               getColorSlot(const SlotID&) const;
    sk_sp<const skresources::ImageAsset> getImageSlot(const SlotID&) const;
    std::optional<float>                 getScalarSlot(const SlotID&) const;
    std::optional<SkV2>                  getVec2Slot(const SlotID&) const;
    std::optional<TextPropertyValue>     getTextSlot(const SlotID&) const;

    struct SlotInfo {
        TArray<SlotID> fColorSlotIDs;
        TArray<SlotID> fScalarSlotIDs;
        TArray<SlotID> fVec2SlotIDs;
        TArray<SlotID> fImageSlotIDs;
        TArray<SlotID> fTextSlotIDs;
    };

    // Helper function to get all slot IDs and their value types
    SlotInfo getSlotInfo() const;

private:

    // pass value to the SlotManager for manipulation and node for invalidation
    void trackColorValue(const SlotID&, ColorValue*,
                         sk_sp<skottie::internal::AnimatablePropertyContainer>);
    sk_sp<skresources::ImageAsset> trackImageValue(const SlotID&, sk_sp<skresources::ImageAsset>);
    void trackScalarValue(const SlotID&, ScalarValue*,
                          sk_sp<skottie::internal::AnimatablePropertyContainer>);
    void trackVec2Value(const SlotID&, Vec2Value*,
                        sk_sp<skottie::internal::AnimatablePropertyContainer>);
    void trackTextValue(const SlotID&, sk_sp<skottie::internal::TextAdapter>);

    // ValuePair tracks a pointer to a value to change, and a means to invalidate the render tree.
    // For the latter, we can take either a node in the scene graph that directly the scene graph,
    // or an adapter which takes the value passed and interprets it before pushing to the scene
    // (clamping, normalizing, etc.)
    // Only one should be set, it is UB to create a ValuePair with both a node and an adapter.
    template <typename T>
    struct ValuePair
    {
        T value;
        sk_sp<skottie::internal::AnimatablePropertyContainer> adapter;
    };

    class ImageAssetProxy;
    template <typename T>
    using SlotMap = THashMap<SlotID, TArray<T>>;

    SlotMap<ValuePair<ColorValue*>>                fColorMap;
    SlotMap<ValuePair<ScalarValue*>>               fScalarMap;
    SlotMap<ValuePair<Vec2Value*>>                 fVec2Map;
    SlotMap<sk_sp<ImageAssetProxy>>                fImageMap;
    SlotMap<sk_sp<skottie::internal::TextAdapter>> fTextMap;

    const sk_sp<skottie::internal::SceneGraphRevalidator> fRevalidator;

    friend class skottie::internal::AnimationBuilder;
    friend class skottie::internal::AnimatablePropertyContainer;
};

} // namespace skottie

#endif // SlotManager_DEFINED
