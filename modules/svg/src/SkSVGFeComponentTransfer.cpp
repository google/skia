/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGFeComponentTransfer.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkImageFilter.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTo.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGTypes.h"

#include <cmath>
#include <cstdint>

sk_sp<SkImageFilter> SkSVGFeComponentTransfer::onMakeImageFilter(
        const SkSVGRenderContext& ctx,
        const SkSVGFilterContext& fctx) const {
    std::vector<uint8_t> a_tbl, b_tbl, g_tbl, r_tbl;

    for (const auto& child : fChildren) {
        switch (child->tag()) {
            case SkSVGTag::kFeFuncA:
                a_tbl = static_cast<const SkSVGFeFunc*>(child.get())->getTable();
                break;
            case SkSVGTag::kFeFuncB:
                b_tbl = static_cast<const SkSVGFeFunc*>(child.get())->getTable();
                break;
            case SkSVGTag::kFeFuncG:
                g_tbl = static_cast<const SkSVGFeFunc*>(child.get())->getTable();
                break;
            case SkSVGTag::kFeFuncR:
                r_tbl = static_cast<const SkSVGFeFunc*>(child.get())->getTable();
                break;
            default:
                break;
        }
    }
    SkASSERT(a_tbl.empty() || a_tbl.size() == 256);
    SkASSERT(b_tbl.empty() || b_tbl.size() == 256);
    SkASSERT(g_tbl.empty() || g_tbl.size() == 256);
    SkASSERT(r_tbl.empty() || r_tbl.size() == 256);

    const SkRect cropRect = this->resolveFilterSubregion(ctx, fctx);
    const sk_sp<SkImageFilter> input = fctx.resolveInput(ctx,
                                                         this->getIn(),
                                                         this->resolveColorspace(ctx, fctx));

    const auto cf =  SkColorFilters::TableARGB(a_tbl.empty() ? nullptr : a_tbl.data(),
                                               r_tbl.empty() ? nullptr : r_tbl.data(),
                                               g_tbl.empty() ? nullptr : g_tbl.data(),
                                               b_tbl.empty() ? nullptr : b_tbl.data());

    return SkImageFilters::ColorFilter(std::move(cf), std::move(input), cropRect);
}

std::vector<uint8_t> SkSVGFeFunc::getTable() const {
    // https://www.w3.org/TR/SVG11/filters.html#feComponentTransferTypeAttribute
    const auto make_linear = [this]() -> std::vector<uint8_t> {
        std::vector<uint8_t> tbl(256);
        const float slope = this->getSlope(),
             intercept255 = this->getIntercept() * 255;

        for (size_t i = 0; i < 256; ++i) {
            tbl[i] = SkTPin<int>(sk_float_round2int(intercept255 + i * slope), 0, 255);
        }

        return tbl;
    };

    const auto make_gamma = [this]() -> std::vector<uint8_t> {
        std::vector<uint8_t> tbl(256);
        const float exponent = this->getExponent(),
                      offset = this->getOffset();

        for (size_t i = 0; i < 256; ++i) {
            const float component = offset + std::pow(i * (1 / 255.f), exponent);
            tbl[i] = SkTPin<int>(sk_float_round2int(component * 255), 0, 255);
        }

        return tbl;
    };

    const auto lerp_from_table_values = [this](auto lerp_func) -> std::vector<uint8_t> {
        const auto& vals = this->getTableValues();
        if (vals.size() < 2 || vals.size() > 255) {
            return {};
        }

        // number of interpolation intervals
        const size_t n = vals.size() - 1;

        std::vector<uint8_t> tbl(256);
        for (size_t k = 0; k < n; ++k) {
            // interpolation values
            const SkSVGNumberType v0 = SkTPin(vals[k + 0], 0.f, 1.f),
                                  v1 = SkTPin(vals[k + 1], 0.f, 1.f);

            // start/end component table indices
            const size_t c_start = k * 255 / n,
                         c_end   = (k + 1) * 255 / n;
            SkASSERT(c_end <= 255);

            for (size_t ci = c_start; ci < c_end; ++ci) {
                const float lerp_t = static_cast<float>(ci - c_start) / (c_end - c_start),
                         component = lerp_func(v0, v1, lerp_t);
                SkASSERT(component >= 0 && component <= 1);

                tbl[ci] = SkToU8(sk_float_round2int(component * 255));
            }
        }

        tbl.back() = SkToU8(sk_float_round2int(255 * SkTPin(vals.back(), 0.f, 1.f)));

        return tbl;
    };

    const auto make_table = [&]() -> std::vector<uint8_t> {
        return lerp_from_table_values([](float v0, float v1, float t) {
            return v0 + (v1 - v0) * t;
        });
    };

    const auto make_discrete = [&]() -> std::vector<uint8_t> {
        return lerp_from_table_values([](float v0, float v1, float t) {
            return v0;
        });
    };

    switch (this->getType()) {
        case SkSVGFeFuncType::kIdentity: return {};
        case SkSVGFeFuncType::kTable:    return make_table();
        case SkSVGFeFuncType::kDiscrete: return make_discrete();
        case SkSVGFeFuncType::kLinear:   return make_linear();
        case SkSVGFeFuncType::kGamma:    return make_gamma();
    }

    SkUNREACHABLE;
}

bool SkSVGFeFunc::parseAndSetAttribute(const char* name, const char* val) {
    return INHERITED::parseAndSetAttribute(name, val) ||
      this->setAmplitude(SkSVGAttributeParser::parse<SkSVGNumberType>("amplitude", name, val)) ||
      this->setExponent(SkSVGAttributeParser::parse<SkSVGNumberType>("exponent", name, val)) ||
      this->setIntercept(SkSVGAttributeParser::parse<SkSVGNumberType>("intercept", name, val)) ||
      this->setOffset(SkSVGAttributeParser::parse<SkSVGNumberType>("offset", name, val)) ||
      this->setSlope(SkSVGAttributeParser::parse<SkSVGNumberType>("slope", name, val)) ||
      this->setTableValues(SkSVGAttributeParser::parse<std::vector<SkSVGNumberType>>("tableValues",
                                                                                     name, val)) ||
      this->setType(SkSVGAttributeParser::parse<SkSVGFeFuncType>("type", name, val));
}

template <>
bool SkSVGAttributeParser::parse(SkSVGFeFuncType* type) {
    static constexpr std::tuple<const char*, SkSVGFeFuncType> gTypeMap[] = {
            { "identity", SkSVGFeFuncType::kIdentity },
            { "table"   , SkSVGFeFuncType::kTable    },
            { "discrete", SkSVGFeFuncType::kDiscrete },
            { "linear"  , SkSVGFeFuncType::kLinear   },
            { "gamma"   , SkSVGFeFuncType::kGamma    },
    };

    return this->parseEnumMap(gTypeMap, type) && this->parseEOSToken();
}
