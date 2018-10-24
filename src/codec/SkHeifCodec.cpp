/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#ifdef SK_HAS_HEIF_LIBRARY
#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkColorData.h"
#include "SkEndian.h"
#include "SkStream.h"
#include "SkHeifCodec.h"

#define FOURCC(c1, c2, c3, c4) \
    ((c1) << 24 | (c2) << 16 | (c3) << 8 | (c4))

bool SkHeifCodec::IsHeif(const void* buffer, size_t bytesRead) {
    // Parse the ftyp box up to bytesRead to determine if this is HEIF.
    // Any valid ftyp box should have at least 8 bytes.
    if (bytesRead < 8) {
        return false;
    }

    uint32_t* ptr = (uint32_t*)buffer;
    uint64_t chunkSize = SkEndian_SwapBE32(ptr[0]);
    uint32_t chunkType = SkEndian_SwapBE32(ptr[1]);

    if (chunkType != FOURCC('f', 't', 'y', 'p')) {
        return false;
    }

    int64_t offset = 8;
    if (chunkSize == 1) {
        // This indicates that the next 8 bytes represent the chunk size,
        // and chunk data comes after that.
        if (bytesRead < 16) {
            return false;
        }
        auto* chunkSizePtr = SkTAddOffset<const uint64_t>(buffer, offset);
        chunkSize = SkEndian_SwapBE64(*chunkSizePtr);
        if (chunkSize < 16) {
            // The smallest valid chunk is 16 bytes long in this case.
            return false;
        }
        offset += 8;
    } else if (chunkSize < 8) {
        // The smallest valid chunk is 8 bytes long.
        return false;
    }

    if (chunkSize > bytesRead) {
        chunkSize = bytesRead;
    }
    int64_t chunkDataSize = chunkSize - offset;
    // It should at least have major brand (4-byte) and minor version (4-bytes).
    // The rest of the chunk (if any) is a list of (4-byte) compatible brands.
    if (chunkDataSize < 8) {
        return false;
    }

    uint32_t numCompatibleBrands = (chunkDataSize - 8) / 4;
    for (size_t i = 0; i < numCompatibleBrands + 2; ++i) {
        if (i == 1) {
            // Skip this index, it refers to the minorVersion,
            // not a brand.
            continue;
        }
        auto* brandPtr = SkTAddOffset<const uint32_t>(buffer, offset + 4 * i);
        uint32_t brand = SkEndian_SwapBE32(*brandPtr);
        if (brand == FOURCC('m', 'i', 'f', '1') || brand == FOURCC('h', 'e', 'i', 'c')
         || brand == FOURCC('m', 's', 'f', '1') || brand == FOURCC('h', 'e', 'v', 'c')) {
            return true;
        }
    }
    return false;
}

static SkEncodedOrigin get_orientation(const HeifFrameInfo& frameInfo) {
    switch (frameInfo.mRotationAngle) {
        case 0:   return kTopLeft_SkEncodedOrigin;
        case 90:  return kRightTop_SkEncodedOrigin;
        case 180: return kBottomRight_SkEncodedOrigin;
        case 270: return kLeftBottom_SkEncodedOrigin;
    }
    return kDefault_SkEncodedOrigin;
}

struct SkHeifStreamWrapper : public HeifStream {
    SkHeifStreamWrapper(SkStream* stream) : fStream(stream) {}

    ~SkHeifStreamWrapper() override {}

    size_t read(void* buffer, size_t size) override {
        return fStream->read(buffer, size);
    }

    bool rewind() override {
        return fStream->rewind();
    }

    bool seek(size_t position) override {
        return fStream->seek(position);
    }

    bool hasLength() const override {
        return fStream->hasLength();
    }

    size_t getLength() const override {
        return fStream->getLength();
    }

private:
    std::unique_ptr<SkStream> fStream;
};

std::unique_ptr<SkCodec> SkHeifCodec::MakeFromStream(
        std::unique_ptr<SkStream> stream, Result* result) {
    std::unique_ptr<HeifDecoder> heifDecoder(createHeifDecoder());
    if (heifDecoder.get() == nullptr) {
        *result = kInternalError;
        return nullptr;
    }

    HeifFrameInfo frameInfo;
    if (!heifDecoder->init(new SkHeifStreamWrapper(stream.release()),
                           &frameInfo)) {
        *result = kInvalidInput;
        return nullptr;
    }

    std::unique_ptr<SkEncodedInfo::ICCProfile> profile = nullptr;
    if ((frameInfo.mIccSize > 0) && (frameInfo.mIccData != nullptr)) {
        // FIXME: Would it be possible to use MakeWithoutCopy?
        auto icc = SkData::MakeWithCopy(frameInfo.mIccData.get(), frameInfo.mIccSize);
        profile = SkEncodedInfo::ICCProfile::Make(std::move(icc));
    }
    if (profile && profile->profile()->data_color_space != skcms_Signature_RGB) {
        // This will result in sRGB.
        profile = nullptr;
    }

    SkEncodedInfo info = SkEncodedInfo::Make(frameInfo.mWidth, frameInfo.mHeight,
            SkEncodedInfo::kYUV_Color, SkEncodedInfo::kOpaque_Alpha, 8, std::move(profile));
    SkEncodedOrigin orientation = get_orientation(frameInfo);

    *result = kSuccess;
    return std::unique_ptr<SkCodec>(new SkHeifCodec(std::move(info), heifDecoder.release(),
                                                    orientation));
}

SkHeifCodec::SkHeifCodec(SkEncodedInfo&& info, HeifDecoder* heifDecoder, SkEncodedOrigin origin)
    : INHERITED(std::move(info), skcms_PixelFormat_RGBA_8888, nullptr, origin)
    , fHeifDecoder(heifDecoder)
    , fSwizzleSrcRow(nullptr)
    , fColorXformSrcRow(nullptr)
{}


bool SkHeifCodec::conversionSupported(const SkImageInfo& dstInfo, bool srcIsOpaque,
                                      bool needsColorXform) {
    SkASSERT(srcIsOpaque);

    if (kUnknown_SkAlphaType == dstInfo.alphaType()) {
        return false;
    }

    if (kOpaque_SkAlphaType != dstInfo.alphaType()) {
        SkCodecPrintf("Warning: an opaque image should be decoded as opaque "
                "- it is being decoded as non-opaque, which will draw slower\n");
    }

    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            return fHeifDecoder->setOutputColor(kHeifColorFormat_RGBA_8888);

        case kBGRA_8888_SkColorType:
            return fHeifDecoder->setOutputColor(kHeifColorFormat_BGRA_8888);

        case kRGB_565_SkColorType:
            if (needsColorXform) {
                return fHeifDecoder->setOutputColor(kHeifColorFormat_RGBA_8888);
            } else {
                return fHeifDecoder->setOutputColor(kHeifColorFormat_RGB565);
            }

        case kRGBA_F16_SkColorType:
            SkASSERT(needsColorXform);
            return fHeifDecoder->setOutputColor(kHeifColorFormat_RGBA_8888);

        default:
            return false;
    }
}

int SkHeifCodec::readRows(const SkImageInfo& dstInfo, void* dst, size_t rowBytes, int count,
                          const Options& opts) {
    // When fSwizzleSrcRow is non-null, it means that we need to swizzle.  In this case,
    // we will always decode into fSwizzlerSrcRow before swizzling into the next buffer.
    // We can never swizzle "in place" because the swizzler may perform sampling and/or
    // subsetting.
    // When fColorXformSrcRow is non-null, it means that we need to color xform and that
    // we cannot color xform "in place" (many times we can, but not when the dst is F16).
    // In this case, we will color xform from fColorXformSrcRow into the dst.
    uint8_t* decodeDst = (uint8_t*) dst;
    uint32_t* swizzleDst = (uint32_t*) dst;
    size_t decodeDstRowBytes = rowBytes;
    size_t swizzleDstRowBytes = rowBytes;
    int dstWidth = opts.fSubset ? opts.fSubset->width() : dstInfo.width();
    if (fSwizzleSrcRow && fColorXformSrcRow) {
        decodeDst = fSwizzleSrcRow;
        swizzleDst = fColorXformSrcRow;
        decodeDstRowBytes = 0;
        swizzleDstRowBytes = 0;
        dstWidth = fSwizzler->swizzleWidth();
    } else if (fColorXformSrcRow) {
        decodeDst = (uint8_t*) fColorXformSrcRow;
        swizzleDst = fColorXformSrcRow;
        decodeDstRowBytes = 0;
        swizzleDstRowBytes = 0;
    } else if (fSwizzleSrcRow) {
        decodeDst = fSwizzleSrcRow;
        decodeDstRowBytes = 0;
        dstWidth = fSwizzler->swizzleWidth();
    }

    for (int y = 0; y < count; y++) {
        if (!fHeifDecoder->getScanline(decodeDst)) {
            return y;
        }

        if (fSwizzler) {
            fSwizzler->swizzle(swizzleDst, decodeDst);
        }

        if (this->colorXform()) {
            this->applyColorXform(dst, swizzleDst, dstWidth);
            dst = SkTAddOffset<void>(dst, rowBytes);
        }

        decodeDst = SkTAddOffset<uint8_t>(decodeDst, decodeDstRowBytes);
        swizzleDst = SkTAddOffset<uint32_t>(swizzleDst, swizzleDstRowBytes);
    }

    return count;
}

/*
 * Performs the heif decode
 */
SkCodec::Result SkHeifCodec::onGetPixels(const SkImageInfo& dstInfo,
                                         void* dst, size_t dstRowBytes,
                                         const Options& options,
                                         int* rowsDecoded) {
    if (options.fSubset) {
        // Not supporting subsets on this path for now.
        // TODO: if the heif has tiles, we can support subset here, but
        // need to retrieve tile config from metadata retriever first.
        return kUnimplemented;
    }

    if (!fHeifDecoder->decode(&fFrameInfo)) {
        return kInvalidInput;
    }

    fSwizzler.reset(nullptr);
    this->allocateStorage(dstInfo);

    int rows = this->readRows(dstInfo, dst, dstRowBytes, dstInfo.height(), options);
    if (rows < dstInfo.height()) {
        *rowsDecoded = rows;
        return kIncompleteInput;
    }

    return kSuccess;
}

void SkHeifCodec::allocateStorage(const SkImageInfo& dstInfo) {
    int dstWidth = dstInfo.width();

    size_t swizzleBytes = 0;
    if (fSwizzler) {
        swizzleBytes = fFrameInfo.mBytesPerPixel * fFrameInfo.mWidth;
        dstWidth = fSwizzler->swizzleWidth();
        SkASSERT(!this->colorXform() || SkIsAlign4(swizzleBytes));
    }

    size_t xformBytes = 0;
    if (this->colorXform() && (kRGBA_F16_SkColorType == dstInfo.colorType() ||
                               kRGB_565_SkColorType == dstInfo.colorType())) {
        xformBytes = dstWidth * sizeof(uint32_t);
    }

    size_t totalBytes = swizzleBytes + xformBytes;
    fStorage.reset(totalBytes);
    if (totalBytes > 0) {
        fSwizzleSrcRow = (swizzleBytes > 0) ? fStorage.get() : nullptr;
        fColorXformSrcRow = (xformBytes > 0) ?
                SkTAddOffset<uint32_t>(fStorage.get(), swizzleBytes) : nullptr;
    }
}

void SkHeifCodec::initializeSwizzler(
        const SkImageInfo& dstInfo, const Options& options) {
    SkImageInfo swizzlerDstInfo = dstInfo;
    if (this->colorXform()) {
        // The color xform will be expecting RGBA 8888 input.
        swizzlerDstInfo = swizzlerDstInfo.makeColorType(kRGBA_8888_SkColorType);
    }

    int srcBPP = 4;
    if (dstInfo.colorType() == kRGB_565_SkColorType && !this->colorXform()) {
        srcBPP = 2;
    }

    fSwizzler = SkSwizzler::MakeSimple(srcBPP, swizzlerDstInfo, options);
    SkASSERT(fSwizzler);
}

SkSampler* SkHeifCodec::getSampler(bool createIfNecessary) {
    if (!createIfNecessary || fSwizzler) {
        SkASSERT(!fSwizzler || (fSwizzleSrcRow && fStorage.get() == fSwizzleSrcRow));
        return fSwizzler.get();
    }

    this->initializeSwizzler(this->dstInfo(), this->options());
    this->allocateStorage(this->dstInfo());
    return fSwizzler.get();
}

SkCodec::Result SkHeifCodec::onStartScanlineDecode(
        const SkImageInfo& dstInfo, const Options& options) {
    // TODO: For now, just decode the whole thing even when there is a subset.
    // If the heif image has tiles, we could potentially do this much faster,
    // but the tile configuration needs to be retrieved from the metadata.
    if (!fHeifDecoder->decode(&fFrameInfo)) {
        return kInvalidInput;
    }

    if (options.fSubset) {
        this->initializeSwizzler(dstInfo, options);
    } else {
        fSwizzler.reset(nullptr);
    }

    this->allocateStorage(dstInfo);

    return kSuccess;
}

int SkHeifCodec::onGetScanlines(void* dst, int count, size_t dstRowBytes) {
    return this->readRows(this->dstInfo(), dst, dstRowBytes, count, this->options());
}

bool SkHeifCodec::onSkipScanlines(int count) {
    return count == (int) fHeifDecoder->skipScanlines(count);
}

#endif // SK_HAS_HEIF_LIBRARY
