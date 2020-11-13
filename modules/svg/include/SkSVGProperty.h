/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGProperty_DEFINED
#define SkSVGProperty_DEFINED

#include "modules/svg/include/SkSVGTypes.h"
#include "src/core/SkTLazy.h"

// https://www.w3.org/TR/SVG11/intro.html#TermProperty
template <typename T> class SkSVGProperty {
public:
    SkSVGProperty() : fType(Type::kUnknown), fValue(nullptr) {}

    explicit SkSVGProperty(bool inheritable)
            : fType(inheritable ? Type::kInheritable : Type::kUninheritable), fValue(nullptr) {}

    template <typename... Args> static SkSVGProperty Make(Args&&... args) {
        SkSVGProperty result;
        result.fValue.init(std::forward<Args>(args)...);
        return result;
    }

    bool isInherit() const { return !fValue.isValid(); }
    bool isInheritable() const {
        SkASSERT(fType != Type::kUnknown);
        return fType == Type::kInheritable;
    }

    void setInherit() { fValue.reset(); }
    void set(const T& value) { fValue.set(value); }
    void set(T&& value) { fValue.set(std::move(value)); }

    T* get() const {
        SkASSERT(!this->isInherit());
        return fValue.getMaybeNull();
    }
    T* operator->() const { return this->get(); }
    T& operator*() const { return *this->get(); }

private:
    enum class Type {
        kUnknown,
        kInheritable,
        kUninheritable,
    };

    Type fType;
    SkTLazy<T> fValue;
};

template <typename T> struct SkSVGProperty_ : std::false_type {};
template <typename T> struct SkSVGProperty_<SkSVGProperty<T>> : std::true_type {};
template <typename T> inline constexpr bool SkSVGIsProperty = SkSVGProperty_<T>::value;

template <typename T> struct SkSVGPropertyInnerType_ { using type = T; };
template <typename T> struct SkSVGPropertyInnerType_<SkSVGProperty<T>> { using type = T; };
template <typename T> using SkSVGPropertyInnerType = typename SkSVGPropertyInnerType_<T>::type;

// https://www.w3.org/TR/SVG11/intro.html#TermPresentationAttribute
struct SkSVGPresentationAttributes {
    static SkSVGPresentationAttributes MakeInitial();

    // TODO: SkTLazy adds an extra ptr per attribute; refactor to reduce overhead.

    SkTLazy<SkSVGPaint>      fFill;
    SkTLazy<SkSVGNumberType> fFillOpacity;
    SkTLazy<SkSVGFillRule>   fFillRule;
    SkTLazy<SkSVGFillRule>   fClipRule;

    SkTLazy<SkSVGPaint>      fStroke;
    SkTLazy<SkSVGDashArray>  fStrokeDashArray;
    SkTLazy<SkSVGLength>     fStrokeDashOffset;
    SkTLazy<SkSVGLineCap>    fStrokeLineCap;
    SkTLazy<SkSVGLineJoin>   fStrokeLineJoin;
    SkTLazy<SkSVGNumberType> fStrokeMiterLimit;
    SkTLazy<SkSVGNumberType> fStrokeOpacity;
    SkTLazy<SkSVGLength>     fStrokeWidth;

    SkSVGProperty<SkSVGVisibility> fVisibility{true};

    SkTLazy<SkSVGColorType>  fColor;

    SkTLazy<SkSVGFontFamily> fFontFamily;
    SkTLazy<SkSVGFontStyle>  fFontStyle;
    SkTLazy<SkSVGFontSize>   fFontSize;
    SkTLazy<SkSVGFontWeight> fFontWeight;
    SkTLazy<SkSVGTextAnchor> fTextAnchor;

    // TODO(tdenniston): add SkSVGStopColor

    // uninherited
    SkTLazy<SkSVGNumberType> fOpacity;
    SkTLazy<SkSVGClip>       fClipPath;
    SkTLazy<SkSVGFilterType> fFilter;
};

class SkSVGPropertyContext {
public:
    SkSVGPropertyContext() : fProps(kDefaults) {}

    SkSVGPresentationAttributes* writableProps() { return fProps.writable(); }

    const SkSVGPresentationAttributes& props() const { return *fProps; }

    static const SkSVGPresentationAttributes& Defaults() { return kDefaults; }

private:
    static inline const SkSVGPresentationAttributes kDefaults =
            SkSVGPresentationAttributes::MakeInitial();

    SkTCopyOnFirstWrite<SkSVGPresentationAttributes> fProps;
};

#endif  // SkSVGProperty_DEFINED
