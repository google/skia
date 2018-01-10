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

class SkCodec;

/**
 *  Thread unsafe drawable for drawing animated images (e.g. GIF).
 */
class SkAnimatedImage : public SkDrawable {
public:
    /**
     *  Create an SkAnimatedImage from the SkCodec.
     *
     *  Returns null on failure to allocate pixels. On success, this will
     *  decode the first frame. It will not animate until start() is called.
     */
    static sk_sp<SkAnimatedImage> MakeFromCodec(std::unique_ptr<SkCodec>);

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
     *  Update the current time. If the image is animating, this may decode
     *  a new frame.
     */
    void update(double msecs);

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

    std::unique_ptr<SkCodec> fCodec;
    const SkImageInfo        fInfo;
    bool                     fFinished;
    bool                     fRunning;
    double                   fNowMS;
    double                   fRemainingMS;
    Frame                    fActiveFrame;
    Frame                    fRestoreFrame;

    SkAnimatedImage(std::unique_ptr<SkCodec>);

    typedef SkDrawable INHERITED;
};

#endif // SkAnimatedImage_DEFINED
