/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkBlendMode.h"
#include "src/core/SkVM.h"
#include "src/core/SkVMBlitter.h"

bool skvm::BlendModeSupported(SkBlendMode mode) {
    switch (mode) {
        default: break;
        case SkBlendMode::kSrc:
        case SkBlendMode::kSrcOver: return true;
    }
    return false;
}

skvm::Color skvm::BlendModeProgram(skvm::Builder* p, SkBlendMode mode, const skvm::Color& src,
                                   const skvm::Color& dst) {
    skvm::Color res;
    switch (mode) {
        default: SkASSERT(false);

        case SkBlendMode::kSrc: res = src; break;

        case SkBlendMode::kSrcOver: {
            auto invA = p->sub(p->splat(255), src.a);
            res.r = p->add(src.r, p->scale_unorm8(dst.r, invA));
            res.g = p->add(src.g, p->scale_unorm8(dst.g, invA));
            res.b = p->add(src.b, p->scale_unorm8(dst.b, invA));
            res.a = p->add(src.a, p->scale_unorm8(dst.a, invA));
        } break;
    }
    return res;
}

