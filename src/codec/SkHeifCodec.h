/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHeifCodec_DEFINED
#define SkHeifCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "src/codec/SkFrameHolder.h"
#include "src/codec/SkSwizzler.h"

#if __has_include("HeifDecoderAPI.h")
    #include "HeifDecoderAPI.h"
#else
    #include "src/codec/SkStubHeifDecoderAPI.h"
#endif

class SkHeifCodec : public SkCodec {
public:
    /*
     * Returns true if one of kHEIF or kAVIF images were detected. If |format|
     * is not nullptr, it will contain the detected format. Returns false
     * otherwise.
     */
    static bool IsSupported(const void*, size_t, SkEncodedImageFormat* format);

    /*
     * Assumes IsSupported was called and it returned a non-nullopt value.
     */
    static std::unique_ptr<SkCodec> MakeFromStream(
            std::unique_ptr<SkStream>, SkCodec::SelectionPolicy selectionPolicy,
            SkEncodedImageFormat, Result*);

protected:

    Result onGetPixels(
            const SkImageInfo& dstInfo,
            void* dst, size_t dstRowBytes,
            const Options& options,
            int* rowsDecoded) override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return fFormat;
    }

    int onGetFrameCount() override;
    bool onGetFrameInfo(int, FrameInfo*) const override;
    int onGetRepetitionCount() override;
    const SkFrameHolder* getFrameHolder() const override {
        return &fFrameHolder;
    }

    bool conversionSupported(const SkImageInfo&, bool, bool) override;

    bool onRewind() override;

private:
    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream
     */
    SkHeifCodec(SkEncodedInfo&&, HeifDecoder*, SkEncodedOrigin, bool animation,
            SkEncodedImageFormat);

    void initializeSwizzler(const SkImageInfo& dstInfo, const Options& options);
    void allocateStorage(const SkImageInfo& dstInfo);
    int readRows(const SkImageInfo& dstInfo, void* dst,
            size_t rowBytes, int count, const Options&);

    /*
     * Scanline decoding.
     */
    SkSampler* getSampler(bool createIfNecessary) override;
    Result onStartScanlineDecode(const SkImageInfo& dstInfo,
            const Options& options) override;
    int onGetScanlines(void* dst, int count, size_t rowBytes) override;
    bool onSkipScanlines(int count) override;

    std::unique_ptr<HeifDecoder>       fHeifDecoder;
    HeifFrameInfo                      fFrameInfo;
    SkAutoTMalloc<uint8_t>             fStorage;
    uint8_t*                           fSwizzleSrcRow;
    uint32_t*                          fColorXformSrcRow;

    std::unique_ptr<SkSwizzler>        fSwizzler;
    bool                               fUseAnimation;
    const SkEncodedImageFormat         fFormat;

    class Frame : public SkFrame {
    public:
        Frame(int i) : INHERITED(i) {}

    protected:
        SkEncodedInfo::Alpha onReportedAlpha() const override {
            return SkEncodedInfo::Alpha::kOpaque_Alpha;
        }

    private:
        using INHERITED = SkFrame;
    };

    class FrameHolder : public SkFrameHolder {
    public:
        ~FrameHolder() override {}
        void setScreenSize(int w, int h) {
            fScreenWidth = w;
            fScreenHeight = h;
        }
        Frame* appendNewFrame();
        const Frame* frame(int i) const;
        Frame* editFrameAt(int i);
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
    using INHERITED = SkCodec;
};

#endif // SkHeifCodec_DEFINED
