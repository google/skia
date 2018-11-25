/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkDeduper.h"
#include "SkImage.h"
#include "SkImageGenerator.h"
#include "SkMakeUnique.h"
#include "SkMathPriv.h"
#include "SkMatrixPriv.h"
#include "SkReadBuffer.h"
#include "SkSafeMath.h"
#include "SkStream.h"
#include "SkTypeface.h"

namespace {
    // This generator intentionally should always fail on all attempts to get its pixels,
    // simulating a bad or empty codec stream.
    class EmptyImageGenerator final : public SkImageGenerator {
    public:
        EmptyImageGenerator(const SkImageInfo& info) : INHERITED(info) { }

    private:
        typedef SkImageGenerator INHERITED;
    };

    static sk_sp<SkImage> MakeEmptyImage(int width, int height) {
        return SkImage::MakeFromGenerator(
              skstd::make_unique<EmptyImageGenerator>(SkImageInfo::MakeN32Premul(width, height)));
    }

} // anonymous namespace


SkReadBuffer::SkReadBuffer() {
    fVersion = 0;
    fMemoryPtr = nullptr;

    fTFArray = nullptr;
    fTFCount = 0;

    fFactoryArray = nullptr;
    fFactoryCount = 0;
#ifdef DEBUG_NON_DETERMINISTIC_ASSERT
    fDecodedBitmapIndex = -1;
#endif // DEBUG_NON_DETERMINISTIC_ASSERT
}

SkReadBuffer::SkReadBuffer(const void* data, size_t size) {
    fVersion = 0;
    this->setMemory(data, size);
    fMemoryPtr = nullptr;

    fTFArray = nullptr;
    fTFCount = 0;

    fFactoryArray = nullptr;
    fFactoryCount = 0;
#ifdef DEBUG_NON_DETERMINISTIC_ASSERT
    fDecodedBitmapIndex = -1;
#endif // DEBUG_NON_DETERMINISTIC_ASSERT
}

SkReadBuffer::~SkReadBuffer() {
    sk_free(fMemoryPtr);
}

void SkReadBuffer::setMemory(const void* data, size_t size) {
    this->validate(IsPtrAlign4(data) && (SkAlign4(size) == size));
    if (!fError) {
        fReader.setMemory(data, size);
    }
}
void SkReadBuffer::setInvalid() {
    if (!fError) {
        // When an error is found, send the read cursor to the end of the stream
        fReader.skip(fReader.available());
        fError = true;
    }
}

const void* SkReadBuffer::skip(size_t size) {
    size_t inc = SkAlign4(size);
    this->validate(inc >= size);
    const void* addr = fReader.peek();
    this->validate(IsPtrAlign4(addr) && fReader.isAvailable(inc));
    if (fError) {
        return nullptr;
    }

    fReader.skip(size);
    return addr;
}

const void* SkReadBuffer::skip(size_t count, size_t size) {
    return this->skip(SkSafeMath::Mul(count, size));
}

void SkReadBuffer::setDeserialProcs(const SkDeserialProcs& procs) {
    fProcs = procs;
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
    this->validate(IsPtrAlign4(fReader.peek()) && fReader.isAvailable(inc));
    return fError ? 0 : fReader.readInt();
}

SkScalar SkReadBuffer::readScalar() {
    const size_t inc = sizeof(SkScalar);
    this->validate(IsPtrAlign4(fReader.peek()) && fReader.isAvailable(inc));
    return fError ? 0 : fReader.readScalar();
}

uint32_t SkReadBuffer::readUInt() {
    return this->readInt();
}

int32_t SkReadBuffer::read32() {
    return this->readInt();
}

uint8_t SkReadBuffer::peekByte() {
    if (fReader.available() <= 0) {
        fError = true;
        return 0;
    }
    return *((uint8_t*) fReader.peek());
}

bool SkReadBuffer::readPad32(void* buffer, size_t bytes) {
    if (const void* src = this->skip(bytes)) {
        memcpy(buffer, src, bytes);
        return true;
    }
    return false;
}

void SkReadBuffer::readString(SkString* string) {
    const size_t len = this->readUInt();
    // skip over the string + '\0'
    if (const char* src = this->skipT<char>(len + 1)) {
        if (this->validate(src[len] == 0)) {
            string->set(src, len);
            return;
        }
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

void SkReadBuffer::readMatrix(SkMatrix* matrix) {
    size_t size = 0;
    if (this->isValid()) {
        size = SkMatrixPriv::ReadFromMemory(matrix, fReader.peek(), fReader.available());
        if (!this->validate((SkAlign4(size) == size) && (0 != size))) {
            matrix->reset();
        }
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

void SkReadBuffer::readRRect(SkRRect* rrect) {
    if (!this->validate(fReader.readRRect(rrect))) {
        rrect->setEmpty();
    }
}

void SkReadBuffer::readRegion(SkRegion* region) {
    size_t size = 0;
    if (!fError) {
        size = region->readFromMemory(fReader.peek(), fReader.available());
        if (!this->validate((SkAlign4(size) == size) && (0 != size))) {
            region->setEmpty();
        }
    }
    (void)this->skip(size);
}

void SkReadBuffer::readPath(SkPath* path) {
    size_t size = 0;
    if (!fError) {
        size = path->readFromMemory(fReader.peek(), fReader.available());
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

bool SkReadBuffer::readColorArray(SkColor* colors, size_t size) {
    return this->readArray(colors, size, sizeof(SkColor));
}

bool SkReadBuffer::readColor4fArray(SkColor4f* colors, size_t size) {
    return this->readArray(colors, size, sizeof(SkColor4f));
}

bool SkReadBuffer::readIntArray(int32_t* values, size_t size) {
    return this->readArray(values, size, sizeof(int32_t));
}

bool SkReadBuffer::readPointArray(SkPoint* points, size_t size) {
    return this->readArray(points, size, sizeof(SkPoint));
}

bool SkReadBuffer::readScalarArray(SkScalar* values, size_t size) {
    return this->readArray(values, size, sizeof(SkScalar));
}

uint32_t SkReadBuffer::getArrayCount() {
    const size_t inc = sizeof(uint32_t);
    fError = fError || !IsPtrAlign4(fReader.peek()) || !fReader.isAvailable(inc);
    return fError ? 0 : *(uint32_t*)fReader.peek();
}

sk_sp<SkImage> SkReadBuffer::readImage() {
    if (fInflator) {
        SkImage* img = fInflator->getImage(this->read32());
        return img ? sk_ref_sp(img) : nullptr;
    }

    int width = this->read32();
    int height = this->read32();
    if (width <= 0 || height <= 0) {    // SkImage never has a zero dimension
        this->validate(false);
        return nullptr;
    }

    /*
     *  What follows is a 32bit encoded size.
     *   0 : failure, nothing else to do
     *  <0 : negative (int32_t) of a custom encoded blob using SerialProcs
     *  >0 : standard encoded blob size (use MakeFromEncoded)
     */

    int32_t encoded_size = this->read32();
    if (encoded_size == 0) {
        // The image could not be encoded at serialization time - return an empty placeholder.
        return MakeEmptyImage(width, height);
    }
    if (encoded_size == 1) {
        // legacy check (we stopped writing this for "raw" images Nov-2017)
        this->validate(false);
        return nullptr;
    }

    size_t size = SkAbs32(encoded_size);
    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    if (!this->readPad32(data->writable_data(), size)) {
        this->validate(false);
        return nullptr;
    }
    int32_t originX = this->read32();
    int32_t originY = this->read32();
    if (originX < 0 || originY < 0) {
        this->validate(false);
        return nullptr;
    }

    sk_sp<SkImage> image;
    if (encoded_size < 0) {     // custom encoded, need serial proc
        if (fProcs.fImageProc) {
            image = fProcs.fImageProc(data->data(), data->size(), fProcs.fImageCtx);
        } else {
            // Nothing to do (no client proc), but since we've already "read" the custom data,
            // wee just leave image as nullptr.
        }
    } else {
        SkIRect subset = SkIRect::MakeXYWH(originX, originY, width, height);
        image = SkImage::MakeFromEncoded(std::move(data), &subset);
    }
    // Question: are we correct to return an "empty" image instead of nullptr, if the decoder
    //           failed for some reason?
    return image ? image : MakeEmptyImage(width, height);
}

sk_sp<SkTypeface> SkReadBuffer::readTypeface() {
    if (fInflator) {
        return sk_ref_sp(fInflator->getTypeface(this->read32()));
    }

    // Read 32 bits (signed)
    //   0 -- return null (default font)
    //  >0 -- index
    //  <0 -- custom (serial procs) : negative size in bytes

    int32_t index = this->read32();
    if (index == 0) {
        return nullptr;
    } else if (index > 0) {
        if (!this->validate(index <= fTFCount)) {
            return nullptr;
        }
        return sk_ref_sp(fTFArray[index - 1]);
    } else {    // custom
        size_t size = sk_negate_to_size_t(index);
        const void* data = this->skip(size);
        if (!this->validate(data != nullptr && fProcs.fTypefaceProc)) {
            return nullptr;
        }
        return fProcs.fTypefaceProc(data, size, fProcs.fTypefaceCtx);
    }
}

SkFlattenable* SkReadBuffer::readFlattenable(SkFlattenable::Type ft) {
    SkFlattenable::Factory factory = nullptr;

    if (fInflator) {
        factory = fInflator->getFactory(this->read32());
        if (!factory) {
            return nullptr;
        }
    } else if (fFactoryCount > 0) {
        int32_t index = this->read32();
        if (0 == index || !this->isValid()) {
            return nullptr; // writer failed to give us the flattenable
        }
        index -= 1;     // we stored the index-base-1
        if ((unsigned)index >= (unsigned)fFactoryCount) {
            this->validate(false);
            return nullptr;
        }
        factory = fFactoryArray[index];
    } else {
        SkString name;
        if (this->peekByte()) {
            // If the first byte is non-zero, the flattenable is specified by a string.
            this->readString(&name);

            // Add the string to the dictionary.
            fFlattenableDict.set(fFlattenableDict.count() + 1, name);
        } else {
            // Read the index.  We are guaranteed that the first byte
            // is zeroed, so we must shift down a byte.
            uint32_t index = this->readUInt() >> 8;
            if (index == 0) {
                return nullptr; // writer failed to give us the flattenable
            }
            SkString* namePtr = fFlattenableDict.find(index);
            if (!this->validate(namePtr != nullptr)) {
                return nullptr;
            }
            name = *namePtr;
        }

        // Check if a custom Factory has been specified for this flattenable.
        if (!(factory = this->getCustomFactory(name))) {
            // If there is no custom Factory, check for a default.
            if (!(factory = SkFlattenable::NameToFactory(name.c_str()))) {
                return nullptr; // writer failed to give us the flattenable
            }
        }
    }

    // if we get here, factory may still be null, but if that is the case, the
    // failure was ours, not the writer.
    sk_sp<SkFlattenable> obj;
    uint32_t sizeRecorded = this->read32();
    if (factory) {
        size_t offset = fReader.offset();
        obj = (*factory)(*this);
        // check that we read the amount we expected
        size_t sizeRead = fReader.offset() - offset;
        if (sizeRecorded != sizeRead) {
            this->validate(false);
            return nullptr;
        }
        if (obj && obj->getFlattenableType() != ft) {
            this->validate(false);
            return nullptr;
        }
    } else {
        // we must skip the remaining data
        fReader.skip(sizeRecorded);
    }
    return obj.release();
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

SkFilterQuality SkReadBuffer::checkFilterQuality() {
    return this->checkRange<SkFilterQuality>(kNone_SkFilterQuality, kLast_SkFilterQuality);
}
