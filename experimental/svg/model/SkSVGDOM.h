/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGDOM_DEFINED
#define SkSVGDOM_DEFINED

#include "SkRefCnt.h"
#include "SkSize.h"
#include "SkSVGIDMapper.h"
#include "SkTemplates.h"

class SkCanvas;
class SkDOM;
class SkStream;
class SkSVGNode;

class SkSVGDOM : public SkRefCnt {
public:
    SkSVGDOM();
    ~SkSVGDOM() = default;

    static sk_sp<SkSVGDOM> MakeFromDOM(const SkDOM&);
    static sk_sp<SkSVGDOM> MakeFromStream(SkStream&);

    const SkSize& containerSize() const;
    void setContainerSize(const SkSize&);

    void setRoot(sk_sp<SkSVGNode>);

    void render(SkCanvas*) const;

private:
    SkSize intrinsicSize() const;

    SkSize           fContainerSize;
    sk_sp<SkSVGNode> fRoot;
    SkSVGIDMapper    fIDMapper;

    typedef SkRefCnt INHERITED;
};

#endif // SkSVGDOM_DEFINED
