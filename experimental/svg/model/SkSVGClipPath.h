/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGClipPath_DEFINED
#define SkSVGClipPath_DEFINED

#include "experimental/svg/model/SkSVGHiddenContainer.h"
#include "experimental/svg/model/SkSVGTypes.h"

class SkSVGClipPath final : public SkSVGHiddenContainer {
public:
    virtual ~SkSVGClipPath() = default;
    static sk_sp<SkSVGClipPath> Make() {
        return sk_sp<SkSVGClipPath>(new SkSVGClipPath());
    }

protected:

private:
    SkSVGClipPath();

    typedef SkSVGHiddenContainer INHERITED;
};

#endif // SkSVGClipPath_DEFINED
