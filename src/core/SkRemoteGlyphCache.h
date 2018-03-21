/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteGlyphCache_DEFINED
#define SkRemoteGlyphCache_DEFINED

#include <err.h>
#include <memory>
#include <sys/uio.h>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "SkData.h"
#include "SkDescriptor.h"
#include "SkDevice.h"
#include "SkDrawLooper.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkGlyphCache.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkSerialProcs.h"
#include "SkTextBlobRunIterator.h"
#include "SkTHash.h"
#include "SkTypeface.h"
#include "SkTypeface_remote.h"

static const size_t kPageSize = 4096;

template <typename T>
class SkArraySlice : public std::tuple<const T*, size_t> {
public:
    // Additional constructors as needed.
    SkArraySlice(const T* data, size_t size) : std::tuple<const T*, size_t>{data, size} { }
    SkArraySlice() : SkArraySlice<T>(nullptr, 0) { }
    friend const T* begin(const SkArraySlice<T>& slice) {
        return slice.data();
    }

    friend const T* end(const SkArraySlice<T>& slice) {
        return &slice.data()[slice.size()];
    }

    const T* data() const {
        return std::get<0>(*this);
    }

    size_t size() const {
        return std::get<1>(*this);
    }
};

class SkTransport {
public:
    enum IOResult : bool {kFail = false, kSuccess = true};

    virtual ~SkTransport() {}
    virtual void writeStart() = 0;
    virtual void write(const SkArraySlice<uint8_t>&) = 0;
    virtual IOResult writeEnd() = 0;
    virtual void readStart() = 0;
    virtual size_t read(uint8_t*, size_t) = 0;
    virtual IOResult readEnd() = 0;
    IOResult writeSkData(const SkData&);
    IOResult
};

// TODO: handle alignment
// TODO: handle overflow
class AllInOneTransport {
public:
    enum IOResult : bool {kFail = false, kSuccess = true};

    AllInOneTransport(AllInOneTransport&& t)
            : fReadFd{t.fReadFd}
            , fWriteFd{t.fWriteFd}
            , fBuffer{std::move(t.fBuffer)}
            , fCloser{t.fCloser} { }

    AllInOneTransport(const AllInOneTransport& t)
            : fReadFd{t.fReadFd}
            , fWriteFd{t.fWriteFd}
            , fBuffer{new uint8_t[kBufferSize]}
            , fCloser{t.fCloser} { }

    AllInOneTransport(int readFd, int writeFd)
            : fReadFd{readFd}
            , fWriteFd{writeFd}
            , fCloser{std::make_shared<Closer>(readFd, writeFd)} { }

    static AllInOneTransport DoubleBuffer(const AllInOneTransport& transport) {
        return AllInOneTransport{transport};
    }

    struct Closer {
        Closer(int readFd, int writeFd) : fReadFd{readFd}, fWriteFd{writeFd} { }
        ~Closer() {
            close(fWriteFd);
            close(fReadFd);
        }
        int fReadFd,
                fWriteFd;
    };

    void startRead() {
        fCursor = 0;
        fEnd = 0;
    }

    template <typename T>
    T* startRead() {
        this->startRead();
        return this->read<T>();
    }

    template <typename T>
    T* read() {
        T* result = (T*)this->ensureAtLeast(sizeof(T));
        fCursor += sizeof(T);
        return result;
    }

    SkDescriptor* readDescriptor() {
        SkDescriptor* result = (SkDescriptor*)this->ensureAtLeast(sizeof(SkDescriptor));
        size_t size = result->getLength();
        this->ensureAtLeast(size);
        fCursor += size;
        return result;
    }

    template <typename T>
    SkArraySlice<T> readArray(int count) {
        size_t size = count * sizeof(T);
        const T* base = (const T*)this->ensureAtLeast(size);
        SkArraySlice<T> result = SkArraySlice<T>{base, (uint32_t)count};
        fCursor += size;
        return result;
    }

    size_t endRead() {return size();}

    sk_sp<SkData> readEntireData() {
        size_t* size = this->startRead<size_t>();
        if (size == nullptr) {
            return nullptr;
        }
        const uint8_t* data = this->readArray<uint8_t>(*size).data();
        if (size == nullptr || data == nullptr) {
            this->endRead();
            return sk_sp<SkData>(nullptr);
        }
        auto result = SkData::MakeWithCopy(data, *size);
        this->endRead();
        return result;
    }

    void startWrite() {
        fCursor = 0;
    }

    template <typename T>
    void startWrite(const T& data) {
        this->startWrite();
        this->write<T>(data);
    }

    template <typename T, typename... Args>
    T* startEmplace(Args&&... args) {
        this->startWrite();
        return this->emplace<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    T* emplace(Args&&... args) {
        T* result = new (&fBuffer[fCursor]) T{std::forward<Args>(args)...};
        fCursor += sizeof(T);
        return result;
    }

    template <typename T>
    void write(const T& data) {
        // TODO: guard against bad T.
        memcpy(&fBuffer[fCursor], &data, sizeof(data));
        fCursor += sizeof(data);
    }

    void writeDescriptor(const SkDescriptor& desc) {
        memcpy(&fBuffer[fCursor], &desc, desc.getLength());
        fCursor += desc.getLength();
    }

    template <typename T>
    T* allocateArray(int count) {
        T* result = (T*)&fBuffer[fCursor];
        fCursor += count * sizeof(T);
        return result;
    }

    IOResult endWrite() {
        ssize_t written;
        if((written = ::write(fWriteFd, fBuffer.get(), fCursor)) < 0) {
            return kFail;
        }
        return kSuccess;
    }

    IOResult writeEntireData(const SkData& data) {
        size_t size = data.size();
        iovec vec[2];
        vec[0].iov_base = &size;
        vec[0].iov_len = sizeof(size);
        vec[1].iov_base = (void *)data.data();
        vec[1].iov_len = size;

        if(::writev(fWriteFd, vec, 2) < 0) {
            return kFail;
        }
        return kSuccess;
    }

    size_t size() {return fCursor;}

private:
    void* ensureAtLeast(size_t size) {
        if (size > fEnd - fCursor) {
            if (readAtLeast(size) == kFail) {
                return nullptr;
            }
        }
        return &fBuffer[fCursor];
    }

    IOResult readAtLeast(size_t size) {
        size_t readSoFar = 0;
        size_t bufferLeft = kBufferSize - fCursor;
        size_t needed = size - (fEnd - fCursor);
        while (readSoFar < needed) {
            ssize_t readSize;
            if ((readSize = ::read(fReadFd, &fBuffer[fEnd+readSoFar], bufferLeft - readSoFar)) <= 0) {
                if (readSize != 0) {
                    err(1,"Failed read %zu", size);
                }
                return kFail;
            }
            readSoFar += readSize;
        }
        fEnd += readSoFar;
        return kSuccess;
    }

    static constexpr size_t kBufferSize = kPageSize * 2000;
    const int fReadFd,
            fWriteFd;

    std::unique_ptr<uint8_t[]> fBuffer{new uint8_t[kBufferSize]};
    std::shared_ptr<Closer> fCloser;

    size_t fCursor{0};
    size_t fEnd{0};
};

enum class OpCode : int32_t {
    kFontMetrics          = 0,
    kGlyphPath            = 1,
    kGlyphMetricsAndImage = 2,
    kPrepopulateCache     = 3,
};

class SkScalerContextRecDescriptor {
public:
    SkScalerContextRecDescriptor() {}
    explicit SkScalerContextRecDescriptor(const SkScalerContextRec& rec) {
        auto desc = reinterpret_cast<SkDescriptor*>(&fDescriptor);
        desc->init();
        desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
        desc->computeChecksum();
        SkASSERT(sizeof(fDescriptor) == desc->getLength());
    }

    explicit SkScalerContextRecDescriptor(const SkDescriptor& desc)
            : SkScalerContextRecDescriptor(ExtractRec(desc)) { }

    SkScalerContextRecDescriptor& operator=(const SkScalerContextRecDescriptor& rhs) {
        std::memcpy(&fDescriptor, &rhs.fDescriptor, rhs.desc().getLength());
        return *this;
    }

    const SkDescriptor& desc() const {
        return *reinterpret_cast<const SkDescriptor*>(&fDescriptor);
    }

    struct Hash {
        uint32_t operator()(SkScalerContextRecDescriptor const& s) const {
            return s.desc().getChecksum();
        }
    };

    friend bool operator==(const SkScalerContextRecDescriptor& lhs,
                           const SkScalerContextRecDescriptor& rhs ) {
        return lhs.desc() == rhs.desc();
    }

private:
    static SkScalerContextRec ExtractRec(const SkDescriptor& desc) {
        uint32_t size;
        auto recPtr = desc.findEntry(kRec_SkDescriptorTag, &size);

        SkScalerContextRec result;
        std::memcpy(&result, recPtr, size);
        return result;
    }
    // The system only passes descriptors without effects. That is why it uses a fixed size
    // descriptor. storageFor is needed because some of the constructors below are private.
    template <typename T>
    using storageFor = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    struct {
        storageFor<SkDescriptor>        dummy1;
        storageFor<SkDescriptor::Entry> dummy2;
        storageFor<SkScalerContextRec>  dummy3;
    } fDescriptor;
};

class SkRemoteGlyphCacheRenderer {
public:
    void prepareSerializeProcs(SkSerialProcs* procs);

    SkScalerContext* generateScalerContext(
            const SkScalerContextRecDescriptor& desc, SkFontID typefaceId);

private:
    sk_sp<SkData> encodeTypeface(SkTypeface* tf);

    SkTHashMap<SkFontID, sk_sp<SkTypeface>> fTypefaceMap;

    using DescriptorToContextMap = SkTHashMap<SkScalerContextRecDescriptor,
            std::unique_ptr<SkScalerContext>,
            SkScalerContextRecDescriptor::Hash>;

    DescriptorToContextMap fScalerContextMap;
};

class SkRemoteGlyphCacheGPU {
public:
    explicit SkRemoteGlyphCacheGPU(std::unique_ptr<SkRemoteScalerContext> remoteScalerContext);

    void prepareDeserializeProcs(SkDeserialProcs* procs);
    SkTypeface* lookupTypeface(SkFontID id);

private:
    sk_sp<SkTypeface> decodeTypeface(const void* buf, size_t len);

    std::unique_ptr<SkRemoteScalerContext> fRemoteScalerContext;
    // TODO: Figure out how to manage the entries for the following maps.
    SkTHashMap<SkFontID, sk_sp<SkTypefaceProxy>> fMapIdToTypeface;
};

class Op {
public:
    Op(OpCode opCode, SkFontID typefaceId, const SkScalerContextRec& rec)
            : opCode{opCode}
            , typefaceId{typefaceId}
            , descriptor{rec} { }
    const OpCode opCode;
    const SkFontID typefaceId;
    const SkScalerContextRecDescriptor descriptor;
    union {
        // op 0
        SkPaint::FontMetrics fontMetrics;
        // op 1, 2, and 4
        SkGlyph glyph;
        // op 3
        struct {
            SkGlyphID glyphId;
            size_t pathSize;
        };
    };
};

class SkStrikeCacheDifferenceSpec {
    class StrikeDifferences {
    public:
        StrikeDifferences(SkFontID typefaceID, std::unique_ptr<SkDescriptor> desc)
                : fTypefaceID{typefaceID}
                , fDesc{std::move(desc)} { }

        void operator()(uint16_t glyphID, SkIPoint pos) {
            SkPackedGlyphID packedGlyphID{glyphID, pos.x(), pos.y()};
            fGlyphIDs->add(packedGlyphID);
        }

        SkFontID fTypefaceID;
        std::unique_ptr<SkDescriptor> fDesc;
        std::unique_ptr<SkTHashSet<SkPackedGlyphID>> fGlyphIDs =
                skstd::make_unique<SkTHashSet<SkPackedGlyphID>>();
    };

    struct Header {
        Header(int strikeCount_) : strikeCount{strikeCount_} {}
        const int strikeCount;
    };

    struct StrikeSpec {
        StrikeSpec(SkFontID typefaceID_, uint32_t descLength_, int glyphCount_)
                : typefaceID{typefaceID_}
                , descLength{descLength_}
                , glyphCount{glyphCount_} { }
        SkFontID typefaceID;
        uint32_t descLength;
        int glyphCount;
        /* desc */
        /* n X (glyphs ids) */
    };

public:
    StrikeDifferences& findStrikeDifferences(const SkDescriptor& desc, SkFontID typefaceID) {
        auto mapIter = fDescMap.find(&desc);
        if (mapIter == fDescMap.end()) {
            auto newDesc = desc.copy();
            auto newDescPtr = newDesc.get();
            StrikeDifferences strikeDiffs{typefaceID, std::move(newDesc)};

            mapIter = fDescMap.emplace_hint(mapIter, newDescPtr, std::move(strikeDiffs));
        }

        return mapIter->second;
    }

    void writeSpecToTransport(AllInOneTransport* transport) {
        transport->emplace<Header>((int)fDescMap.size());
        for (auto& i : fDescMap) {
            auto accum = &i.second;
            transport->emplace<StrikeSpec>(
                    accum->fTypefaceID, accum->fDesc->getLength(), accum->fGlyphIDs->count());
            transport->writeDescriptor(*accum->fDesc);
            accum->fGlyphIDs->foreach([&](SkPackedGlyphID id) {
                transport->write<SkPackedGlyphID>(id);
            });
        }
    }

private:
    struct DescHash {
        size_t operator()(const SkDescriptor* key) const {
            return key->getChecksum();
        }
    };

    struct DescEq {
        bool operator()(const SkDescriptor* lhs, const SkDescriptor* rhs) const {
            return lhs->getChecksum() == rhs->getChecksum();
        }
    };

    using DescMap = std::unordered_map<const SkDescriptor*, StrikeDifferences, DescHash, DescEq>;
    DescMap fDescMap{16, DescHash(), DescEq()};
    std::vector<std::unique_ptr<SkDescriptor>> fUniqueDescriptors;
};


class SkTextBlobCacheDiffCanvas : public SkNoDrawCanvas {


public:
    struct StrikeSpec {
        StrikeSpec(SkFontID typefaceID_, uint32_t descLength_, int glyphCount_)
                : typefaceID{typefaceID_}
                , descLength{descLength_}
                , glyphCount{glyphCount_} { }
        SkFontID typefaceID;
        uint32_t descLength;
        int glyphCount;
        /* desc */
        /* n X (glyphs ids) */
    };

    struct Header {
        Header(int strikeCount_) : strikeCount{strikeCount_} {}
        const int strikeCount;
    };

    SkTextBlobCacheDiffCanvas(int width, int height,
                              const SkMatrix& deviceMatrix,
                              const SkSurfaceProps& props,
                              SkScalerContextFlags flags,
                              SkStrikeCacheDifferenceSpec* strikeDiffs);

    static void WriteDataToTransport(
            AllInOneTransport* in, AllInOneTransport* out, SkRemoteGlyphCacheRenderer* rc) {
        auto perHeader = [out](Header* header) {
            out->write<Header>(*header);
        };

        struct {
            SkScalerContext* scaler{nullptr};
        } strikeData;

        auto perStrike = [out, &strikeData, rc](StrikeSpec* spec, SkDescriptor* desc) {
            out->write<StrikeSpec>(*spec);
            out->writeDescriptor(*desc);
            SkScalerContextRecDescriptor recDesc{*desc};
            strikeData.scaler = rc->generateScalerContext(recDesc, spec->typefaceID);
            SkPaint::FontMetrics fontMetrics;
            strikeData.scaler->getFontMetrics(&fontMetrics);
            out->write<SkPaint::FontMetrics>(fontMetrics);
        };

        auto perGlyph = [out, &strikeData](SkPackedGlyphID glyphID) {
            SkGlyph glyph;
            glyph.initWithGlyphID(glyphID);
            strikeData.scaler->getMetrics(&glyph);
            auto imageSize = glyph.computeImageSize();
            glyph.fImage = nullptr;
            glyph.fPathData = nullptr;
            out->write<SkGlyph>(glyph);

            if (imageSize > 0) {
                glyph.fImage = out->allocateArray<uint8_t>(imageSize);
                strikeData.scaler->getImage(glyph);
            }
        };

        ReadSpecFromTransport(in, perHeader, perStrike, perGlyph);
    }

    template <typename PerHeader, typename PerStrike, typename PerGlyph>
    static void ReadSpecFromTransport(AllInOneTransport* transport,
                                      PerHeader perHeader,
                                      PerStrike perStrike,
                                      PerGlyph perGlyph) {
        auto header = transport->read<SkTextBlobCacheDiffCanvas::Header>();
        perHeader(header);
        for (int i = 0; i < header->strikeCount; i++) {
            auto strike = transport->read<SkTextBlobCacheDiffCanvas::StrikeSpec>();
            auto desc = transport->readDescriptor();
            //desc->assertChecksum();
            perStrike(strike, desc);
            auto glyphIDs = transport->readArray<SkPackedGlyphID>(strike->glyphCount);
            for (auto glyphID : glyphIDs) {
                perGlyph(glyphID);
            }
        }
    }

    template <typename PerStrike, typename PerGlyph, typename FinishStrike>
    void readDataFromTransport(
            AllInOneTransport* transport, PerStrike perStrike, PerGlyph perGlyph, FinishStrike finishStrike) {
        auto header = transport->read<Header>();
        for (int i = 0; i < header->strikeCount; i++) {
            auto strike = transport->read<StrikeSpec>();
            auto desc = transport->readDescriptor();
            auto fontMetrics = transport->read<SkPaint::FontMetrics>();
            perStrike(strike, desc, fontMetrics);
            for (int j = 0; j < strike->glyphCount; j++) {
                auto glyph = transport->read<SkGlyph>();
                SkArraySlice<uint8_t> image = SkArraySlice<uint8_t>{};
                auto imageSize = glyph->computeImageSize();
                if (imageSize != 0) {
                    image = transport->readArray<uint8_t>(imageSize);
                }
                perGlyph(glyph, image);
            }
            finishStrike();
        }
    }

protected:
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override;

    void onDrawTextBlob(
            const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint) override;

private:
    void processLooper(
            const SkPoint& position,
            const SkTextBlobRunIterator& it,
            const SkPaint& origPaint,
            SkDrawLooper* looper);

    void processGlyphRun(
            const SkPoint& position,
            const SkTextBlobRunIterator& it,
            const SkPaint& runPaint);

    const SkMatrix fDeviceMatrix;
    const SkSurfaceProps fSurfaceProps;
    const SkScalerContextFlags fScalerContextFlags;

    SkStrikeCacheDifferenceSpec* const fStrikeCacheDiff;
};


#endif  // SkRemoteGlyphCache_DEFINED
