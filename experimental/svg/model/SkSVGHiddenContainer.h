/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGHiddenContainer_DEFINED
#define SkSVGHiddenContainer_DEFINED

#include "SkSVGContainer.h"

class SkSVGHiddenContainer : public SkSVGContainer {
public:
    virtual ~SkSVGHiddenContainer() = default;

protected:
    explicit SkSVGHiddenContainer(SkSVGTag t) : INHERITED(t) {}

    void onRender(const SkSVGRenderContext&) const final {}

private:
    typedef SkSVGContainer INHERITED;
};

#endif // SkSVGHiddenContainer_DEFINED
