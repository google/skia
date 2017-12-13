/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkImageDeserializer.h"
#include "SkImageGenerator.h"
#include "SkPicture.h"
#include "SkPictureCommon.h"
#include "SkPictureData.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkPictureRecorder.h"
#include "SkSerialProcs.h"

#if defined(SK_DISALLOW_CROSSPROCESS_PICTUREIMAGEFILTERS) || \
    defined(SK_ENABLE_PICTURE_IO_SECURITY_PRECAUTIONS)
static bool g_AllPictureIOSecurityPrecautionsEnabled = true;
#else
static bool g_AllPictureIOSecurityPrecautionsEnabled = false;
#endif

/* SkPicture impl.  This handles generic responsibilities like unique IDs and serialization. */

SkPicture::SkPicture() : fUniqueID(0) {}

uint32_t SkPicture::uniqueID() const {
    static uint32_t gNextID = 1;
    uint32_t id = sk_atomic_load(&fUniqueID, sk_memory_order_relaxed);
    while (id == 0) {
        uint32_t next = sk_atomic_fetch_add(&gNextID, 1u);
        if (sk_atomic_compare_exchange(&fUniqueID, &id, next,
                                       sk_memory_order_relaxed,
                                       sk_memory_order_relaxed)) {
            id = next;
        } else {
            // sk_atomic_compare_exchange replaced id with the current value of fUniqueID.
        }
    }
    return id;
}

static const char kMagic[] = { 's', 'k', 'i', 'a', 'p', 'i', 'c', 't' };

SkPictInfo SkPicture::createHeader() const {
    SkPictInfo info;
    // Copy magic bytes at the beginning of the header
    static_assert(sizeof(kMagic) == 8, "");
    static_assert(sizeof(kMagic) == sizeof(info.fMagic), "");
    memcpy(info.fMagic, kMagic, sizeof(kMagic));

    // Set picture info after magic bytes in the header
    info.setVersion(CURRENT_PICTURE_VERSION);
    info.fCullRect = this->cullRect();
    info.fFlags = SkPictInfo::kCrossProcess_Flag;
    // TODO: remove this flag, since we're always float (now)
    info.fFlags |= SkPictInfo::kScalarIsFloat_Flag;

    if (8 == sizeof(void*)) {
        info.fFlags |= SkPictInfo::kPtrIs64Bit_Flag;
    }
    return info;
}

bool SkPicture::IsValidPictInfo(const SkPictInfo& info) {
    if (0 != memcmp(info.fMagic, kMagic, sizeof(kMagic))) {
        return false;
    }
    if (info.getVersion() < MIN_PICTURE_VERSION || info.getVersion() > CURRENT_PICTURE_VERSION) {
        return false;
    }
    return true;
}

bool SkPicture::InternalOnly_StreamIsSKP(SkStream* stream, SkPictInfo* pInfo) {
    if (!stream) {
        return false;
    }

    SkPictInfo info;
    SkASSERT(sizeof(kMagic) == sizeof(info.fMagic));
    if (!stream->read(&info.fMagic, sizeof(kMagic))) {
        return false;
    }

    info.setVersion(         stream->readU32());
    info.fCullRect.fLeft   = stream->readScalar();
    info.fCullRect.fTop    = stream->readScalar();
    info.fCullRect.fRight  = stream->readScalar();
    info.fCullRect.fBottom = stream->readScalar();
    info.fFlags            = stream->readU32();

    if (IsValidPictInfo(info)) {
        if (pInfo) { *pInfo = info; }
        return true;
    }
    return false;
}

bool SkPicture::InternalOnly_BufferIsSKP(SkReadBuffer* buffer, SkPictInfo* pInfo) {
    SkPictInfo info;
    SkASSERT(sizeof(kMagic) == sizeof(info.fMagic));
    if (!buffer->readByteArray(&info.fMagic, sizeof(kMagic))) {
        return false;
    }

    info.setVersion(buffer->readUInt());
    buffer->readRect(&info.fCullRect);
    info.fFlags = buffer->readUInt();

    if (IsValidPictInfo(info)) {
        if (pInfo) { *pInfo = info; }
        return true;
    }
    return false;
}

sk_sp<SkPicture> SkPicture::Forwardport(const SkPictInfo& info,
                                        const SkPictureData* data,
                                        SkReadBuffer* buffer) {
    if (!data) {
        return nullptr;
    }
    SkPicturePlayback playback(data);
    SkPictureRecorder r;
    playback.draw(r.beginRecording(info.fCullRect), nullptr/*no callback*/, buffer);
    return r.finishRecordingAsPicture();
}

sk_sp<SkPicture> SkPicture::MakeFromStream(SkStream* stream, SkImageDeserializer* factory) {
    SkDeserialProcs procs;
    procs.fImageProc = ImageDeserializer_SkDeserialImageProc;
    procs.fImageCtx  = factory;
    return MakeFromStream(stream, procs, nullptr);
}

sk_sp<SkPicture> SkPicture::MakeFromStream(SkStream* stream) {
    SkImageDeserializer factory;
    return MakeFromStream(stream, &factory);
}

sk_sp<SkPicture> SkPicture::MakeFromData(const void* data, size_t size,
                                         SkImageDeserializer* factory) {
    SkMemoryStream stream(data, size);
    return MakeFromStream(&stream, factory);
}

sk_sp<SkPicture> SkPicture::MakeFromData(const SkData* data, SkImageDeserializer* factory) {
    if (!data) {
        return nullptr;
    }
    SkMemoryStream stream(data->data(), data->size());
    return MakeFromStream(&stream, factory);
}

sk_sp<SkPicture> SkPicture::MakeFromData(const SkData* data, const SkDeserialProcs& procs) {
    if (!data) {
        return nullptr;
    }
    SkMemoryStream stream(data->data(), data->size());
    return MakeFromStream(&stream, procs, nullptr);
}

sk_sp<SkPicture> SkPicture::MakeFromData(sk_sp<SkData> data, const SkDeserialProcs& procs) {
    return MakeFromData(data.get(), procs);
}

sk_sp<SkPicture> SkPicture::MakeFromStream(SkStream* stream, const SkDeserialProcs& procs) {
    return MakeFromStream(stream, procs, nullptr);
}

sk_sp<SkPicture> SkPicture::MakeFromStream(SkStream* stream, const SkDeserialProcs& procs,
                                           SkTypefacePlayback* typefaces) {
    SkPictInfo info;
    if (!InternalOnly_StreamIsSKP(stream, &info) || !stream->readBool()) {
        return nullptr;
    }
    std::unique_ptr<SkPictureData> data(
            SkPictureData::CreateFromStream(stream, info, procs, typefaces));
    return Forwardport(info, data.get(), nullptr);
}

sk_sp<SkPicture> SkPicture::MakeFromBuffer(SkReadBuffer& buffer) {
    SkPictInfo info;
    if (!InternalOnly_BufferIsSKP(&buffer, &info) || !buffer.readBool()) {
        return nullptr;
    }
    std::unique_ptr<SkPictureData> data(SkPictureData::CreateFromBuffer(buffer, info));
    return Forwardport(info, data.get(), &buffer);
}

SkPictureData* SkPicture::backport() const {
    SkPictInfo info = this->createHeader();
    SkPictureRecord rec(SkISize::Make(info.fCullRect.width(), info.fCullRect.height()), 0/*flags*/);
    rec.beginRecording();
        this->playback(&rec);
    rec.endRecording();
    return new SkPictureData(rec, info);
}

void SkPicture::serialize(SkWStream* stream, SkPixelSerializer* pixelSerializer) const {
    SkSerialProcs procs;
    if (pixelSerializer) {
        procs.fImageProc = PixelSerializer_SkSerialImageProc;
        procs.fImageCtx  = pixelSerializer;
    }
    this->serialize(stream, procs, nullptr);
}

sk_sp<SkData> SkPicture::serialize(SkPixelSerializer* pixelSerializer) const {
    SkDynamicMemoryWStream stream;
    this->serialize(&stream, pixelSerializer);
    return stream.detachAsData();
}

sk_sp<SkData> SkPicture::serialize(const SkSerialProcs& procs) const {
    SkDynamicMemoryWStream stream;
    this->serialize(&stream, procs, nullptr);
    return stream.detachAsData();
}

void SkPicture::serialize(SkWStream* stream, const SkSerialProcs& procs,
                          SkRefCntSet* typefaceSet) const {
    SkPictInfo info = this->createHeader();
    std::unique_ptr<SkPictureData> data(this->backport());

    stream->write(&info, sizeof(info));
    if (data) {
        stream->writeBool(true);
        data->serialize(stream, procs, typefaceSet);
    } else {
        stream->writeBool(false);
    }
}

void SkPicture::flatten(SkWriteBuffer& buffer) const {
    SkPictInfo info = this->createHeader();
    std::unique_ptr<SkPictureData> data(this->backport());

    buffer.writeByteArray(&info.fMagic, sizeof(info.fMagic));
    buffer.writeUInt(info.getVersion());
    buffer.writeRect(info.fCullRect);
    buffer.writeUInt(info.fFlags);
    if (data) {
        buffer.writeBool(true);
        data->flatten(buffer);
    } else {
        buffer.writeBool(false);
    }
}

#ifdef SK_SUPPORT_LEGACY_PICTURE_GPUVETO
bool SkPicture::suitableForGpuRasterization(GrContext*, const char** whyNot) const {
    if (this->numSlowPaths() > 5) {
        if (whyNot) { *whyNot = "Too many slow paths (either concave or dashed)."; }
        return false;
    }
    return true;
}
#endif

// Global setting to disable security precautions for serialization.
void SkPicture::SetPictureIOSecurityPrecautionsEnabled_Dangerous(bool set) {
    g_AllPictureIOSecurityPrecautionsEnabled = set;
}

bool SkPicture::PictureIOSecurityPrecautionsEnabled() {
    return g_AllPictureIOSecurityPrecautionsEnabled;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkData> PixelSerializer_SkSerialImageProc(SkImage* img, void* ctx) {
    SkASSERT(ctx);
    return img->encodeToData(static_cast<SkPixelSerializer*>(ctx));
}

sk_sp<SkImage> ImageDeserializer_SkDeserialImageProc(const void* data, size_t length, void* ctx) {
    SkASSERT(ctx);
    SkImageDeserializer* imd = static_cast<SkImageDeserializer*>(ctx);
    const SkIRect* subset = nullptr;
    return imd->makeFromMemory(data, length, subset);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_LEGACY_SERIAL_BUFFER_OBJECTS
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

void SkBinaryWriteBuffer::setPixelSerializer(sk_sp<SkPixelSerializer> ps) {
    fPS = ps;
    if (ps) {
        fProcs.fImageProc = PixelSerializer_SkSerialImageProc;
        fProcs.fImageCtx  = ps.get();
    } else {
        fProcs.fImageProc = nullptr;
        fProcs.fImageCtx  = nullptr;
    }
}

void SkReadBuffer::setImageDeserializer(SkImageDeserializer* factory) {
    if (factory) {
        fProcs.fImageProc = ImageDeserializer_SkDeserialImageProc;
        fProcs.fImageCtx  = factory;
    } else {
        fProcs.fImageProc = nullptr;
        fProcs.fImageCtx  = nullptr;
    }
}
#endif

