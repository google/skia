/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/src/reader/StreamReader.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"

#include <vector>

namespace skrive::internal {

namespace {

class BinaryReader final : public StreamReader {
public:
    explicit BinaryReader(std::unique_ptr<SkStreamAsset> stream)
        : fStream(std::move(stream)) {}

private:
    bool validateSize(size_t sz) const {
        const auto next_pos  = fStream->getPosition() + sz,
                   block_end = fBlockStack.empty() ? fStream->getLength()
                                                   : fBlockStack.back().block_end;
        return next_pos <= block_end;
    }

    uint16_t readId(const char label[]) override {
        return this->readUInt16(label);
    }

    bool readBool(const char[]) override {
        uint8_t v;

        return validateSize(sizeof(v)) && fStream->readU8(&v)
                ? v == 1
                : false;
    }

    float readFloat(const char[]) override {
        float v;

        return validateSize(sizeof(v)) && fStream->readScalar(&v)
                ? v
                : 0.0f;
    }

    uint8_t readUInt8(const char[]) override {
        uint8_t v;

        return validateSize(sizeof(v)) && fStream->readU8(&v)
                ? v
                : 0;
    }

    uint16_t readUInt16(const char[]) override {
        uint16_t v;

        return validateSize(sizeof(v)) && fStream->readU16(&v)
                ? v
                : 0;
    }

    uint32_t readUInt32(const char[]) override {
        uint32_t v;

        return validateSize(sizeof(v)) && fStream->readU32(&v)
                ? v
                : 0;
    }

    SkString readString(const char[]) override {
        uint32_t length;
        if (!validateSize(sizeof(length)) || !fStream->readU32(&length)) {
            return SkString();
        }

        SkString str(length);
        return validateSize(length) && fStream->read(str.writable_str(), length) == length
            ? str
            : SkString();
    }

    size_t readFloatArray(const char[], float dst[], size_t count) override {
        if (!validateSize(count * sizeof(float))) {
            return 0;
        }

        return fStream->read(dst, count * sizeof(float)) / sizeof(float);
    }

    uint8_t readLength8() override {
        return this->readUInt8(nullptr);
    }

    uint16_t readLength16() override {
        return this->readUInt16(nullptr);
    }

    // nops
    bool   openArray(const char[]) override { return true; }
    void  closeArray()             override {}
    bool  openObject(const char[]) override { return true; }
    void closeObject()             override {}

    BlockType openBlock() override {
        uint8_t  block_type;
        uint32_t block_size;

        if (this->validateSize(sizeof(block_type) + sizeof(block_size)) &&
            fStream->readU8 (&block_type) &&
            fStream->readU32(&block_size)) {
            const auto block_end = std::min(fStream->getPosition() + block_size,
                                            fStream->getLength());
            fBlockStack.push_back({block_end});
            return static_cast<BlockType>(block_type);
        }

        return BlockType::kEoB;
    }

    void closeBlock() override {
        SkASSERT(!fBlockStack.empty());
        SkASSERT(fStream->getPosition() <= fBlockStack.back().block_end);

        if (fStream->getPosition() < fBlockStack.back().block_end) {
            const auto skip = fBlockStack.back().block_end - fStream->getPosition();
            SkDebugf("!! skipping %zu bytes in block\n", skip);
            fStream->skip(skip);
        }

        fBlockStack.pop_back();
    }

    const std::unique_ptr<SkStreamAsset> fStream;

    struct BlockRec {
        size_t block_end;
    };

    std::vector<BlockRec> fBlockStack;
};

} // namespace

std::unique_ptr<StreamReader> MakeBinaryStreamReader(std::unique_ptr<SkStreamAsset> stream) {
    return std::make_unique<BinaryReader>(std::move(stream));
}

}  // namespace skrive::internal
