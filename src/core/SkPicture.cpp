/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPictureFlat.h"
#include "SkPictureData.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkPictureRecorder.h"

#include "SkAtomics.h"
#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkChunkAlloc.h"
#include "SkMessageBus.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkPicture.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTLogic.h"
#include "SkTSearch.h"
#include "SkTime.h"

#include "SkReader32.h"
#include "SkWriter32.h"
#include "SkRTree.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"

DECLARE_SKMESSAGEBUS_MESSAGE(SkPicture::DeletionMessage);

template <typename T> int SafeCount(const T* obj) {
    return obj ? obj->count() : 0;
}

///////////////////////////////////////////////////////////////////////////////

namespace {

// Some commands have a paint, some have an optional paint.  Either way, get back a pointer.
static const SkPaint* AsPtr(const SkPaint& p) { return &p; }
static const SkPaint* AsPtr(const SkRecords::Optional<SkPaint>& p) { return p; }

/** SkRecords visitor to determine whether an instance may require an
    "external" bitmap to rasterize. May return false positives.
    Does not return true for bitmap text.

    Expected use is to determine whether images need to be decoded before
    rasterizing a particular SkRecord.
 */
struct BitmapTester {
    // Helpers.  These create HasMember_bitmap and HasMember_paint.
    SK_CREATE_MEMBER_DETECTOR(bitmap);
    SK_CREATE_MEMBER_DETECTOR(paint);


    // Main entry for visitor:
    // If the command is a DrawPicture, recurse.
    // If the command has a bitmap directly, return true.
    // If the command has a paint and the paint has a bitmap, return true.
    // Otherwise, return false.
    bool operator()(const SkRecords::DrawPicture& op) { return op.picture->willPlayBackBitmaps(); }

    template <typename T>
    bool operator()(const T& r) { return CheckBitmap(r); }


    // If the command has a bitmap, of course we're going to play back bitmaps.
    template <typename T>
    static SK_WHEN(HasMember_bitmap<T>, bool) CheckBitmap(const T&) { return true; }

    // If not, look for one in its paint (if it has a paint).
    template <typename T>
    static SK_WHEN(!HasMember_bitmap<T>, bool) CheckBitmap(const T& r) { return CheckPaint(r); }

    // If we have a paint, dig down into the effects looking for a bitmap.
    template <typename T>
    static SK_WHEN(HasMember_paint<T>, bool) CheckPaint(const T& r) {
        const SkPaint* paint = AsPtr(r.paint);
        if (paint) {
            const SkShader* shader = paint->getShader();
            if (shader &&
                shader->asABitmap(NULL, NULL, NULL) == SkShader::kDefault_BitmapType) {
                return true;
            }
        }
        return false;
    }

    // If we don't have a paint, that non-paint has no bitmap.
    template <typename T>
    static SK_WHEN(!HasMember_paint<T>, bool) CheckPaint(const T&) { return false; }
};

bool WillPlaybackBitmaps(const SkRecord& record) {
    BitmapTester tester;
    for (unsigned i = 0; i < record.count(); i++) {
        if (record.visit<bool>(i, tester)) {
            return true;
        }
    }
    return false;
}

// SkRecord visitor to find recorded text.
struct TextHunter {
    // All ops with text have that text as a char array member named "text".
    SK_CREATE_MEMBER_DETECTOR(text);
    bool operator()(const SkRecords::DrawPicture& op) { return op.picture->hasText(); }
    template <typename T> SK_WHEN(HasMember_text<T>,  bool) operator()(const T&) { return true;  }
    template <typename T> SK_WHEN(!HasMember_text<T>, bool) operator()(const T&) { return false; }
};

} // namespace

/** SkRecords visitor to determine heuristically whether or not a SkPicture
    will be performant when rasterized on the GPU.
 */
struct SkPicture::PathCounter {
    SK_CREATE_MEMBER_DETECTOR(paint);

    PathCounter() : fNumSlowPathsAndDashEffects(0) {}

    // Recurse into nested pictures.
    void operator()(const SkRecords::DrawPicture& op) {
        const SkPicture::Analysis& analysis = op.picture->analysis();
        fNumSlowPathsAndDashEffects += analysis.fNumSlowPathsAndDashEffects;
    }

    void checkPaint(const SkPaint* paint) {
        if (paint && paint->getPathEffect()) {
            // Initially assume it's slow.
            fNumSlowPathsAndDashEffects++;
        }
    }

    void operator()(const SkRecords::DrawPoints& op) {
        this->checkPaint(&op.paint);
        const SkPathEffect* effect = op.paint.getPathEffect();
        if (effect) {
            SkPathEffect::DashInfo info;
            SkPathEffect::DashType dashType = effect->asADash(&info);
            if (2 == op.count && SkPaint::kRound_Cap != op.paint.getStrokeCap() &&
                SkPathEffect::kDash_DashType == dashType && 2 == info.fCount) {
                fNumSlowPathsAndDashEffects--;
            }
        }
    }

    void operator()(const SkRecords::DrawPath& op) {
        this->checkPaint(&op.paint);
        if (op.paint.isAntiAlias() && !op.path.isConvex()) {
            SkPaint::Style paintStyle = op.paint.getStyle();
            const SkRect& pathBounds = op.path.getBounds();
            if (SkPaint::kStroke_Style == paintStyle &&
                0 == op.paint.getStrokeWidth()) {
                // AA hairline concave path is not slow.
            } else if (SkPaint::kFill_Style == paintStyle && pathBounds.width() < 64.f &&
                       pathBounds.height() < 64.f && !op.path.isVolatile()) {
                // AADF eligible concave path is not slow.
            } else {
                fNumSlowPathsAndDashEffects++;
            }
        }
    }

    template <typename T>
    SK_WHEN(HasMember_paint<T>, void) operator()(const T& op) {
        this->checkPaint(AsPtr(op.paint));
    }

    template <typename T>
    SK_WHEN(!HasMember_paint<T>, void) operator()(const T& op) { /* do nothing */ }

    int fNumSlowPathsAndDashEffects;
};

SkPicture::Analysis::Analysis(const SkRecord& record) {
    fWillPlaybackBitmaps = WillPlaybackBitmaps(record);

    PathCounter counter;
    for (unsigned i = 0; i < record.count(); i++) {
        record.visit<void>(i, counter);
    }
    fNumSlowPathsAndDashEffects = SkTMin<int>(counter.fNumSlowPathsAndDashEffects, 255);

    fHasText = false;
    TextHunter text;
    for (unsigned i = 0; i < record.count(); i++) {
        if (record.visit<bool>(i, text)) {
            fHasText = true;
            break;
        }
    }
}

bool SkPicture::Analysis::suitableForGpuRasterization(const char** reason,
                                                      int sampleCount) const {
    // TODO: the heuristic used here needs to be refined
    static const int kNumSlowPathsTol = 6;

    bool ret = fNumSlowPathsAndDashEffects < kNumSlowPathsTol;

    if (!ret && reason) {
        *reason = "Too many slow paths (either concave or dashed).";
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

int SkPicture::drawableCount() const {
    return fDrawablePicts.get() ? fDrawablePicts->count() : 0;
}

SkPicture const* const* SkPicture::drawablePicts() const {
    return fDrawablePicts.get() ? fDrawablePicts->begin() : NULL;
}

SkPicture::~SkPicture() {
    // If the ID is still zero, no one has read it, so no need to send this message.
    uint32_t id = sk_atomic_load(&fUniqueID, sk_memory_order_relaxed);
    if (id != 0) {
        SkPicture::DeletionMessage msg;
        msg.fUniqueID = id;
        SkMessageBus<SkPicture::DeletionMessage>::Post(msg);
    }
}

const SkPicture::AccelData* SkPicture::EXPERIMENTAL_getAccelData(
        SkPicture::AccelData::Key key) const {
    if (fAccelData.get() && fAccelData->getKey() == key) {
        return fAccelData.get();
    }
    return NULL;
}

SkPicture::AccelData::Domain SkPicture::AccelData::GenerateDomain() {
    static int32_t gNextID = 0;

    int32_t id = sk_atomic_inc(&gNextID);
    if (id >= 1 << (8 * sizeof(Domain))) {
        SK_CRASH();
    }

    return static_cast<Domain>(id);
}

///////////////////////////////////////////////////////////////////////////////

void SkPicture::playback(SkCanvas* canvas, AbortCallback* callback) const {
    SkASSERT(canvas);

    // If the query contains the whole picture, don't bother with the BBH.
    SkRect clipBounds = { 0, 0, 0, 0 };
    (void)canvas->getClipBounds(&clipBounds);
    const bool useBBH = !clipBounds.contains(this->cullRect());

    SkRecordDraw(*fRecord, canvas, this->drawablePicts(), NULL, this->drawableCount(),
                 useBBH ? fBBH.get() : NULL, callback);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

static const char kMagic[] = { 's', 'k', 'i', 'a', 'p', 'i', 'c', 't' };

bool SkPicture::IsValidPictInfo(const SkPictInfo& info) {
    if (0 != memcmp(info.fMagic, kMagic, sizeof(kMagic))) {
        return false;
    }

    if (info.fVersion < MIN_PICTURE_VERSION ||
        info.fVersion > CURRENT_PICTURE_VERSION) {
        return false;
    }

    return true;
}

bool SkPicture::InternalOnly_StreamIsSKP(SkStream* stream, SkPictInfo* pInfo) {
    if (NULL == stream) {
        return false;
    }

    // Check magic bytes.
    SkPictInfo info;
    SkASSERT(sizeof(kMagic) == sizeof(info.fMagic));

    if (!stream->read(&info.fMagic, sizeof(kMagic))) {
        return false;
    }

    info.fVersion = stream->readU32();
    info.fCullRect.fLeft = stream->readScalar();
    info.fCullRect.fTop = stream->readScalar();
    info.fCullRect.fRight = stream->readScalar();
    info.fCullRect.fBottom = stream->readScalar();

    info.fFlags = stream->readU32();

    if (!IsValidPictInfo(info)) {
        return false;
    }

    if (pInfo != NULL) {
        *pInfo = info;
    }
    return true;
}

bool SkPicture::InternalOnly_BufferIsSKP(SkReadBuffer* buffer, SkPictInfo* pInfo) {
    // Check magic bytes.
    SkPictInfo info;
    SkASSERT(sizeof(kMagic) == sizeof(info.fMagic));

    if (!buffer->readByteArray(&info.fMagic, sizeof(kMagic))) {
        return false;
    }

    info.fVersion = buffer->readUInt();
    buffer->readRect(&info.fCullRect);
    info.fFlags = buffer->readUInt();

    if (!IsValidPictInfo(info)) {
        return false;
    }

    if (pInfo != NULL) {
        *pInfo = info;
    }
    return true;
}

SkPicture* SkPicture::Forwardport(const SkPictInfo& info, const SkPictureData* data) {
    if (!data) {
        return NULL;
    }
    SkPicturePlayback playback(data);
    SkPictureRecorder r;
    playback.draw(r.beginRecording(SkScalarCeilToInt(info.fCullRect.width()),
                                   SkScalarCeilToInt(info.fCullRect.height())),
                  NULL/*no callback*/);
    return r.endRecording();
}

SkPicture* SkPicture::CreateFromStream(SkStream* stream, InstallPixelRefProc proc) {
    SkPictInfo info;
    if (!InternalOnly_StreamIsSKP(stream, &info) || !stream->readBool()) {
        return NULL;
    }
    SkAutoTDelete<SkPictureData> data(SkPictureData::CreateFromStream(stream, info, proc));
    return Forwardport(info, data);
}

SkPicture* SkPicture::CreateFromBuffer(SkReadBuffer& buffer) {
    SkPictInfo info;
    if (!InternalOnly_BufferIsSKP(&buffer, &info) || !buffer.readBool()) {
        return NULL;
    }
    SkAutoTDelete<SkPictureData> data(SkPictureData::CreateFromBuffer(buffer, info));
    return Forwardport(info, data);
}

void SkPicture::createHeader(SkPictInfo* info) const {
    // Copy magic bytes at the beginning of the header
    SkASSERT(sizeof(kMagic) == 8);
    SkASSERT(sizeof(kMagic) == sizeof(info->fMagic));
    memcpy(info->fMagic, kMagic, sizeof(kMagic));

    // Set picture info after magic bytes in the header
    info->fVersion = CURRENT_PICTURE_VERSION;
    info->fCullRect = this->cullRect();
    info->fFlags = SkPictInfo::kCrossProcess_Flag;
    // TODO: remove this flag, since we're always float (now)
    info->fFlags |= SkPictInfo::kScalarIsFloat_Flag;

    if (8 == sizeof(void*)) {
        info->fFlags |= SkPictInfo::kPtrIs64Bit_Flag;
    }
}

// This for compatibility with serialization code only.  This is not cheap.
SkPictureData* SkPicture::Backport(const SkRecord& src, const SkPictInfo& info,
                                   SkPicture const* const drawablePicts[], int drawableCount) {
    SkPictureRecord rec(SkISize::Make(info.fCullRect.width(), info.fCullRect.height()), 0/*flags*/);
    rec.beginRecording();
        SkRecordDraw(src, &rec, drawablePicts, NULL, drawableCount, NULL/*bbh*/, NULL/*callback*/);
    rec.endRecording();
    return SkNEW_ARGS(SkPictureData, (rec, info, false/*deep copy ops?*/));
}

void SkPicture::serialize(SkWStream* stream, SkPixelSerializer* pixelSerializer) const {
    SkPictInfo info;
    this->createHeader(&info);
    SkAutoTDelete<SkPictureData> data(Backport(*fRecord, info, this->drawablePicts(),
                                               this->drawableCount()));

    stream->write(&info, sizeof(info));
    if (data) {
        stream->writeBool(true);
        data->serialize(stream, pixelSerializer);
    } else {
        stream->writeBool(false);
    }
}

void SkPicture::flatten(SkWriteBuffer& buffer) const {
    SkPictInfo info;
    this->createHeader(&info);
    SkAutoTDelete<SkPictureData> data(Backport(*fRecord, info, this->drawablePicts(),
                                               this->drawableCount()));

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

const SkPicture::Analysis& SkPicture::analysis() const {
    auto create = [&](){ return SkNEW_ARGS(Analysis, (*fRecord)); };
    return *fAnalysis.get(create);
}

#if SK_SUPPORT_GPU
bool SkPicture::suitableForGpuRasterization(GrContext*, const char **reason) const {
    return this->analysis().suitableForGpuRasterization(reason, 0);
}
#endif

bool SkPicture::hasText()             const { return this->analysis().fHasText; }
bool SkPicture::willPlayBackBitmaps() const { return this->analysis().fWillPlaybackBitmaps; }
int  SkPicture::approximateOpCount()  const { return fRecord->count(); }

SkPicture::SkPicture(const SkRect& cullRect,
                     SkRecord* record,
                     SnapshotArray* drawablePicts,
                     SkBBoxHierarchy* bbh,
                     AccelData* accelData,
                     size_t approxBytesUsedBySubPictures)
    : fUniqueID(0)
    , fCullRect(cullRect)
    , fRecord(record)               // Take ownership of caller's ref.
    , fDrawablePicts(drawablePicts) // Take ownership.
    , fBBH(bbh)                     // Take ownership of caller's ref.
    , fAccelData(accelData)         // Take ownership of caller's ref.
    , fApproxBytesUsedBySubPictures(approxBytesUsedBySubPictures)
{}


static uint32_t gNextID = 1;
uint32_t SkPicture::uniqueID() const {
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
