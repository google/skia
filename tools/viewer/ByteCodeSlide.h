/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ByteCodeSlide_DEFINED
#define ByteCodeSlide_DEFINED

#include "tools/viewer/Slide.h"

#include "src/sksl/SkSLByteCode.h"

#include <map>
#include <vector>

class ByteCodeSlide : public Slide {
public:
    ByteCodeSlide();

    SkISize getDimensions() const override { return SkISize::MakeEmpty(); }
    void draw(SkCanvas* canvas) override;

private:
    SkString fCode;

    std::unique_ptr<SkSL::ByteCode> fByteCode = nullptr;
    const SkSL::ByteCodeFunction* fMain = nullptr;

    std::vector<SkSL::String> fLines;
    std::map<int, int> fLookup;

    int fIP = -1;
    int fCount = 0;
};

#endif
