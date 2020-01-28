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
 * A "relaxation" for GM verifiers allows per-pixel allowance for errors/pixel mismatches. This is
 * typically invoked by verifiers like so:
 *
 * for y in range(0, height) {
 *   for x in range(0, width) {
 *     if (pixelFails(x, y) && !relaxPixel(x, y)) {
 *       failedPixels++;
 *     }
 *   }
 * }
 *
 * */
class VerifierRelaxation {
public:
    virtual ~VerifierRelaxation() {}

    /** Initializes the relaxation check (where applicable) with the given gold image. */
    virtual void init(const SkBitmap& goldBmp) {}

    /**
     * Return true if the given pixel should be "relaxed", i.e. should not count towards the failed
     * pixel count.
     */
    virtual bool relaxPixel(int x, int y) = 0;

protected:
    /** The preprocessed bitmap data for use by the relaxation check. */
    std::unique_ptr<SkBitmap> fBitmap;
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

    /**
     * Adds a rectangular region to constrain the verifier. The verification will only run on the
     * pixels contained in the configured regions of the GM output image.
     *
     * If no regions are added (which is the default), the bounds of the entire image will be
     * verified.
     */
    void addRegion(const SkIRect& rect);

    /** Sets the optional "relaxation" to use during verification. */
    void setRelaxation(std::unique_ptr<VerifierRelaxation> relax);

    /**
     * Write image generated (if any) by the verifier for help debugging tests that fail
     * verification.
     *
     * @param dirPath Path of directory to write images to
     * @param prefix Prefix added to written image filenames
     */
    void writeDebugImages(const SkString& dirPath, const SkString& prefix) const;

    /** Renders the GM using the "golden" configuration. This is common across all GMs/verifiers. */
    static SkBitmap renderGoldBmp(skiagm::GM* gm);

protected:
    /**
     * If non-empty, a list of rectangular regions that will constrain the verifier. Each region
     * should be half-open on the right and bottom, i.e. [top, bottom) and [left, right).
     */
    std::vector<SkIRect> fRegions;

    /** Relaxation to use. May be null if none was set. */
    std::unique_ptr<VerifierRelaxation> fRelaxation;

    /** An image generated during verification that can help in debugging failed verifications. */
    std::unique_ptr<SkBitmap> fDebugImage;

    /**
     * Returns true if the given pixel should be relaxed, i.e. not included in the failed pixel count.
     * If no relaxation was configured, this will always return false (meaning all failed pixels
     * count towards the failed pixel count).
     */
    bool relaxPixel(int x, int y) const;

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

    /**
     * Write image generated (if any) by the first failing verifier for help debugging tests that
     * fail verification.
     *
     * @param dirPath Path of directory to write images to
     * @param prefix Prefix added to written image filenames
     */
    void writeDebugImage(const SkString& dirPath, const SkString& prefix) const;

private:
    /** The list of verifiers. */
    std::vector<std::unique_ptr<GMVerifier>> fVerifiers;

    /** After running, set to the first verifier that failed, or nullptr if none failed. */
    const GMVerifier* fFailedVerifier;
};

}
}

#endif
