/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegCodec.h"
#include "src/codec/SkRawCodec.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkStreamPriv.h"
#include "src/core/SkTaskGroup.h"

#include "dng_area_task.h"
#include "dng_color_space.h"
#include "dng_errors.h"
#include "dng_exceptions.h"
#include "dng_host.h"
#include "dng_info.h"
#include "dng_memory.h"
#include "dng_render.h"
#include "dng_stream.h"

#include "src/piex.h"

#include <cmath>  // for std::round,floor,ceil
#include <limits>

namespace {

// Caluclates the number of tiles of tile_size that fit into the area in vertical and horizontal
// directions.
dng_point num_tiles_in_area(const dng_point &areaSize,
                            const dng_point_real64 &tileSize) {
  // FIXME: Add a ceil_div() helper in SkCodecPriv.h
  return dng_point(static_cast<int32>((areaSize.v + tileSize.v - 1) / tileSize.v),
                   static_cast<int32>((areaSize.h + tileSize.h - 1) / tileSize.h));
}

int num_tasks_required(const dng_point& tilesInTask,
                         const dng_point& tilesInArea) {
  return ((tilesInArea.v + tilesInTask.v - 1) / tilesInTask.v) *
         ((tilesInArea.h + tilesInTask.h - 1) / tilesInTask.h);
}

// Calculate the number of tiles to process per task, taking into account the maximum number of
// tasks. It prefers to increase horizontally for better locality of reference.
dng_point num_tiles_per_task(const int maxTasks,
                             const dng_point &tilesInArea) {
  dng_point tilesInTask = {1, 1};
  while (num_tasks_required(tilesInTask, tilesInArea) > maxTasks) {
      if (tilesInTask.h < tilesInArea.h) {
          ++tilesInTask.h;
      } else if (tilesInTask.v < tilesInArea.v) {
          ++tilesInTask.v;
      } else {
          ThrowProgramError("num_tiles_per_task calculation is wrong.");
      }
  }
  return tilesInTask;
}

std::vector<dng_rect> compute_task_areas(const int maxTasks, const dng_rect& area,
                                         const dng_point& tileSize) {
  std::vector<dng_rect> taskAreas;
  const dng_point tilesInArea = num_tiles_in_area(area.Size(), tileSize);
  const dng_point tilesPerTask = num_tiles_per_task(maxTasks, tilesInArea);
  const dng_point taskAreaSize = {tilesPerTask.v * tileSize.v,
                                    tilesPerTask.h * tileSize.h};
  for (int v = 0; v < tilesInArea.v; v += tilesPerTask.v) {
    for (int h = 0; h < tilesInArea.h; h += tilesPerTask.h) {
      dng_rect taskArea;
      taskArea.t = area.t + v * tileSize.v;
      taskArea.l = area.l + h * tileSize.h;
      taskArea.b = Min_int32(taskArea.t + taskAreaSize.v, area.b);
      taskArea.r = Min_int32(taskArea.l + taskAreaSize.h, area.r);

      taskAreas.push_back(taskArea);
    }
  }
  return taskAreas;
}

class SkDngHost : public dng_host {
public:
    explicit SkDngHost(dng_memory_allocator* allocater) : dng_host(allocater) {}

    void PerformAreaTask(dng_area_task& task, const dng_rect& area) override {
        SkTaskGroup taskGroup;

        // tileSize is typically 256x256
        const dng_point tileSize(task.FindTileSize(area));
        const std::vector<dng_rect> taskAreas = compute_task_areas(this->PerformAreaTaskThreads(),
                                                                   area, tileSize);
        const int numTasks = static_cast<int>(taskAreas.size());

        SkMutex mutex;
        SkTArray<dng_exception> exceptions;
        task.Start(numTasks, tileSize, &Allocator(), Sniffer());
        for (int taskIndex = 0; taskIndex < numTasks; ++taskIndex) {
            taskGroup.add([&mutex, &exceptions, &task, this, taskIndex, taskAreas, tileSize] {
                try {
                    task.ProcessOnThread(taskIndex, taskAreas[taskIndex], tileSize, this->Sniffer());
                } catch (dng_exception& exception) {
                    SkAutoMutexExclusive lock(mutex);
                    exceptions.push_back(exception);
                } catch (...) {
                    SkAutoMutexExclusive lock(mutex);
                    exceptions.push_back(dng_exception(dng_error_unknown));
                }
            });
        }

        taskGroup.wait();
        task.Finish(numTasks);

        // We only re-throw the first exception.
        if (!exceptions.empty()) {
            Throw_dng_error(exceptions.front().ErrorCode(), nullptr, nullptr);
        }
    }

    uint32 PerformAreaTaskThreads() override {
#ifdef SK_BUILD_FOR_ANDROID
        // Only use 1 thread. DNGs with the warp effect require a lot of memory,
        // and the amount of memory required scales linearly with the number of
        // threads. The sample used in CTS requires over 500 MB, so even two
        // threads is significantly expensive. There is no good way to tell
        // whether the image has the warp effect.
        return 1;
#else
        return kMaxMPThreads;
#endif
    }

private:
    typedef dng_host INHERITED;
};

// T must be unsigned type.
template <class T>
bool safe_add_to_size_t(T arg1, T arg2, size_t* result) {
    SkASSERT(arg1 >= 0);
    SkASSERT(arg2 >= 0);
    if (arg1 >= 0 && arg2 <= std::numeric_limits<T>::max() - arg1) {
        T sum = arg1 + arg2;
        if (sum <= std::numeric_limits<size_t>::max()) {
            *result = static_cast<size_t>(sum);
            return true;
        }
    }
    return false;
}

bool is_asset_stream(const SkStream& stream) {
    return stream.hasLength() && stream.hasPosition();
}

}  // namespace

class SkRawStream {
public:
    virtual ~SkRawStream() {}

   /*
    * Gets the length of the stream. Depending on the type of stream, this may require reading to
    * the end of the stream.
    */
   virtual uint64 getLength() = 0;

   virtual bool read(void* data, size_t offset, size_t length) = 0;

    /*
     * Creates an SkMemoryStream from the offset with size.
     * Note: for performance reason, this function is destructive to the SkRawStream. One should
     *       abandon current object after the function call.
     */
   virtual std::unique_ptr<SkMemoryStream> transferBuffer(size_t offset, size_t size) = 0;
};

class SkRawLimitedDynamicMemoryWStream : public SkDynamicMemoryWStream {
public:
    ~SkRawLimitedDynamicMemoryWStream() override {}

    bool write(const void* buffer, size_t size) override {
        size_t newSize;
        if (!safe_add_to_size_t(this->bytesWritten(), size, &newSize) ||
            newSize > kMaxStreamSize)
        {
            SkCodecPrintf("Error: Stream size exceeds the limit.\n");
            return false;
        }
        return this->INHERITED::write(buffer, size);
    }

private:
    // Most of valid RAW images will not be larger than 100MB. This limit is helpful to avoid
    // streaming too large data chunk. We can always adjust the limit here if we need.
    const size_t kMaxStreamSize = 100 * 1024 * 1024;  // 100MB

    typedef SkDynamicMemoryWStream INHERITED;
};

// Note: the maximum buffer size is 100MB (limited by SkRawLimitedDynamicMemoryWStream).
class SkRawBufferedStream : public SkRawStream {
public:
    explicit SkRawBufferedStream(std::unique_ptr<SkStream> stream)
        : fStream(std::move(stream))
        , fWholeStreamRead(false)
    {
        // Only use SkRawBufferedStream when the stream is not an asset stream.
        SkASSERT(!is_asset_stream(*fStream));
    }

    ~SkRawBufferedStream() override {}

    uint64 getLength() override {
        if (!this->bufferMoreData(kReadToEnd)) {  // read whole stream
            ThrowReadFile();
        }
        return fStreamBuffer.bytesWritten();
    }

    bool read(void* data, size_t offset, size_t length) override {
        if (length == 0) {
            return true;
        }

        size_t sum;
        if (!safe_add_to_size_t(offset, length, &sum)) {
            return false;
        }

        return this->bufferMoreData(sum) && fStreamBuffer.read(data, offset, length);
    }

    std::unique_ptr<SkMemoryStream> transferBuffer(size_t offset, size_t size) override {
        sk_sp<SkData> data(SkData::MakeUninitialized(size));
        if (offset > fStreamBuffer.bytesWritten()) {
            // If the offset is not buffered, read from fStream directly and skip the buffering.
            const size_t skipLength = offset - fStreamBuffer.bytesWritten();
            if (fStream->skip(skipLength) != skipLength) {
                return nullptr;
            }
            const size_t bytesRead = fStream->read(data->writable_data(), size);
            if (bytesRead < size) {
                data = SkData::MakeSubset(data.get(), 0, bytesRead);
            }
        } else {
            const size_t alreadyBuffered = SkTMin(fStreamBuffer.bytesWritten() - offset, size);
            if (alreadyBuffered > 0 &&
                !fStreamBuffer.read(data->writable_data(), offset, alreadyBuffered)) {
                return nullptr;
            }

            const size_t remaining = size - alreadyBuffered;
            if (remaining) {
                auto* dst = static_cast<uint8_t*>(data->writable_data()) + alreadyBuffered;
                const size_t bytesRead = fStream->read(dst, remaining);
                size_t newSize;
                if (bytesRead < remaining) {
                    if (!safe_add_to_size_t(alreadyBuffered, bytesRead, &newSize)) {
                        return nullptr;
                    }
                    data = SkData::MakeSubset(data.get(), 0, newSize);
                }
            }
        }
        return SkMemoryStream::Make(data);
    }

private:
    // Note: if the newSize == kReadToEnd (0), this function will read to the end of stream.
    bool bufferMoreData(size_t newSize) {
        if (newSize == kReadToEnd) {
            if (fWholeStreamRead) {  // already read-to-end.
                return true;
            }

            // TODO: optimize for the special case when the input is SkMemoryStream.
            return SkStreamCopy(&fStreamBuffer, fStream.get());
        }

        if (newSize <= fStreamBuffer.bytesWritten()) {  // already buffered to newSize
            return true;
        }
        if (fWholeStreamRead) {  // newSize is larger than the whole stream.
            return false;
        }

        // Try to read at least 8192 bytes to avoid to many small reads.
        const size_t kMinSizeToRead = 8192;
        const size_t sizeRequested = newSize - fStreamBuffer.bytesWritten();
        const size_t sizeToRead = SkTMax(kMinSizeToRead, sizeRequested);
        SkAutoSTMalloc<kMinSizeToRead, uint8> tempBuffer(sizeToRead);
        const size_t bytesRead = fStream->read(tempBuffer.get(), sizeToRead);
        if (bytesRead < sizeRequested) {
            return false;
        }
        return fStreamBuffer.write(tempBuffer.get(), bytesRead);
    }

    std::unique_ptr<SkStream> fStream;
    bool fWholeStreamRead;

    // Use a size-limited stream to avoid holding too huge buffer.
    SkRawLimitedDynamicMemoryWStream fStreamBuffer;

    const size_t kReadToEnd = 0;
};

class SkRawAssetStream : public SkRawStream {
public:
    explicit SkRawAssetStream(std::unique_ptr<SkStream> stream)
        : fStream(std::move(stream))
    {
        // Only use SkRawAssetStream when the stream is an asset stream.
        SkASSERT(is_asset_stream(*fStream));
    }

    ~SkRawAssetStream() override {}

    uint64 getLength() override {
        return fStream->getLength();
    }


    bool read(void* data, size_t offset, size_t length) override {
        if (length == 0) {
            return true;
        }

        size_t sum;
        if (!safe_add_to_size_t(offset, length, &sum)) {
            return false;
        }

        return fStream->seek(offset) && (fStream->read(data, length) == length);
    }

    std::unique_ptr<SkMemoryStream> transferBuffer(size_t offset, size_t size) override {
        if (fStream->getLength() < offset) {
            return nullptr;
        }

        size_t sum;
        if (!safe_add_to_size_t(offset, size, &sum)) {
            return nullptr;
        }

        // This will allow read less than the requested "size", because the JPEG codec wants to
        // handle also a partial JPEG file.
        const size_t bytesToRead = SkTMin(sum, fStream->getLength()) - offset;
        if (bytesToRead == 0) {
            return nullptr;
        }

        if (fStream->getMemoryBase()) {  // directly copy if getMemoryBase() is available.
            sk_sp<SkData> data(SkData::MakeWithCopy(
                static_cast<const uint8_t*>(fStream->getMemoryBase()) + offset, bytesToRead));
            fStream.reset();
            return SkMemoryStream::Make(data);
        } else {
            sk_sp<SkData> data(SkData::MakeUninitialized(bytesToRead));
            if (!fStream->seek(offset)) {
                return nullptr;
            }
            const size_t bytesRead = fStream->read(data->writable_data(), bytesToRead);
            if (bytesRead < bytesToRead) {
                data = SkData::MakeSubset(data.get(), 0, bytesRead);
            }
            return SkMemoryStream::Make(data);
        }
    }
private:
    std::unique_ptr<SkStream> fStream;
};

class SkPiexStream : public ::piex::StreamInterface {
public:
    // Will NOT take the ownership of the stream.
    explicit SkPiexStream(SkRawStream* stream) : fStream(stream) {}

    ~SkPiexStream() override {}

    ::piex::Error GetData(const size_t offset, const size_t length,
                          uint8* data) override {
        return fStream->read(static_cast<void*>(data), offset, length) ?
            ::piex::Error::kOk : ::piex::Error::kFail;
    }

private:
    SkRawStream* fStream;
};

class SkDngStream : public dng_stream {
public:
    // Will NOT take the ownership of the stream.
    SkDngStream(SkRawStream* stream) : fStream(stream) {}

    ~SkDngStream() override {}

    uint64 DoGetLength() override { return fStream->getLength(); }

    void DoRead(void* data, uint32 count, uint64 offset) override {
        size_t sum;
        if (!safe_add_to_size_t(static_cast<uint64>(count), offset, &sum) ||
            !fStream->read(data, static_cast<size_t>(offset), static_cast<size_t>(count))) {
            ThrowReadFile();
        }
    }

private:
    SkRawStream* fStream;
};

class SkDngImage {
public:
    /*
     * Initializes the object with the information from Piex in a first attempt. This way it can
     * save time and storage to obtain the DNG dimensions and color filter array (CFA) pattern
     * which is essential for the demosaicing of the sensor image.
     * Note: this will take the ownership of the stream.
     */
    static SkDngImage* NewFromStream(SkRawStream* stream) {
        std::unique_ptr<SkDngImage> dngImage(new SkDngImage(stream));
#if defined(IS_FUZZING_WITH_LIBFUZZER)
        // Libfuzzer easily runs out of memory after here. To avoid that
        // We just pretend all streams are invalid. Our AFL-fuzzer
        // should still exercise this code; it's more resistant to OOM.
        return nullptr;
#endif
        if (!dngImage->initFromPiex() && !dngImage->readDng()) {
            return nullptr;
        }

        return dngImage.release();
    }

    /*
     * Renders the DNG image to the size. The DNG SDK only allows scaling close to integer factors
     * down to 80 pixels on the short edge. The rendered image will be close to the specified size,
     * but there is no guarantee that any of the edges will match the requested size. E.g.
     *   100% size:              4000 x 3000
     *   requested size:         1600 x 1200
     *   returned size could be: 2000 x 1500
     */
    dng_image* render(int width, int height) {
        if (!fHost || !fInfo || !fNegative || !fDngStream) {
            if (!this->readDng()) {
                return nullptr;
            }
        }

        // DNG SDK preserves the aspect ratio, so it only needs to know the longer dimension.
        const int preferredSize = SkTMax(width, height);
        try {
            // render() takes ownership of fHost, fInfo, fNegative and fDngStream when available.
            std::unique_ptr<dng_host> host(fHost.release());
            std::unique_ptr<dng_info> info(fInfo.release());
            std::unique_ptr<dng_negative> negative(fNegative.release());
            std::unique_ptr<dng_stream> dngStream(fDngStream.release());

            host->SetPreferredSize(preferredSize);
            host->ValidateSizes();

            negative->ReadStage1Image(*host, *dngStream, *info);

            if (info->fMaskIndex != -1) {
                negative->ReadTransparencyMask(*host, *dngStream, *info);
            }

            negative->ValidateRawImageDigest(*host);
            if (negative->IsDamaged()) {
                return nullptr;
            }

            const int32 kMosaicPlane = -1;
            negative->BuildStage2Image(*host);
            negative->BuildStage3Image(*host, kMosaicPlane);

            dng_render render(*host, *negative);
            render.SetFinalSpace(dng_space_sRGB::Get());
            render.SetFinalPixelType(ttByte);

            dng_point stage3_size = negative->Stage3Image()->Size();
            render.SetMaximumSize(SkTMax(stage3_size.h, stage3_size.v));

            return render.Render();
        } catch (...) {
            return nullptr;
        }
    }

    int width() const {
        return fWidth;
    }

    int height() const {
        return fHeight;
    }

    bool isScalable() const {
        return fIsScalable;
    }

    bool isXtransImage() const {
        return fIsXtransImage;
    }

    // Quick check if the image contains a valid TIFF header as requested by DNG format.
    // Does not affect ownership of stream.
    static bool IsTiffHeaderValid(SkRawStream* stream) {
        const size_t kHeaderSize = 4;
        unsigned char header[kHeaderSize];
        if (!stream->read(header, 0 /* offset */, kHeaderSize)) {
            return false;
        }

        // Check if the header is valid (endian info and magic number "42").
        bool littleEndian;
        if (!is_valid_endian_marker(header, &littleEndian)) {
            return false;
        }

        return 0x2A == get_endian_short(header + 2, littleEndian);
    }

private:
    bool init(int width, int height, const dng_point& cfaPatternSize) {
        fWidth = width;
        fHeight = height;

        // The DNG SDK scales only during demosaicing, so scaling is only possible when
        // a mosaic info is available.
        fIsScalable = cfaPatternSize.v != 0 && cfaPatternSize.h != 0;
        fIsXtransImage = fIsScalable ? (cfaPatternSize.v == 6 && cfaPatternSize.h == 6) : false;

        return width > 0 && height > 0;
    }

    bool initFromPiex() {
        // Does not take the ownership of rawStream.
        SkPiexStream piexStream(fStream.get());
        ::piex::PreviewImageData imageData;
        if (::piex::IsRaw(&piexStream)
            && ::piex::GetPreviewImageData(&piexStream, &imageData) == ::piex::Error::kOk)
        {
            dng_point cfaPatternSize(imageData.cfa_pattern_dim[1], imageData.cfa_pattern_dim[0]);
            return this->init(static_cast<int>(imageData.full_width),
                              static_cast<int>(imageData.full_height), cfaPatternSize);
        }
        return false;
    }

    bool readDng() {
        try {
            // Due to the limit of DNG SDK, we need to reset host and info.
            fHost.reset(new SkDngHost(&fAllocator));
            fInfo.reset(new dng_info);
            fDngStream.reset(new SkDngStream(fStream.get()));

            fHost->ValidateSizes();
            fInfo->Parse(*fHost, *fDngStream);
            fInfo->PostParse(*fHost);
            if (!fInfo->IsValidDNG()) {
                return false;
            }

            fNegative.reset(fHost->Make_dng_negative());
            fNegative->Parse(*fHost, *fDngStream, *fInfo);
            fNegative->PostParse(*fHost, *fDngStream, *fInfo);
            fNegative->SynchronizeMetadata();

            dng_point cfaPatternSize(0, 0);
            if (fNegative->GetMosaicInfo() != nullptr) {
                cfaPatternSize = fNegative->GetMosaicInfo()->fCFAPatternSize;
            }
            return this->init(static_cast<int>(fNegative->DefaultCropSizeH().As_real64()),
                              static_cast<int>(fNegative->DefaultCropSizeV().As_real64()),
                              cfaPatternSize);
        } catch (...) {
            return false;
        }
    }

    SkDngImage(SkRawStream* stream)
        : fStream(stream)
    {}

    dng_memory_allocator fAllocator;
    std::unique_ptr<SkRawStream> fStream;
    std::unique_ptr<dng_host> fHost;
    std::unique_ptr<dng_info> fInfo;
    std::unique_ptr<dng_negative> fNegative;
    std::unique_ptr<dng_stream> fDngStream;

    int fWidth;
    int fHeight;
    bool fIsScalable;
    bool fIsXtransImage;
};

/*
 * Tries to handle the image with PIEX. If PIEX returns kOk and finds the preview image, create a
 * SkJpegCodec. If PIEX returns kFail, then the file is invalid, return nullptr. In other cases,
 * fallback to create SkRawCodec for DNG images.
 */
std::unique_ptr<SkCodec> SkRawCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                    Result* result) {
    std::unique_ptr<SkRawStream> rawStream;
    if (is_asset_stream(*stream)) {
        rawStream.reset(new SkRawAssetStream(std::move(stream)));
    } else {
        rawStream.reset(new SkRawBufferedStream(std::move(stream)));
    }

    // Does not take the ownership of rawStream.
    SkPiexStream piexStream(rawStream.get());
    ::piex::PreviewImageData imageData;
    if (::piex::IsRaw(&piexStream)) {
        ::piex::Error error = ::piex::GetPreviewImageData(&piexStream, &imageData);
        if (error == ::piex::Error::kFail) {
            *result = kInvalidInput;
            return nullptr;
        }

        std::unique_ptr<SkEncodedInfo::ICCProfile> profile;
        if (imageData.color_space == ::piex::PreviewImageData::kAdobeRgb) {
            skcms_ICCProfile skcmsProfile;
            skcms_Init(&skcmsProfile);
            skcms_SetTransferFunction(&skcmsProfile, &SkNamedTransferFn::k2Dot2);
            skcms_SetXYZD50(&skcmsProfile, &SkNamedGamut::kAdobeRGB);
            profile = SkEncodedInfo::ICCProfile::Make(skcmsProfile);
        }

        //  Theoretically PIEX can return JPEG compressed image or uncompressed RGB image. We only
        //  handle the JPEG compressed preview image here.
        if (error == ::piex::Error::kOk && imageData.preview.length > 0 &&
            imageData.preview.format == ::piex::Image::kJpegCompressed)
        {
            // transferBuffer() is destructive to the rawStream. Abandon the rawStream after this
            // function call.
            // FIXME: one may avoid the copy of memoryStream and use the buffered rawStream.
            auto memoryStream = rawStream->transferBuffer(imageData.preview.offset,
                                                          imageData.preview.length);
            if (!memoryStream) {
                *result = kInvalidInput;
                return nullptr;
            }
            return SkJpegCodec::MakeFromStream(std::move(memoryStream), result,
                                               std::move(profile));
        }
    }

    if (!SkDngImage::IsTiffHeaderValid(rawStream.get())) {
        *result = kUnimplemented;
        return nullptr;
    }

    // Takes the ownership of the rawStream.
    std::unique_ptr<SkDngImage> dngImage(SkDngImage::NewFromStream(rawStream.release()));
    if (!dngImage) {
        *result = kInvalidInput;
        return nullptr;
    }

    *result = kSuccess;
    return std::unique_ptr<SkCodec>(new SkRawCodec(dngImage.release()));
}

SkCodec::Result SkRawCodec::onGetPixels(const SkImageInfo& dstInfo, void* dst,
                                        size_t dstRowBytes, const Options& options,
                                        int* rowsDecoded) {
    const int width = dstInfo.width();
    const int height = dstInfo.height();
    std::unique_ptr<dng_image> image(fDngImage->render(width, height));
    if (!image) {
        return kInvalidInput;
    }

    // Because the DNG SDK can not guarantee to render to requested size, we allow a small
    // difference. Only the overlapping region will be converted.
    const float maxDiffRatio = 1.03f;
    const dng_point& imageSize = image->Size();
    if (imageSize.h / (float) width > maxDiffRatio || imageSize.h < width ||
        imageSize.v / (float) height > maxDiffRatio || imageSize.v < height) {
        return SkCodec::kInvalidScale;
    }

    void* dstRow = dst;
    SkAutoTMalloc<uint8_t> srcRow(width * 3);

    dng_pixel_buffer buffer;
    buffer.fData = &srcRow[0];
    buffer.fPlane = 0;
    buffer.fPlanes = 3;
    buffer.fColStep = buffer.fPlanes;
    buffer.fPlaneStep = 1;
    buffer.fPixelType = ttByte;
    buffer.fPixelSize = sizeof(uint8_t);
    buffer.fRowStep = width * 3;

    constexpr auto srcFormat = skcms_PixelFormat_RGB_888;
    skcms_PixelFormat dstFormat;
    if (!sk_select_xform_format(dstInfo.colorType(), false, &dstFormat)) {
        return kInvalidConversion;
    }

    const skcms_ICCProfile* const srcProfile = this->getEncodedInfo().profile();
    skcms_ICCProfile dstProfileStorage;
    const skcms_ICCProfile* dstProfile = nullptr;
    if (auto cs = dstInfo.colorSpace()) {
        cs->toProfile(&dstProfileStorage);
        dstProfile = &dstProfileStorage;
    }

    for (int i = 0; i < height; ++i) {
        buffer.fArea = dng_rect(i, 0, i + 1, width);

        try {
            image->Get(buffer, dng_image::edge_zero);
        } catch (...) {
            *rowsDecoded = i;
            return kIncompleteInput;
        }

        if (!skcms_Transform(&srcRow[0], srcFormat, skcms_AlphaFormat_Unpremul, srcProfile,
                             dstRow,     dstFormat, skcms_AlphaFormat_Unpremul, dstProfile,
                             dstInfo.width())) {
            SkDebugf("failed to transform\n");
            *rowsDecoded = i;
            return kInternalError;
        }

        dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
    }
    return kSuccess;
}

SkISize SkRawCodec::onGetScaledDimensions(float desiredScale) const {
    SkASSERT(desiredScale <= 1.f);

    const SkISize dim = this->dimensions();
    SkASSERT(dim.fWidth != 0 && dim.fHeight != 0);

    if (!fDngImage->isScalable()) {
        return dim;
    }

    // Limits the minimum size to be 80 on the short edge.
    const float shortEdge = static_cast<float>(SkTMin(dim.fWidth, dim.fHeight));
    if (desiredScale < 80.f / shortEdge) {
        desiredScale = 80.f / shortEdge;
    }

    // For Xtrans images, the integer-factor scaling does not support the half-size scaling case
    // (stronger downscalings are fine). In this case, returns the factor "3" scaling instead.
    if (fDngImage->isXtransImage() && desiredScale > 1.f / 3.f && desiredScale < 1.f) {
        desiredScale = 1.f / 3.f;
    }

    // Round to integer-factors.
    const float finalScale = std::floor(1.f/ desiredScale);
    return SkISize::Make(static_cast<int32_t>(std::floor(dim.fWidth / finalScale)),
                         static_cast<int32_t>(std::floor(dim.fHeight / finalScale)));
}

bool SkRawCodec::onDimensionsSupported(const SkISize& dim) {
    const SkISize fullDim = this->dimensions();
    const float fullShortEdge = static_cast<float>(SkTMin(fullDim.fWidth, fullDim.fHeight));
    const float shortEdge = static_cast<float>(SkTMin(dim.fWidth, dim.fHeight));

    SkISize sizeFloor = this->onGetScaledDimensions(1.f / std::floor(fullShortEdge / shortEdge));
    SkISize sizeCeil = this->onGetScaledDimensions(1.f / std::ceil(fullShortEdge / shortEdge));
    return sizeFloor == dim || sizeCeil == dim;
}

SkRawCodec::~SkRawCodec() {}

SkRawCodec::SkRawCodec(SkDngImage* dngImage)
    : INHERITED(SkEncodedInfo::Make(dngImage->width(), dngImage->height(),
                                    SkEncodedInfo::kRGB_Color,
                                    SkEncodedInfo::kOpaque_Alpha, 8),
                skcms_PixelFormat_RGBA_8888, nullptr)
    , fDngImage(dngImage) {}
