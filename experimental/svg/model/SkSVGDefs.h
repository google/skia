/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGDefs_DEFINED
#define SkSVGDefs_DEFINED

#include "experimental/svg/model/SkSVGHiddenContainer.h"

class SkSVGDefs : public SkSVGHiddenContainer {
public:
    virtual ~SkSVGDefs() = default;
    static sk_sp<SkSVGDefs> Make() { return sk_sp<SkSVGDefs>(new SkSVGDefs()); }

private:
    SkSVGDefs() : INHERITED(SkSVGTag::kDefs) {}

    typedef SkSVGHiddenContainer INHERITED;
};

#endif // SkSVGDefs_DEFINED
