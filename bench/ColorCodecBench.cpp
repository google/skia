/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ColorCodecBench.h"
#include "Resources.h"
#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkColorSpace_XYZ.h"
#include "SkColorSpaceXform.h"
#include "SkCommandLineFlags.h"

DEFINE_bool(xform_only, false, "Only time the color xform, do not include the decode time");
DEFINE_bool(srgb,       false, "Convert to srgb dst space");
DEFINE_bool(nonstd,     false, "Convert to non-standard dst space");
DEFINE_bool(half,       false, "Convert to half floats");

ColorCodecBench::ColorCodecBench(const char* name, sk_sp<SkData> encoded)
    : fEncoded(std::move(encoded))
{
    fName.appendf("Color%s", FLAGS_xform_only ? "Xform" : "Codec");
    fName.appendf("_%s", name);
}

const char* ColorCodecBench::onGetName() {
    return fName.c_str();
}

bool ColorCodecBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

void ColorCodecBench::decodeAndXform() {
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(fEncoded));
    SkASSERT(codec);

#ifdef SK_DEBUG
    SkCodec::Result result =
#endif
    codec->getPixels(fDstInfo, fDst.get(), fDstInfo.minRowBytes());
    SkASSERT(SkCodec::kSuccess == result);
}

void ColorCodecBench::xformOnly() {
    std::unique_ptr<SkColorSpaceXform> xform = SkColorSpaceXform::New(fSrcSpace.get(),
                                                                      fDstSpace.get());
    SkASSERT(xform);

    void* dst = fDst.get();
    void* src = fSrc.get();
    for (int y = 0; y < fSrcInfo.height(); y++) {
        SkAssertResult(xform->apply(select_xform_format(fDstInfo.colorType()), dst,
                                    SkColorSpaceXform::kRGBA_8888_ColorFormat, src,
                                    fSrcInfo.width(), fDstInfo.alphaType()));
        dst = SkTAddOffset<void>(dst, fDstInfo.minRowBytes());
        src = SkTAddOffset<void>(src, fSrcInfo.minRowBytes());
    }
}

void ColorCodecBench::onDelayedSetup() {
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(fEncoded));
    fSrcInfo = codec->getInfo().makeColorType(kRGBA_8888_SkColorType);
    fDstInfo = fSrcInfo;

    fDstSpace = nullptr;
    if (FLAGS_srgb) {
        fDstSpace = SkColorSpace::MakeSRGB();
    } else if (FLAGS_nonstd) {
        SkColorSpaceTransferFn gamma;
        gamma.fA = 1.0f;
        gamma.fB = gamma.fC = gamma.fD = gamma.fE = gamma.fF = 0.0f;
        gamma.fG = 4.0f;
        SkMatrix44 matrix = SkMatrix44(SkMatrix44::kUninitialized_Constructor);
        matrix.set3x3(0.30f, 0.31f, 0.28f, 0.32f, 0.33f, 0.29f, 0.27f, 0.30f, 0.30f);
        fDstSpace = SkColorSpace::MakeRGB(gamma, matrix);
    } else {
        sk_sp<SkData> dstData = SkData::MakeFromFileName(
                GetResourcePath("icc_profiles/HP_ZR30w.icc").c_str());
        SkASSERT(dstData);
        fDstSpace = SkColorSpace::MakeICC(dstData->data(), dstData->size());
    }
    SkASSERT(fDstSpace);
    fDstInfo = fDstInfo.makeColorSpace(fDstSpace);

    if (FLAGS_half) {
        fDstInfo = fDstInfo.makeColorType(kRGBA_F16_SkColorType);
        SkASSERT(SkColorSpace_Base::Type::kXYZ == as_CSB(fDstSpace)->type());
        fDstSpace = static_cast<SkColorSpace_XYZ*>(fDstSpace.get())->makeLinearGamma();
    }

    fDst.reset(fDstInfo.getSafeSize(fDstInfo.minRowBytes()));

    if (FLAGS_xform_only) {
        fSrc.reset(fSrcInfo.getSafeSize(fSrcInfo.minRowBytes()));
        fSrcSpace = codec->getInfo().refColorSpace();
        codec->getPixels(fSrcInfo, fSrc.get(), fSrcInfo.minRowBytes());
    }
}

void ColorCodecBench::onDraw(int n, SkCanvas*) {
    for (int i = 0; i < n; i++) {
        if (FLAGS_xform_only) {
            this->xformOnly();
        } else {
            this->decodeAndXform();
        }
    }
}
