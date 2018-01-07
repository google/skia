/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

namespace skqp {
const char* kDoNotExecuteInExperimentalMode[] = {
    nullptr  // list is null-terminated
};

const char* kNoScoreInCompatibilityTestMode[] = {
    "circular-clips",
    "colorcomposefilter_wacky",
    "coloremoji_blendmodes",
    "colormatrix",
    "complexclip_bw",
    "complexclip_bw_invert",
    "complexclip_bw_layer",
    "complexclip_bw_layer_invert",
    "convex-lineonly-paths-stroke-and-fill",
    "dftext",
    "downsamplebitmap_image_high_mandrill_512.png",
    "downsamplebitmap_image_medium_mandrill_512.png",
    "filterbitmap_image_mandrill_16.png",
    "filterbitmap_image_mandrill_64.png",
    "filterbitmap_image_mandrill_64.png_g8",
    "gradients_degenerate_2pt",
    "gradients_degenerate_2pt_nodither",
    "gradients_local_perspective",
    "gradients_local_perspective_nodither",
    "imagefilterstransformed",
    "image_scale_aligned",
    "lattice",
    "linear_gradient",
    "mipmap_srgb",
    "mixedtextblobs",
    "OverStroke",
    "simple-offsetimagefilter",
    "strokerect",
    "textblobmixedsizes",
    "textblobmixedsizes_df",
    nullptr  // list is null-terminated
};
}  // namespace skqp
