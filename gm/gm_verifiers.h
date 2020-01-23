/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gm_verifiers_DEFINED
#define gm_verifiers_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"

#include <vector>

class SkBitmap;

namespace skiagm {

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
            kOk,
            kFail
        };

        /** Result code */
        Code fCode;

        /** Result message (may be empty). */
        SkString fMessage;

        /** Private constructor for a result with a specific code and message. */
        VerifierResult(Code code, const SkString& msg);
    };

    class VerifierRelaxation {
    public:
        virtual ~VerifierRelaxation() {}

        virtual void init(const SkBitmap& bmp) {}

        virtual bool relaxPixel(int x, int y) { return false; }

    protected:
        std::unique_ptr<SkBitmap> fBitmap;
    };

    class RelaxNearEdges : public VerifierRelaxation {
    public:
        void init(const SkBitmap& bmp) override;

        bool relaxPixel(int x, int y) override;

    private:
        const int kPixelRadius = 1;
        static void edgeDetect(const SkBitmap& input_bmp, SkBitmap* result_bmp);
    };

    /**
     * Abstract base class for GM verifiers. A verifier checks the image output of a GM against
     * a given correct ("gold") image. Different verifiers perform different types of transforms
     * and checks.
     */
    class GMVerifier {
    public:
        GMVerifier();

        virtual ~GMVerifier();

        /** Returns the human-friendly name of the verifier. */
        virtual SkString name() const = 0;

        /** Runs the verifier. */
        VerifierResult verify(const SkBitmap& gold, const SkBitmap& actual);

        void setRelaxation(std::unique_ptr<VerifierRelaxation> relax);

        /**
         * Returns a pointer to the gold image used by the verifier (may be mutated from the
         * original given gold image).
         */
        const SkBitmap* goldImg() const;

        /**
         * Returns a pointer to "image under test" used by the verifier (may be mutated from the
         * original given image).
         */
        const SkBitmap* imgUnderTest() const;

    protected:
        /** Current "gold" image. */
        std::unique_ptr<SkBitmap> fCurrentGoldImg;

        /** Current "actual" image (image under test). */
        std::unique_ptr<SkBitmap> fCurrentImgUnderTest;

        /**
         * If non-empty, a list of rectangular regions that will constrain the verifier. Each region
         * should be half-open on the right and bottom, i.e. [top, bottom) and [left, right).
         */
        std::vector<SkIRect> fRegions;

        std::unique_ptr<VerifierRelaxation> fRelaxation;

        bool fHighlightChangesIfPossible;

        /**
         * Adds a rectangular region to constrain the verifier. If no regions are added (the
         * default), the bounds of the entire image will be verified.
         */
        void addRegion(const SkIRect& rect);

        bool relaxPixel(int x, int y) const;

        /** Runs the verifier. */
        virtual VerifierResult verifyImpl(const SkBitmap& gold, const SkBitmap& actual) = 0;

        /** Returns an error result formatted appropriately. */
        VerifierResult makeError(const SkString& msg) const;

        /** Converts an SkIRect to a string representation. */
        static SkString toString(const SkIRect& r);

        /** Converts an SkColor to a string representation. */
        static SkString toString(const SkColor& c);

        /** Duplicates a bitmap */
        static std::unique_ptr<SkBitmap> duplicate(const SkBitmap& bmp);

        /** Produces a mask of a bitmap. */
        static std::unique_ptr<SkBitmap> mask(const SkBitmap& bmp);

        /**
         * Returns true if a color is found within a pixel neighborhood of a bitmap.
         *
         * @param bitmap Bitmap to check
         * @param x X coord of pixel
         * @param y Y coord of pixel
         * @param color Color to search for
         * @param n Pixel radius around x,y to search for color.
         * @param colorDist The color distance used for fuzzy color equality. The default of 0
         *    means that the pixels much exactly match.
         * @return True if the color is found within n pixels of (x,y) in the bitmap.
         */
        static bool colorInNeighborhood(const SkBitmap& bitmap,
                                        int x,
                                        int y,
                                        SkColor color,
                                        int n = 2,
                                        uint32_t dist = 0);

        /**
         * Returns a distance between two colors.
         *
         * dist(a, b) = sum_{c in channels} |a_c - b_c|
         */
        static uint32_t colorDist(SkColor a, SkColor b);

        /**
         * Returns max distance between corresponding channels of two colors.
         *
         * dist(a, b) = max_{c in channels} |a_c - b_c|
         */
        static uint32_t maxChannelDist(SkColor a, SkColor b);
    };

    /** Verifier that performs an exact per-pixel comparison against the gold image. */
    class ExactPixelMatch : public GMVerifier {
    public:
        SkString name() const override;

        VerifierResult verifyImpl(const SkBitmap& gold, const SkBitmap& actual) override;

    private:
        /** Returns Ok if the two bitmaps are exactly equal. */
        VerifierResult bitmapsEqual(const SkBitmap& a, const SkBitmap& b);
    };

    /**
     * Verifier that checks that pixels appearing in the actual image were present in the gold
     * image, ignoring color and discarding "mostly invisible" pixels.
     */
    class CompareToMask : public GMVerifier {
    public:
        SkString name() const override;

        VerifierResult verifyImpl(const SkBitmap& gold, const SkBitmap& actual) override;

    private:
        /** Background color used to determine which pixels are "mostly invisible." */
        const SkColor kBackgroundColor = SK_ColorWHITE;

        /** Distance threshold used to determine which pixels are "mostly invisible." */
        const uint8_t kBackgroundColorDistanceThreshold = 0;

        /** Acceptable percentage of pixels that can be different from the mask. */
        const float kPercentPixelDifferenceThreshold = 0.01f;

        /** Runs the verification. */
        VerifierResult pixelsAreInMask(SkBitmap& a, SkBitmap& mask);
    };

    /**
     * Verifier that checks that pixels appearing in the actual image appear "nearby" (and within
     * some small delta) in the gold image.
     */
    class CheckPixelColorNearby : public GMVerifier {
    public:
        SkString name() const override;

        VerifierResult verifyImpl(const SkBitmap& gold, const SkBitmap& actual) override;

    private:
        /** Pixel radius to search for color match. */
        const int kNeighborhoodSize = 2;

        /** Distance threshold used to compare pixel color values. */
        const uint32_t kColorDistanceThreshold = 16;

        /** Acceptable percentage of pixels that can be different from the gold image. */
        const float kPercentPixelDifferenceThreshold = 0.05f;

        /** Runs the verification. */
        VerifierResult pixelColorsAreNearby(SkBitmap& gold, SkBitmap& actual);
    };

    /**
     * Verifier that checks that no pixels appear outside of the added region(s). If 'inverted',
     * instead checks that no pixels appear within the added region(s).
     */
    class CheckNoPixelsOutsideRegion : public GMVerifier {
    public:
        CheckNoPixelsOutsideRegion();

        SkString name() const override;

        VerifierResult verifyImpl(const SkBitmap& gold, const SkBitmap& actual) override;

        /** Inverts the added region(s). */
        void toggleInverted();

    private:
        /** Background color used to determine whether pixels were drawn or not. */
        const SkColor kBackgroundColor = SK_ColorWHITE;

        bool fInverted;

        VerifierResult checkRegion(SkBitmap& actual, const SkIRect& region);
    };

    /** A list of GM verifiers. */
    class GMVerifiers {
    public:
        void add(std::unique_ptr<GMVerifier> verifier);

        VerifierResult verify(const SkBitmap& gold, const SkBitmap& actual);

        const SkBitmap* goldImg() const;

        const SkBitmap* imgUnderTest() const;
    private:
        std::vector<std::unique_ptr<GMVerifier>> fVerifiers;
        const GMVerifier* fFailedVerifier;
    };
}

#endif
