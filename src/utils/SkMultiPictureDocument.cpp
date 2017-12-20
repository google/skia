/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <map>
#include "SkMultiPictureDocument.h"
#include "SkMultiPictureDocumentPriv.h"
#include "SkNWayCanvas.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkSerialProcs.h"
#include "SkStream.h"
#include "SkTArray.h"

#include <limits.h>

/*
  File format:
      BEGINNING_OF_FILE:
        kMagic
        uint32_t version_number (==2)
        uint32_t page_count
        {
          float sizeX
          float sizeY
        } * page_count
        skp file
*/

namespace {
// The unique file signature for this file type.
static constexpr char kMagic[] = "Skia Multi-Picture Doc\n\n";

static constexpr char kEndPage[] = "SkMultiPictureEndPage";

const uint32_t kVersion = 2;

struct SerializeContext {
  uint32_t process_id;
  const std::vector<PicIdType>& pic_ids;
};

static SkSize join(const SkTArray<SkSize>& sizes) {
    SkSize joined = {0, 0};
    for (SkSize s : sizes) {
        joined = SkSize{SkTMax(joined.width(), s.width()), SkTMax(joined.height(), s.height())};
    }
    return joined;
}

sk_sp<SkData> serializeOopPicture(SkPicture* pic, void* ctx) {
  SerializeContext* context = reinterpret_cast<SerializeContext*>(ctx);
  PicIdType pic_id = pic->uniqueID();
  for (PicIdType id : context->pic_ids) {
    if (id == pic_id) {
      OOPPicIdType unique_id = context->process_id;
      unique_id = (unique_id << 32) | id;
      return SkData::MakeWithCopy(&unique_id, sizeof(unique_id));
    }
  }
  return nullptr;
}

sk_sp<SkPicture> GetEmptyPicture() {
  SkPictureRecorder rec;
  SkCanvas* c = rec.beginRecording(100, 100);
  c->save();
  c->restore();
  return rec.finishRecordingAsPicture();
}

sk_sp<SkPicture> deserializeOopPicture(const void* data,
                                       size_t length, void* ctx) {
  SkASSERT(ctx);
  auto pic_map =
      reinterpret_cast<std::map<OOPPicIdType, sk_sp<SkPicture>>*>(ctx);
  OOPPicIdType id;
  sk_sp<SkPicture> empty_pic = GetEmptyPicture();
  if (length < sizeof(id)) {
    SkASSERT(false);
    return empty_pic;
  }
  memcpy(&id, data, sizeof(id));
  auto iter = pic_map->find(id);
  if (iter == pic_map->end()) {
    // When we don't have the out-of-process picture available, we return
    // an empty picture. Returning a nullptr will cause the deserialization
    // crash.
    return empty_pic;
  }
  return iter->second;
}

SkSerialProcs serialProcs(void* ctx) {
  SkSerialProcs procs;
  procs.fPictureProc = serializeOopPicture;
  procs.fPictureCtx = ctx;
  return procs;
}

SkDeserialProcs deserialProcs(void* ctx) {
  SkDeserialProcs procs;
  procs.fPictureProc = deserializeOopPicture;
  procs.fPictureCtx = ctx;
  return procs;
}

struct MultiPictureDocument final : public SkDocument {
    SkPictureRecorder fPictureRecorder;
    SkSize fCurrentPageSize;
    SkTArray<sk_sp<SkPicture>> fPages;
    SkTArray<SkSize> fSizes;
    uint32_t proc_id;
    std::vector<PicIdType> subframe_content_ids;
    MultiPictureDocument(SkWStream* s, void (*d)(SkWStream*, bool),
                         uint32_t process_id,
                         const std::vector<PicIdType>& ids)
        : SkDocument(s, d),
          proc_id(process_id),
          subframe_content_ids(ids) {}
    ~MultiPictureDocument() override { this->close(); }

    SkCanvas* onBeginPage(SkScalar w, SkScalar h) override {
        fCurrentPageSize.set(w, h);
        return fPictureRecorder.beginRecording(w, h);
    }
    void onEndPage() override {
        fSizes.push_back(fCurrentPageSize);
        fPages.push_back(fPictureRecorder.finishRecordingAsPicture());
    }
    void onClose(SkWStream* wStream) override {
        SkASSERT(wStream);
        SkASSERT(wStream->bytesWritten() == 0);
        wStream->writeText(kMagic);
        wStream->write32(kVersion);
        wStream->write32(SkToU32(fPages.count()));
        for (SkSize s : fSizes) {
            wStream->write(&s, sizeof(s));
        }
        SkSize bigsize = join(fSizes);
        SkCanvas* c = fPictureRecorder.beginRecording(SkRect::MakeSize(bigsize));
        for (const sk_sp<SkPicture>& page : fPages) {
            c->drawPicture(page);
            c->drawAnnotation(SkRect::MakeEmpty(), kEndPage, nullptr);
        }
        sk_sp<SkPicture> p = fPictureRecorder.finishRecordingAsPicture();
        SerializeContext ctx = {proc_id, subframe_content_ids};
        p->serialize(wStream, serialProcs(&ctx));
        fPages.reset();
        fSizes.reset();
        return;
    }
    void onAbort() override {
        fPages.reset();
        fSizes.reset();
    }
};

}

void SkSerializePictureWithOopContent(sk_sp<SkPicture> pic,
                                      SkWStream* wStream,
                                      uint32_t process_id,
                                      const std::vector<PicIdType>& pic_ids) {
  SerializeContext ctx = {process_id, pic_ids};
  pic->serialize(wStream, serialProcs(&ctx));
}

sk_sp<SkPicture> SkDeserializePictureWithOopContent(SkStream* stream,
                                                    void* deserialize_context) {
  return SkPicture::MakeFromStream(stream, deserialProcs(deserialize_context));
}

sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* wStream) {
  return sk_make_sp<MultiPictureDocument>(wStream, nullptr,
                                          0, std::vector<PicIdType>());
}

sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* wStream,
    uint32_t process_id, const std::vector<PicIdType>& pic_ids) {
  return sk_make_sp<MultiPictureDocument>(wStream, nullptr,
                                          process_id, pic_ids);
}

////////////////////////////////////////////////////////////////////////////////

int SkMultiPictureDocumentReadPageCount(SkStreamSeekable* stream) {
    if (!stream) {
        return 0;
    }
    stream->seek(0);
    const size_t size = sizeof(kMagic) - 1;
    char buffer[size];
    if (size != stream->read(buffer, size) || 0 != memcmp(kMagic, buffer, size)) {
        stream = nullptr;
        return 0;
    }
    uint32_t versionNumber = stream->readU32();
    if (versionNumber != kVersion) {
        return 0;
    }
    uint32_t pageCount = stream->readU32();
    if (pageCount > INT_MAX) {
        return 0;
    }
    // leave stream position right here.
    return (int)pageCount;
}

bool SkMultiPictureDocumentReadPageSizes(SkStreamSeekable* stream,
                                         SkDocumentPage* dstArray,
                                         int dstArrayCount) {
    if (!dstArray || dstArrayCount < 1) {
        return false;
    }
    int pageCount = SkMultiPictureDocumentReadPageCount(stream);
    if (pageCount < 1 || pageCount != dstArrayCount) {
        return false;
    }
    for (int i = 0; i < pageCount; ++i) {
        SkSize& s = dstArray[i].fSize;
        if (sizeof(s) != stream->read(&s, sizeof(s))) {
            return false;
        }
    }
    // leave stream position right here.
    return true;
}

namespace {
struct PagerCanvas : public SkNWayCanvas {
    SkPictureRecorder fRecorder;
    SkDocumentPage* fDst;
    int fCount;
    int fIndex = 0;
    PagerCanvas(SkISize wh, SkDocumentPage* dst, int count)
            : SkNWayCanvas(wh.width(), wh.height()), fDst(dst), fCount(count) {
        this->nextCanvas();
    }
    void nextCanvas() {
        if (fIndex < fCount) {
            SkRect bounds = SkRect::MakeSize(fDst[fIndex].fSize);
            this->addCanvas(fRecorder.beginRecording(bounds));
        }
    }
    void onDrawAnnotation(const SkRect& r, const char* key, SkData* d) override {
        if (0 == strcmp(key, kEndPage)) {
            this->removeAll();
            if (fIndex < fCount) {
                fDst[fIndex].fPicture = fRecorder.finishRecordingAsPicture();
                ++fIndex;
            }
            this->nextCanvas();
        } else {
            this->SkNWayCanvas::onDrawAnnotation(r, key, d);
        }
    }
};
}  // namespace

bool SkMultiPictureDocumentRead(SkStreamSeekable* stream,
                                SkDocumentPage* dstArray,
                                int dstArrayCount,
                                void* deserialize_context) {
    if (!SkMultiPictureDocumentReadPageSizes(stream, dstArray, dstArrayCount)) {
        return false;
    }
    SkSize joined = {0.0f, 0.0f};
    for (int i = 0; i < dstArrayCount; ++i) {
        joined = SkSize{SkTMax(joined.width(), dstArray[i].fSize.width()),
                        SkTMax(joined.height(), dstArray[i].fSize.height())};
    }

    auto picture = SkPicture::MakeFromStream(
        stream, deserialProcs(deserialize_context));

    PagerCanvas canvas(joined.toCeil(), dstArray, dstArrayCount);
    // Must call playback(), not drawPicture() to reach
    // PagerCanvas::onDrawAnnotation().
    picture->playback(&canvas);
    if (canvas.fIndex != dstArrayCount) {
        SkDEBUGF(("Malformed SkMultiPictureDocument\n"));
    }
    return true;
}
