/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAnimatedImage_DEFINED
#define SkAnimatedImage_DEFINED

#include "SkBitmap.h"
#include "SkCodecAnimation.h"
#include "SkDrawable.h"
#include "SkMatrix.h"
#include "SkRect.h"

class SkAndroidCodec;
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
     *  decode the first frame. It will not animate until start() is called.
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
     *  Start or resume the animation. update() must be called to advance the
     *  time.
     */
    void start();

    /**
     *  Stop the animation. update() has no effect while the animation is
     *  stopped.
     */
    void stop();

    /**
     *  Reset the animation to the beginning.
     */
    void reset();

    /**
     *  Whether the animation is active.
     *
     *  If true, update() can be called to animate.
     */
    bool isRunning() const { return fRunning && !fFinished; }

    /**
     *  Update the current time. If the image is animating, this may decode
     *  a new frame.
     *
     *  @return the time to show the next frame.
     *      Returns numeric_limits<double>::max() if there is no max frame to
     *      show, and -1.0 if the animation is not running.
     */
    double update(double msecs);

protected:
    SkRect onGetBounds() override;
    void onDraw(SkCanvas*) override;

private:
    struct Frame {
        SkBitmap fBitmap;
        int      fIndex;
        SkCodecAnimation::DisposalMethod fDisposalMethod;

        Frame();
        bool copyTo(Frame*) const;
    };

    std::unique_ptr<SkAndroidCodec> fCodec;
    const SkISize                   fScaledSize;
    const SkImageInfo               fDecodeInfo;
    const SkIRect                   fCropRect;
    const sk_sp<SkPicture>          fPostProcess;
    const bool                      fSimple;     // no crop, scale, or postprocess
    SkMatrix                        fMatrix;     // used only if !fSimple

    bool                            fFinished;
    bool                            fRunning;
    double                          fNowMS;
    double                          fRemainingMS;
    Frame                           fActiveFrame;
    Frame                           fRestoreFrame;

    SkAnimatedImage(std::unique_ptr<SkAndroidCodec>, SkISize scaledSize,
            SkImageInfo decodeInfo, SkIRect cropRect, sk_sp<SkPicture> postProcess);

    typedef SkDrawable INHERITED;
};

#endif // SkAnimatedImage_DEFINED
