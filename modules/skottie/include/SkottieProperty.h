/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieProperty_DEFINED
#define SkottieProperty_DEFINED

#include "SkColor.h"
#include "SkPoint.h"
#include "SkRefCnt.h"

#include <functional>

class SkMatrix;

namespace sksg {

class Color;
class OpacityEffect;

} // namespace sksg

namespace skottie {

class ColorPropertyHandle;
class OpacityPropertyHandle;
class TransformPropertyHandle;

/**
 * A PropertyObserver can be used to track and manipulate certain properties of "interesting"
 * Lottie nodes.
 *
 * When registered with an animation builder, PropertyObserver receives notifications for
 * various properties of layer and shape nodes.  The |node_name| argument corresponds to the
 * name ("nm") node property.
 */
class PropertyObserver : public SkRefCnt {
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

class ColorPropertyHandle final {
public:
    explicit ColorPropertyHandle(sk_sp<sksg::Color>);
    ~ColorPropertyHandle();

    SkColor getColor() const;
    void setColor(SkColor);

private:
    const sk_sp<sksg::Color> fColor;
};

class OpacityPropertyHandle final {
public:
    explicit OpacityPropertyHandle(sk_sp<sksg::OpacityEffect>);
    ~OpacityPropertyHandle();

    float getOpacity() const;
    void setOpacity(float);

private:
    const sk_sp<sksg::OpacityEffect> fOpacity;
};

class TransformAdapter;

class TransformPropertyHandle final {
public:
    explicit TransformPropertyHandle(sk_sp<TransformAdapter>);
    ~TransformPropertyHandle();

    const SkPoint& getAnchorPoint() const;
    void setAnchorPoint(const SkPoint&);

    const SkPoint& getPosition() const;
    void setPosition(const SkPoint&);

    const SkVector& getScale() const;
    void setScale(const SkVector&);

    SkScalar getRotation() const;
    void setRotation(SkScalar);

    SkScalar getSkew() const;
    void setSkew(SkScalar);

    SkScalar getSkewAxis() const;
    void setSkewAxis(SkScalar);

    SkMatrix getTotalMatrix() const;

private:
    const sk_sp<TransformAdapter> fTransform;
};

} // namespace skottie

#endif // SkottieProperty_DEFINED
