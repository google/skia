/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGText_DEFINED
#define SkSVGText_DEFINED

#include <vector>

#include "modules/svg/include/SkSVGTransformableNode.h"
#include "modules/svg/include/SkSVGTypes.h"

class SkSVGTextContext;

// Base class for text-rendering nodes.
class SkSVGTextFragment : public SkSVGTransformableNode {
public:
    void renderText(const SkSVGRenderContext&, SkSVGTextContext*, SkSVGXmlSpace) const;

protected:
    explicit SkSVGTextFragment(SkSVGTag t) : INHERITED(t) {}

    virtual void onRenderText(const SkSVGRenderContext&, SkSVGTextContext*,
                              SkSVGXmlSpace) const = 0;

private:
    SkPath onAsPath(const SkSVGRenderContext&) const final;

    using INHERITED = SkSVGTransformableNode;
};

// Base class for nestable text containers (<text>, <tspan>, etc).
class SkSVGTextContainer : public SkSVGTextFragment {
public:
    SVG_ATTR(X, std::vector<SkSVGLength>, {})
    SVG_ATTR(Y, std::vector<SkSVGLength>, {})

    SVG_ATTR(XmlSpace, SkSVGXmlSpace, SkSVGXmlSpace::kDefault)

    void appendChild(sk_sp<SkSVGNode>) final;

protected:
    explicit SkSVGTextContainer(SkSVGTag t) : INHERITED(t) {}

private:
    void onRender(const SkSVGRenderContext&) const final;
    void onRenderText(const SkSVGRenderContext&, SkSVGTextContext*, SkSVGXmlSpace) const final;

    bool parseAndSetAttribute(const char*, const char*) override;

    std::vector<sk_sp<SkSVGTextFragment>> fChildren;

    using INHERITED = SkSVGTextFragment;
};

class SkSVGText final : public SkSVGTextContainer {
public:
    static sk_sp<SkSVGText> Make() { return sk_sp<SkSVGText>(new SkSVGText()); }

private:
    SkSVGText() : INHERITED(SkSVGTag::kText) {}

    using INHERITED = SkSVGTextContainer;
};

class SkSVGTSpan final : public SkSVGTextContainer {
public:
    static sk_sp<SkSVGTSpan> Make() { return sk_sp<SkSVGTSpan>(new SkSVGTSpan()); }

private:
    SkSVGTSpan() : INHERITED(SkSVGTag::kTSpan) {}

    using INHERITED = SkSVGTextContainer;
};

class SkSVGTextLiteral final : public SkSVGTextFragment {
public:
    static sk_sp<SkSVGTextLiteral> Make() {
        return sk_sp<SkSVGTextLiteral>(new SkSVGTextLiteral());
    }

    SVG_ATTR(Text, SkSVGStringType, SkSVGStringType())

private:
    SkSVGTextLiteral() : INHERITED(SkSVGTag::kTextLiteral) {}

    void onRender(const SkSVGRenderContext&) const override {}
    void onRenderText(const SkSVGRenderContext&, SkSVGTextContext*, SkSVGXmlSpace) const override;

    void appendChild(sk_sp<SkSVGNode>) override {}

    using INHERITED = SkSVGTextFragment;
};

#endif  // SkSVGText_DEFINED
