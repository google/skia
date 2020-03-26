/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGText_DEFINED
#define SkSVGText_DEFINED

#include "experimental/svg/model/SkSVGShape.h"
#include "experimental/svg/model/SkSVGTypes.h"
#include "include/core/SkFont.h"
#include "include/utils/SkTextUtils.h"

class SkRRect;

class SkSVGText final : public SkSVGShape {
 public:
  ~SkSVGText() override = default;
  static sk_sp<SkSVGText> Make() {
    return sk_sp<SkSVGText>(new SkSVGText()); }

  void setX(const SkSVGLength&);
  void setY(const SkSVGLength&);
  void setFontFamily(const SkSVGStringType&);
  void setFontSize(const SkSVGLength&);
  void setText(const SkSVGStringType&);
  void setTextAnchor(const SkSVGStringType&);

 protected:
  void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

  void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
              SkPathFillType) const override;

  SkPath onAsPath(const SkSVGRenderContext&) const override;

 private:
  SkSVGText();
  SkSVGLength fX = SkSVGLength(0);
  SkSVGLength fY = SkSVGLength(0);
  SkSVGStringType fText;
  sk_sp<SkTypeface> fTypeface;
  SkSVGLength fFontSize;
  SkTextUtils::Align fTextAlign = SkTextUtils::Align::kLeft_Align;
  typedef SkSVGShape INHERITED;
};

#endif  // SkSVGText_DEFINED
