/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGSVG_DEFINED
#define SkSVGSVG_DEFINED

#include "SkSVGContainer.h"

class SkSVGSVG : public SkSVGContainer {
public:
    virtual ~SkSVGSVG() = default;

    static sk_sp<SkSVGSVG> Make() { return sk_sp<SkSVGSVG>(new SkSVGSVG()); }

private:
    SkSVGSVG();

    typedef SkSVGContainer INHERITED;
};

#endif // SkSVGSVG_DEFINED
