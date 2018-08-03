// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "../gm/gm.h"
#include "SkTHash.h"
#include "ValidateGMNames.h"

static const char* kGrandfatherClauseNames[] = {
    "3dgm",
    "3x3bitmaprect",
    "animated-image-blurs",
    "bitmap-image-srgb-legacy",
    "blurroundrect-WH-100x100-unevenCorners",
    "circular-clips",
    "clipped-bitmap-shaders-clamp",
    "clipped-bitmap-shaders-clamp-hq",
    "clipped-bitmap-shaders-mirror",
    "clipped-bitmap-shaders-mirror-hq",
    "clipped-bitmap-shaders-tile",
    "clipped-bitmap-shaders-tile-hq",
    "combo-patheffects",
    "convex-lineonly-paths",
    "convex-lineonly-paths-stroke-and-fill",
    "convex-polygon-inset",
    "convex-polygon-inset-v",
    "downsamplebitmap_text_high_72.00pt",
    "downsamplebitmap_text_low_72.00pt",
    "downsamplebitmap_text_medium_72.00pt",
    "downsamplebitmap_text_none_72.00pt",
    "draw-atlas",
    "draw-atlas-colors",
    "drawbitmaprect-imagerect",
    "drawbitmaprect-imagerect-subset",
    "drawbitmaprect-subset",
    "encode-alpha-jpeg",
    "encode-platform",
    "encode-srgb-jpg",
    "encode-srgb-png",
    "encode-srgb-webp",
    "filterbitmap_image_color_wheel.png",
    "filterbitmap_image_mandrill_128.png",
    "filterbitmap_image_mandrill_16.png",
    "filterbitmap_image_mandrill_256.png",
    "filterbitmap_image_mandrill_32.png",
    "filterbitmap_image_mandrill_512.png",
    "filterbitmap_image_mandrill_64.png",
    "filterbitmap_image_mandrill_64.png_g8",
    "filterbitmap_text_10.00pt",
    "filterbitmap_text_3.00pt",
    "filterbitmap_text_7.00pt",
    "fontcache-mt",
    "fontmgr_bounds_0.75_0",
    "fontmgr_bounds_1_-0.25",
    "gamma_encoded_premul_dst-v-src_from1.8",
    "gamma_encoded_premul_dst-v-src_fromLinear",
    "gamma_encoded_premul_dst-v-src_fromWideGamut",
    "gamma_encoded_premul_dst-v-src_toLinear",
    "gamma_encoded_premul_dst-v-src_toWideGamut",
    "image-cacherator-from-picture",
    "image-cacherator-from-raster",
    "image-cacherator-from-texture",
    "image-picture",
    "image-shader",
    "image-surface",
    "jpg-color-cube",
    "ninepatch-stretch",
    "path-reverse",
    "scale-pixels",
    "simple-magnification",
    "simple-offsetimagefilter",
    "simple-polygon-offset",
    "simple-polygon-offset-v",
    "stroke-fill",
};

static const char kFirstOkay[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static const char kRestOkay[] = "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static bool good_name(const SkString& name) {
    SkASSERT(name.size() > 0);
    if (NULL == strchr(kFirstOkay, name[0])) {
        return false;
    }
    for (unsigned i = 1; i < name.size(); ++i) {
        if (NULL == strchr(kRestOkay, name[i])) {
            return false;
        }
    }
    return true;
}

void ValidateGMNames() {
    SkTHashSet<SkString> okay;
    for (auto name : kGrandfatherClauseNames) {
        okay.add(SkString(name));
    }
    SkDEBUGCODE(bool good = true);
    for (skiagm::GMFactory factory : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(factory(nullptr));
        SkString name(gm->getName());
        if (!okay.contains(name) && !good_name(name)) {
            SkDEBUGCODE(good = false);
            SkDebugf("ERROR: '%s' is a bad name.\n", name.c_str());
        }
    }
    SkASSERT(good);
}
