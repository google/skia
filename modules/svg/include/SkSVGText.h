/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGText_DEFINED
#define SkSVGText_DEFINED

#include "modules/svg/include/SkSVGContainer.h"
#include "modules/svg/include/SkSVGTypes.h"

// Base class for nestable text containers (<text>, <tspan>, etc).
class SkSVGTextContainer : public SkSVGContainer {
public:
    // TODO: these should be arrays
    SVG_ATTR(X, SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Y, SkSVGLength, SkSVGLength(0))

    SVG_ATTR(XmlSpace, SkSVGXmlSpace, SkSVGXmlSpace::kDefault)

protected:
    explicit SkSVGTextContainer(SkSVGTag t) : INHERITED(t) {}

private:
    void appendChild(sk_sp<SkSVGNode>) final;
    bool onPrepareToRender(SkSVGRenderContext*) const final;

    bool parseAndSetAttribute(const char*, const char*) override;

    using INHERITED = SkSVGContainer;
};

class SkSVGText final : public SkSVGTextContainer {
public:
    static sk_sp<SkSVGText> Make() { return sk_sp<SkSVGText>(new SkSVGText()); }

private:
    SkSVGText() : INHERITED(SkSVGTag::kText) {}

    void onRender(const SkSVGRenderContext&) const override;

    using INHERITED = SkSVGTextContainer;
};

class SkSVGTSpan final : public SkSVGTextContainer {
public:
    static sk_sp<SkSVGTSpan> Make() { return sk_sp<SkSVGTSpan>(new SkSVGTSpan()); }

private:
    SkSVGTSpan() : INHERITED(SkSVGTag::kTSpan) {}

    using INHERITED = SkSVGTextContainer;
};

class SkSVGTextLiteral final : public SkSVGNode {
public:
    static sk_sp<SkSVGTextLiteral> Make() {
        return sk_sp<SkSVGTextLiteral>(new SkSVGTextLiteral());
    }

    SVG_ATTR(Text, SkSVGStringType, SkSVGStringType())

private:
    SkSVGTextLiteral() : INHERITED(SkSVGTag::kTextLiteral) {}

    void onRender(const SkSVGRenderContext&) const override;
    SkPath onAsPath(const SkSVGRenderContext&) const override;

    void appendChild(sk_sp<SkSVGNode>) override {}

    using INHERITED = SkSVGNode;
};

#endif  // SkSVGText_DEFINED
