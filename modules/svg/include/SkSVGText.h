/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGText_DEFINED
#define SkSVGText_DEFINED

#include "include/core/SkFont.h"
#include "include/utils/SkTextUtils.h"
#include "modules/svg/include/SkSVGTransformableNode.h"
#include "modules/svg/include/SkSVGTypes.h"

class SkRRect;

class SkSVGText final : public SkSVGTransformableNode {
 public:
  ~SkSVGText() override = default;
  static sk_sp<SkSVGText> Make() {
    return sk_sp<SkSVGText>(new SkSVGText()); }

  void setX(const SkSVGLength&);
  void setY(const SkSVGLength&);
  void setText(const SkSVGStringType&);
  void setTextAnchor(const SkSVGStringType&);

 protected:
  void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

  void onRender(const SkSVGRenderContext&) const override;
  void appendChild(sk_sp<SkSVGNode>) override;

  SkPath onAsPath(const SkSVGRenderContext&) const override;

  void loadFont();

 private:
  SkSVGText();

  SkFont resolveFont(const SkSVGRenderContext&) const;

  SkSVGLength        fX = SkSVGLength(0);
  SkSVGLength        fY = SkSVGLength(0);
  SkSVGStringType    fText;
  sk_sp<SkTypeface>  fTypeface;
  SkTextUtils::Align fTextAlign = SkTextUtils::Align::kLeft_Align;

  using INHERITED = SkSVGTransformableNode;
};

#endif  // SkSVGText_DEFINED
