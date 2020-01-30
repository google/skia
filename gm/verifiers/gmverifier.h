/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gmverifier_DEFINED
#define gmverifier_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"

#include <vector>

class SkBitmap;

namespace skiagm {

class GM;

namespace verifiers {

/** Result type for GM verifiers. */
class VerifierResult {
public:
    VerifierResult();

    /** Returns true if the result is ok (non-error). */
    bool ok() const;

    /** Returns reference to any message associated with the result. */
    const SkString& message() const;

    /** Constructs an "ok" (non-error) result. */
    static VerifierResult Ok();

    /** Constructs a "fail" (error) result with a specific message. */
    static VerifierResult Fail(const SkString& msg);

private:
    /** Underlying error code. */
    enum class Code {
        kOk, kFail
    };

    /** Result code */
    Code fCode;

    /** Result message (may be empty). */
    SkString fMessage;

    /** Private constructor for a result with a specific code and message. */
    VerifierResult(Code code, const SkString& msg);
};

/**
 * Abstract base class for GM verifiers. A verifier checks the rendered output image of a GM.
 *
 * Different verifiers perform different types of transforms and checks. Most commonly verifiers
 * check the output of a GM against a given "golden" image which represents the correct output.
 * Most verifiers have configurable fuzziness in the comparisons performed against the golden image.
 *
 * Some verifiers ignore the input golden image and perform checks against the GM output only.
 */
class GMVerifier {
public:
    GMVerifier();

    virtual ~GMVerifier();

    /** Returns the human-friendly name of the verifier. */
    virtual SkString name() const = 0;

    /**
     * Runs the verifier.
     *
     * @param gold Bitmap containing the "correct" image.
     * @param actual Bitmap containing rendered output of a GM.
     * @return Ok if the verification passed, or an error if not.
     */
    VerifierResult verify(const SkBitmap& gold, const SkBitmap& actual);

    /** Renders the GM using the "golden" configuration. This is common across all GMs/verifiers. */
    static SkBitmap RenderGoldBmp(
        skiagm::GM* gm, SkColorType colorType, SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace);

    /**
     * Gets the colorspace information that all verifier inputs should be transformed into.
     *
     * @param outColorType Set to the verifier input color type
     * @param outAlphaType Set to the verifier input alpha type
     * @param outColorSpace Set to the verifier input colorspace
     */
    static void VerifierColorInfo(
        SkColorType* outColorType, SkAlphaType* outAlphaType, sk_sp<SkColorSpace>* outColorSpace);

protected:
    /** Implementation of the verification. */
    virtual VerifierResult verifyImpl(
        const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) = 0;

    /** Returns an error result formatted appropriately. */
    VerifierResult makeError(const SkString& msg) const;
};

/** A list of GM verifiers. */
class VerifierList {
public:
    /** Adds a verifier to the list of verifiers. */
    void add(std::unique_ptr<GMVerifier> verifier);

    /**
     * Runs all verifiers against the given input. If any verifiers fail, returns the first error.
     * Else, returns ok.
     */
    VerifierResult verifyAll(const SkBitmap& gold, const SkBitmap& actual);

private:
    /** The list of verifiers. */
    std::vector<std::unique_ptr<GMVerifier>> fVerifiers;

    /** After running, set to the first verifier that failed, or nullptr if none failed. */
    const GMVerifier* fFailedVerifier;
};

}
}

#endif
