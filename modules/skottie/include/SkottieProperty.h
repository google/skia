/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieProperty_DEFINED
#define SkottieProperty_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"

#include <functional>

class SkMatrix;

namespace sksg {

class Color;
class OpacityEffect;

} // namespace sksg

namespace skottie {

using ColorPropertyValue   = SkColor;
using OpacityPropertyValue = float;

struct TransformPropertyValue {
    SkPoint  fAnchorPoint,
             fPosition;
    SkVector fScale;
    SkScalar fRotation,
             fSkew,
             fSkewAxis;

    bool operator==(const TransformPropertyValue& other) const;
    bool operator!=(const TransformPropertyValue& other) const;
};

namespace internal { class AnimationBuilder; }

/**
 * Property handles are adapters between user-facing AE model/values
 * and the internal scene-graph representation.
 */
template <typename ValueT, typename NodeT>
class SK_API PropertyHandle final {
public:
    ~PropertyHandle();

    ValueT get() const;
    void set(const ValueT&);

private:
    explicit PropertyHandle(sk_sp<NodeT> node) : fNode(std::move(node)) {}

    friend class skottie::internal::AnimationBuilder;

    const sk_sp<NodeT> fNode;
};

class TransformAdapter2D;

using ColorPropertyHandle     = PropertyHandle<ColorPropertyValue    , sksg::Color         >;
using OpacityPropertyHandle   = PropertyHandle<OpacityPropertyValue  , sksg::OpacityEffect >;
using TransformPropertyHandle = PropertyHandle<TransformPropertyValue, TransformAdapter2D  >;

/**
 * A PropertyObserver can be used to track and manipulate certain properties of "interesting"
 * Lottie nodes.
 *
 * When registered with an animation builder, PropertyObserver receives notifications for
 * various properties of layer and shape nodes.  The |node_name| argument corresponds to the
 * name ("nm") node property.
 */
class SK_API PropertyObserver : public SkRefCnt {
public:
    template <typename T>
    using LazyHandle = std::function<std::unique_ptr<T>()>;

    virtual void onColorProperty    (const char node_name[],
                                     const LazyHandle<ColorPropertyHandle>&);
    virtual void onOpacityProperty  (const char node_name[],
                                     const LazyHandle<OpacityPropertyHandle>&);
    virtual void onTransformProperty(const char node_name[],
                                     const LazyHandle<TransformPropertyHandle>&);
};

} // namespace skottie

#endif // SkottieProperty_DEFINED
