/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWebpCodec_DEFINED
#define SkWebpCodec_DEFINED

#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkEncodedImageFormat.h"
#include "SkFrameHolder.h"
#include "SkImageInfo.h"
#include "SkTypes.h"

#include <vector>

class SkStream;
extern "C" {
    struct WebPDemuxer;
    void WebPDemuxDelete(WebPDemuxer* dmux);
}

static const size_t WEBP_VP8_HEADER_SIZE = 30;

class SkWebpCodec final : public SkCodec {
public:
    // Assumes IsWebp was called and returned true.
    static SkCodec* NewFromStream(SkStream*, Result*);
    static bool IsWebp(const void*, size_t);
protected:
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, int*) override;
    SkEncodedImageFormat onGetEncodedFormat() const override { return SkEncodedImageFormat::kWEBP; }

    SkISize onGetScaledDimensions(float desiredScale) const override;

    bool onDimensionsSupported(const SkISize&) override;

    bool onGetValidSubset(SkIRect* /* desiredSubset */) const override;

    int onGetFrameCount() override;
    bool onGetFrameInfo(int, FrameInfo*) const override;
    int onGetRepetitionCount() override;

    const SkFrameHolder* getFrameHolder() const override {
        return &fFrameHolder;
    }

private:
    SkWebpCodec(int width, int height, const SkEncodedInfo&, sk_sp<SkColorSpace>, SkStream*,
                WebPDemuxer*, sk_sp<SkData>);

    SkAutoTCallVProc<WebPDemuxer, WebPDemuxDelete> fDemux;

    // fDemux has a pointer into this data.
    // This should not be freed until the decode is completed.
    sk_sp<SkData> fData;

    class Frame : public SkFrame {
    public:
        Frame(int i, bool alpha)
            : INHERITED(i)
            , fReportsAlpha(alpha)
        {}
        Frame(Frame&& other)
            : INHERITED(other.frameId())
            , fReportsAlpha(other.fReportsAlpha)
        {}

    protected:
        bool onReportsAlpha() const override {
            return fReportsAlpha;
        }

    private:
        const bool fReportsAlpha;

        typedef SkFrame INHERITED;
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
        int size() const {
            return static_cast<int>(fFrames.size());
        }
        void reserve(int size) {
            fFrames.reserve(size);
        }

    protected:
        const SkFrame* onGetFrame(int i) const override;

    private:
        std::vector<Frame> fFrames;
    };

    FrameHolder fFrameHolder;
    // Set to true if WebPDemuxGetFrame fails. This only means
    // that we will cap the frame count to the frames that
    // succeed.
    bool        fFailed;

    typedef SkCodec INHERITED;
};
#endif // SkWebpCodec_DEFINED
