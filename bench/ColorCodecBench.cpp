/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ColorCodecBench.h"
#include "Resources.h"
#include "SkCodec.h"
#include "SkColorSpaceXform.h"
#include "SkCommandLineFlags.h"

#if defined(SK_TEST_QCMS)
DEFINE_bool(qcms,       false, "Bench qcms color conversion");
#endif
DEFINE_bool(xform_only, false, "Only time the color xform, do not include the decode time");
DEFINE_bool(srgb,       false, "Convert to srgb dst space");

ColorCodecBench::ColorCodecBench(const char* name, sk_sp<SkData> encoded)
    : fEncoded(std::move(encoded))
#if defined(SK_TEST_QCMS)
    , fDstSpaceQCMS(nullptr)
#endif
{
    fName.appendf("Color%s", FLAGS_xform_only ? "Xform" : "Codec");
#if defined(SK_TEST_QCMS)
    fName.appendf("%s", FLAGS_qcms ? "QCMS" : "");
#endif
    fName.appendf("_%s", name);
}

const char* ColorCodecBench::onGetName() {
    return fName.c_str();
}

bool ColorCodecBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

void ColorCodecBench::decodeAndXform() {
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(fEncoded.get()));
#ifdef SK_DEBUG
    const SkCodec::Result result =
#endif
    codec->startScanlineDecode(fInfo);
    SkASSERT(SkCodec::kSuccess == result);

    sk_sp<SkColorSpace> srcSpace = sk_ref_sp(codec->getColorSpace());
    if (!srcSpace) {
        srcSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
    }
    std::unique_ptr<SkColorSpaceXform> xform = SkColorSpaceXform::New(srcSpace, fDstSpace);
    SkASSERT(xform);

    void* dst = fDst.get();
    for (int y = 0; y < fInfo.height(); y++) {
#ifdef SK_DEBUG
        const int rows =
#endif
        codec->getScanlines(fSrc.get(), 1, 0);
        SkASSERT(1 == rows);

        xform->xform_RGB1_8888((uint32_t*) dst, (uint32_t*) fSrc.get(), fInfo.width());
        dst = SkTAddOffset<void>(dst, fInfo.minRowBytes());
    }
}

#if defined(SK_TEST_QCMS)
void ColorCodecBench::decodeAndXformQCMS() {
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(fEncoded.get()));
#ifdef SK_DEBUG
    const SkCodec::Result result =
#endif
    codec->startScanlineDecode(fInfo);
    SkASSERT(SkCodec::kSuccess == result);

    SkAutoTCallVProc<qcms_profile, qcms_profile_release>
            srcSpace(qcms_profile_from_memory(fSrcData->data(), fSrcData->size()));
    SkASSERT(srcSpace);

    SkAutoTCallVProc<qcms_transform, qcms_transform_release>
            transform (qcms_transform_create(srcSpace, QCMS_DATA_RGBA_8, fDstSpaceQCMS.get(),
                                             QCMS_DATA_RGBA_8, QCMS_INTENT_PERCEPTUAL));
    SkASSERT(transform);

#ifdef SK_PMCOLOR_IS_RGBA
    qcms_output_type outType = QCMS_OUTPUT_RGBX;
#else
    qcms_output_type outType = QCMS_OUTPUT_BGRX;
#endif

    void* dst = fDst.get();
    for (int y = 0; y < fInfo.height(); y++) {
#ifdef SK_DEBUG
        const int rows =
#endif
        codec->getScanlines(fSrc.get(), 1, 0);
        SkASSERT(1 == rows);

        qcms_transform_data_type(transform, fSrc.get(), dst, fInfo.width(), outType);
        dst = SkTAddOffset<void>(dst, fInfo.minRowBytes());
    }
}
#endif

void ColorCodecBench::xformOnly() {
    sk_sp<SkColorSpace> srcSpace = SkColorSpace::NewICC(fSrcData->data(), fSrcData->size());
    if (!srcSpace) {
        srcSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
    }
    std::unique_ptr<SkColorSpaceXform> xform = SkColorSpaceXform::New(srcSpace, fDstSpace);
    SkASSERT(xform);

    void* dst = fDst.get();
    void* src = fSrc.get();
    for (int y = 0; y < fInfo.height(); y++) {
        // Transform in place
        xform->xform_RGB1_8888((uint32_t*) dst, (uint32_t*) src, fInfo.width());
        dst = SkTAddOffset<void>(dst, fInfo.minRowBytes());
        src = SkTAddOffset<void>(src, fInfo.minRowBytes());
    }
}

#if defined(SK_TEST_QCMS)
void ColorCodecBench::xformOnlyQCMS() {
    SkAutoTCallVProc<qcms_profile, qcms_profile_release>
            srcSpace(qcms_profile_from_memory(fSrcData->data(), fSrcData->size()));
    SkASSERT(srcSpace);

    SkAutoTCallVProc<qcms_transform, qcms_transform_release>
            transform (qcms_transform_create(srcSpace, QCMS_DATA_RGBA_8, fDstSpaceQCMS.get(),
                                             QCMS_DATA_RGBA_8, QCMS_INTENT_PERCEPTUAL));
    SkASSERT(transform);

#ifdef SK_PMCOLOR_IS_RGBA
    qcms_output_type outType = QCMS_OUTPUT_RGBX;
#else
    qcms_output_type outType = QCMS_OUTPUT_BGRX;
#endif

    void* dst = fDst.get();
    void* src = fSrc.get();
    for (int y = 0; y < fInfo.height(); y++) {
        // Transform in place
        qcms_transform_data_type(transform, src, dst, fInfo.width(), outType);
        dst = SkTAddOffset<void>(dst, fInfo.minRowBytes());
        src = SkTAddOffset<void>(src, fInfo.minRowBytes());
    }
}
#endif

void ColorCodecBench::onDelayedSetup() {
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(fEncoded.get()));
    fInfo = codec->getInfo().makeColorType(kRGBA_8888_SkColorType);

    fDst.reset(fInfo.getSafeSize(fInfo.minRowBytes()));
    if (FLAGS_xform_only) {
        fSrc.reset(fInfo.getSafeSize(fInfo.minRowBytes()));
        codec->getPixels(fInfo, fSrc.get(), fInfo.minRowBytes());
    } else {
        // Set-up a row buffer to decode into before transforming to dst.
        fSrc.reset(fInfo.minRowBytes());
    }

    fSrcData = codec->getICCData();
    sk_sp<SkData> dstData = SkData::MakeFromFileName(
            GetResourcePath("monitor_profiles/HP_ZR30w.icc").c_str());
    SkASSERT(dstData);


#if defined(SK_TEST_QCMS)
    if (FLAGS_qcms) {
        fDstSpaceQCMS.reset(FLAGS_srgb ?
                qcms_profile_sRGB() :
                qcms_profile_from_memory(dstData->data(), dstData->size()));
        SkASSERT(fDstSpaceQCMS);

        // This call takes a non-trivial amount of time, but I think it's the most fair to
        // treat it as overhead.  It only needs to happen once.
        qcms_profile_precache_output_transform(fDstSpaceQCMS);
    } else
#endif
    {
        fDstSpace = FLAGS_srgb ? SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named) :
                                 SkColorSpace::NewICC(dstData->data(), dstData->size());
        SkASSERT(fDstSpace);
    }
}

void ColorCodecBench::onDraw(int n, SkCanvas*) {
    for (int i = 0; i < n; i++) {
#if defined(SK_TEST_QCMS)
        if (FLAGS_qcms) {
            if (FLAGS_xform_only) {
                this->xformOnlyQCMS();
            } else {
                this->decodeAndXformQCMS();
            }
        } else
#endif
        {
            if (FLAGS_xform_only) {
                this->xformOnly();
            } else {
                this->decodeAndXform();
            }
        }
    }
}
