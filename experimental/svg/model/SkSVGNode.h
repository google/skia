/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGNode_DEFINED
#define SkSVGNode_DEFINED

#include "SkRefCnt.h"
#include "SkSVGAttribute.h"

class SkCanvas;
class SkMatrix;
class SkSVGRenderContext;
class SkSVGValue;

enum class SkSVGTag {
    g,
    path,
    svg
};

class SkSVGNode : public SkRefCnt {
public:
    virtual ~SkSVGNode();

    SkSVGTag tag() const { return fTag; }

    virtual void appendChild(sk_sp<SkSVGNode>) = 0;

    void render(SkCanvas*) const;
    void render(SkCanvas*, const SkSVGRenderContext&) const;

    void setAttribute(SkSVGAttribute, const SkSVGValue&);

protected:
    SkSVGNode(SkSVGTag);

    virtual void onRender(SkCanvas*, const SkSVGRenderContext&) const = 0;

    virtual void onSetAttribute(SkSVGAttribute, const SkSVGValue&);

    virtual const SkMatrix& onLocalMatrix() const;

private:
    SkSVGTag                    fTag;

    // FIXME: this should be sparse
    SkSVGPresentationAttributes fPresentationAttributes;

    typedef SkRefCnt INHERITED;
};

#endif // SkSVGNode_DEFINED
