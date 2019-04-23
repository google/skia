/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPicture.h"

#include "include/core/SkImageGenerator.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSerialProcs.h"
#include "include/private/SkTo.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkPictureCommon.h"
#include "src/core/SkPictureData.h"
#include "src/core/SkPicturePlayback.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkPictureRecord.h"
#include <atomic>

// When we read/write the SkPictInfo via a stream, we have a sentinel byte right after the info.
// Note: in the read/write buffer versions, we have a slightly different convention:
//      We have a sentinel int32_t:
//          0 : failure
//          1 : PictureData
//         <0 : -size of the custom data
enum {
    kFailure_TrailingStreamByteAfterPictInfo     = 0,   // nothing follows
    kPictureData_TrailingStreamByteAfterPictInfo = 1,   // SkPictureData follows
    kCustom_TrailingStreamByteAfterPictInfo      = 2,   // -size32 follows
};

/* SkPicture impl.  This handles generic responsibilities like unique IDs and serialization. */

SkPicture::SkPicture() {
    static std::atomic<uint32_t> nextID{1};
    do {
        fUniqueID = nextID.fetch_add(+1, std::memory_order_relaxed);
    } while (fUniqueID == 0);
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

bool SkPicture::StreamIsSKP(SkStream* stream, SkPictInfo* pInfo) {
    if (!stream) {
        return false;
    }

    SkPictInfo info;
    SkASSERT(sizeof(kMagic) == sizeof(info.fMagic));
    if (stream->read(&info.fMagic, sizeof(kMagic)) != sizeof(kMagic)) {
        return false;
    }

    uint32_t version;
    if (!stream->readU32(&version)) { return false; }
    info.setVersion(version);
    if (!stream->readScalar(&info.fCullRect.fLeft  )) { return false; }
    if (!stream->readScalar(&info.fCullRect.fTop   )) { return false; }
    if (!stream->readScalar(&info.fCullRect.fRight )) { return false; }
    if (!stream->readScalar(&info.fCullRect.fBottom)) { return false; }
    if (info.getVersion() < SkReadBuffer::kRemoveHeaderFlags_Version) {
        if (!stream->readU32(nullptr)) { return false; }
    }

    if (!IsValidPictInfo(info)) { return false; }

    if (pInfo) { *pInfo = info; }
    return true;
}
bool SkPicture_StreamIsSKP(SkStream* stream, SkPictInfo* pInfo) {
    return SkPicture::StreamIsSKP(stream, pInfo);
}

bool SkPicture::BufferIsSKP(SkReadBuffer* buffer, SkPictInfo* pInfo) {
    SkPictInfo info;
    SkASSERT(sizeof(kMagic) == sizeof(info.fMagic));
    if (!buffer->readByteArray(&info.fMagic, sizeof(kMagic))) {
        return false;
    }

    info.setVersion(buffer->readUInt());
    buffer->readRect(&info.fCullRect);
    if (info.getVersion() < SkReadBuffer::kRemoveHeaderFlags_Version) {
        (void)buffer->readUInt();   // used to be flags
    }

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
    if (!data->opData()) {
        return nullptr;
    }
    SkPicturePlayback playback(data);
    SkPictureRecorder r;
    playback.draw(r.beginRecording(info.fCullRect), nullptr/*no callback*/, buffer);
    return r.finishRecordingAsPicture();
}

sk_sp<SkPicture> SkPicture::MakeFromStream(SkStream* stream, const SkDeserialProcs* procs) {
    return MakeFromStream(stream, procs, nullptr);
}

sk_sp<SkPicture> SkPicture::MakeFromData(const void* data, size_t size,
                                         const SkDeserialProcs* procs) {
    if (!data) {
        return nullptr;
    }
    SkMemoryStream stream(data, size);
    return MakeFromStream(&stream, procs, nullptr);
}

sk_sp<SkPicture> SkPicture::MakeFromData(const SkData* data, const SkDeserialProcs* procs) {
    if (!data) {
        return nullptr;
    }
    SkMemoryStream stream(data->data(), data->size());
    return MakeFromStream(&stream, procs, nullptr);
}

sk_sp<SkPicture> SkPicture::MakeFromStream(SkStream* stream, const SkDeserialProcs* procsPtr,
                                           SkTypefacePlayback* typefaces) {
    SkPictInfo info;
    if (!StreamIsSKP(stream, &info)) {
        return nullptr;
    }

    SkDeserialProcs procs;
    if (procsPtr) {
        procs = *procsPtr;
    }

    uint8_t trailingStreamByteAfterPictInfo;
    if (!stream->readU8(&trailingStreamByteAfterPictInfo)) { return nullptr; }
    switch (trailingStreamByteAfterPictInfo) {
        case kPictureData_TrailingStreamByteAfterPictInfo: {
            std::unique_ptr<SkPictureData> data(
                    SkPictureData::CreateFromStream(stream, info, procs, typefaces));
            return Forwardport(info, data.get(), nullptr);
        }
        case kCustom_TrailingStreamByteAfterPictInfo: {
            int32_t ssize;
            if (!stream->readS32(&ssize) || ssize >= 0 || !procs.fPictureProc) {
                return nullptr;
            }
            size_t size = sk_negate_to_size_t(ssize);
            auto data = SkData::MakeUninitialized(size);
            if (stream->read(data->writable_data(), size) != size) {
                return nullptr;
            }
            return procs.fPictureProc(data->data(), size, procs.fPictureCtx);
        }
        default:    // fall through to error return
            break;
    }
    return nullptr;
}

sk_sp<SkPicture> SkPicturePriv::MakeFromBuffer(SkReadBuffer& buffer) {
    SkPictInfo info;
    if (!SkPicture::BufferIsSKP(&buffer, &info)) {
        return nullptr;
    }
    // size should be 0, 1, or negative
    int32_t ssize = buffer.read32();
    if (ssize < 0) {
        const SkDeserialProcs& procs = buffer.getDeserialProcs();
        if (!procs.fPictureProc) {
            return nullptr;
        }
        size_t size = sk_negate_to_size_t(ssize);
        return procs.fPictureProc(buffer.skip(size), size, procs.fPictureCtx);
    }
    if (ssize != 1) {
        // 1 is the magic 'size' that means SkPictureData follows
        return nullptr;
    }
   std::unique_ptr<SkPictureData> data(SkPictureData::CreateFromBuffer(buffer, info));
    return SkPicture::Forwardport(info, data.get(), &buffer);
}

SkPictureData* SkPicture::backport() const {
    SkPictInfo info = this->createHeader();
    SkPictureRecord rec(SkISize::Make(info.fCullRect.width(), info.fCullRect.height()), 0/*flags*/);
    rec.beginRecording();
        this->playback(&rec);
    rec.endRecording();
    return new SkPictureData(rec, info);
}

void SkPicture::serialize(SkWStream* stream, const SkSerialProcs* procs) const {
    this->serialize(stream, procs, nullptr);
}

sk_sp<SkData> SkPicture::serialize(const SkSerialProcs* procs) const {
    SkDynamicMemoryWStream stream;
    this->serialize(&stream, procs, nullptr);
    return stream.detachAsData();
}

static sk_sp<SkData> custom_serialize(const SkPicture* picture, const SkSerialProcs& procs) {
    if (procs.fPictureProc) {
        auto data = procs.fPictureProc(const_cast<SkPicture*>(picture), procs.fPictureCtx);
        if (data) {
            size_t size = data->size();
            if (!SkTFitsIn<int32_t>(size) || size <= 1) {
                return SkData::MakeEmpty();
            }
            return data;
        }
    }
    return nullptr;
}

static bool write_pad32(SkWStream* stream, const void* data, size_t size) {
    if (!stream->write(data, size)) {
        return false;
    }
    if (size & 3) {
        uint32_t zero = 0;
        return stream->write(&zero, 4 - (size & 3));
    }
    return true;
}

void SkPicture::serialize(SkWStream* stream, const SkSerialProcs* procsPtr,
                          SkRefCntSet* typefaceSet) const {
    SkSerialProcs procs;
    if (procsPtr) {
        procs = *procsPtr;
    }

    SkPictInfo info = this->createHeader();
    stream->write(&info, sizeof(info));

    if (auto custom = custom_serialize(this, procs)) {
        int32_t size = SkToS32(custom->size());
        if (size == 0) {
            stream->write8(kFailure_TrailingStreamByteAfterPictInfo);
            return;
        }
        stream->write8(kCustom_TrailingStreamByteAfterPictInfo);
        stream->write32(-size);    // negative for custom format
        write_pad32(stream, custom->data(), size);
        return;
    }

    std::unique_ptr<SkPictureData> data(this->backport());
    if (data) {
        stream->write8(kPictureData_TrailingStreamByteAfterPictInfo);
        data->serialize(stream, procs, typefaceSet);
    } else {
        stream->write8(kFailure_TrailingStreamByteAfterPictInfo);
    }
}

void SkPicturePriv::Flatten(const sk_sp<const SkPicture> picture, SkWriteBuffer& buffer) {
    SkPictInfo info = picture->createHeader();
    std::unique_ptr<SkPictureData> data(picture->backport());

    buffer.writeByteArray(&info.fMagic, sizeof(info.fMagic));
    buffer.writeUInt(info.getVersion());
    buffer.writeRect(info.fCullRect);

    if (auto custom = custom_serialize(picture.get(), buffer.fProcs)) {
        int32_t size = SkToS32(custom->size());
        buffer.write32(-size);    // negative for custom format
        buffer.writePad32(custom->data(), size);
        return;
    }

    if (data) {
        buffer.write32(1); // special size meaning SkPictureData
        data->flatten(buffer);
    } else {
        buffer.write32(0); // signal no content
    }
}

sk_sp<SkPicture> SkPicture::MakePlaceholder(SkRect cull) {
    struct Placeholder : public SkPicture {
          explicit Placeholder(SkRect cull) : fCull(cull) {}

          void playback(SkCanvas*, AbortCallback*) const override { }

          // approximateOpCount() needs to be greater than kMaxPictureOpsToUnrollInsteadOfRef
          // in SkCanvas.cpp to avoid that unrolling.  SK_MaxS32 can't not be big enough!
          int    approximateOpCount()   const override { return SK_MaxS32; }
          size_t approximateBytesUsed() const override { return sizeof(*this); }
          SkRect cullRect()             const override { return fCull; }

          SkRect fCull;
    };
    return sk_make_sp<Placeholder>(cull);
}
