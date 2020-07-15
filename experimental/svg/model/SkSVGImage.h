/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGImage_DEFINED
#define SkSVGImage_DEFINED

#include "experimental/svg/model/SkSVGTransformableNode.h"
#include "experimental/svg/model/SkSVGTypes.h"
#include "include/core/SkImage.h"

class SkSVGImage final : public SkSVGTransformableNode {
 public:
  ~SkSVGImage() override = default;

  static sk_sp<SkSVGImage> Make() {
    return sk_sp<SkSVGImage>(new SkSVGImage());
  }

  void appendChild(sk_sp<SkSVGNode>) override;

  void setHref(const SkSVGStringType&);
  void setX(const SkSVGLength&);
  void setY(const SkSVGLength&);
  void setWidth(const SkSVGLength&);
  void setHeight(const SkSVGLength&);

 protected:
  void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;
  void onRender(const SkSVGRenderContext&) const final;
  SkPath onAsPath(const SkSVGRenderContext&) const override;

 private:
  SkSVGImage();

  SkRect resolve(const SkSVGLengthContext&) const;

  SkSVGLength fX = SkSVGLength(0);
  SkSVGLength fY = SkSVGLength(0);
  SkSVGLength fWidth = SkSVGLength(0);
  SkSVGLength fHeight = SkSVGLength(0);
  sk_sp<SkImage> fImage;

  typedef SkSVGTransformableNode INHERITED;
};

#endif  // SkSVGImage_DEFINED
