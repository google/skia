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

class SkMatrix;

namespace sksg {

class Color;
class OpacityEffect;

} // namespace sksg

namespace skottie {

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

/**
 * A PropertyObserver can be used to track and manipulate certain properties of "interesting"
 * Lottie nodes.
 *
 * When registered with an animation builder, its |accept| method will be invoked for every
 * layer and shape node (with an argument corresponding to the node name "nm" property).  The
 * returned property mask controls which property handles associated with the node get to be
 * dispatched to the observer.
 */
class PropertyObserver : public SkRefCnt {
public:
    enum PropertyType : uint32_t {
        kNone_PropertyType      = 0x00,
        kColor_PropertyType     = 0x01,
        kOpacity_PropertyType   = 0x02,
        kTransform_PropertyType = 0x04,
    };
    using PropertyMask = uint32_t;

    virtual PropertyMask accept(const char node_name[]) = 0;

    virtual void onColorProperty(const char node_name[],
                                 std::unique_ptr<ColorPropertyHandle>);
    virtual void onOpacityProperty(const char node_name[],
                                   std::unique_ptr<OpacityPropertyHandle>);
    virtual void onTransformProperty(const char node_name[],
                                     std::unique_ptr<TransformPropertyHandle>);

private:
    using INHERITED = SkRefCnt;
};

} // namespace skottie

#endif // SkottieProperty_DEFINED
