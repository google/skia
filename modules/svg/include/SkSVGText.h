/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGText_DEFINED
#define SkSVGText_DEFINED

#include "include/utils/SkTextUtils.h"
#include "modules/svg/include/SkSVGTransformableNode.h"
#include "modules/svg/include/SkSVGTypes.h"

// Base class for text-related nodes: <text>, <tspan>, etc, AND text literals.
class SkSVGTextFragment : public SkSVGTransformableNode {
public:
    struct TextContext {
        SkV2   currentPos;
        size_t currentIndex;
    };

protected:
    SkSVGTextFragment(SkSVGTag t) : INHERITED(t) {}

    // Applies local styling and further dispatches via onRenderText.
    void renderText(const SkSVGRenderContext&, TextContext*) const;

    virtual void onRenderText(const SkSVGRenderContext&, TextContext*) const = 0;

private:
    friend class SkSVGTextContainer;

    using INHERITED = SkSVGTransformableNode;
};

// Actual text string payload.
class SkSVGTextLiteral final : public SkSVGTextFragment {
public:
    static sk_sp<SkSVGTextLiteral> Make() {
        return sk_sp<SkSVGTextLiteral>(new SkSVGTextLiteral());
    }

    SVG_ATTR(Text, SkSVGStringType, SkSVGStringType())

private:
    SkSVGTextLiteral() : INHERITED(SkSVGTag::kTextLiteral) {}

    void onRender(const SkSVGRenderContext&) const override;
    void onRenderText(const SkSVGRenderContext&, TextContext*) const override;

    void appendChild(sk_sp<SkSVGNode>) override {}

    SkPath onAsPath(const SkSVGRenderContext&) const override;

    using INHERITED = SkSVGTextFragment;
};

// Container for nestable text nodes - e.g. <text>, <tspan>
class SkSVGTextContainer : public SkSVGTextFragment {
public:
    // TODO: these should be arrays
    SVG_ATTR(X, SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Y, SkSVGLength, SkSVGLength(0))

protected:
    SkSVGTextContainer(SkSVGTag t) : INHERITED(t) {}

private:
    void onRender(const SkSVGRenderContext&) const final;
    void onRenderText(const SkSVGRenderContext&, TextContext*) const final;

    void appendChild(sk_sp<SkSVGNode>) final;
    SkPath onAsPath(const SkSVGRenderContext&) const final;

    bool parseAndSetAttribute(const char*, const char*) override;

    std::vector<sk_sp<SkSVGTextFragment>> fFragments;

    using INHERITED = SkSVGTextFragment;
};

// Concrete <text> node.
class SkSVGText final : public SkSVGTextContainer {
 public:
  static sk_sp<SkSVGText> Make() { return sk_sp<SkSVGText>(new SkSVGText()); }

 private:
  SkSVGText() : INHERITED(SkSVGTag::kText) {}

  using INHERITED = SkSVGTextContainer;
};

// Concrete <tspan> node.
class SkSVGTSpan final : public SkSVGTextContainer {
 public:
  static sk_sp<SkSVGTSpan> Make() { return sk_sp<SkSVGTSpan>(new SkSVGTSpan()); }

 private:
  SkSVGTSpan() : INHERITED(SkSVGTag::kTSpan) {}

  using INHERITED = SkSVGTextContainer;
};

#endif  // SkSVGText_DEFINED
