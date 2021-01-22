/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGNode_DEFINED
#define SkSVGNode_DEFINED

#include "include/core/SkRefCnt.h"
#include "modules/svg/include/SkSVGAttribute.h"
#include "modules/svg/include/SkSVGAttributeParser.h"

class SkCanvas;
class SkMatrix;
class SkPaint;
class SkPath;
class SkSVGLengthContext;
class SkSVGRenderContext;
class SkSVGValue;

enum class SkSVGTag {
    kCircle,
    kClipPath,
    kDefs,
    kEllipse,
    kFeColorMatrix,
    kFeComposite,
    kFeTurbulence,
    kFilter,
    kG,
    kLine,
    kLinearGradient,
    kPath,
    kPattern,
    kPolygon,
    kPolyline,
    kRadialGradient,
    kRect,
    kStop,
    kSvg,
    kText,
    kUse
};

#define SVG_PRES_ATTR(attr_name, attr_type, attr_inherited)                  \
private:                                                                     \
    bool set##attr_name(SkSVGAttributeParser::ParseResult<attr_type>&& pr) { \
        if (pr.isValid()) { this->set##attr_name(std::move(*pr)); }          \
        return pr.isValid();                                                 \
    }                                                                        \
public:                                                                      \
    const attr_type* get##attr_name() const {                                \
        return fPresentationAttributes.f##attr_name.getMaybeNull();          \
    }                                                                        \
    void set##attr_name(const attr_type& v) {                                \
        if (!attr_inherited || v.type() != attr_type::Type::kInherit) {      \
            fPresentationAttributes.f##attr_name.set(v);                     \
        } else {                                                             \
            /* kInherited values are semantically equivalent to              \
               the absence of a local presentation attribute.*/              \
            fPresentationAttributes.f##attr_name.reset();                    \
        }                                                                    \
    }                                                                        \
    void set##attr_name(attr_type&& v) {                                     \
        if (!attr_inherited || v.type() != attr_type::Type::kInherit) {      \
            fPresentationAttributes.f##attr_name.set(std::move(v));          \
        } else {                                                             \
            /* kInherited values are semantically equivalent to              \
               the absence of a local presentation attribute.*/              \
            fPresentationAttributes.f##attr_name.reset();                    \
        }                                                                    \
    }

class SkSVGNode : public SkRefCnt {
public:
    ~SkSVGNode() override;

    SkSVGTag tag() const { return fTag; }

    virtual void appendChild(sk_sp<SkSVGNode>) = 0;

    void render(const SkSVGRenderContext&) const;
    bool asPaint(const SkSVGRenderContext&, SkPaint*) const;
    SkPath asPath(const SkSVGRenderContext&) const;
    SkRect objectBoundingBox(const SkSVGRenderContext&) const;

    void setAttribute(SkSVGAttribute, const SkSVGValue&);
    bool setAttribute(const char* attributeName, const char* attributeValue);

    // TODO: consolidate with existing setAttribute
    virtual bool parseAndSetAttribute(const char* name, const char* value);

    void setColor(const SkSVGColorType&);
    void setFillOpacity(const SkSVGNumberType&);
    void setOpacity(const SkSVGNumberType&);
    void setStrokeDashOffset(const SkSVGLength&);
    void setStrokeOpacity(const SkSVGNumberType&);
    void setStrokeMiterLimit(const SkSVGNumberType&);
    void setStrokeWidth(const SkSVGLength&);

    // inherited
    SVG_PRES_ATTR(ClipRule       , SkSVGFillRule  , true)
    SVG_PRES_ATTR(FillRule       , SkSVGFillRule  , true)
    SVG_PRES_ATTR(Fill           , SkSVGPaint     , true)
    SVG_PRES_ATTR(FontFamily     , SkSVGFontFamily, true)
    SVG_PRES_ATTR(FontSize       , SkSVGFontSize  , true)
    SVG_PRES_ATTR(FontStyle      , SkSVGFontStyle , true)
    SVG_PRES_ATTR(FontWeight     , SkSVGFontWeight, true)
    SVG_PRES_ATTR(Stroke         , SkSVGPaint     , true)
    SVG_PRES_ATTR(StrokeDashArray, SkSVGDashArray , true)
    SVG_PRES_ATTR(StrokeLineCap  , SkSVGLineCap   , true)
    SVG_PRES_ATTR(StrokeLineJoin , SkSVGLineJoin  , true)
    SVG_PRES_ATTR(TextAnchor     , SkSVGTextAnchor, true)
    SVG_PRES_ATTR(Visibility     , SkSVGVisibility, true)

    // not inherited
    SVG_PRES_ATTR(ClipPath       , SkSVGClip      , false)
    SVG_PRES_ATTR(Filter         , SkSVGFilterType, false)

protected:
    SkSVGNode(SkSVGTag);

    // Called before onRender(), to apply local attributes to the context.  Unlike onRender(),
    // onPrepareToRender() bubbles up the inheritance chain: overriders should always call
    // INHERITED::onPrepareToRender(), unless they intend to short-circuit rendering
    // (return false).
    // Implementations are expected to return true if rendering is to continue, or false if
    // the node/subtree rendering is disabled.
    virtual bool onPrepareToRender(SkSVGRenderContext*) const;

    virtual void onRender(const SkSVGRenderContext&) const = 0;

    virtual bool onAsPaint(const SkSVGRenderContext&, SkPaint*) const { return false; }

    virtual SkPath onAsPath(const SkSVGRenderContext&) const = 0;

    virtual void onSetAttribute(SkSVGAttribute, const SkSVGValue&);

    virtual bool hasChildren() const { return false; }

    virtual SkRect onObjectBoundingBox(const SkSVGRenderContext&) const {
        return SkRect::MakeEmpty();
    }

private:
    SkSVGTag                    fTag;

    // FIXME: this should be sparse
    SkSVGPresentationAttributes fPresentationAttributes;

    using INHERITED = SkRefCnt;
};

#undef SVG_PRES_ATTR // presentation attributes are only defined for the base class

#define _SVG_ATTR_SETTERS(attr_name, attr_type, attr_default, set_cp, set_mv) \
    private:                                                                  \
        bool set##attr_name(                                                  \
                const SkSVGAttributeParser::ParseResult<attr_type>& pr) {     \
            if (pr.isValid()) { this->set##attr_name(*pr); }                  \
            return pr.isValid();                                              \
        }                                                                     \
        bool set##attr_name(                                                  \
                SkSVGAttributeParser::ParseResult<attr_type>&& pr) {          \
            if (pr.isValid()) { this->set##attr_name(std::move(*pr)); }       \
            return pr.isValid();                                              \
        }                                                                     \
    public:                                                                   \
        void set##attr_name(const attr_type& a) { set_cp(a); }                \
        void set##attr_name(attr_type&& a) { set_mv(std::move(a)); }

#define SVG_ATTR(attr_name, attr_type, attr_default)                        \
    private:                                                                \
        attr_type f##attr_name = attr_default;                              \
    public:                                                                 \
        const attr_type& get##attr_name() const { return f##attr_name; }    \
    _SVG_ATTR_SETTERS(                                                      \
            attr_name, attr_type, attr_default,                             \
            [this](const attr_type& a) { this->f##attr_name = a; },         \
            [this](attr_type&& a) { this->f##attr_name = std::move(a); })

#define SVG_OPTIONAL_ATTR(attr_name, attr_type)                                   \
    private:                                                                      \
        SkTLazy<attr_type> f##attr_name;                                          \
    public:                                                                       \
        const SkTLazy<attr_type>& get##attr_name() const { return f##attr_name; } \
    _SVG_ATTR_SETTERS(                                                            \
            attr_name, attr_type, attr_default,                                   \
            [this](const attr_type& a) { this->f##attr_name.set(a); },            \
            [this](attr_type&& a) { this->f##attr_name.set(std::move(a)); })

#endif // SkSVGNode_DEFINED
