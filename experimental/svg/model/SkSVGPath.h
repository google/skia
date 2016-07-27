/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGPath_DEFINED
#define SkSVGPath_DEFINED

#include "SkPath.h"
#include "SkSVGTransformableNode.h"

class SkSVGPath final : public SkSVGTransformableNode {
public:
    virtual ~SkSVGPath() = default;
    static sk_sp<SkSVGPath> Make() { return sk_sp<SkSVGPath>(new SkSVGPath()); }

    void appendChild(sk_sp<SkSVGNode>) override { }

    void setPath(const SkPath& path) { fPath = path; }

protected:
    void onRender(SkCanvas*, const SkSVGRenderContext&) const override;

    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

private:
    SkSVGPath();

    void doRender(SkCanvas*, const SkPaint*) const;

    SkPath fPath;

    typedef SkSVGTransformableNode INHERITED;
};

#endif // SkSVGPath_DEFINED
