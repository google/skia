/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAvifCodec_DEFINED
#define SkAvifCodec_DEFINED

#include "include/codec/SkEncodedImageFormat.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkEncodedInfo.h"
#include "src/codec/SkFrameHolder.h"
#include "src/codec/SkScalingCodec.h"

#include <cstddef>
#include <memory>
#include <vector>

class SkCodec;
class SkStream;
struct SkImageInfo;
struct avifDecoder;
struct AvifDecoderDeleter {
    void operator()(avifDecoder* decoder) const;
};
using AvifDecoder = std::unique_ptr<avifDecoder, AvifDecoderDeleter>;

class SkAvifCodec : public SkScalingCodec {
public:
    /*
     * Returns true if an AVIF image is detected. Returns false otherwise.
     */
    static bool IsAvif(const void*, size_t);

    /*
     * Assumes IsAvif() was called and it returned true.
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

protected:
    Result onGetPixels(const SkImageInfo& dstInfo,
                       void* dst,
                       size_t dstRowBytes,
                       const Options& options,
                       int* rowsDecoded) override;

    SkEncodedImageFormat onGetEncodedFormat() const override { return SkEncodedImageFormat::kAVIF; }

    int onGetFrameCount() override;
    bool onGetFrameInfo(int, FrameInfo*) const override;
    int onGetRepetitionCount() override;
    IsAnimated onIsAnimated() override;
    const SkFrameHolder* getFrameHolder() const override { return &fFrameHolder; }

private:
    SkAvifCodec(SkEncodedInfo&&,
                std::unique_ptr<SkStream>,
                sk_sp<SkData>,
                AvifDecoder,
                SkEncodedOrigin,
                bool);

    // fAvifDecoder has a pointer to this data. This should not be freed until
    // the decode is completed. To ensure that, we declare this before
    // fAvifDecoder.
    sk_sp<SkData> fData;

    AvifDecoder fAvifDecoder;
    bool fUseAnimation;

    class Frame : public SkFrame {
    public:
        Frame(int i, SkEncodedInfo::Alpha alpha) : INHERITED(i), fReportedAlpha(alpha) {}

    protected:
        SkEncodedInfo::Alpha onReportedAlpha() const override { return fReportedAlpha; }

    private:
        const SkEncodedInfo::Alpha fReportedAlpha;

        using INHERITED = SkFrame;
    };

    class FrameHolder : public SkFrameHolder {
    public:
        ~FrameHolder() override {}
        void setScreenSize(int w, int h) {
            fScreenWidth = w;
            fScreenHeight = h;
        }
        Frame* appendNewFrame(bool hasAlpha);
        const Frame* frame(int i) const;
        int size() const { return static_cast<int>(fFrames.size()); }
        void reserve(int size) { fFrames.reserve(size); }

    protected:
        const SkFrame* onGetFrame(int i) const override;

    private:
        std::vector<Frame> fFrames;
    };

    FrameHolder fFrameHolder;
    using INHERITED = SkScalingCodec;
};

#endif  // SkAvifCodec_DEFINED
