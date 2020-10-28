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

class SkRRect;

class SkSVGText final : public SkSVGTransformableNode {
 public:
  ~SkSVGText() override = default;
  static sk_sp<SkSVGText> Make() {
    return sk_sp<SkSVGText>(new SkSVGText()); }

  SVG_ATTR(X   , SkSVGLength    , SkSVGLength(0))
  SVG_ATTR(Y   , SkSVGLength    , SkSVGLength(0))
  SVG_ATTR(Text, SkSVGStringType, SkSVGStringType())

 protected:
  void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

  void onRender(const SkSVGRenderContext&) const override;
  void appendChild(sk_sp<SkSVGNode>) override;

  SkPath onAsPath(const SkSVGRenderContext&) const override;

  void loadFont();

 private:
  SkSVGText();

  SkFont resolveFont(const SkSVGRenderContext&) const;

  using INHERITED = SkSVGTransformableNode;
};

#endif  // SkSVGText_DEFINED
