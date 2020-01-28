/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "gm/verifiers/gmverifier.h"
#include "gm/verifiers/utils.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
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
    if (fRelaxation) {
        fRelaxation->init(gold);
    }

    if (fRegions.empty()) {
        // No regions specified => check entire image.
        return verifyImpl(actual.bounds(), gold, actual);
    } else {
        // Check each region, returning early if any fail.
        for (const auto& r : fRegions) {
            if (!gold.bounds().contains(r) || !actual.bounds().contains(r)) {
                return makeError(
                    SkStringPrintf(
                        "region (%s) exceeds bitmap bounds ((%s) or (%s))",
                        utils::toString(r).c_str(), utils::toString(gold.bounds()).c_str(),
                        utils::toString(actual.bounds()).c_str()));
            }

            auto res = verifyImpl(r, gold, actual);
            if (!res.ok()) {
                return res;
            }
        }
    }

    return VerifierResult::Ok();
}

void GMVerifier::setRelaxation(std::unique_ptr<VerifierRelaxation> relax) {
    fRelaxation = std::move(relax);
}

void GMVerifier::writeDebugImages(const SkString& dirPath, const SkString& prefix) const {
    if (fDebugImage && !fDebugImage->drawsNothing()) {
        SkPixmap pm;
        SkAssertResult(fDebugImage->peekPixels(&pm));

        const SkString filename = SkStringPrintf("%s-%s.png", prefix.c_str(), name().c_str());
        const SkString path = SkOSPath::Join(dirPath.c_str(), filename.c_str());
        SkFILEWStream fileStream(path.c_str());
        SkASSERT(fileStream.isValid());

        SkPngEncoder::Options options;
        options.fFilterFlags = SkPngEncoder::FilterFlag::kNone;
        SkPngEncoder::Encode(&fileStream, pm, options);
        fileStream.flush();
    }
}

SkBitmap GMVerifier::renderGoldBmp(skiagm::GM* gm) {
    SkASSERT(gm);

    const SkColorType colorType = kN32_SkColorType;
    const SkISize size = gm->getISize();

    // If there's an appropriate alpha type for this color type, use it, otherwise use premul.
    SkAlphaType alphaType = kPremul_SkAlphaType;
    (void)SkColorTypeValidateAlphaType(colorType, alphaType, &alphaType);

    SkBitmap gold_bmp;
    gold_bmp.allocPixelsFlags(
        SkImageInfo::Make(size, colorType, alphaType, nullptr), SkBitmap::kZeroPixels_AllocFlag);

    SkCanvas canvas(gold_bmp);
    gm->draw(&canvas);
    return gold_bmp;
}

void GMVerifier::addRegion(const SkIRect& rect) {
    fRegions.emplace_back(rect);
}

bool GMVerifier::relaxPixel(int x, int y) const {
    if (fRelaxation) {
        return fRelaxation->relaxPixel(x, y);
    } else {
        return false;
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

void VerifierList::writeDebugImage(const SkString& dirPath, const SkString& prefix) const {
    if (fFailedVerifier) {
        fFailedVerifier->writeDebugImages(dirPath, prefix);
    }
}

}
}
