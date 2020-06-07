/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/src/reader/StreamReader.h"
#include "src/utils/SkJSON.h"

#include <algorithm>
#include <iterator>
#include <memory>

namespace skrive::internal {
namespace {

using namespace skjson;

template <typename T>
bool parse(const Value&, T*);

template <typename T>
bool parse_integral(const Value& v, T* result) {
    if (const NumberValue* num = v) {
        const auto dbl = **num;
        *result = static_cast<T>(dbl);
        return static_cast<double>(*result) == dbl;
    }

    return false;
}

template <typename T>
T parse_default(const Value& jv, const T& default_value) {
    T res;
    if (!parse(jv, &res)) {
        res = default_value;
    }
    return res;
}

//template <>
//bool parse<int>(const Value& jv, int* res) { return parse_integral(jv, res); }

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
    BlockType openBlock() override {
        switch (fContextStack.back().fContainer->getType()) {
            case Value::Type::kObject: return this->openObjectBlock();
            case Value::Type::kArray:  return this->openArrayBlock();
            default: break;
        }
        SkUNREACHABLE;
    }

    BlockType openObjectBlock() {
        auto& ctx = fContextStack.back();
        const auto& container = ctx.fContainer->as<ObjectValue>();

        while (ctx.fMemberIndex < container.size()) {
            const auto& m = container[ctx.fMemberIndex];
            if (m.fValue.is<ObjectValue>() || m.fValue.is<ArrayValue>()) {
                fContextStack.push_back({&m.fValue, 0});
                return block_type(m.fKey.begin());
            }

            ctx.fMemberIndex++;
        }

        return BlockType::kEoB;
    }

    BlockType openArrayBlock() {
        auto& ctx = fContextStack.back();
        const auto& container = ctx.fContainer->as<ArrayValue>();

        while (ctx.fMemberIndex < container.size()) {
            const auto& m = container[ctx.fMemberIndex];
            if (m.is<ObjectValue>()) {
                if (const StringValue* jtype = m.as<ObjectValue>()["type"]) {
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
        const  Value* fContainer;
        size_t        fMemberIndex;
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
