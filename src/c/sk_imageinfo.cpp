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

const struct {
    sk_colortype_t  fC;
    SkColorType     fSK;
} gColorTypeMap[] = {
    { UNKNOWN_SK_COLORTYPE,     kUnknown_SkColorType    },
    { RGBA_8888_SK_COLORTYPE,   kRGBA_8888_SkColorType  },
    { BGRA_8888_SK_COLORTYPE,   kBGRA_8888_SkColorType  },
    { ALPHA_8_SK_COLORTYPE,     kAlpha_8_SkColorType    },
    { GRAY_8_SK_COLORTYPE,      kGray_8_SkColorType     },
    { RGBA_F16_SK_COLORTYPE,    kRGBA_F16_SkColorType   },
    { RGBA_F32_SK_COLORTYPE,    kRGBA_F32_SkColorType   },
};

const struct {
    sk_alphatype_t  fC;
    SkAlphaType     fSK;
} gAlphaTypeMap[] = {
    { OPAQUE_SK_ALPHATYPE,      kOpaque_SkAlphaType     },
    { PREMUL_SK_ALPHATYPE,      kPremul_SkAlphaType     },
    { UNPREMUL_SK_ALPHATYPE,    kUnpremul_SkAlphaType   },
};

static bool from_c_colortype(sk_colortype_t cCT, SkColorType* skCT) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gColorTypeMap); ++i) {
        if (gColorTypeMap[i].fC == cCT) {
            if (skCT) {
                *skCT = gColorTypeMap[i].fSK;
            }
            return true;
        }
    }
    return false;
}

static bool to_c_colortype(SkColorType skCT, sk_colortype_t* cCT) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gColorTypeMap); ++i) {
        if (gColorTypeMap[i].fSK == skCT) {
            if (cCT) {
                *cCT = gColorTypeMap[i].fC;
            }
            return true;
        }
    }
    return false;
}

static bool from_c_alphatype(sk_alphatype_t cAT, SkAlphaType* skAT) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gAlphaTypeMap); ++i) {
        if (gAlphaTypeMap[i].fC == cAT) {
            if (skAT) {
                *skAT = gAlphaTypeMap[i].fSK;
            }
            return true;
        }
    }
    return false;
}

static bool to_c_alphatype(SkAlphaType skAT, sk_alphatype_t* cAT) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gAlphaTypeMap); ++i) {
        if (gAlphaTypeMap[i].fSK == skAT) {
            if (cAT) {
                *cAT = gAlphaTypeMap[i].fC;
            }
            return true;
        }
    }
    return false;
}

const SkImageInfo* ToImageInfo(const sk_imageinfo_t* cinfo) {
    return reinterpret_cast<const SkImageInfo*>(cinfo);
}

/////////////////////////////////////////////////////////////////////////////////////////////

sk_imageinfo_t* sk_imageinfo_new(int w, int h, sk_colortype_t cct, sk_alphatype_t cat,
                                 sk_colorspace_t* ccs) {
    SkColorType ct;
    SkAlphaType at;
    if (!from_c_colortype(cct, &ct) || !from_c_alphatype(cat, &at)) {
        return nullptr;
    }
    SkColorSpace* cs = (SkColorSpace*)ccs;

    SkImageInfo* info = new SkImageInfo(SkImageInfo::Make(w, h, ct, at, sk_ref_sp(cs)));
    return reinterpret_cast<sk_imageinfo_t*>(info);
}

void sk_imageinfo_delete(sk_imageinfo_t* cinfo) {
    delete ToImageInfo(cinfo);
}

int sk_imageinfo_get_width(const sk_imageinfo_t* cinfo) {
    return ToImageInfo(cinfo)->width();
}

int sk_imageinfo_get_height(const sk_imageinfo_t* cinfo) {
    return ToImageInfo(cinfo)->height();
}

sk_colortype_t sk_imageinfo_get_colortype(const sk_imageinfo_t* cinfo) {
    sk_colortype_t ct;
    return to_c_colortype(ToImageInfo(cinfo)->colorType(), &ct) ? ct : UNKNOWN_SK_COLORTYPE;
}

sk_alphatype_t sk_imageinfo_get_alphatype(const sk_imageinfo_t* cinfo) {
    sk_alphatype_t at;
    // odd that we return premul on failure...
    return to_c_alphatype(ToImageInfo(cinfo)->alphaType(), &at) ? at : PREMUL_SK_ALPHATYPE;
}

sk_colorspace_t* sk_imageinfo_get_colorspace(const sk_imageinfo_t* cinfo) {
    return reinterpret_cast<sk_colorspace_t*>(ToImageInfo(cinfo)->colorSpace());
}

/////////////////////////////////////////////////////////////////////////////////////////////

sk_colorspace_t* sk_colorspace_new_srgb() {
    return reinterpret_cast<sk_colorspace_t*>(SkColorSpace::MakeSRGB().release());
}

void sk_colorspace_ref(sk_colorspace_t* cs) {
    SkSafeRef(reinterpret_cast<SkColorSpace*>(cs));
}

void sk_colorspace_unref(sk_colorspace_t* cs) {
    SkSafeUnref(reinterpret_cast<SkColorSpace*>(cs));
}

