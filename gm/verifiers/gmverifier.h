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
 * Different verifiers perform different types of transforms and checks. Verifiers may check the
 * output of a GM against a given "golden" image which represents the correct output, or just
 * check the output image of the GM by itself.
 *
 * Most verifiers have configurable fuzziness in the comparisons performed against the golden image.
 *
 * Subclasses should inherit from one of StandaloneVerifier or GoldImageVerifier instead of
 * directly from this base class.
 */
class GMVerifier {
public:
    GMVerifier() = delete;

    virtual ~GMVerifier();

    /** Returns the human-friendly name of the verifier. */
    virtual SkString name() const = 0;

    /** Returns true if this verifier needs the gold image as input. */
    bool needsGoldImage() const;

    /**
     * Runs the verifier. This method should be used if the verifier needs the gold image as input.
     *
     * @param gold Bitmap containing the "correct" image.
     * @param actual Bitmap containing rendered output of a GM.
     * @return Ok if the verification passed, or an error if not.
     */
    VerifierResult verify(const SkBitmap& gold, const SkBitmap& actual);

    /**
     * Runs the verifier.
     *
     * @param actual Bitmap containing rendered output of a GM.
     * @return Ok if the verification passed, or an error if not.
     */
    VerifierResult verify(const SkBitmap& actual);

    /** Renders the GM using the "golden" configuration. This is common across all GMs/verifiers. */
    static SkBitmap RenderGoldBmp(skiagm::GM* gm, const SkColorInfo& colorInfo);

    /**
     * Gets the color information that all verifier inputs should be transformed into.
     *
     * The primary reason for having a single shared colorspace/color type is making per-pixel
     * comparisons easier. Both the image under test and gold image are transformed into a shared
     * colorspace which allows for getting per-pixel colors in SkColor4f.
     */
    static SkColorInfo VerifierColorInfo();

protected:
    /** The type of input required for the verifier. */
    enum class InputType {
        kGoldImageRequired, kStandalone
    };

    /** Set depending if the verifier needs a golden image as an input. */
    InputType fInputType;

    /** Constructor. */
    GMVerifier(InputType inputType);

    /** Implementation of the verification. */
    virtual VerifierResult verifyWithGold(
        const SkIRect& region, const SkBitmap& gold, const SkBitmap& actual) = 0;

    /** Implementation of the verification. */
    virtual VerifierResult verify(const SkIRect& region, const SkBitmap& actual) = 0;

    /** Returns an error result formatted appropriately. */
    VerifierResult makeError(const SkString& msg) const;
};

/**
 * A verifier that operates standalone on the given input image (no comparison against a golden
 * image).
 */
class StandaloneVerifier : public GMVerifier {
public:
    StandaloneVerifier() : GMVerifier(InputType::kStandalone) {}

protected:
    VerifierResult verifyWithGold(const SkIRect&, const SkBitmap&, const SkBitmap&) final {
        return makeError(SkString("Verifier does not accept gold image input"));
    }
};

/**
 * A verifier that operates compares input image against a golden image.
 */
class GoldImageVerifier : public GMVerifier {
public:
    GoldImageVerifier() : GMVerifier(InputType::kGoldImageRequired) {}

protected:
    VerifierResult verify(const SkIRect&, const SkBitmap&) final {
        return makeError(SkString("Verifier does not accept standalone input"));
    }
};

/** A list of GM verifiers. */
class VerifierList {
public:
    /** Constructs a VerifierList with the given gm instance. */
    explicit VerifierList(GM* gm);

    /** Adds a verifier to the list of verifiers. */
    void add(std::unique_ptr<GMVerifier> verifier);

    /**
     * Runs all verifiers against the given input. If any verifiers fail, returns the first error.
     * Else, returns ok. This version can be used if no verifiers in the list require the gold
     * image as input.
     */
    VerifierResult verifyAll(const SkColorInfo& colorInfo, const SkBitmap& actual);

private:
    /** The parent GM instance of this VerifierList. */
    GM* fGM;

    /** The list of verifiers. */
    std::vector<std::unique_ptr<GMVerifier>> fVerifiers;

    /** After running, set to the first verifier that failed, or nullptr if none failed. */
    const GMVerifier* fFailedVerifier;

    /** Returns true if any verifiers in the list need the gold image as input. */
    bool needsGoldImage() const;
};

}
}

#endif
