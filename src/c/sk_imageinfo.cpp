/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"

#include "include/c/sk_colorspace.h"
#include "include/c/sk_imageinfo.h"

sk_imageinfo_t* sk_imageinfo_new(int w, int h, sk_colortype_t cct, sk_alphatype_t cat, sk_colorspace_t* ccs) {
    SkColorType ct = (SkColorType)cct;
    SkAlphaType at = (SkAlphaType)cat;
    SkColorSpace* cs = ToColorSpace(ccs);

    SkImageInfo* info = new SkImageInfo(SkImageInfo::Make(w, h, ct, at, sk_ref_sp(cs)));
    return AsImageInfo(info);
}

void sk_imageinfo_delete(sk_imageinfo_t* cinfo) {
    delete ToImageInfo(cinfo);
}

int32_t sk_imageinfo_get_width(const sk_imageinfo_t* cinfo) {
    return ToImageInfo(cinfo)->width();
}

int32_t sk_imageinfo_get_height(const sk_imageinfo_t* cinfo) {
    return ToImageInfo(cinfo)->height();
}

sk_colortype_t sk_imageinfo_get_colortype(const sk_imageinfo_t* cinfo) {
    return (sk_colortype_t)ToImageInfo(cinfo)->colorType();
}

sk_alphatype_t sk_imageinfo_get_alphatype(const sk_imageinfo_t* cinfo) {
    return (sk_alphatype_t)ToImageInfo(cinfo)->alphaType();
}

sk_colorspace_t* sk_imageinfo_get_colorspace(const sk_imageinfo_t* cinfo) {
    return AsColorSpace(ToImageInfo(cinfo)->refColorSpace());
}
