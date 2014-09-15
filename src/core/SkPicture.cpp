
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
#include "SkPictureStateTree.h"

#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkChunkAlloc.h"
#include "SkDrawPictureCallback.h"
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
#include "SkBBoxHierarchyRecord.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"

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

    PathCounter()
        : numPaintWithPathEffectUses (0)
        , numFastPathDashEffects (0)
        , numAAConcavePaths (0)
        , numAAHairlineConcavePaths (0) {
    }

    // Recurse into nested pictures.
    void operator()(const SkRecords::DrawPicture& op) {
        const SkPicture::Analysis& analysis = op.picture->fAnalysis;
        numPaintWithPathEffectUses += analysis.fNumPaintWithPathEffectUses;
        numFastPathDashEffects     += analysis.fNumFastPathDashEffects;
        numAAConcavePaths          += analysis.fNumAAConcavePaths;
        numAAHairlineConcavePaths  += analysis.fNumAAHairlineConcavePaths;
    }

    void checkPaint(const SkPaint* paint) {
        if (paint && paint->getPathEffect()) {
            numPaintWithPathEffectUses++;
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
                numFastPathDashEffects++;
            }
        }
    }

    void operator()(const SkRecords::DrawPath& op) {
        this->checkPaint(&op.paint);
        if (op.paint.isAntiAlias() && !op.path.isConvex()) {
            numAAConcavePaths++;

            if (SkPaint::kStroke_Style == op.paint.getStyle() &&
                0 == op.paint.getStrokeWidth()) {
                numAAHairlineConcavePaths++;
            }
        }
    }

    template <typename T>
    SK_WHEN(HasMember_paint<T>, void) operator()(const T& op) {
        this->checkPaint(AsPtr(op.paint));
    }

    template <typename T>
    SK_WHEN(!HasMember_paint<T>, void) operator()(const T& op) { /* do nothing */ }

    int numPaintWithPathEffectUses;
    int numFastPathDashEffects;
    int numAAConcavePaths;
    int numAAHairlineConcavePaths;
};

SkPicture::Analysis::Analysis(const SkRecord& record) {
    fWillPlaybackBitmaps = WillPlaybackBitmaps(record);

    PathCounter counter;
    for (unsigned i = 0; i < record.count(); i++) {
        record.visit<void>(i, counter);
    }
    fNumPaintWithPathEffectUses = counter.numPaintWithPathEffectUses;
    fNumFastPathDashEffects     = counter.numFastPathDashEffects;
    fNumAAConcavePaths          = counter.numAAConcavePaths;
    fNumAAHairlineConcavePaths  = counter.numAAHairlineConcavePaths;

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
    static const int kNumPaintWithPathEffectsUsesTol = 1;
    static const int kNumAAConcavePathsTol = 5;

    int numNonDashedPathEffects = fNumPaintWithPathEffectUses -
                                  fNumFastPathDashEffects;
    bool suitableForDash = (0 == fNumPaintWithPathEffectUses) ||
                           (numNonDashedPathEffects < kNumPaintWithPathEffectsUsesTol
                               && 0 == sampleCount);

    bool ret = suitableForDash &&
               (fNumAAConcavePaths - fNumAAHairlineConcavePaths)
                   < kNumAAConcavePathsTol;

    if (!ret && reason) {
        if (!suitableForDash) {
            if (0 != sampleCount) {
                *reason = "Can't use multisample on dash effect.";
            } else {
                *reason = "Too many non dashed path effects.";
            }
        } else if ((fNumAAConcavePaths - fNumAAHairlineConcavePaths)
                    >= kNumAAConcavePathsTol)
            *reason = "Too many anti-aliased concave paths.";
        else
            *reason = "Unknown reason for GPU unsuitability.";
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

// fRecord OK
SkPicture::SkPicture(SkScalar width, SkScalar height,
                     const SkPictureRecord& record,
                     bool deepCopyOps)
    : fCullWidth(width)
    , fCullHeight(height)
    , fAnalysis() {
    this->needsNewGenID();

    SkPictInfo info;
    this->createHeader(&info);
    fData.reset(SkNEW_ARGS(SkPictureData, (record, info, deepCopyOps)));
}

// Create an SkPictureData-backed SkPicture from an SkRecord.
// This for compatibility with serialization code only.  This is not cheap.
static SkPicture* backport(const SkRecord& src, const SkRect& cullRect) {
    SkPictureRecorder recorder;
    SkRecordDraw(src,
                 recorder.DEPRECATED_beginRecording(cullRect.width(), cullRect.height()),
                 NULL/*bbh*/, NULL/*callback*/);
    return recorder.endRecording();
}

// fRecord OK
SkPicture::~SkPicture() {
    this->callDeletionListeners();
}

// fRecord OK
#ifdef SK_SUPPORT_LEGACY_PICTURE_CLONE
SkPicture* SkPicture::clone() const {
    return SkRef(const_cast<SkPicture*>(this));
}
#endif//SK_SUPPORT_LEGACY_PICTURE_CLONE

// fRecord OK
void SkPicture::EXPERIMENTAL_addAccelData(const SkPicture::AccelData* data) const {
    fAccelData.reset(SkRef(data));
}

// fRecord OK
const SkPicture::AccelData* SkPicture::EXPERIMENTAL_getAccelData(
        SkPicture::AccelData::Key key) const {
    if (fAccelData.get() && fAccelData->getKey() == key) {
        return fAccelData.get();
    }
    return NULL;
}

// fRecord OK
SkPicture::AccelData::Domain SkPicture::AccelData::GenerateDomain() {
    static int32_t gNextID = 0;

    int32_t id = sk_atomic_inc(&gNextID);
    if (id >= 1 << (8 * sizeof(Domain))) {
        SK_CRASH();
    }

    return static_cast<Domain>(id);
}

///////////////////////////////////////////////////////////////////////////////

uint32_t SkPicture::OperationList::offset(int index) const {
    SkASSERT(index < fOps.count());
    return ((SkPictureStateTree::Draw*)fOps[index])->fOffset;
}

const SkMatrix& SkPicture::OperationList::matrix(int index) const {
    SkASSERT(index < fOps.count());
    return *((SkPictureStateTree::Draw*)fOps[index])->fMatrix;
}

// fRecord OK
void SkPicture::playback(SkCanvas* canvas, SkDrawPictureCallback* callback) const {
    SkASSERT(canvas);
    SkASSERT(fData.get() || fRecord.get());

    // If the query contains the whole picture, don't bother with the BBH.
    SkRect clipBounds = { 0, 0, 0, 0 };
    (void)canvas->getClipBounds(&clipBounds);
    const bool useBBH = !clipBounds.contains(this->cullRect());

    if (fData.get()) {
        SkPicturePlayback playback(this);
        playback.setUseBBH(useBBH);
        playback.draw(canvas, callback);
    }
    if (fRecord.get()) {
        SkRecordDraw(*fRecord, canvas, useBBH ? fBBH.get() : NULL, callback);
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

static const char kMagic[] = { 's', 'k', 'i', 'a', 'p', 'i', 'c', 't' };

// fRecord OK
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

// fRecord OK
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

#ifndef V35_COMPATIBILITY_CODE
    if (info.fVersion < 35) {
        info.fCullRect.fLeft = 0;
        info.fCullRect.fTop = 0;
        info.fCullRect.fRight = SkIntToScalar(stream->readU32());
        info.fCullRect.fBottom = SkIntToScalar(stream->readU32());
    } else {
#endif
        info.fCullRect.fLeft = stream->readScalar();
        info.fCullRect.fTop = stream->readScalar();
        info.fCullRect.fRight = stream->readScalar();
        info.fCullRect.fBottom = stream->readScalar();
#ifndef V35_COMPATIBILITY_CODE
    }
#endif

    info.fFlags = stream->readU32();

    if (!IsValidPictInfo(info)) {
        return false;
    }

    if (pInfo != NULL) {
        *pInfo = info;
    }
    return true;
}

// fRecord OK
bool SkPicture::InternalOnly_BufferIsSKP(SkReadBuffer* buffer, SkPictInfo* pInfo) {
    // Check magic bytes.
    SkPictInfo info;
    SkASSERT(sizeof(kMagic) == sizeof(info.fMagic));

    if (!buffer->readByteArray(&info.fMagic, sizeof(kMagic))) {
        return false;
    }

    info.fVersion = buffer->readUInt();

#ifndef V35_COMPATIBILITY_CODE
    if (info.fVersion < 35) {
        info.fCullRect.fLeft = 0;
        info.fCullRect.fTop = 0;
        info.fCullRect.fRight = SkIntToScalar(buffer->readUInt());
        info.fCullRect.fBottom = SkIntToScalar(buffer->readUInt());
    } else {
#endif
        buffer->readRect(&info.fCullRect);
#ifndef V35_COMPATIBILITY_CODE
    }
#endif

    info.fFlags = buffer->readUInt();

    if (!IsValidPictInfo(info)) {
        return false;
    }

    if (pInfo != NULL) {
        *pInfo = info;
    }
    return true;
}

// fRecord OK
SkPicture::SkPicture(SkPictureData* data, SkScalar width, SkScalar height)
    : fData(data)
    , fCullWidth(width)
    , fCullHeight(height)
    , fAnalysis() {
    this->needsNewGenID();
}

SkPicture* SkPicture::Forwardport(const SkPicture& src) {
    SkAutoTDelete<SkRecord> record(SkNEW(SkRecord));
    SkRecorder canvas(record.get(), src.cullRect().width(), src.cullRect().height());
    src.playback(&canvas);
    return SkNEW_ARGS(SkPicture, (src.cullRect().width(), src.cullRect().height(),
                                  record.detach(), NULL/*bbh*/));
}

// fRecord OK
SkPicture* SkPicture::CreateFromStream(SkStream* stream, InstallPixelRefProc proc) {
    SkPictInfo info;

    if (!InternalOnly_StreamIsSKP(stream, &info)) {
        return NULL;
    }

    // Check to see if there is a playback to recreate.
    if (stream->readBool()) {
        SkPictureData* data = SkPictureData::CreateFromStream(stream, info, proc);
        if (NULL == data) {
            return NULL;
        }
        const SkPicture src(data, info.fCullRect.width(), info.fCullRect.height());
        return Forwardport(src);
    }

    return NULL;
}

// fRecord OK
SkPicture* SkPicture::CreateFromBuffer(SkReadBuffer& buffer) {
    SkPictInfo info;

    if (!InternalOnly_BufferIsSKP(&buffer, &info)) {
        return NULL;
    }

    // Check to see if there is a playback to recreate.
    if (buffer.readBool()) {
        SkPictureData* data = SkPictureData::CreateFromBuffer(buffer, info);
        if (NULL == data) {
            return NULL;
        }
        const SkPicture src(data, info.fCullRect.width(), info.fCullRect.height());
        return Forwardport(src);
    }

    return NULL;
}

// fRecord OK
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

// fRecord OK
void SkPicture::serialize(SkWStream* stream, EncodeBitmap encoder) const {
    const SkPictureData* data = fData.get();

    // If we're a new-format picture, backport to old format for serialization.
    SkAutoTDelete<SkPicture> oldFormat;
    if (NULL == data && fRecord.get()) {
        oldFormat.reset(backport(*fRecord, this->cullRect()));
        data = oldFormat->fData.get();
        SkASSERT(data);
    }

    SkPictInfo info;
    this->createHeader(&info);
    SkASSERT(sizeof(SkPictInfo) == 32);
    stream->write(&info, sizeof(info));

    if (data) {
        stream->writeBool(true);
        data->serialize(stream, encoder);
    } else {
        stream->writeBool(false);
    }
}

// fRecord OK
void SkPicture::flatten(SkWriteBuffer& buffer) const {
    const SkPictureData* data = fData.get();

    // If we're a new-format picture, backport to old format for serialization.
    SkAutoTDelete<SkPicture> oldFormat;
    if (NULL == data && fRecord.get()) {
        oldFormat.reset(backport(*fRecord, this->cullRect()));
        data = oldFormat->fData.get();
        SkASSERT(data);
    }

    SkPictInfo info;
    this->createHeader(&info);
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

#if SK_SUPPORT_GPU
// fRecord OK
bool SkPicture::suitableForGpuRasterization(GrContext* context, const char **reason) const {
    if (fRecord.get()) {
        return fAnalysis.suitableForGpuRasterization(reason, 0);
    }
    if (NULL == fData.get()) {
        if (reason) {
            *reason = "Missing internal data.";
        }
        return false;
    }

    return fData->suitableForGpuRasterization(context, reason);
}
#endif

// fRecord OK
bool SkPicture::hasText() const {
    if (fRecord.get()) {
        return fAnalysis.fHasText;
    }
    if (fData.get()) {
        return fData->hasText();
    }
    SkFAIL("Unreachable");
    return false;
}

// fRecord OK
bool SkPicture::willPlayBackBitmaps() const {
    if (fRecord.get()) {
        return fAnalysis.fWillPlaybackBitmaps;
    }
    if (fData.get()) {
        return fData->containsBitmaps();
    }
    SkFAIL("Unreachable");
    return false;
}

// fRecord OK
static int32_t next_picture_generation_id() {
    static int32_t  gPictureGenerationID = 0;
    // do a loop in case our global wraps around, as we never want to
    // return a 0
    int32_t genID;
    do {
        genID = sk_atomic_inc(&gPictureGenerationID) + 1;
    } while (SK_InvalidGenID == genID);
    return genID;
}

// fRecord OK
uint32_t SkPicture::uniqueID() const {
    if (SK_InvalidGenID == fUniqueID) {
        fUniqueID = next_picture_generation_id();
    }
    return fUniqueID;
}


static SkRecord* optimized(SkRecord* r) {
#ifdef SK_PICTURE_OPTIMIZE_SK_RECORD
    SkRecordOptimize(r);
#endif
    return r;
}

// fRecord OK
SkPicture::SkPicture(SkScalar width, SkScalar height, SkRecord* record, SkBBoxHierarchy* bbh)
    : fCullWidth(width)
    , fCullHeight(height)
    , fRecord(optimized(record))
    , fBBH(SkSafeRef(bbh))
    , fAnalysis(*fRecord) {
    // TODO: delay as much of this work until just before first playback?
    if (fBBH.get()) {
        SkRecordFillBounds(*fRecord, fBBH.get());
    }
    this->needsNewGenID();
}

// Note that we are assuming that this entry point will only be called from
// one thread. Currently the only client of this method is
// SkGpuDevice::EXPERIMENTAL_optimize which should be only called from a single
// thread.
void SkPicture::addDeletionListener(DeletionListener* listener) const {
    SkASSERT(listener);

    *fDeletionListeners.append() = SkRef(listener);
}

void SkPicture::callDeletionListeners() {
    for (int i = 0; i < fDeletionListeners.count(); ++i) {
        fDeletionListeners[i]->onDeletion(this->uniqueID());
    }

    fDeletionListeners.unrefAll();
}

// fRecord OK
int SkPicture::approximateOpCount() const {
    SkASSERT(fRecord.get() || fData.get());
    if (fRecord.get()) {
        return fRecord->count();
    }
    if (fData.get()) {
        return fData->opCount();
    }
    return 0;
}
