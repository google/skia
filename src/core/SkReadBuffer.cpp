/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkReadBuffer.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkMalloc.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMipmapBuilder.h"
#include "src/core/SkWriteBuffer.h"

#include <memory>
#include <optional>
#include <utility>

namespace {
    // This generator intentionally should always fail on all attempts to get its pixels,
    // simulating a bad or empty codec stream.
    class EmptyImageGenerator final : public SkImageGenerator {
    public:
        EmptyImageGenerator(const SkImageInfo& info) : SkImageGenerator(info) { }

    };

    static sk_sp<SkImage> MakeEmptyImage(int width, int height) {
        return SkImages::DeferredFromGenerator(
                std::make_unique<EmptyImageGenerator>(SkImageInfo::MakeN32Premul(width, height)));
    }

} // anonymous namespace

void SkReadBuffer::setMemory(const void* data, size_t size) {
    this->validate(IsPtrAlign4(data) && (SkAlign4(size) == size));
    if (!fError) {
        fBase = fCurr = (const char*)data;
        fStop = fBase + size;
    }
}

void SkReadBuffer::setInvalid() {
    if (!fError) {
        // When an error is found, send the read cursor to the end of the stream
        fCurr = fStop;
        fError = true;
    }
}

const void* SkReadBuffer::skip(size_t size) {
    size_t inc = SkAlign4(size);
    this->validate(inc >= size);
    const void* addr = fCurr;
    this->validate(IsPtrAlign4(addr) && this->isAvailable(inc));
    if (fError) {
        return nullptr;
    }

    fCurr += inc;
    return addr;
}

const void* SkReadBuffer::skip(size_t count, size_t size) {
    return this->skip(SkSafeMath::Mul(count, size));
}

void SkReadBuffer::setDeserialProcs(const SkDeserialProcs& procs) {
    fProcs = procs;
    this->setAllowSkSL(procs.fAllowSkSL);
}

bool SkReadBuffer::readBool() {
    uint32_t value = this->readUInt();
    // Boolean value should be either 0 or 1
    this->validate(!(value & ~1));
    return value != 0;
}

SkColor SkReadBuffer::readColor() {
    return this->readUInt();
}

int32_t SkReadBuffer::readInt() {
    const size_t inc = sizeof(int32_t);
    if (!this->validate(IsPtrAlign4(fCurr) && this->isAvailable(inc))) {
        return 0;
    }
    int32_t value = *((const int32_t*)fCurr);
    fCurr += inc;
    return value;
}

SkScalar SkReadBuffer::readScalar() {
    const size_t inc = sizeof(SkScalar);
    if (!this->validate(IsPtrAlign4(fCurr) && this->isAvailable(inc))) {
        return 0;
    }
    SkScalar value = *((const SkScalar*)fCurr);
    fCurr += inc;
    return value;
}

uint32_t SkReadBuffer::readUInt() {
    return this->readInt();
}

int32_t SkReadBuffer::read32() {
    return this->readInt();
}

uint8_t SkReadBuffer::peekByte() {
    if (this->available() <= 0) {
        fError = true;
        return 0;
    }
    return *((const uint8_t*)fCurr);
}

bool SkReadBuffer::readPad32(void* buffer, size_t bytes) {
    if (const void* src = this->skip(bytes)) {
        // buffer might be null if bytes is zero (see SkAutoMalloc), hence we call
        // the careful version of memcpy.
        sk_careful_memcpy(buffer, src, bytes);
        return true;
    }
    return false;
}

const char* SkReadBuffer::readString(size_t* len) {
    *len = this->readUInt();

    // The string is len characters and a terminating \0.
    const char* c_str = this->skipT<char>(*len+1);

    if (this->validate(c_str && c_str[*len] == '\0')) {
        return c_str;
    }
    return nullptr;
}

void SkReadBuffer::readString(SkString* string) {
    size_t len;
    if (const char* c_str = this->readString(&len)) {
        string->set(c_str, len);
        return;
    }
    string->reset();
}

void SkReadBuffer::readColor4f(SkColor4f* color) {
    if (!this->readPad32(color, sizeof(SkColor4f))) {
        *color = {0, 0, 0, 0};
    }
}

void SkReadBuffer::readPoint(SkPoint* point) {
    point->fX = this->readScalar();
    point->fY = this->readScalar();
}

void SkReadBuffer::readPoint3(SkPoint3* point) {
    this->readPad32(point, sizeof(SkPoint3));
}

void SkReadBuffer::read(SkM44* matrix) {
    if (this->isValid()) {
        if (const float* m = (const float*)this->skip(sizeof(float) * 16)) {
            *matrix = SkM44::ColMajor(m);
        }
    }
    if (!this->isValid()) {
        *matrix = SkM44();
    }
}

void SkReadBuffer::readMatrix(SkMatrix* matrix) {
    size_t size = 0;
    if (this->isValid()) {
        size = SkMatrixPriv::ReadFromMemory(matrix, fCurr, this->available());
        (void)this->validate((SkAlign4(size) == size) && (0 != size));
    }
    if (!this->isValid()) {
        matrix->reset();
    }
    (void)this->skip(size);
}

void SkReadBuffer::readIRect(SkIRect* rect) {
    if (!this->readPad32(rect, sizeof(SkIRect))) {
        rect->setEmpty();
    }
}

void SkReadBuffer::readRect(SkRect* rect) {
    if (!this->readPad32(rect, sizeof(SkRect))) {
        rect->setEmpty();
    }
}

SkRect SkReadBuffer::readRect() {
    SkRect r;
    if (!this->readPad32(&r, sizeof(SkRect))) {
        r.setEmpty();
    }
    return r;
}

SkSamplingOptions SkReadBuffer::readSampling() {
    if (!this->isVersionLT(SkPicturePriv::kAnisotropicFilter)) {
        int maxAniso = this->readInt();
        if (maxAniso != 0) {
            return SkSamplingOptions::Aniso(maxAniso);
        }
    }
    if (this->readBool()) {
        float B = this->readScalar();
        float C = this->readScalar();
        return SkSamplingOptions({B, C});
    } else {
        SkFilterMode filter = this->read32LE(SkFilterMode::kLinear);
        SkMipmapMode mipmap = this->read32LE(SkMipmapMode::kLinear);
        return SkSamplingOptions(filter, mipmap);
    }
}

void SkReadBuffer::readRRect(SkRRect* rrect) {
    size_t size = 0;
    if (!fError) {
        size = rrect->readFromMemory(fCurr, this->available());
        if (!this->validate((SkAlign4(size) == size) && (0 != size))) {
            rrect->setEmpty();
        }
    }
    (void)this->skip(size);
}

void SkReadBuffer::readRegion(SkRegion* region) {
    size_t size = 0;
    if (!fError) {
        size = region->readFromMemory(fCurr, this->available());
        if (!this->validate((SkAlign4(size) == size) && (0 != size))) {
            region->setEmpty();
        }
    }
    (void)this->skip(size);
}

void SkReadBuffer::readPath(SkPath* path) {
    size_t size = 0;
    if (!fError) {
        size = path->readFromMemory(fCurr, this->available());
        if (!this->validate((SkAlign4(size) == size) && (0 != size))) {
            path->reset();
        }
    }
    (void)this->skip(size);
}

bool SkReadBuffer::readArray(void* value, size_t size, size_t elementSize) {
    const uint32_t count = this->readUInt();
    return this->validate(size == count) &&
           this->readPad32(value, SkSafeMath::Mul(size, elementSize));
}

bool SkReadBuffer::readByteArray(void* value, size_t size) {
    return this->readArray(value, size, sizeof(uint8_t));
}

bool SkReadBuffer::readColorArray(SkSpan<SkColor> colors) {
    return this->readArray(colors.data(), colors.size(), sizeof(SkColor));
}

bool SkReadBuffer::readColor4fArray(SkSpan<SkColor4f> colors) {
    return this->readArray(colors.data(), colors.size(), sizeof(SkColor4f));
}

bool SkReadBuffer::readIntArray(SkSpan<int32_t> values) {
    return this->readArray(values.data(), values.size(), sizeof(int32_t));
}

bool SkReadBuffer::readPointArray(SkSpan<SkPoint> points) {
    return this->readArray(points.data(), points.size(), sizeof(SkPoint));
}

bool SkReadBuffer::readScalarArray(SkSpan<SkScalar> values) {
    return this->readArray(values.data(), values.size(), sizeof(SkScalar));
}

const void* SkReadBuffer::skipByteArray(size_t* size) {
    const uint32_t count = this->readUInt();
    const void* buf = this->skip(count);
    if (size) {
        *size = this->isValid() ? count : 0;
    }
    return buf;
}

sk_sp<SkData> SkReadBuffer::readByteArrayAsData() {
    size_t numBytes = this->getArrayCount();
    if (!this->validate(this->isAvailable(numBytes))) {
        return nullptr;
    }

    SkAutoMalloc buffer(numBytes);
    if (!this->readByteArray(buffer.get(), numBytes)) {
        return nullptr;
    }
    return SkData::MakeFromMalloc(buffer.release(), numBytes);
}

uint32_t SkReadBuffer::getArrayCount() {
    const size_t inc = sizeof(uint32_t);
    if (!this->validate(IsPtrAlign4(fCurr) && this->isAvailable(inc))) {
        return 0;
    }
    return *((const uint32_t*)fCurr);
}

static sk_sp<SkImage> deserialize_image(sk_sp<SkData> data, SkDeserialProcs dProcs,
                                        std::optional<SkAlphaType> alphaType) {
    sk_sp<SkImage> image;
    if (dProcs.fImageDataProc) {
        image = dProcs.fImageDataProc(data, alphaType, dProcs.fImageCtx);
    } else if (dProcs.fImageProc) {
#if !defined(SK_LEGACY_DESERIAL_IMAGE_PROC)
        image = dProcs.fImageProc(data->data(), data->size(), dProcs.fImageCtx);
#else
        image = dProcs.fImageProc(data->data(), data->size(), alphaType, dProcs.fImageCtx);
#endif
    }
    if (image) {
        return image;
    }
#if !defined(SK_DISABLE_LEGACY_IMAGE_READBUFFER)
    // The default implementation will encode to PNG unless the input SkImages came from
    // a codec that was built-in (e.g. JPEG/WEBP). Thus, we should be sure to try all
    // available codecs when reading images out of an SKP.
    return SkImages::DeferredFromEncodedData(std::move(data), alphaType);
#else
    SkDEBUGFAIL("Need to set image proc in SkDeserialProcs");
    return nullptr;
#endif
}

static sk_sp<SkImage> add_mipmaps(sk_sp<SkImage> img, sk_sp<SkData> data,
                                  SkDeserialProcs dProcs, std::optional<SkAlphaType> alphaType) {
    SkMipmapBuilder builder(img->imageInfo());

    SkReadBuffer buffer(data->data(), data->size());
    int count = buffer.read32();
    if (builder.countLevels() != count) {
        return img;
    }
    for (int i = 0; i < count; ++i) {
        size_t size = buffer.read32();
        const void* ptr = buffer.skip(size);
        if (!ptr) {
            return img;
        }
        // This use of SkData::MakeWithoutCopy is safe because the image goes
        // out of scope after we read the pixels from it, so we are sure the
        // data (from buffer) outlives the image.
        sk_sp<SkImage> mip = deserialize_image(SkData::MakeWithoutCopy(ptr, size), dProcs,
                                               alphaType);
        if (!mip) {
            return img;
        }

        SkPixmap pm = builder.level(i);
        if (mip->dimensions() != pm.dimensions()) {
            return img;
        }
        if (!mip->readPixels(nullptr, pm, 0, 0)) {
            return img;
        }
    }
    if (!buffer.isValid()) {
        return img;
    }
    sk_sp<SkImage> raster = img->makeRasterImage();
    if (!raster) {
        return img;
    }
    sk_sp<SkImage> rasterWithMips = builder.attachTo(raster);
    SkASSERT(rasterWithMips); // attachTo should never return null
    return rasterWithMips;
}


// If we see a corrupt stream, we return null (fail). If we just fail trying to decode
// the image, we don't fail, but return a 1x1 empty image.
sk_sp<SkImage> SkReadBuffer::readImage() {
    uint32_t flags = this->read32();

    std::optional<SkAlphaType> alphaType = std::nullopt;
    if (flags & SkWriteBufferImageFlags::kUnpremul) {
        alphaType = kUnpremul_SkAlphaType;
    }
    sk_sp<SkImage> image;
    {
        sk_sp<SkData> data = this->readByteArrayAsData();
        if (!data) {
            this->validate(false);
            return nullptr;
        }
        image = deserialize_image(data, fProcs, alphaType);
    }

    // This flag is not written by new SKPs anymore.
    if (flags & SkWriteBufferImageFlags::kHasSubsetRect) {
        SkIRect subset;
        this->readIRect(&subset);
        if (image) {
            image = image->makeSubset(nullptr, subset);
        }
    }

    if (flags & SkWriteBufferImageFlags::kHasMipmap) {
        sk_sp<SkData> data = this->readByteArrayAsData();
        if (!data) {
            this->validate(false);
            return nullptr;
        }
        if (image) {
            image = add_mipmaps(image, std::move(data), fProcs, alphaType);
        }
    }
    return image ? image : MakeEmptyImage(1, 1);
}

sk_sp<SkTypeface> SkReadBuffer::readTypeface() {
    // Read 32 bits (signed)
    //   0 -- return null (empty font)
    //  >0 -- index
    //  <0 -- custom (serial procs) : negative size in bytes

    int32_t index = this->read32();
    if (index == 0) {
        return nullptr;
    } else if (index > 0) {
        if (!this->validate(index <= fTFCount)) {
            return nullptr;
        }
        return fTFArray[index - 1];
    } else {    // custom
        size_t size = sk_negate_to_size_t(index);
        const void* data = this->skip(size);
        if (!this->validate(data != nullptr && fProcs.fTypefaceProc)) {
            return nullptr;
        }
        return fProcs.fTypefaceProc(data, size, fProcs.fTypefaceCtx);
    }
}

SkFlattenable* SkReadBuffer::readRawFlattenable() {
    SkFlattenable::Factory factory = nullptr;

    if (fFactoryCount > 0) {
        int32_t index = this->read32();
        if (0 == index || !this->isValid()) {
            return nullptr; // writer failed to give us the flattenable
        }
        if (index < 0) {
            this->validate(false);
            return nullptr;
        }
        index -= 1;     // we stored the index-base-1
        if ((unsigned)index >= (unsigned)fFactoryCount) {
            this->validate(false);
            return nullptr;
        }
        factory = fFactoryArray[index];
    } else {
        if (this->peekByte() != 0) {
            // If the first byte is non-zero, the flattenable is specified by a string.
            size_t ignored_length;
            if (const char* name = this->readString(&ignored_length)) {
                factory = SkFlattenable::NameToFactory(name);
                fFlattenableDict.set(fFlattenableDict.count() + 1, factory);
            }
        } else {
            // Read the index.  We are guaranteed that the first byte
            // is zeroed, so we must shift down a byte.
            uint32_t index = this->readUInt() >> 8;
            if (index == 0) {
                return nullptr; // writer failed to give us the flattenable
            }

            if (SkFlattenable::Factory* found = fFlattenableDict.find(index)) {
                factory = *found;
            }
        }

        if (!this->validate(factory != nullptr)) {
            return nullptr;
        }
    }

    // if we get here, factory may still be null, but if that is the case, the
    // failure was ours, not the writer.
    sk_sp<SkFlattenable> obj;
    uint32_t sizeRecorded = this->read32();
    if (factory) {
        size_t offset = this->offset();
        obj = (*factory)(*this);
        // check that we read the amount we expected
        size_t sizeRead = this->offset() - offset;
        if (sizeRecorded != sizeRead) {
            this->validate(false);
            return nullptr;
        }
    } else {
        // we must skip the remaining data
        this->skip(sizeRecorded);
    }
    if (!this->isValid()) {
        return nullptr;
    }
    return obj.release();
}

SkFlattenable* SkReadBuffer::readFlattenable(SkFlattenable::Type ft) {
    SkFlattenable* obj = this->readRawFlattenable();
    if (obj && obj->getFlattenableType() != ft) {
        this->validate(false);
        obj->unref();
        return nullptr;
    }
    return obj;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int32_t SkReadBuffer::checkInt(int32_t min, int32_t max) {
    SkASSERT(min <= max);
    int32_t value = this->read32();
    if (value < min || value > max) {
        this->validate(false);
        value = min;
    }
    return value;
}

SkLegacyFQ SkReadBuffer::checkFilterQuality() {
    return this->checkRange<SkLegacyFQ>(kNone_SkLegacyFQ, kLast_SkLegacyFQ);
}
