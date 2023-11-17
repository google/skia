/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/codec/SkHeifCodec.h"

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkEndian.h"
#include "src/codec/SkCodecPriv.h"

#define FOURCC(c1, c2, c3, c4) \
    ((c1) << 24 | (c2) << 16 | (c3) << 8 | (c4))

bool SkHeifCodec::IsSupported(const void* buffer, size_t bytesRead,
                              SkEncodedImageFormat* format) {
    // Parse the ftyp box up to bytesRead to determine if this is HEIF or AVIF.
    // Any valid ftyp box should have at least 8 bytes.
    if (bytesRead < 8) {
        return false;
    }

    const uint32_t* ptr = (const uint32_t*)buffer;
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
    bool isHeif = false;
    for (size_t i = 0; i < numCompatibleBrands + 2; ++i) {
        if (i == 1) {
            // Skip this index, it refers to the minorVersion,
            // not a brand.
            continue;
        }
        auto* brandPtr = SkTAddOffset<const uint32_t>(buffer, offset + 4 * i);
        uint32_t brand = SkEndian_SwapBE32(*brandPtr);
        if (brand == FOURCC('m', 'i', 'f', '1') || brand == FOURCC('h', 'e', 'i', 'c')
         || brand == FOURCC('m', 's', 'f', '1') || brand == FOURCC('h', 'e', 'v', 'c')
         || brand == FOURCC('a', 'v', 'i', 'f') || brand == FOURCC('a', 'v', 'i', 's')) {
            // AVIF files could have "mif1" as the major brand. So we cannot
            // distinguish whether the image is AVIF or HEIC just based on the
            // "mif1" brand. So wait until we see a specific avif brand to
            // determine whether it is AVIF or HEIC.
            isHeif = true;
            if (brand == FOURCC('a', 'v', 'i', 'f')
              || brand == FOURCC('a', 'v', 'i', 's')) {
                if (format != nullptr) {
                    *format = SkEncodedImageFormat::kAVIF;
                }
                return true;
            }
        }
    }
    if (isHeif) {
        if (format != nullptr) {
            *format = SkEncodedImageFormat::kHEIF;
        }
        return true;
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

static void releaseProc(const void* ptr, void* context) {
    delete reinterpret_cast<std::vector<uint8_t>*>(context);
}

std::unique_ptr<SkCodec> SkHeifCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
        SkCodec::SelectionPolicy selectionPolicy, Result* result) {
    SkASSERT(result);
    if (!stream) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }
    std::unique_ptr<HeifDecoder> heifDecoder(createHeifDecoder());
    if (heifDecoder == nullptr) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    constexpr size_t bytesToRead = MinBufferedBytesNeeded();
    char buffer[bytesToRead];
    size_t bytesRead = stream->peek(buffer, bytesToRead);
    if (0 == bytesRead) {
        // It is possible the stream does not support peeking, but does support rewinding.
        // Attempt to read() and pass the actual amount read to the decoder.
        bytesRead = stream->read(buffer, bytesToRead);
        if (!stream->rewind()) {
            SkCodecPrintf("Encoded image data could not peek or rewind to determine format!\n");
            *result = kCouldNotRewind;
            return nullptr;
        }
    }

    SkEncodedImageFormat format;
    if (!SkHeifCodec::IsSupported(buffer, bytesRead, &format)) {
        SkCodecPrintf("Failed to get format despite earlier detecting it");
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    HeifFrameInfo heifInfo;
    if (!heifDecoder->init(new SkHeifStreamWrapper(stream.release()), &heifInfo)) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }

    size_t frameCount = 1;
    if (selectionPolicy == SkCodec::SelectionPolicy::kPreferAnimation) {
        HeifFrameInfo sequenceInfo;
        if (heifDecoder->getSequenceInfo(&sequenceInfo, &frameCount) &&
                frameCount > 1) {
            heifInfo = std::move(sequenceInfo);
        }
    }

    std::unique_ptr<SkEncodedInfo::ICCProfile> profile = nullptr;
    if (heifInfo.mIccData.size() > 0) {
        auto iccData = new std::vector<uint8_t>(std::move(heifInfo.mIccData));
        auto icc = SkData::MakeWithProc(iccData->data(), iccData->size(), releaseProc, iccData);
        profile = SkEncodedInfo::ICCProfile::Make(std::move(icc));
    }
    if (profile && profile->profile()->data_color_space != skcms_Signature_RGB) {
        // This will result in sRGB.
        profile = nullptr;
    }

    uint8_t colorDepth = heifDecoder->getColorDepth();

    SkEncodedInfo info = SkEncodedInfo::Make(heifInfo.mWidth, heifInfo.mHeight,
            SkEncodedInfo::kYUV_Color, SkEncodedInfo::kOpaque_Alpha,
            /*bitsPerComponent*/ 8, std::move(profile), colorDepth);
    SkEncodedOrigin orientation = get_orientation(heifInfo);

    *result = SkCodec::kSuccess;
    return std::unique_ptr<SkCodec>(new SkHeifCodec(
            std::move(info), heifDecoder.release(), orientation, frameCount > 1, format));
}

SkHeifCodec::SkHeifCodec(
        SkEncodedInfo&& info,
        HeifDecoder* heifDecoder,
        SkEncodedOrigin origin,
        bool useAnimation,
        SkEncodedImageFormat format)
    : INHERITED(std::move(info), skcms_PixelFormat_RGBA_8888, nullptr, origin)
    , fHeifDecoder(heifDecoder)
    , fSwizzleSrcRow(nullptr)
    , fColorXformSrcRow(nullptr)
    , fUseAnimation(useAnimation)
    , fFormat(format)
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

    uint8_t colorDepth = fHeifDecoder->getColorDepth();
    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            this->setSrcXformFormat(skcms_PixelFormat_RGBA_8888);
            return fHeifDecoder->setOutputColor(kHeifColorFormat_RGBA_8888);

        case kBGRA_8888_SkColorType:
            this->setSrcXformFormat(skcms_PixelFormat_RGBA_8888);
            return fHeifDecoder->setOutputColor(kHeifColorFormat_BGRA_8888);

        case kRGB_565_SkColorType:
            this->setSrcXformFormat(skcms_PixelFormat_RGBA_8888);
            if (needsColorXform) {
                return fHeifDecoder->setOutputColor(kHeifColorFormat_RGBA_8888);
            } else {
                return fHeifDecoder->setOutputColor(kHeifColorFormat_RGB565);
            }

        case kRGBA_1010102_SkColorType:
            this->setSrcXformFormat(skcms_PixelFormat_RGBA_1010102);
            return fHeifDecoder->setOutputColor(kHeifColorFormat_RGBA_1010102);

        case kRGBA_F16_SkColorType:
            SkASSERT(needsColorXform);
            if (srcIsOpaque && colorDepth == 10) {
                this->setSrcXformFormat(skcms_PixelFormat_RGBA_1010102);
                return fHeifDecoder->setOutputColor(kHeifColorFormat_RGBA_1010102);
            } else {
                this->setSrcXformFormat(skcms_PixelFormat_RGBA_8888);
                return fHeifDecoder->setOutputColor(kHeifColorFormat_RGBA_8888);
            }

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

int SkHeifCodec::onGetFrameCount() {
    if (!fUseAnimation) {
        return 1;
    }

    if (fFrameHolder.size() == 0) {
        size_t frameCount;
        HeifFrameInfo frameInfo;
        if (!fHeifDecoder->getSequenceInfo(&frameInfo, &frameCount)
                || frameCount <= 1) {
            fUseAnimation = false;
            return 1;
        }
        fFrameHolder.reserve(frameCount);
        for (size_t i = 0; i < frameCount; i++) {
            Frame* frame = fFrameHolder.appendNewFrame();
            frame->setXYWH(0, 0, frameInfo.mWidth, frameInfo.mHeight);
            frame->setDisposalMethod(SkCodecAnimation::DisposalMethod::kKeep);
            // Currently we don't know the duration until the frame is actually
            // decoded (onGetFrameInfo is also called before frame is decoded).
            // For now, fill it base on the value reported for the sequence.
            frame->setDuration(frameInfo.mDurationUs / 1000);
            frame->setRequiredFrame(SkCodec::kNoFrame);
            frame->setHasAlpha(false);
        }
    }

    return fFrameHolder.size();
}

const SkFrame* SkHeifCodec::FrameHolder::onGetFrame(int i) const {
    return static_cast<const SkFrame*>(this->frame(i));
}

SkHeifCodec::Frame* SkHeifCodec::FrameHolder::appendNewFrame() {
    const int i = this->size();
    fFrames.emplace_back(i); // TODO: need to handle frame duration here
    return &fFrames[i];
}

const SkHeifCodec::Frame* SkHeifCodec::FrameHolder::frame(int i) const {
    SkASSERT(i >= 0 && i < this->size());
    return &fFrames[i];
}

SkHeifCodec::Frame* SkHeifCodec::FrameHolder::editFrameAt(int i) {
    SkASSERT(i >= 0 && i < this->size());
    return &fFrames[i];
}

bool SkHeifCodec::onGetFrameInfo(int i, FrameInfo* frameInfo) const {
    if (i >= fFrameHolder.size()) {
        return false;
    }

    const Frame* frame = fFrameHolder.frame(i);
    if (!frame) {
        return false;
    }

    if (frameInfo) {
        frame->fillIn(frameInfo, true);
    }

    return true;
}

int SkHeifCodec::onGetRepetitionCount() {
    return kRepetitionCountInfinite;
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

    bool success;
    if (fUseAnimation) {
        success = fHeifDecoder->decodeSequence(options.fFrameIndex, &fFrameInfo);
        fFrameHolder.editFrameAt(options.fFrameIndex)->setDuration(
                fFrameInfo.mDurationUs / 1000);
    } else {
        success = fHeifDecoder->decode(&fFrameInfo);
    }

    if (!success) {
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
    switch (this->getSrcXformFormat()) {
        case skcms_PixelFormat_RGBA_8888:
            swizzlerDstInfo = swizzlerDstInfo.makeColorType(kRGBA_8888_SkColorType);
            break;
        case skcms_PixelFormat_RGBA_1010102:
            swizzlerDstInfo = swizzlerDstInfo.makeColorType(kRGBA_1010102_SkColorType);
            break;
        default:
            SkASSERT(false);
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

bool SkHeifCodec::onRewind() {
    fSwizzler.reset(nullptr);
    fSwizzleSrcRow = nullptr;
    fColorXformSrcRow = nullptr;
    fStorage.reset();

    return true;
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

namespace SkHeifDecoder {
bool IsHeif(const void* data, size_t len) {
    return SkHeifCodec::IsSupported(data, len, nullptr);
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext ctx) {
    SkASSERT(ctx);
    SkCodec::Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }
    auto policy = static_cast<SkCodec::SelectionPolicy*>(ctx);
    return SkHeifCodec::MakeFromStream(std::move(stream), *policy, outResult);
}

std::unique_ptr<SkCodec> Decode(sk_sp<SkData> data,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext ctx) {
    if (!data) {
        if (outResult) {
            *outResult = SkCodec::kInvalidInput;
        }
        return nullptr;
    }
    return Decode(SkMemoryStream::Make(std::move(data)), outResult, ctx);
}
}  // namespace SkHeifDecoder

