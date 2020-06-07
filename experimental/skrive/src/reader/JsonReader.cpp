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

template <>
bool parse<int>(const Value& jv, int* res) { return parse_integral(jv, res); }

StreamReader::BlockType block_type(const char* type_name) {
    static constexpr struct TypeMapEntry {
        const char*             name;
        StreamReader::BlockType block_type;
    } gTypeMap[] = {
        {"node"   , StreamReader::BlockType::kActorNode },
        {"nodes"  , StreamReader::BlockType::kComponents},
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
        fContext.push_back({&fDom->root(), 0});
    }

    ~JsonReader() override {
        SkASSERT(fContext.size() == 1);
    }

private:
    BlockType openBlock() override {
        auto& ctx = fContext.back();

        while (ctx.fMemberIndex < ctx.member_count()) {
            const auto& m = ctx.current_member();

            if (m.is<skjson::ObjectValue>() ||
                m.is<skjson:: ArrayValue>()) {
                break;
            }

            ctx.fMemberIndex++;
        }

        if (ctx.fMemberIndex >= ctx.member_count()) {
            return BlockType::kEoB;
        }


        return BlockType::kUnknown;
    }

    void closeBlock() override {

    }

    struct ContextRec {
        const skjson::Value* fContainer;
        size_t               fMemberIndex;

        const skjson::Value& current_member() const {
            return fContainer->is<skjson::ObjectValue>()
                    ? fContainer->as<skjson::ObjectValue>()[fMemberIndex].fValue
                    : fContainer->as<skjson:: ArrayValue>()[fMemberIndex];
        }

        size_t member_count() const {
            return fContainer->is<skjson::ObjectValue>()
                    ? fContainer->as<skjson::ObjectValue>().size()
                    : fContainer->as<skjson:: ArrayValue>().size();
        }
    };

    const std::unique_ptr<skjson::DOM> fDom;

    std::vector<ContextRec>            fContext;
};

} // namespace

std::unique_ptr<StreamReader> MakeJsonStreamReader(const char json[], size_t len) {
    auto dom = std::make_unique<skjson::DOM>(json, len);

    return dom->root().is<skjson::ObjectValue>() ? std::make_unique<JsonReader>(std::move(dom))
                                                 : nullptr;
}

} // namespace skrive::internal
