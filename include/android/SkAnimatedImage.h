/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAnimatedImage_DEFINED
#define SkAnimatedImage_DEFINED

#include "include/codec/SkCodecAnimation.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"

class SkAndroidCodec;
class SkImage;
class SkPicture;

/**
 *  Thread unsafe drawable for drawing animated images (e.g. GIF).
 */
class SK_API SkAnimatedImage : public SkDrawable {
public:
    /**
     *  Create an SkAnimatedImage from the SkAndroidCodec.
     *
     *  Returns null on failure to allocate pixels. On success, this will
     *  decode the first frame.
     *
     *  @param info Width and height may require scaling.
     *  @param cropRect Rectangle to crop to after scaling.
     *  @param postProcess Picture to apply after scaling and cropping.
     */
    static sk_sp<SkAnimatedImage> Make(std::unique_ptr<SkAndroidCodec>,
            const SkImageInfo& info, SkIRect cropRect, sk_sp<SkPicture> postProcess);

    /**
     *  Create an SkAnimatedImage from the SkAndroidCodec.
     *
     *  Returns null on failure to allocate pixels. On success, this will
     *  decode the first frame.
     *
     *  @param scaledSize Size to draw the image, possibly requiring scaling.
     *  @param cropRect Rectangle to crop to after scaling.
     *  @param postProcess Picture to apply after scaling and cropping.
     */
    static sk_sp<SkAnimatedImage> Make(std::unique_ptr<SkAndroidCodec>,
            SkISize scaledSize, SkIRect cropRect, sk_sp<SkPicture> postProcess);

    /**
     *  Simpler version that uses the default size, no cropping, and no postProcess.
     */
    static sk_sp<SkAnimatedImage> Make(std::unique_ptr<SkAndroidCodec>);

    ~SkAnimatedImage() override;

    /**
     *  Reset the animation to the beginning.
     */
    void reset();

    /**
     *  Whether the animation completed.
     *
     *  Returns true after all repetitions are complete, or an error stops the
     *  animation. Gets reset to false if the animation is restarted.
     */
    bool isFinished() const { return fFinished; }

    /**
     * Returned by decodeNextFrame and currentFrameDuration if the animation
     * is not running.
     */
    static constexpr int kFinished = -1;

    /**
     *  Decode the next frame.
     *
     *  If the animation is on the last frame or has hit an error, returns
     *  kFinished.
     */
    int decodeNextFrame();

    /**
     *  Returns the current frame as an SkImage. The SkImage will not change
     *  after it has been returned.
     *  If there is no current frame, nullptr will be returned.
     */
    sk_sp<SkImage> getCurrentFrame();

    /**
     *  How long to display the current frame.
     *
     *  Useful for the first frame, for which decodeNextFrame is called
     *  internally.
     */
    int currentFrameDuration() {
        return fCurrentFrameDuration;
    }

    /**
     *  Change the repetition count.
     *
     *  By default, the image will repeat the number of times indicated in the
     *  encoded data.
     *
     *  Use SkCodec::kRepetitionCountInfinite for infinite, and 0 to show all
     *  frames once and then stop.
     */
    void setRepetitionCount(int count);

    /**
     *  Return the currently set repetition count.
     */
    int getRepetitionCount() const {
        return fRepetitionCount;
    }

    /**
     *  Return the total number of frames in the animation.
     */
    int getFrameCount() const { return fFrameCount; }

    /**
     * Return the (possibly scaled) dimensions of the image.
     */
    SkISize dimensions() const { return fScaledSize; }

protected:
    SkRect onGetBounds() override;
    void onDraw(SkCanvas*) override;

private:
    struct Frame {
        SkBitmap fBitmap;
        int      fIndex;
        SkCodecAnimation::DisposalMethod fDisposalMethod;

        // init() may have to create a new SkPixelRef, if the
        // current one is already in use by another owner (e.g.
        // an SkPicture). This determines whether to copy the
        // existing one to the new one.
        enum class OnInit {
            // Restore the image from the old SkPixelRef to the
            // new one.
            kRestoreIfNecessary,
            // No need to restore.
            kNoRestore,
        };

        Frame();
        bool init(const SkImageInfo& info, OnInit);
        bool copyTo(Frame*) const;
    };

    std::unique_ptr<SkAndroidCodec> fCodec;
    const SkISize                   fScaledSize;
    const SkImageInfo               fDecodeInfo;
    const SkIRect                   fCropRect;
    const sk_sp<SkPicture>          fPostProcess;
    const int                       fFrameCount;
    const bool                      fSimple;     // no crop, scale, or postprocess
    SkMatrix                        fMatrix;     // used only if !fSimple

    bool                            fFinished;
    int                             fCurrentFrameDuration;
    Frame                           fDisplayFrame;
    Frame                           fDecodingFrame;
    Frame                           fRestoreFrame;
    int                             fRepetitionCount;
    int                             fRepetitionsCompleted;

    SkAnimatedImage(std::unique_ptr<SkAndroidCodec>, SkISize scaledSize,
            SkImageInfo decodeInfo, SkIRect cropRect, sk_sp<SkPicture> postProcess);
    SkAnimatedImage(std::unique_ptr<SkAndroidCodec>);

    int computeNextFrame(int current, bool* animationEnded);
    double finish();

    typedef SkDrawable INHERITED;
};

#endif // SkAnimatedImage_DEFINED
