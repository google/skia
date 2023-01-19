/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFrameHolder_DEFINED
#define SkFrameHolder_DEFINED

#include "include/codec/SkCodec.h"
#include "include/codec/SkCodecAnimation.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkNoncopyable.h"

/**
 *  Base class for a single frame of an animated image.
 *
 *  Separate from SkCodec::FrameInfo, which is a pared down
 *  interface that only contains the info the client needs.
 */
class SkFrame : public SkNoncopyable {
public:
    SkFrame(int id)
        : fId(id)
        , fHasAlpha(false)
        , fRequiredFrame(kUninitialized)
        , fDisposalMethod(SkCodecAnimation::DisposalMethod::kKeep)
        , fDuration(0)
        , fBlend(SkCodecAnimation::Blend::kSrcOver)
    {
        fRect.setEmpty();
    }

    virtual ~SkFrame() {}

    /**
     * An explicit move constructor, as
     * https://en.cppreference.com/w/cpp/language/move_constructor says that
     * there is no implicit move constructor if there are user-declared
     * destructors, and we have one, immediately above.
     *
     * Without a move constructor, it is harder to use an SkFrame, or an
     * SkFrame subclass, inside a std::vector.
     */
    SkFrame(SkFrame&&) = default;

    /**
     *  0-based index of the frame in the image sequence.
     */
    int frameId() const { return fId; }

    /**
     *  How this frame reports its alpha.
     *
     *  This only considers the rectangle of this frame, and
     *  considers it to have alpha even if it is opaque once
     *  blended with the frame behind it.
     */
    SkEncodedInfo::Alpha reportedAlpha() const {
        return this->onReportedAlpha();
    }

    /**
     *  Cached value representing whether the frame has alpha,
     *  after compositing with the prior frame.
     */
    bool hasAlpha() const { return fHasAlpha; }

    /**
     *  Cache whether the finished frame has alpha.
     */
    void setHasAlpha(bool alpha) { fHasAlpha = alpha; }

    /**
     *  Whether enough of the frame has been read to determine
     *  fRequiredFrame and fHasAlpha.
     */
    bool reachedStartOfData() const { return fRequiredFrame != kUninitialized; }

    /**
     *  The frame this one depends on.
     *
     *  Must not be called until fRequiredFrame has been set properly.
     */
    int getRequiredFrame() const {
        SkASSERT(this->reachedStartOfData());
        return fRequiredFrame;
    }

    /**
     *  Set the frame that this frame depends on.
     */
    void setRequiredFrame(int req) { fRequiredFrame = req; }

    /**
     *  Set the rectangle that is updated by this frame.
     */
    void setXYWH(int x, int y, int width, int height) {
        fRect.setXYWH(x, y, width, height);
    }

    /**
     *  The rectangle that is updated by this frame.
     */
    SkIRect frameRect() const { return fRect; }

    int xOffset() const { return fRect.x(); }
    int yOffset() const { return fRect.y(); }
    int width()   const { return fRect.width(); }
    int height()  const { return fRect.height(); }

    SkCodecAnimation::DisposalMethod getDisposalMethod() const {
        return fDisposalMethod;
    }

    void setDisposalMethod(SkCodecAnimation::DisposalMethod disposalMethod) {
        fDisposalMethod = disposalMethod;
    }

    /**
     * Set the duration (in ms) to show this frame.
     */
    void setDuration(int duration) {
        fDuration = duration;
    }

    /**
     *  Duration in ms to show this frame.
     */
    int getDuration() const {
        return fDuration;
    }

    void setBlend(SkCodecAnimation::Blend blend) {
        fBlend = blend;
    }

    SkCodecAnimation::Blend getBlend() const {
        return fBlend;
    }

    /**
     * Fill in the FrameInfo with details from this object.
     */
    void fillIn(SkCodec::FrameInfo*, bool fullyReceived) const;

protected:
    virtual SkEncodedInfo::Alpha onReportedAlpha() const = 0;

private:
    inline static constexpr int kUninitialized = -2;

    const int                           fId;
    bool                                fHasAlpha;
    int                                 fRequiredFrame;
    SkIRect                             fRect;
    SkCodecAnimation::DisposalMethod    fDisposalMethod;
    int                                 fDuration;
    SkCodecAnimation::Blend             fBlend;
};

/**
 *  Base class for an object which holds the SkFrames of an
 *  image sequence.
 */
class SkFrameHolder : public SkNoncopyable {
public:
    SkFrameHolder()
        : fScreenWidth(0)
        , fScreenHeight(0)
    {}

    virtual ~SkFrameHolder() {}

    /**
     *  Size of the image. Each frame will be contained in
     *  these dimensions (possibly after clipping).
     */
    int screenWidth() const { return fScreenWidth; }
    int screenHeight() const { return fScreenHeight; }

    /**
     *  Compute the opacity and required frame, based on
     *  the frame's reportedAlpha and how it blends
     *  with prior frames.
     */
    void setAlphaAndRequiredFrame(SkFrame*);

    /**
     *  Return the frame with frameId i.
     */
    const SkFrame* getFrame(int i) const {
        return this->onGetFrame(i);
    }

protected:
    int fScreenWidth;
    int fScreenHeight;

    virtual const SkFrame* onGetFrame(int i) const = 0;
};

#endif // SkFrameHolder_DEFINED
