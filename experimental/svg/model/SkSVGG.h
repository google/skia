/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGG_DEFINED
#define SkSVGG_DEFINED

#include "experimental/svg/model/SkSVGContainer.h"

class SkSVGG : public SkSVGContainer {
public:
    virtual ~SkSVGG() = default;

    static sk_sp<SkSVGG> Make() { return sk_sp<SkSVGG>(new SkSVGG()); }

private:
    SkSVGG() : INHERITED(SkSVGTag::kG) { }

    typedef SkSVGContainer INHERITED;
};

#endif // SkSVGG_DEFINED
