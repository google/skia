/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAIndex.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"

#include "tests/Test.h"

static constexpr int kW = 64;
static constexpr int kH = 64;

static sk_sp<SkImage> make_tri_plane_yuv(GrContext* context, GrBackendFormat format,
                                         SkColorChannel channel) {
    GrBackendTexture beTextures[3];
    SkColor4f colors[3] = { SkColors::kBlack, SkColors::kRed, SkColors::kGreen };

    for (int i = 0; i < 3; ++i) {
        beTextures[i] = context->createBackendTexture(kW, kH, format, colors[i],
                                                      GrMipMapped::kNo, GrRenderable::kNo,
                                                      GrProtected::kNo);
    }

    SkYUVAIndex yuvaIndices[4];
    yuvaIndices[0].fIndex = 0;
    yuvaIndices[0].fChannel = channel;
    yuvaIndices[1].fIndex = 1;
    yuvaIndices[1].fChannel = channel;
    yuvaIndices[2].fIndex = 2;
    yuvaIndices[2].fChannel = channel;
    yuvaIndices[3].fIndex = -1;

    return SkImage::MakeFromYUVATextures(context, kJPEG_SkYUVColorSpace, beTextures,
                                         yuvaIndices, { kW, kH }, kTopLeft_GrSurfaceOrigin);
}

static void draw_and_read_back(skiatest::Reporter* reporter, GrContext* context,
    sk_sp<SkImage> image) {

    SkImageInfo info = SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info));
    SkCanvas* canvas = surface->getCanvas();

    canvas->drawImage(image, 0, 0);
}

// This tests that the single channel index values in SkYUVAIndex are meaningless. Regardless
// of what value is placed in there, Ganesh will sample the single channel.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TriPlaneYUV, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrCaps* caps = context->priv().caps();

    const std::vector<GrCaps::TestFormatColorTypeCombination>& combos =
                                                                    caps->getTestingCombinations();

    for (auto combo : combos) {
        SkASSERT(combo.fColorType != GrColorType::kUnknown);
        SkASSERT(combo.fFormat.isValid());

        for (auto channel : { SkColorChannel::kR, SkColorChannel::kG, SkColorChannel::kB, SkColorChannel::kA }) {
            sk_sp<SkImage> image = make_tri_plane_yuv(context, combo.fFormat, channel);

            draw_and_read_back(reporter, context, std::move(image));
        }
    }
}
