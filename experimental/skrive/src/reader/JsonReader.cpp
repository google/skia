/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/src/reader/StreamReader.h"
#include "include/core/SkString.h"
#include "src/utils/SkJSON.h"

#include <algorithm>
#include <iterator>
#include <memory>

namespace skrive::internal {
namespace {

StreamReader::BlockType block_type(const char* type_name) {
    static constexpr struct TypeMapEntry {
        const char*             name;
        StreamReader::BlockType block_type;
    } gTypeMap[] = {
        {"artboard" , StreamReader::BlockType::kActorArtboard },
        {"artboards", StreamReader::BlockType::kArtboards     },
        {"node"     , StreamReader::BlockType::kActorNode     },
        {"nodes"    , StreamReader::BlockType::kComponents    },
    };

    const TypeMapEntry key = { type_name, StreamReader::BlockType::kUnknown };
    const auto* map_entry = std::lower_bound(std::begin(gTypeMap),
                                             std::end  (gTypeMap),
                                             key,
                                             [](const TypeMapEntry& a, const TypeMapEntry& b) {
                                                 return strcmp(a.name, b.name) < 0;
                                             });

    return (map_entry != std::end(gTypeMap) && !strcmp(map_entry->name, key.name))
        ? map_entry->block_type
        : StreamReader::BlockType::kUnknown;
}

class JsonReader final : public StreamReader {
public:
    explicit JsonReader(std::unique_ptr<skjson::DOM> dom)
        : fDom(std::move(dom)) {
        fContextStack.push_back({&fDom->root(), 0});
    }

    ~JsonReader() override {
        SkASSERT(fContextStack.size() == 1);
    }

private:
    template <typename T>
    const T* readProp(const char label[]) const {
        const auto* current_container = fContextStack.back().fContainer;

        return current_container->is<skjson::ObjectValue>()
                ? static_cast<const T*>(current_container->as<skjson::ObjectValue>()[label])
                : nullptr;
    }

    bool readBool(const char label[]) override {
        const auto* jbool = this->readProp<skjson::BoolValue>(label);

        return jbool ? **jbool : false;
    }

    float readFloat(const char label[]) override {
        const auto* jnum = this->readProp<skjson::NumberValue>(label);

        return jnum ? static_cast<float>(**jnum) : 0.0f;
    }

    uint16_t readUInt16(const char label[]) override {
        return static_cast<uint16_t>(this->readUInt32(label));
    }

    uint32_t readUInt32(const char label[]) override {
        const auto* jnum = this->readProp<skjson::NumberValue>(label);

        return jnum ? static_cast<uint32_t>(**jnum) : 0;
    }

    SkString readString(const char label[]) override {
        const auto* jstr = this->readProp<skjson::StringValue>(label);

        return SkString(jstr ? jstr->begin() : nullptr);
    }

    size_t readFloatArray(const char label[], float dst[], size_t count) override {
        const auto* jarr = this->readProp<skjson::ArrayValue>(label);

        if (!jarr) {
            return 0;
        }

        count = std::min(count, jarr->size());

        for (size_t i = 0; i < count; ++i) {
            const skjson::NumberValue* jnum = (*jarr)[i];
            dst[i] = jnum ? static_cast<float>(**jnum) : 0.0f;
        }

        return count;
    }

    uint16_t readLength16() override {
        return SkToU16(this->currentLength());
    }

    size_t currentLength() const {
        const auto& ctx = fContextStack.back();
        return ctx.fContainer->is<skjson::ObjectValue>()
            ? ctx.fContainer->as<skjson::ObjectValue>().size()
            : ctx.fContainer->as<skjson:: ArrayValue>().size();
    }

    // "Blocks" map to either objects or arrays.  For object containers, the block type is encoded
    // as the key; for array containers, the type is an explicit "type" property *inside* the block
    // entry - which must be an object in this case.
    BlockType openBlock() override {
        switch (fContextStack.back().fContainer->getType()) {
            case skjson::Value::Type::kObject: return this->openObjectBlock();
            case skjson::Value::Type::kArray:  return this->openArrayBlock();
            default: break;
        }
        SkUNREACHABLE;
    }

    BlockType openObjectBlock() {
        auto& ctx = fContextStack.back();
        const auto& container = ctx.fContainer->as<skjson::ObjectValue>();

        while (ctx.fMemberIndex < container.size()) {
            const auto& m = container[ctx.fMemberIndex];
            if (m.fValue.is<skjson::ObjectValue>() || m.fValue.is<skjson::ArrayValue>()) {
                fContextStack.push_back({&m.fValue, 0});
                return block_type(m.fKey.begin());
            }

            ctx.fMemberIndex++;
        }

        return BlockType::kEoB;
    }

    BlockType openArrayBlock() {
        auto& ctx = fContextStack.back();
        const auto& container = ctx.fContainer->as<skjson::ArrayValue>();

        while (ctx.fMemberIndex < container.size()) {
            const auto& m = container[ctx.fMemberIndex];
            if (m.is<skjson::ObjectValue>()) {
                if (const skjson::StringValue* jtype = m.as<skjson::ObjectValue>()["type"]) {
                    fContextStack.push_back({&m, 0});
                    return block_type(jtype->begin());
                }
            }

            ctx.fMemberIndex++;
        }

        return BlockType::kEoB;
    }

    void closeBlock() override {
        SkASSERT(fContextStack.size() > 1);
        fContextStack.pop_back();
        fContextStack.back().fMemberIndex++;
    }

    struct ContextRec {
        const skjson::Value* fContainer;
        size_t               fMemberIndex;
    };

    const std::unique_ptr<skjson::DOM> fDom;

    std::vector<ContextRec>            fContextStack;
};

} // namespace

std::unique_ptr<StreamReader> MakeJsonStreamReader(const char json[], size_t len) {
    auto dom = std::make_unique<skjson::DOM>(json, len);

    return dom->root().is<skjson::ObjectValue>() ? std::make_unique<JsonReader>(std::move(dom))
                                                 : nullptr;
}

} // namespace skrive::internal
