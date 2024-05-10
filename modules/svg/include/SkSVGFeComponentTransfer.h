/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFeComponentTransfer_DEFINED
#define SkSVGFeComponentTransfer_DEFINED

#include "modules/svg/include/SkSVGFe.h"
#include "modules/svg/include/SkSVGHiddenContainer.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGTypes.h"

#include <cstdint>

class SkSVGFeFunc final : public SkSVGHiddenContainer {
public:
    static sk_sp<SkSVGFeFunc> MakeFuncA() {
        return sk_sp<SkSVGFeFunc>(new SkSVGFeFunc(SkSVGTag::kFeFuncA));
    }

    static sk_sp<SkSVGFeFunc> MakeFuncR() {
        return sk_sp<SkSVGFeFunc>(new SkSVGFeFunc(SkSVGTag::kFeFuncR));
    }

    static sk_sp<SkSVGFeFunc> MakeFuncG() {
        return sk_sp<SkSVGFeFunc>(new SkSVGFeFunc(SkSVGTag::kFeFuncG));
    }

    static sk_sp<SkSVGFeFunc> MakeFuncB() {
        return sk_sp<SkSVGFeFunc>(new SkSVGFeFunc(SkSVGTag::kFeFuncB));
    }

    SVG_ATTR(Amplitude  , SkSVGNumberType,                          1)
    SVG_ATTR(Exponent   , SkSVGNumberType,                          1)
    SVG_ATTR(Intercept  , SkSVGNumberType,                          0)
    SVG_ATTR(Offset     , SkSVGNumberType,                          0)
    SVG_ATTR(Slope      , SkSVGNumberType,                          1)
    SVG_ATTR(TableValues, std::vector<SkSVGNumberType>,            {})
    SVG_ATTR(Type       , SkSVGFeFuncType, SkSVGFeFuncType::kIdentity)

    std::vector<uint8_t> getTable() const;

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

private:
    SkSVGFeFunc(SkSVGTag tag) : INHERITED(tag) {}

    using INHERITED = SkSVGHiddenContainer;
};

class SK_API SkSVGFeComponentTransfer final : public SkSVGFe {
public:
    static constexpr SkSVGTag tag = SkSVGTag::kFeComponentTransfer;

    static sk_sp<SkSVGFeComponentTransfer> Make() {
        return sk_sp<SkSVGFeComponentTransfer>(new SkSVGFeComponentTransfer());
    }

protected:
    sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                           const SkSVGFilterContext&) const override;

    std::vector<SkSVGFeInputType> getInputs() const override { return {this->getIn()}; }

private:
    SkSVGFeComponentTransfer() : INHERITED(tag) {}

    using INHERITED = SkSVGFe;
};

#endif //  SkSVGFeComponentTransfer_DEFINED
