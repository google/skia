/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "gm/verifiers/gmverifier.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "include/encode/SkPngEncoder.h"
#include "src/utils/SkOSPath.h"

/** Checks the given VerifierResult. If it is not ok, returns it. */
#define RETURN_NOT_OK(res)  if (!(res).ok()) return (res)

namespace skiagm {
namespace verifiers {

VerifierResult::VerifierResult() : VerifierResult(Code::kOk, SkString("Ok")) {}

VerifierResult::VerifierResult(VerifierResult::Code code, const SkString& msg) :
    fCode(code), fMessage(msg) {}

bool VerifierResult::ok() const {
    return fCode == Code::kOk;
}

const SkString& VerifierResult::message() const {
    return fMessage;
}

VerifierResult VerifierResult::Ok() {
    return VerifierResult(Code::kOk, SkString("Ok"));
}

VerifierResult VerifierResult::Fail(const SkString& msg) {
    return VerifierResult(Code::kFail, msg);
}

GMVerifier::GMVerifier() {}

GMVerifier::~GMVerifier() {}

VerifierResult GMVerifier::verify(const SkBitmap& gold, const SkBitmap& actual) {
    // Call into specific implementation.
    return verifyImpl(actual.bounds(), gold, actual);
}

SkBitmap GMVerifier::RenderGoldBmp(
    skiagm::GM* gm, SkColorType colorType, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    SkASSERT(gm);
    const SkISize size = gm->getISize();
    const SkImageInfo imgInfo = SkImageInfo::Make(
        size.width(), size.height(), colorType, alphaType, colorSpace);
    SkBitmap goldBmp;
    goldBmp.allocPixels(imgInfo);
    SkCanvas canvas(goldBmp);
    gm->draw(&canvas);

    // Convert into common verifier colorspace.
    SkColorType verifierColorType;
    SkAlphaType verifierAlphaType;
    sk_sp<SkColorSpace> verifierColorSpace;
    VerifierColorInfo(&verifierColorType, &verifierAlphaType, &verifierColorSpace);

    SkBitmap goldVerifierBmp;
    goldVerifierBmp.allocPixels(
        SkImageInfo::Make(
            size.width(), size.height(), verifierColorType, verifierAlphaType, verifierColorSpace));

    SkCanvas verifierCanvas(goldVerifierBmp);
    verifierCanvas.drawBitmap(goldBmp, 0, 0);

    return goldVerifierBmp;
}

void GMVerifier::VerifierColorInfo(
    SkColorType* outColorType, SkAlphaType* outAlphaType, sk_sp<SkColorSpace>* outColorSpace) {
    if (outColorType) {
        *outColorType = kRGBA_F16_SkColorType;
    }
    if (outAlphaType) {
        *outAlphaType = kPremul_SkAlphaType;
    }
    if (outColorSpace) {
        *outColorSpace = SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020);
    }
}

VerifierResult GMVerifier::makeError(const SkString& msg) const {
    return VerifierResult::Fail(SkStringPrintf("[%s] %s", name().c_str(), msg.c_str()));
}

void VerifierList::add(std::unique_ptr<GMVerifier> verifier) {
    fVerifiers.push_back(std::move(verifier));
}

VerifierResult VerifierList::verifyAll(const SkBitmap& gold, const SkBitmap& actual) {
    for (const auto& v : fVerifiers) {
        fFailedVerifier = v.get();
        RETURN_NOT_OK(v->verify(gold, actual));
    }
    fFailedVerifier = nullptr;
    return VerifierResult::Ok();
}

}
}
