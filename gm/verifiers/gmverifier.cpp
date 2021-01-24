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

GMVerifier::GMVerifier(InputType inputType) : fInputType(inputType) {}

GMVerifier::~GMVerifier() {}

bool GMVerifier::needsGoldImage() const {
    return fInputType == InputType::kGoldImageRequired;
}

VerifierResult GMVerifier::verify(const SkBitmap& gold, const SkBitmap& actual) {
    // Call into specific implementation.
    return verifyWithGold(actual.bounds(), gold, actual);
}

VerifierResult GMVerifier::verify(const SkBitmap& actual) {
    // Call into specific implementation.
    return verify(actual.bounds(), actual);
}

SkBitmap GMVerifier::RenderGoldBmp(skiagm::GM* gm, const SkColorInfo& colorInfo) {
    SkASSERT(gm);

    // Call into the GM instance to get the initial image.
    const SkISize size = gm->getISize();
    SkBitmap goldBmp;
    goldBmp.allocPixels(SkImageInfo::Make(size, colorInfo));
    SkCanvas canvas(goldBmp);

    if (gm->gpuSetup(nullptr, &canvas) == DrawResult::kOk) {
        gm->draw(&canvas);
    }

    // Convert into common verifier colorspace.
    SkBitmap goldVerifierBmp;
    goldVerifierBmp.allocPixels(SkImageInfo::Make(size, VerifierColorInfo()));
    SkCanvas verifierCanvas(goldVerifierBmp);
    verifierCanvas.drawImage(goldBmp.asImage(), 0, 0);

    return goldVerifierBmp;
}

SkColorInfo GMVerifier::VerifierColorInfo() {
    return SkColorInfo(
        kRGBA_F16_SkColorType, kPremul_SkAlphaType,
        SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020));
}

VerifierResult GMVerifier::makeError(const SkString& msg) const {
    return VerifierResult::Fail(SkStringPrintf("[%s] %s", name().c_str(), msg.c_str()));
}

VerifierList::VerifierList(GM* gm) : fGM(gm), fFailedVerifier(nullptr) {}

void VerifierList::add(std::unique_ptr<GMVerifier> verifier) {
    fVerifiers.push_back(std::move(verifier));
}

bool VerifierList::needsGoldImage() const {
    for (const auto& v : fVerifiers) {
        if (v->needsGoldImage()) {
            return true;
        }
    }

    return false;
}

VerifierResult VerifierList::verifyAll(const SkColorInfo& colorInfo, const SkBitmap& actual) {
    // Render the golden image if any verifiers need it.
    SkBitmap goldBmp;
    if (needsGoldImage()) {
        goldBmp = GMVerifier::RenderGoldBmp(fGM, colorInfo);
    }

    for (const auto& v : fVerifiers) {
        fFailedVerifier = v.get();
        if (v->needsGoldImage()) {
            RETURN_NOT_OK(v->verify(goldBmp, actual));
        } else {
            RETURN_NOT_OK(v->verify(actual));
        }
    }

    fFailedVerifier = nullptr;
    return VerifierResult::Ok();
}

}  // namespace verifiers
}  // namespace skiagm
