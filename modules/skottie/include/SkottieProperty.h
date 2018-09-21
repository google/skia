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

class ColorPropertyProxy final {
public:
    explicit ColorPropertyProxy(sk_sp<sksg::Color>);
    ~ColorPropertyProxy();

    SkColor getColor() const;
    void setColor(SkColor);

private:
    const sk_sp<sksg::Color> fColor;
};

class OpacityPropertyProxy final {
public:
    explicit OpacityPropertyProxy(sk_sp<sksg::OpacityEffect>);
    ~OpacityPropertyProxy();

    float getOpacity() const;
    void setOpacity(float);

private:
    const sk_sp<sksg::OpacityEffect> fOpacity;
};

class TransformAdapter;

class TransformPropertyProxy final {
public:
    explicit TransformPropertyProxy(sk_sp<TransformAdapter>);
    ~TransformPropertyProxy();

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
                                 std::unique_ptr<ColorPropertyProxy>);
    virtual void onOpacityProperty(const char node_name[],
                                   std::unique_ptr<OpacityPropertyProxy>);
    virtual void onTransformProperty(const char node_name[],
                                     std::unique_ptr<TransformPropertyProxy>);

private:
    using INHERITED = SkRefCnt;
};


} // namespace skottie

#endif // SkottieProperty_DEFINED
