/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkMatrix.h"
#include "SkValue.h"
#include "Test.h"

static const SkValue::Type example_type =
    SkValue::Type(SkValue::kMaxPublicObject + 1);

enum { kExampleS32, kExampleF32, kExampleU32, kExampleObject, kExampleBytes,
       kExampleF32s, kExampleU32s, kExampleU16s, kExampleArray };

static const uint16_t aU16[] = { 1, 2, 3, 4 };
static const uint32_t aU32[] = { 5, 6, 7, 8 };
static const float aF32[] = { 9.0f, 9.125f, 9.25f };
static const char hello[] = "HELLO";

static SkValue make_example(skiatest::Reporter* r, int level = 4) {
    auto value = SkValue::Object(example_type);
    value.set(kExampleU32, SkValue::FromU32(1000));
    value.set(kExampleS32, SkValue::FromS32(-123));
    value.set(kExampleF32, SkValue::FromF32(0.5f));
    value.set(kExampleU32, SkValue::FromU32(1234));
    if (level > 0) {
        value.set(kExampleObject, make_example(r, 0));
        value.set(kExampleObject, make_example(r, level - 1)); // replace
    }
    SkAutoTUnref<SkData> data(SkData::NewWithCString(hello));
    value.set(kExampleBytes, SkValue::FromBytes(data));

    SkAutoTUnref<SkData> dataU16(SkData::NewWithCopy(aU16, sizeof(aU16)));
    SkAutoTUnref<SkData> dataU32(SkData::NewWithCopy(aU32, sizeof(aU32)));
    SkAutoTUnref<SkData> dataF32(SkData::NewWithCopy(aF32, sizeof(aF32)));
    value.set(kExampleU16s, SkValue::FromU16s(dataU16));
    value.set(kExampleU32s, SkValue::FromU32s(dataU32));
    value.set(kExampleF32s, SkValue::FromF32s(dataF32));

    auto varray = SkValue::ValueArray();
    varray.append(SkValue::FromU32(99));
    varray.append(SkValue::FromS32(-99));
    value.set(kExampleArray, std::move(varray));
    return value;
}

DEF_TEST(Value, r) {
    SkValue val = make_example(r);
    REPORTER_ASSERT(r, example_type == val.type());
    SkValue valCopy = val;
    REPORTER_ASSERT(r, example_type == valCopy.type());
    valCopy.set(4321, SkValue());
    auto fn = [&](SkValue::Key k, const SkValue& v){
        int count;
        switch (k) {
            case kExampleS32:
                REPORTER_ASSERT(r, -123 == v.s32());
                break;
            case kExampleF32:
                REPORTER_ASSERT(r, 0.5f == v.f32());
                break;
            case kExampleU32:
                REPORTER_ASSERT(r, 1234 == v.u32());
                break;
            case kExampleObject:
                REPORTER_ASSERT(r, example_type == v.type());
                break;
            case kExampleBytes:
                REPORTER_ASSERT(r, v.type() == SkValue::Bytes && v.bytes()
                                && v.bytes()->size() == sizeof(hello));
                break;
            case kExampleF32s:
                REPORTER_ASSERT(r, v.type() == SkValue::F32s && v.bytes()
                                && v.bytes()->size() == sizeof(aF32)
                                && v.f32s(&count)
                                && count == SK_ARRAY_COUNT(aF32));
                break;
            case kExampleU32s:
                REPORTER_ASSERT(r, v.type() == SkValue::U32s && v.bytes()
                                && v.bytes()->size() == sizeof(aU32)
                                && v.u32s(&count)
                                && count == SK_ARRAY_COUNT(aU32));
                break;
            case kExampleU16s:
                REPORTER_ASSERT(r, v.type() == SkValue::U16s && v.bytes()
                                && v.bytes()->size() == sizeof(aU16)
                                && v.u16s(&count)
                                && count == SK_ARRAY_COUNT(aU16));
                break;
            case kExampleArray:
                REPORTER_ASSERT(r, v.type() == SkValue::Array
                                && v.length() == 2);
                break;
            default:
                ERRORF(r, "unexpected key");
        }
    };
    val.foreach(fn);
}

DEF_TEST(Value_Matrix, r) {
    auto m = SkMatrix::MakeTrans(900.0f, 1000.0f);
    auto val = SkToValue(m);
    SkMatrix dst;
    REPORTER_ASSERT(r, SkFromValue(val, &dst));
    REPORTER_ASSERT(r, dst == m);
}
