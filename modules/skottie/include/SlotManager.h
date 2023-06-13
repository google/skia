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
#include "src/core/SkTHash.h"

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

namespace internal {
class AnimationBuilder;
class SceneGraphRevalidator;
} // namespace internal

using namespace skia_private;

class SK_API SlotManager final : public SkRefCnt {

public:
    using SlotID = SkString;

    SlotManager(sk_sp<skottie::internal::SceneGraphRevalidator>);
    ~SlotManager() override;

    void setColorSlot(SlotID, SkColor);
    void setImageSlot(SlotID, sk_sp<skresources::ImageAsset>);
    void setScalarSlot(SlotID, SkScalar);
    //TODO: surface Text value options

    SkColor getColorSlot(SlotID) const;
    sk_sp<const skresources::ImageAsset> getImageSlot(SlotID) const;
    SkScalar getScalarSlot(SlotID) const;

    struct SlotInfo {
        SlotID slotID;
        int type;
    };

    // Helper function to get all slot IDs and their value types
    const TArray<SlotInfo>& getSlotInfo() const { return fSlotInfos; }

private:

    // pass value to the SlotManager for manipulation and node for invalidation
    void trackColorValue(SlotID, SkColor*, sk_sp<sksg::Node>);
    sk_sp<skresources::ImageAsset> trackImageValue(SlotID, sk_sp<skresources::ImageAsset>,
                                                   sk_sp<sksg::Node>);
    void trackScalarValue(SlotID, SkScalar*, sk_sp<sksg::Node>);

    TArray<SlotInfo> fSlotInfos;

    template <typename T>
    struct ValuePair
    {
        T value;
        sk_sp<sksg::Node> node;
    };

    class ImageAssetProxy;
    template <typename T>
    using SlotMap = THashMap<SlotID, TArray<ValuePair<T>>>;

    SlotMap<SkColor*>               fColorMap;
    SlotMap<SkScalar*>              fScalarMap;
    SlotMap<sk_sp<ImageAssetProxy>> fImageMap;

    const sk_sp<skottie::internal::SceneGraphRevalidator> fRevalidator;

    friend class skottie::internal::AnimationBuilder;
};

} // namespace skottie

#endif // SlotManager_DEFINED
