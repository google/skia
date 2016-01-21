/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArithmeticMode.h"
#include "SkLerpXfermode.h"
#include "SkMatrix.h"
#include "SkPixelXorXfermode.h"
#include "SkToFromValue.h"
#include "SkValueKeys.h"
#include "SkXfermode.h"

////////////////////////////////////////////////////////////////////////////////

#define REQUIRE(cond) do { if (!(cond)) { SkASSERT(false); return false; } } while (false)

template <typename T>
bool getT(const SkValue& obj, SkValue::Key key, T* ptr) {
    auto v = obj.get(key);
    return v ? SkFromValue(*v, ptr) : false;
}

template<> bool SkFromValue<float>(const SkValue& val, float* f) {
    REQUIRE(val.type() == SkValue::F32);
    *f = val.f32();
    return true;
}

template<> bool SkFromValue<int32_t>(const SkValue& val, int32_t* x) {
    REQUIRE(val.type() == SkValue::S32);
    *x = val.s32();
    return true;
}

template<> bool SkFromValue<uint32_t>(const SkValue& val, uint32_t* x) {
    REQUIRE(val.type() == SkValue::U32);
    *x = val.u32();
    return true;
}

////////////////////////////////////////////////////////////////////////////////

template<> SkValue SkToValue<SkMatrix>(const SkMatrix& mat) {
    auto val = SkValue::Object(SkValue::Matrix);
    for (int i = 0; i < 9; ++i) {
        if (mat[i] != SkMatrix::I()[i]) {
            val.set(i, SkValue::FromF32(mat[i]));
        }
    }
    return val;
}

template<> bool SkFromValue<SkMatrix>(const SkValue& val, SkMatrix* m) {
    REQUIRE(val.type() == SkValue::Matrix);
    *m = SkMatrix::I();
    for (int i = 0; i < 9; i++) {
        getT(val, i, &(*m)[i]);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

template<> SkValue SkToValue<SkXfermode>(const SkXfermode* x) {
    return x ? x->asValue() : SkValue::Object(SkValue::DefaultXfermode);
}

static bool from_value_DefaultXfermode(const SkValue& val,
                                       SkAutoTUnref<SkXfermode>* dst) {
    dst->reset(nullptr);
    return true;
}

static bool from_value_ArithmeticXfermode(const SkValue& val,
                                          SkAutoTUnref<SkXfermode>* dst) {
    using namespace SkValueKeys::ArithmeticXfermode;
    float k[4];
    REQUIRE(getT(val, kK0, &k[0]));
    REQUIRE(getT(val, kK1, &k[1]));
    REQUIRE(getT(val, kK2, &k[2]));
    REQUIRE(getT(val, kK3, &k[3]));
    int32_t enforce = true;
    getT(val, kEnforcePMColor, &enforce);
    dst->reset(SkArithmeticMode::Create(
                       k[0], k[1], k[2], k[3], SkToBool(enforce)));
    return true;
}

static bool from_value_LerpXfermode(const SkValue& val,
                                    SkAutoTUnref<SkXfermode>* dst) {
    float scale;
    REQUIRE(getT(val, SkValueKeys::LerpXfermode::kScale, &scale));
    dst->reset(SkLerpXfermode::Create(scale));
    return true;
}

static bool from_value_PixelXorXfermode(const SkValue& val,
                                        SkAutoTUnref<SkXfermode>* dst) {
    uint32_t opColor;
    REQUIRE(getT(val, SkValueKeys::PixelXorXfermode::kOpColor, &opColor));
    dst->reset(SkPixelXorXfermode::Create(opColor));
    return true;
}

static bool from_value_ProcCoeffXfermode(const SkValue& val,
                                         SkAutoTUnref<SkXfermode>* dst) {
    uint32_t mode;
    REQUIRE(getT(val, SkValueKeys::ProcCoeffXfermode::kMode, &mode));
    dst->reset(SkXfermode::Create((SkXfermode::Mode)mode));
    return true;
}

template<> bool SkFromValue< SkAutoTUnref<SkXfermode> >(
        const SkValue& val, SkAutoTUnref<SkXfermode>* dst) {
    switch (val.type()) {
        case SkValue::DefaultXfermode:    return from_value_DefaultXfermode(val, dst);
        case SkValue::ArithmeticXfermode: return from_value_ArithmeticXfermode(val, dst);
        case SkValue::LerpXfermode:       return from_value_LerpXfermode(val, dst);
        case SkValue::PixelXorXfermode:   return from_value_PixelXorXfermode(val, dst);
        case SkValue::ProcCoeffXfermode:  return from_value_ProcCoeffXfermode(val, dst);
        default:                          REQUIRE(false);
    }
}

////////////////////////////////////////////////////////////////////////////////

#undef REQUIRE

