/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkMessageBus.h"
#include "SkPicture.h"
#include "SkPictureData.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkPictureRecorder.h"

#if defined(SK_DISALLOW_CROSSPROCESS_PICTUREIMAGEFILTERS) || \
    defined(SK_ENABLE_PICTURE_IO_SECURITY_PRECAUTIONS)
static bool g_AllPictureIOSecurityPrecautionsEnabled = true;
#else
static bool g_AllPictureIOSecurityPrecautionsEnabled = false;
#endif

DECLARE_SKMESSAGEBUS_MESSAGE(SkPicture::DeletionMessage);

/* SkPicture impl.  This handles generic responsibilities like unique IDs and serialization. */

SkPicture::SkPicture() : fUniqueID(0) {}

SkPicture::~SkPicture() {
    // TODO: move this to ~SkBigPicture() only?

    // If the ID is still zero, no one has read it, so no need to send this message.
    uint32_t id = sk_atomic_load(&fUniqueID, sk_memory_order_relaxed);
    if (id != 0) {
        SkPicture::DeletionMessage msg = { (int32_t)id };
        SkMessageBus<SkPicture::DeletionMessage>::Post(msg);
    }
}

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
    info.fVersion = CURRENT_PICTURE_VERSION;
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
    if (info.fVersion < MIN_PICTURE_VERSION || info.fVersion > CURRENT_PICTURE_VERSION) {
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

    info.fVersion          = stream->readU32();
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

    info.fVersion = buffer->readUInt();
    buffer->readRect(&info.fCullRect);
    info.fFlags = buffer->readUInt();

    if (IsValidPictInfo(info)) {
        if (pInfo) { *pInfo = info; }
        return true;
    }
    return false;
}

SkPicture* SkPicture::Forwardport(const SkPictInfo& info, const SkPictureData* data) {
    if (!data) {
        return nullptr;
    }
    SkPicturePlayback playback(data);
    SkPictureRecorder r;
    playback.draw(r.beginRecording(info.fCullRect), nullptr/*no callback*/);
    return r.endRecording();
}

SkPicture* SkPicture::CreateFromStream(SkStream* stream, InstallPixelRefProc proc) {
    SkPictInfo info;
    if (!InternalOnly_StreamIsSKP(stream, &info) || !stream->readBool()) {
        return nullptr;
    }
    SkAutoTDelete<SkPictureData> data(SkPictureData::CreateFromStream(stream, info, proc));
    return Forwardport(info, data);
}

SkPicture* SkPicture::CreateFromBuffer(SkReadBuffer& buffer) {
    SkPictInfo info;
    if (!InternalOnly_BufferIsSKP(&buffer, &info) || !buffer.readBool()) {
        return nullptr;
    }
    SkAutoTDelete<SkPictureData> data(SkPictureData::CreateFromBuffer(buffer, info));
    return Forwardport(info, data);
}

SkPictureData* SkPicture::backport() const {
    SkPictInfo info = this->createHeader();
    SkPictureRecord rec(SkISize::Make(info.fCullRect.width(), info.fCullRect.height()), 0/*flags*/);
    rec.beginRecording();
        this->playback(&rec);
    rec.endRecording();
    return SkNEW_ARGS(SkPictureData, (rec, info, false/*deep copy ops?*/));
}

void SkPicture::serialize(SkWStream* stream, SkPixelSerializer* pixelSerializer) const {
    SkPictInfo info = this->createHeader();
    SkAutoTDelete<SkPictureData> data(this->backport());

    stream->write(&info, sizeof(info));
    if (data) {
        stream->writeBool(true);
        data->serialize(stream, pixelSerializer);
    } else {
        stream->writeBool(false);
    }
}

void SkPicture::flatten(SkWriteBuffer& buffer) const {
    SkPictInfo info = this->createHeader();
    SkAutoTDelete<SkPictureData> data(this->backport());

    buffer.writeByteArray(&info.fMagic, sizeof(info.fMagic));
    buffer.writeUInt(info.fVersion);
    buffer.writeRect(info.fCullRect);
    buffer.writeUInt(info.fFlags);
    if (data) {
        buffer.writeBool(true);
        data->flatten(buffer);
    } else {
        buffer.writeBool(false);
    }
}

bool SkPicture::suitableForGpuRasterization(GrContext*, const char** whyNot) const {
    if (this->numSlowPaths() > 5) {
        if (whyNot) { *whyNot = "Too many slow paths (either concave or dashed)."; }
        return false;
    }
    return true;
}

// Global setting to disable security precautions for serialization.
void SkPicture::SetPictureIOSecurityPrecautionsEnabled_Dangerous(bool set) {
    g_AllPictureIOSecurityPrecautionsEnabled = set;
}

bool SkPicture::PictureIOSecurityPrecautionsEnabled() {
    return g_AllPictureIOSecurityPrecautionsEnabled;
}
