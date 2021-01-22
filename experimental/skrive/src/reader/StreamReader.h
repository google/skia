/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRive_Streamreader_DEFINED
#define SkRive_Streamreader_DEFINED

#include <memory>

#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"

class SkStreamAsset;
class SkString;

namespace skrive::internal {

class StreamReader {
public:
    virtual ~StreamReader() = default;

    static std::unique_ptr<StreamReader> Make(std::unique_ptr<SkStreamAsset>);
    static std::unique_ptr<StreamReader> Make(const sk_sp<SkData>&);

    enum class BlockType : uint8_t {
        kUnknown              =   0,
        kComponents           =   1,
        kActorNode            =   2,

        kActorShape           = 100,
        kActorPath            = 101,
        kColorFill            = 102,
        kColorStroke          = 103,
        kGradientFill         = 104,
        kGradientStroke       = 105,
        kRadialGradientFill   = 106,
        kRadialGradientStroke = 107,
        kActorEllipse         = 108,
        kActorRectangle       = 109,
        kActorTriangle        = 110,
        kActorStar            = 111,
        kActorPolygon         = 112,
        kActorArtboard        = 114,
        kArtboards            = 115,

        // End-of-block marker
        kEoB           = 0xff,
    };

    // Sequential block API
    virtual BlockType openBlock() = 0;
    virtual void     closeBlock() = 0;

    // Keyed API
    virtual bool   openArray(const char label[]) = 0;
    virtual void  closeArray()                   = 0;
    virtual bool  openObject(const char label[]) = 0;
    virtual void closeObject()                   = 0;


    virtual uint16_t readId    (const char label[]) = 0;
    virtual bool     readBool  (const char label[]) = 0;
    virtual float    readFloat (const char label[]) = 0;
    virtual uint8_t  readUInt8 (const char label[]) = 0;
    virtual uint16_t readUInt16(const char label[]) = 0;
    virtual uint32_t readUInt32(const char label[]) = 0;
    virtual SkString readString(const char label[]) = 0;

    virtual uint8_t  readLength8 () = 0;
    virtual uint16_t readLength16() = 0;

    SkColor4f readColor(const char label[]);
    SkV2      readV2(const char label[]);

    class AutoBlock final {
    public:
        explicit AutoBlock(StreamReader* reader)
            : fReader(reader)
            , fType(reader->openBlock()) {}

        explicit AutoBlock(const std::unique_ptr<StreamReader>& reader)
            : fReader(reader.get())
            , fType(reader->openBlock()) {}

        ~AutoBlock() {
            if (fType != BlockType::kEoB) {
                fReader->closeBlock();
            }
        }

        BlockType type() const { return fType; }

    private:
        StreamReader* fReader;
        BlockType     fType;
    };

protected:
    virtual size_t readFloatArray(const char label[], float dst[], size_t count) = 0;
};

}  // namespace skrive::internal

#endif // SkRiveStreamreader_DEFINED
