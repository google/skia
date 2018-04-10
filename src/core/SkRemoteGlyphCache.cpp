/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRemoteGlyphCache.h"

#include <iterator>
#include <memory>
#include <tuple>
#include <string>

#include "SkDevice.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkTypeface_remote.h"
#include "SkTraceEvent.h"
#include "base/logging.h"

SkScalerContextRec ExtractRec(const SkDescriptor& desc) {
  uint32_t size;
  auto recPtr = desc.findEntry(kRec_SkDescriptorTag, &size);

  SkScalerContextRec result;
  std::memcpy(&result, recPtr, size);
  return result;
}

template <typename T>
class ArraySlice final : public std::tuple<const T*, size_t> {
public:
    // Additional constructors as needed.
    ArraySlice(const T* data, size_t size) : fData{data}, fSize{size} { }
    ArraySlice() : ArraySlice<T>(nullptr, 0) { }

    const T* begin() {
        return this->data();
    }

    const T* end() {
        return &this->data()[this->size()];
    }

    const T* data() const {
        return fData;
    }

    size_t size() const {
        return fSize;
    }

private:
    const T* fData;
    size_t   fSize;
};

// -- Serializer ----------------------------------------------------------------------------------

size_t pad(size_t size, size_t alignment) {
    return (size + (alignment - 1)) & ~(alignment - 1);
}

class Serializer {
public:
    Serializer(std::vector<uint8_t>* buffer) : fBuffer{buffer} { }

    template <typename T, typename... Args>
    T* emplace(Args&&... args) {
        auto result = allocate(sizeof(T), alignof(T));
        return new (result) T{std::forward<Args>(args)...};
    }

    template <typename T>
    void write(const T& data) {
        T* result = (T*)allocate(sizeof(T), alignof(T));
        memcpy(result, &data, sizeof(T));
    }

    template <typename T>
    T* allocate() {
        T* result = (T*)allocate(sizeof(T), alignof(T));
        return result;
    }

    void writeDescriptor(const SkDescriptor& desc) {
        auto result = allocate(desc.getLength(), alignof(SkDescriptor));
        memcpy(result, &desc, desc.getLength());
    }

    template <typename T>
    T* allocateArray(int count) {
        auto result = allocate(sizeof(T) * count, alignof(T));
        return new (result) T[count];
    }

private:
    void* allocate(size_t size, size_t alignment) {
        size_t aligned = pad(fBuffer->size(), alignment);
        fBuffer->resize(aligned + size);
        return &(*fBuffer)[aligned];
    }

    std::vector<uint8_t>* fBuffer;
};

// -- Deserializer -------------------------------------------------------------------------------

class Deserializer {
public:
  Deserializer(sk_sp<SkData> data) : data_(std::move(data)) {}
  ~Deserializer() { CHECK_EQ(data_->size(), bytes_read_); }

    template <typename T>
    T* read() {
        T* result = (T*)this->ensureAtLeast(sizeof(T),
                                            alignof(T));
        bytes_read_ += sizeof(T);
        return result;
    }

    SkDescriptor* readDescriptor() {
        SkDescriptor* result = (SkDescriptor*)this->ensureAtLeast(sizeof(SkDescriptor),
                                                                  alignof(SkDescriptor));
        bytes_read_ += result->getLength();
        return result;
    }

    template <typename T>
    ArraySlice<T> readArray(int count) {
        size_t size = count * sizeof(T);
        const T* base = (const T*)this->ensureAtLeast(size,
                                                      alignof(T));
        ArraySlice<T> result = ArraySlice<T>{base, (uint32_t)count};
        bytes_read_ += size;
        return result;
    }

    bool empty() const { return bytes_read_ == data_->size(); }

private:
    const char* ensureAtLeast(size_t size, size_t alignment) {
      size_t padded = pad(bytes_read_, alignment);
      CHECK_LE(padded + size, data_->size()) << "Not enough data";
      bytes_read_ = padded;
      return static_cast<const char*>(data_->data()) + bytes_read_;
    }

    sk_sp<SkData> data_;
    size_t bytes_read_ = 0u;
};

// -- TrackLayerDevice -----------------------------------------------------------------------------
class TrackLayerDevice : public SkNoPixelsDevice {
public:
    TrackLayerDevice(const SkIRect& bounds, const SkSurfaceProps& props)
            : SkNoPixelsDevice(bounds, props) { }
    SkBaseDevice* onCreateDevice(const CreateInfo& cinfo, const SkPaint*) override {
        const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
        return new TrackLayerDevice(this->getGlobalBounds(), surfaceProps);
    }

    // Stolen from the SkBitmapDevice, but the SkGPUDevice is similar.
    bool onShouldDisableLCD(const SkPaint& paint) const override {
        if (paint.getPathEffect() ||
            paint.isFakeBoldText() ||
            paint.getStyle() != SkPaint::kFill_Style ||
            !paint.isSrcOver())
        {
            return true;
        }
        return false;
    }
};

// -- SkTextBlobCacheDiffCanvas -------------------------------------------------------------------
SkTextBlobCacheDiffCanvas::SkTextBlobCacheDiffCanvas(
        int width, int height,
        const SkMatrix& deviceMatrix,
        const SkSurfaceProps& props,
        SkScalerContextFlags flags,
        SkStrikeServer* strike_server)
        : SkNoDrawCanvas{new TrackLayerDevice{SkIRect::MakeWH(width, height), props}}
        , fDeviceMatrix{deviceMatrix}
        , fSurfaceProps{props}
        , fScalerContextFlags{flags}
        , fStrikeServer{strike_server} {
          CHECK(fStrikeServer);
        }

SkTextBlobCacheDiffCanvas::~SkTextBlobCacheDiffCanvas() = default;

SkCanvas::SaveLayerStrategy SkTextBlobCacheDiffCanvas::getSaveLayerStrategy(
    const SaveLayerRec&rec)
{
    return kFullLayer_SaveLayerStrategy;
}

void SkTextBlobCacheDiffCanvas::onDrawTextBlob(
        const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint) {
    SkPoint position{x, y};

    SkPaint runPaint{paint};
    SkTextBlobRunIterator it(blob);
        for (;!it.done(); it.next()) {
        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        it.applyFontToPaint(&runPaint);
        runPaint.setFlags(this->getTopDevice()->filterTextFlags(runPaint));
        if (auto looper = runPaint.getLooper()) {
            this->processLooper(position, it, runPaint, looper);
        } else {
            this->processGlyphRun(position, it, runPaint);
        }
    }
}

void SkTextBlobCacheDiffCanvas::processLooper(
        const SkPoint& position,
        const SkTextBlobRunIterator& it,
        const SkPaint& origPaint,
        SkDrawLooper* looper)
{
    SkSTArenaAlloc<48> alloc;
    auto context = looper->makeContext(this, &alloc);
    SkPaint runPaint = origPaint;
    while (context->next(this, &runPaint)) {
        this->save();
        this->processGlyphRun(position, it, runPaint);
        this->restore();
        runPaint = origPaint;
    }
}

void SkTextBlobCacheDiffCanvas::processGlyphRun(
        const SkPoint& position,
        const SkTextBlobRunIterator& it,
        const SkPaint& runPaint)
{

    if (runPaint.getTextEncoding() != SkPaint::TextEncoding::kGlyphID_TextEncoding) {
        TRACE_EVENT0("skia", "kGlyphID_TextEncoding");
        return;
    }

    // All other alignment modes need the glyph advances. Use the slow drawing mode.
    if (runPaint.getTextAlign() != SkPaint::kLeft_Align) {
        TRACE_EVENT0("skia", "kLeft_Align");
        return;
    }

    using PosFn = SkPoint(*)(int index, const SkScalar* pos);
    PosFn posFn;
    switch (it.positioning()) {
        case SkTextBlob::kDefault_Positioning: {
            // Default positioning needs advances. Can't do that.
            TRACE_EVENT0("skia", "kDefault_Positioning");
            return;
        }

        case SkTextBlob::kHorizontal_Positioning:
            posFn = [](int index, const SkScalar* pos) {
                return SkPoint{pos[index], 0};
            };

            break;

        case SkTextBlob::kFull_Positioning:
            posFn = [](int index, const SkScalar* pos) {
                return SkPoint{pos[2 * index], pos[2 * index + 1]};
            };
            break;

        default:
            posFn = nullptr;
            SK_ABORT("unhandled positioning mode");
    }

    SkMatrix blobMatrix{fDeviceMatrix};
    blobMatrix.preConcat(this->getTotalMatrix());
    if (blobMatrix.hasPerspective()) {
        TRACE_EVENT0("skia", "hasPerspective");
        return;
    }
    blobMatrix.preTranslate(position.x(), position.y());

    SkMatrix runMatrix{blobMatrix};
    runMatrix.preTranslate(it.offset().x(), it.offset().y());

    using MapFn = SkPoint(*)(const SkMatrix& m, SkPoint pt);
    MapFn mapFn;
    switch ((int)runMatrix.getType()) {
        case SkMatrix::kIdentity_Mask:
        case SkMatrix::kTranslate_Mask:
            mapFn = [](const SkMatrix& m, SkPoint pt) {
                pt.offset(m.getTranslateX(), m.getTranslateY());
                return pt;
            };
            break;
        case SkMatrix::kScale_Mask:
        case SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask:
            mapFn = [](const SkMatrix& m, SkPoint pt) {
                return SkPoint{pt.x() * m.getScaleX() + m.getTranslateX(),
                               pt.y() * m.getScaleY() + m.getTranslateY()};
            };
            break;
        case SkMatrix::kAffine_Mask | SkMatrix::kScale_Mask:
        case SkMatrix::kAffine_Mask | SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask:
            mapFn = [](const SkMatrix& m, SkPoint pt) {
                return SkPoint{
                        pt.x() * m.getScaleX() + pt.y() * m.getSkewX() + m.getTranslateX(),
                        pt.x() * m.getSkewY() + pt.y() * m.getScaleY() + m.getTranslateY()};
            };
            break;
        default:
            mapFn = nullptr;
            SK_ABORT("Bad matrix.");
    }

    SkScalerContextRec rec;
    SkScalerContextEffects effects;

    SkScalerContext::MakeRecAndEffects(runPaint, &fSurfaceProps, &runMatrix,
                                       fScalerContextFlags, &rec, &effects, true);

    TRACE_EVENT1("skia", "RecForDesc", "rec",
                 TRACE_STR_COPY(rec.dump().c_str()));
    //LOG(ERROR) << "Get Glyph state for strikes: " << rec.dump().c_str();
    auto desc = SkScalerContext::DescriptorGivenRecAndEffects(rec, effects);
    auto* glyphCacheState = fStrikeServer->GetOrCreateCache(std::move(desc));
    CHECK(glyphCacheState);

    bool isSubpixel = SkToBool(rec.fFlags & SkScalerContext::kSubpixelPositioning_Flag);
    SkAxisAlignment axisAlignment = SkAxisAlignment::kNone_SkAxisAlignment;
    if (it.positioning() == SkTextBlob::kHorizontal_Positioning) {
        axisAlignment = rec.computeAxisAlignmentForHText();
    }
    auto pos = it.pos();
    const uint16_t* glyphs = it.glyphs();
    for (uint32_t index = 0; index < it.glyphCount(); index++) {
        SkIPoint subPixelPos{0, 0};
        if (runPaint.isAntiAlias() && isSubpixel) {
            SkPoint glyphPos = mapFn(runMatrix, posFn(index, pos));
            subPixelPos = SkFindAndPlaceGlyph::SubpixelAlignment(axisAlignment, glyphPos);
        }
        glyphCacheState->AddGlyph(runPaint.getTypeface(),
                                  SkPackedGlyphID(glyphs[index],
                                                  subPixelPos.x(),
                                                  subPixelPos.y()));
    }
}

struct StrikeSpec {
    StrikeSpec(SkFontID typefaceID_, int glyphCount_,
               DiscardableHandleId discardableHandleId_)
            : typefaceID{typefaceID_}
            , glyphCount{glyphCount_}
            , discardableHandleId(discardableHandleId_) { }
    SkFontID typefaceID;
    int glyphCount;
    DiscardableHandleId discardableHandleId;
    /* desc */
    /* n X (glyphs ids) */
};

struct WireTypeface {
  WireTypeface(SkFontID typeface_id, int glyph_count, SkFontStyle style, bool is_fixed)
    : typefaceID(typeface_id), glyphCount(glyph_count), style(style), isFixed(is_fixed) {}

    // std::thread::id thread_id;  // TODO:need to figure a good solution
    SkFontID        typefaceID;
    int             glyphCount;
    SkFontStyle     style;
    bool            isFixed;
};

// SkStrikeServer -----------------------------------------

SkStrikeServer::SkStrikeServer(SkDiscardableHandleServer* discardable_handle_server)
  : discardable_handle_server_(discardable_handle_server) {
  CHECK(discardable_handle_server_);
}

SkStrikeServer::~SkStrikeServer() = default;

void SkStrikeServer::prepareSerialProcs(SkSerialProcs* procs) {
    auto encode = [](SkTypeface* tf, void* ctx) {
      return static_cast<SkStrikeServer*>(ctx)->encodeTypeface(tf);
    };
    procs->fTypefaceProc = encode;
    procs->fTypefaceCtx = this;
}

sk_sp<SkData> SkStrikeServer::encodeTypeface(SkTypeface* tf) {
 SkFontID typeface_id = SkTypeface::UniqueID(tf);
 auto data = SkData::MakeWithCopy(&typeface_id, sizeof(typeface_id));
 if (cached_typefaces_.contains(typeface_id))
   return data;

  typefaces_.emplace_back(typeface_id,
                          tf->countGlyphs(),
                          tf->fontStyle(),
                          tf->isFixedPitch());
  cached_typefaces_.add(typeface_id);
  return data;
}

void SkStrikeServer::writeStrikeData(std::vector<uint8_t>* memory) {
  if (locked_descriptors_.empty() && typefaces_.empty())
    return;

  Serializer serializer(memory);
  serializer.emplace<size_t>(typefaces_.size());
  for (const auto& tf : typefaces_)
    serializer.write<WireTypeface>(tf);
  typefaces_.clear();

  serializer.emplace<size_t>(locked_descriptors_.size());
  for (const auto* desc : locked_descriptors_) {
      auto it = remote_glyph_state_map_.find(desc);
      CHECK(it != remote_glyph_state_map_.end());

      // TODO: This is unnecessary, write only the descs which has any glyphs
      // to send. It was getting awkward to write the size after writing the
      // descs because the vector reallocs.
      serializer.emplace<bool>(it->second->has_pending_glyphs());
      if (!it->second->has_pending_glyphs())
        continue;

      it->second->writePendingGlyphs(&serializer);
  }
  locked_descriptors_.clear();
}

SkStrikeServer::SkGlyphCacheState* SkStrikeServer::GetOrCreateCache(
    std::unique_ptr<SkDescriptor> desc) {
  CHECK(desc);

  // Already locked.
  if (locked_descriptors_.find(desc.get()) != locked_descriptors_.end()) {
    auto it = remote_glyph_state_map_.find(desc.get());
    CHECK(it != remote_glyph_state_map_.end());
    return it->second.get();
  }

  // Try to lock.
  auto it = remote_glyph_state_map_.find(desc.get());
  if (it != remote_glyph_state_map_.end()) {
    bool locked = discardable_handle_server_->LockHandle(
        it->second->discardable_handle_id());
    if (locked) {
      locked_descriptors_.insert(it->first);
      return it->second.get();
    }

    // If the lock failed, the entry was deleted on the client. Remove our
    // tracking.
    remote_glyph_state_map_.erase(it);
  }

  auto* desc_ptr = desc.get();
  auto new_handle = discardable_handle_server_->CreateHandle();
  auto cache_state = std::make_unique<SkGlyphCacheState>(std::move(desc),
                                                         new_handle);
  auto* cache_state_ptr = cache_state.get();

  locked_descriptors_.insert(desc_ptr);
  remote_glyph_state_map_[desc_ptr] = std::move(cache_state);

  it = remote_glyph_state_map_.find(desc_ptr);
  CHECK(it != remote_glyph_state_map_.end());
  return cache_state_ptr;
}

SkStrikeServer::SkGlyphCacheState::SkGlyphCacheState(
    std::unique_ptr<SkDescriptor> desc,
    uint32_t discardable_handle_id)
  : desc_(std::move(desc)), discardable_handle_id_(discardable_handle_id) {}

SkStrikeServer::SkGlyphCacheState::~SkGlyphCacheState() = default;

void SkStrikeServer::SkGlyphCacheState::AddGlyph(
    SkTypeface* typeface,
    SkPackedGlyphID glyph) {
  // Already cached.
  if (cached_glyphs_.contains(glyph))
    return;

  // Serialize and cache. Also create the scalar context to use when serializing
  // this glyph.
  cached_glyphs_.add(glyph);
  pending_glyphs_.push_back(glyph);
  // TODO: make effects really work.
  SkScalerContextEffects effects;
  if (!context_)
    context_ = typeface->createScalerContext(effects, desc_.get(), false);
}

void SkStrikeServer::SkGlyphCacheState::writePendingGlyphs(
    Serializer* serializer) {
  // Write the desc.
  serializer->emplace<StrikeSpec>(context_->getTypeface()->uniqueID(),
                                  pending_glyphs_.size(),
                                  discardable_handle_id_);
  serializer->writeDescriptor(*desc_.get());

  // Write FontMetrics.
  SkPaint::FontMetrics fontMetrics;
  context_->getFontMetrics(&fontMetrics);
  serializer->write<SkPaint::FontMetrics>(fontMetrics);

  // Write Glyphs.
  for (const auto& glyphID : pending_glyphs_) {
    auto glyph = serializer->emplace<SkGlyph>();
    glyph->initWithGlyphID(glyphID);
    context_->getMetrics(glyph);
    auto imageSize = glyph->computeImageSize();
    glyph->fPathData = nullptr;
    glyph->fImage = nullptr;

    if (imageSize > 0) {
      // Since the allocateArray can move glyph, make one that stays in one place.
      SkGlyph stationaryGlyph = *glyph;
      stationaryGlyph.fImage = serializer->allocateArray<uint8_t>(imageSize);
      context_->getImage(stationaryGlyph);
    }
  }

  pending_glyphs_.clear();
  context_.reset();
}

SkStrikeClient::SkStrikeClient(SkDiscardableHandleClient* client)
  : client_(client) {}

SkStrikeClient::~SkStrikeClient() = default;

void SkStrikeClient::prepareDeserialProcs(SkDeserialProcs* procs) {
  auto decode = [](const void* buf, size_t len, void* ctx) {
          return reinterpret_cast<SkStrikeClient*>(ctx)->decodeTypeface(buf, len);
      };
      procs->fTypefaceProc = decode;
      procs->fTypefaceCtx = this;
}

void SkStrikeClient::readStrikeData(sk_sp<SkData> data) {
  Deserializer deserializer(std::move(data));

  auto* typeface_size = deserializer.read<size_t>();
  for (size_t i = 0; i < *typeface_size; ++i) {
    auto* wire = deserializer.read<WireTypeface>();
    CHECK_EQ(remote_font_id_to_typeface_.find(wire->typefaceID), nullptr);
      auto newTypeface = sk_make_sp<SkTypefaceProxy>(
                    wire->typefaceID,
                    wire->glyphCount,
                    wire->style,
                    wire->isFixed,
                    this);
      remote_font_id_to_typeface_.set(wire->typefaceID, std::move(newTypeface));
  }

  auto* strike_count = deserializer.read<size_t>();
  for (int i = 0; i < *strike_count; ++i) {
    auto* has_glyphs = deserializer.read<bool>();
    if (!*has_glyphs)
      continue;

    auto* spec = deserializer.read<StrikeSpec>();
    auto* desc = deserializer.readDescriptor();
    auto* fontMetrics = deserializer.read<SkPaint::FontMetrics>();

    // Get the local typeface from remote fontID.
    auto* tf = remote_font_id_to_typeface_.find(spec->typefaceID)->get();
    CHECK(tf);

    // Replace the ContextRec in the desc from the server to create the client
    // side descriptor.
    SkAutoDescriptor ad;
    SkScalerContextRec rec = ExtractRec(*desc);
    rec.fFontID = tf->uniqueID();
    SkScalerContextEffects effects;
    auto* client_desc =
        SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

    auto strike = SkGlyphCache::FindStrikeExclusive(*client_desc);
    if (strike == nullptr) {
      //LOG(ERROR) << "Strikes added: " << rec.dump().c_str();
      auto scaler = SkGlyphCache::CreateScalerContext(*client_desc, effects, *tf);
      scaler_to_handle_id_map_.set(scaler.get(), spec->discardableHandleId);
      strike = SkGlyphCache::CreateStrikeExclusive(*client_desc, std::move(scaler), fontMetrics);
    }

    // Make sure we have the same discardable handle, if we found a cached
    // strike.
    DCHECK_EQ(*scaler_to_handle_id_map_.find(strike->getScalerContext()),
              spec->discardableHandleId);

    for (int j = 0; j < spec->glyphCount; j++) {
      auto glyph = deserializer.read<SkGlyph>();
      ArraySlice<uint8_t> image;
      auto imageSize = glyph->computeImageSize();
      if (imageSize != 0) {
        image = deserializer.readArray<uint8_t>(imageSize);
      }
      SkGlyph* allocatedGlyph = strike->getRawGlyphByID(glyph->getPackedID());
      *allocatedGlyph = *glyph;
      allocatedGlyph->allocImage(strike->getAlloc());
      memcpy(allocatedGlyph->fImage, image.data(), image.size());
    }
  }
}

bool SkStrikeClient::canPurgeGlyphCache(SkGlyphCache* cache) {
  auto* handle = scaler_to_handle_id_map_.find(cache->getScalerContext());
  DCHECK(handle);

  bool deleted = client_->DeleteHandle(*handle);
  if (!deleted)
    return false;

  scaler_to_handle_id_map_.remove(cache->getScalerContext());
  return true;
}

sk_sp<SkTypeface> SkStrikeClient::decodeTypeface(const void* buf, size_t len) {
  SkFontID font_id;
  if (len != sizeof(SkFontID)) {
    SK_ABORT("Incomplete transfer");
    return nullptr;
  }
  memcpy(&font_id, buf, sizeof(font_id));

  auto typeFace = remote_font_id_to_typeface_.find(font_id);
  CHECK(typeFace);
  return *typeFace;
}

void SkStrikeClient::generateFontMetrics(
        const SkTypefaceProxy& typefaceProxy,
        const SkScalerContextRec& rec,
        SkPaint::FontMetrics* metrics) {
    TRACE_EVENT1("skia", "generateFontMetrics", "rec",
                 TRACE_STR_COPY(rec.dump().c_str()));
    LOG(ERROR) << "Font requested: " << rec.dump().c_str();
    CHECK(false);
    //SK_ABORT("generateFontMetrics");
}

void SkStrikeClient::generateMetricsAndImage(
        const SkTypefaceProxy& typefaceProxy,
        const SkScalerContextRec& rec,
        SkArenaAlloc* alloc,
        SkGlyph* glyph) {
    TRACE_EVENT1("skia", "generateMetricsAndImage", "rec",
                 TRACE_STR_COPY(rec.dump().c_str()));
    LOG(ERROR) << "Metrics and image requested: " << rec.dump().c_str();
    CHECK(false);
    //SK_ABORT("generateMetricsAndImage");
}

void SkStrikeClient::generatePath(
        const SkTypefaceProxy& typefaceProxy,
        const SkScalerContextRec& rec,
        SkGlyphID glyphID,
        SkPath* path) {
      TRACE_EVENT1("skia", "generateMetricsAndImage", "rec",
               TRACE_STR_COPY(rec.dump().c_str()));
      LOG(ERROR) << "Path requested: " << rec.dump().c_str();
      CHECK(false);
      //SK_ABORT("generatePath");
}
