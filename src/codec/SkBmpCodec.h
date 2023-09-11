/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBmpCodec_DEFINED
#define SkBmpCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkColorType.h"
#include "include/core/SkTypes.h"
#include "modules/skcms/skcms.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class SkStream;
struct SkEncodedInfo;
struct SkImageInfo;

/*
 * This class enables code sharing between its bmp codec subclasses.  The
 * subclasses actually do the work.
 */
class SkBmpCodec : public SkCodec {
public:
    static bool IsBmp(const void*, size_t);

    /*
     * Assumes IsBmp was called and returned true
     * Creates a bmp decoder
     * Reads enough of the stream to determine the image format
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    /*
     * Creates a bmp decoder for a bmp embedded in ico
     * Reads enough of the stream to determine the image format
     */
    static std::unique_ptr<SkCodec> MakeFromIco(std::unique_ptr<SkStream>, Result*);

protected:

    SkBmpCodec(SkEncodedInfo&& info, std::unique_ptr<SkStream>,
            uint16_t bitsPerPixel, SkCodec::SkScanlineOrder rowOrder);

    SkEncodedImageFormat onGetEncodedFormat() const override { return SkEncodedImageFormat::kBMP; }

    /*
     * Read enough of the stream to initialize the SkBmpCodec.
     * On kSuccess, if codecOut is not nullptr, it will be set to a new SkBmpCodec.
     */
    static Result ReadHeader(SkStream*, bool inIco, std::unique_ptr<SkCodec>* codecOut);

    bool onRewind() override;

    /*
     * Returns whether this BMP is part of an ICO image.
     */
    bool inIco() const {
        return this->onInIco();
    }

    virtual bool onInIco() const {
        return false;
    }

    /*
     * Get the destination row number corresponding to the encoded row number.
     * For kTopDown, we simply return y, but for kBottomUp, the rows will be
     * decoded in reverse order.
     *
     * @param y      Iterates from 0 to height, indicating the current row.
     * @param height The height of the current subset of the image that we are
     *               decoding.  This is generally equal to the full height
     *               when we want to decode the full or one when we are
     *               sampling.
     */
    int32_t getDstRow(int32_t y, int32_t height) const;

    /*
     * Accessors used by subclasses
     */
    uint16_t bitsPerPixel() const { return fBitsPerPixel; }
    SkScanlineOrder onGetScanlineOrder() const override { return fRowOrder; }
    size_t srcRowBytes() const { return fSrcRowBytes; }

    /*
     * To be overriden by bmp subclasses, which provide unique implementations.
     * Performs subclass specific setup.
     *
     * @param dstInfo         Contains output information.  Height specifies
     *                        the total number of rows that will be decoded.
     * @param options         Additonal options to pass to the decoder.
     */
    virtual SkCodec::Result onPrepareToDecode(const SkImageInfo& dstInfo,
            const SkCodec::Options& options) = 0;
    SkCodec::Result prepareToDecode(const SkImageInfo& dstInfo,
            const SkCodec::Options& options);

    uint32_t* xformBuffer() const { return fXformBuffer.get(); }
    void resetXformBuffer(int count) { fXformBuffer.reset(new uint32_t[count]); }

    /*
     * BMPs are typically encoded as BGRA/BGR so this is a more efficient choice
     * than RGBA.
     */
    inline static constexpr SkColorType kXformSrcColorType = kBGRA_8888_SkColorType;
    inline static constexpr auto kXformSrcColorFormat = skcms_PixelFormat_BGRA_8888;

private:

    /*
     * Creates a bmp decoder
     * Reads enough of the stream to determine the image format
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*, bool inIco);

    /*
     * Decodes the next dstInfo.height() lines.
     *
     * onGetPixels() uses this for full image decodes.
     * SkScaledCodec::onGetPixels() uses the scanline decoder to call this with
     * dstInfo.height() = 1, in order to implement sampling.
     * A potential future use is to allow the caller to decode a subset of the
     * lines in the image.
     *
     * @param dstInfo     Contains output information.  Height specifies the
     *                    number of rows to decode at this time.
     * @param dst         Memory location to store output pixels
     * @param dstRowBytes Bytes in a row of the destination
     * @return            Number of rows successfully decoded
     */
    virtual int decodeRows(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes,
            const Options& opts) = 0;

    virtual bool skipRows(int count);

    Result onStartScanlineDecode(const SkImageInfo& dstInfo,
            const SkCodec::Options&) override;

    int onGetScanlines(void* dst, int count, size_t rowBytes) override;

    bool onSkipScanlines(int count) override;

    const uint16_t              fBitsPerPixel;
    const SkScanlineOrder       fRowOrder;
    const size_t                fSrcRowBytes;
    std::unique_ptr<uint32_t[]> fXformBuffer;

    using INHERITED = SkCodec;
};

#endif
