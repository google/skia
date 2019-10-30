/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/geometry/GrQuadBuffer.h"

#include <vector>

#define ASSERT(cond) REPORTER_ASSERT(r, cond)
#define ASSERTF(cond, ...) REPORTER_ASSERT(r, cond, __VA_ARGS__)
#define TEST(name) DEF_TEST(GrQuadBuffer##name, r)

struct TestData {
    int fItem1;
    float fItem2;
};

static void assert_quad_eq(skiatest::Reporter* r, const GrQuad& expected, const GrQuad& actual) {
    ASSERTF(expected.quadType() == actual.quadType(), "Expected type %d, got %d",
            (int) expected.quadType(), (int) actual.quadType());
    for (int i = 0; i < 4; ++i) {
        ASSERTF(expected.x(i) == actual.x(i), "Expected x(%d) = %f, got %d",
                i, expected.x(i), actual.x(i));
        ASSERTF(expected.y(i) == actual.y(i), "Expected y(%d) = %f, got %d",
                i, expected.y(i), actual.y(i));
        ASSERTF(expected.w(i) == actual.w(i), "Expected w(%d) = %f, got %d",
                i, expected.w(i), actual.w(i));
    }
}

static void assert_metadata_eq(skiatest::Reporter* r, const TestData& expected,
                               const TestData& actual) {
    ASSERTF(expected.fItem1 == actual.fItem1 && expected.fItem2 == actual.fItem2,
            "Expected { %d, %f } for metadata, got: { %d %f }",
            expected.fItem1, expected.fItem2, actual.fItem1, actual.fItem2);
}

static std::vector<GrQuad> generate_quads(float seed, int cnt, const GrQuad::Type types[]) {
    // For convenience use matrix to derive each quad type, rely on different seed values to
    // differentiate between quads of the same type
    SkMatrix rotate;
    rotate.setRotate(45.f);
    SkMatrix skew;
    skew.setSkew(0.5f, 0.5f);
    SkMatrix perspective;
    perspective.setPerspX(0.01f);
    perspective.setPerspY(0.001f);

    std::vector<GrQuad> quads;
    SkRect rect = SkRect::MakeXYWH(seed, 2.f * seed, 2.f * seed, seed);
    for (int i = 0; i < cnt; ++i) {
        GrQuad quad;
        switch(types[i]) {
            case GrQuad::Type::kAxisAligned:
                quad = GrQuad(rect);
                break;
            case GrQuad::Type::kRectilinear:
                quad = GrQuad::MakeFromRect(rect, rotate);
                break;
            case GrQuad::Type::kGeneral:
                quad = GrQuad::MakeFromRect(rect, skew);
                break;
            default:
                SkASSERT(types[i] == GrQuad::Type::kPerspective);
                quad = GrQuad::MakeFromRect(rect, perspective);
                break;
        }

        SkASSERT(quad.quadType() == types[i]); // sanity check
        quads.push_back(quad);
    }
    return quads;
}

TEST(Append) {
    // Generate test data, which includes all quad types out of enum-order and duplicates
    static const int kQuadCount = 6;
    static const GrQuad::Type kDeviceTypes[] = {
        GrQuad::Type::kAxisAligned, GrQuad::Type::kRectilinear, GrQuad::Type::kGeneral,
        GrQuad::Type::kPerspective, GrQuad::Type::kRectilinear, GrQuad::Type::kAxisAligned
    };
    // Odd indexed quads will be ignored and not stored in the buffer
    static const GrQuad::Type kLocalTypes[] = {
        GrQuad::Type::kGeneral, GrQuad::Type::kGeneral, GrQuad::Type::kRectilinear,
        GrQuad::Type::kRectilinear, GrQuad::Type::kAxisAligned, GrQuad::Type::kAxisAligned
    };
    static_assert(SK_ARRAY_COUNT(kDeviceTypes) == kQuadCount, "device quad count");
    static_assert(SK_ARRAY_COUNT(kLocalTypes) == kQuadCount, "local quad count");

    std::vector<GrQuad> expectedDeviceQuads = generate_quads(1.f, kQuadCount, kDeviceTypes);
    std::vector<GrQuad> expectedLocalQuads = generate_quads(2.f, kQuadCount, kLocalTypes);

    // Fill in the buffer with the device quads, and a local quad if the index is even
    GrQuadBuffer<TestData> buffer;
    for (int i = 0; i < kQuadCount; ++i) {
        buffer.append(expectedDeviceQuads[i],                          // device quad
                      { 2 * i, 3.f * i },                              // metadata
                      i % 2 == 0 ? &expectedLocalQuads[i] : nullptr);  // optional local quad
    }

    // Confirm the state of the buffer
    ASSERT(kQuadCount == buffer.count());
    ASSERT(GrQuad::Type::kPerspective == buffer.deviceQuadType());
    ASSERT(GrQuad::Type::kGeneral == buffer.localQuadType());

    int i = 0;
    auto iter = buffer.iterator();
    while(iter.next()) {
        // Each entry always has the device quad
        assert_quad_eq(r, expectedDeviceQuads[i], iter.deviceQuad());
        assert_metadata_eq(r, {2 * i, 3.f * i}, iter.metadata());

        if (i % 2 == 0) {
            // Confirm local quads included on even entries
            ASSERT(iter.isLocalValid());
            assert_quad_eq(r, expectedLocalQuads[i], iter.localQuad());
        } else {
            // Should not have locals
            ASSERT(!iter.isLocalValid());
        }

        i++;
    }
    ASSERTF(i == kQuadCount, "Expected %d iterations, got: %d", kQuadCount, i);
}

TEST(Concat) {
    static const int kQuadCount = 2;
    static const GrQuad::Type kTypesA[] = { GrQuad::Type::kAxisAligned, GrQuad::Type::kRectilinear };
    static const GrQuad::Type kTypesB[] = { GrQuad::Type::kGeneral, GrQuad::Type::kPerspective };
    static_assert(SK_ARRAY_COUNT(kTypesA) == kQuadCount, "quadsA count");
    static_assert(SK_ARRAY_COUNT(kTypesB) == kQuadCount, "quadsB count");

    std::vector<GrQuad> quadsA = generate_quads(1.f, kQuadCount, kTypesA);
    std::vector<GrQuad> quadsB = generate_quads(2.f, kQuadCount, kTypesB);
    // Make two buffers, the first uses 'quadsA' for device quads and 'quadsB' for local quads
    // on even indices. The second uses 'quadsB' for device quads and 'quadsA' for local quads
    // on odd indices.
    GrQuadBuffer<TestData> buffer1;
    GrQuadBuffer<TestData> buffer2;
    for (int i = 0; i < kQuadCount; ++i) {
        buffer1.append(quadsA[i], {i, 2.f * i}, i % 2 == 0 ? &quadsB[i] : nullptr);
        buffer2.append(quadsB[i], {2 * i, 0.5f * i}, i % 2 == 0 ? nullptr : &quadsA[i]);
    }

    // Sanity check
    ASSERT(kQuadCount == buffer1.count());
    ASSERT(kQuadCount == buffer2.count());

    // Perform the concatenation and then confirm the new state of buffer1
    buffer1.concat(buffer2);

    ASSERT(2 * kQuadCount == buffer1.count());
    int i = 0;
    auto iter = buffer1.iterator();
    while(iter.next()) {
        if (i < kQuadCount) {
            // First half should match original buffer1
            assert_quad_eq(r, quadsA[i], iter.deviceQuad());
            assert_metadata_eq(r, {i, 2.f * i}, iter.metadata());
            if (i % 2 == 0) {
                ASSERT(iter.isLocalValid());
                assert_quad_eq(r, quadsB[i], iter.localQuad());
            } else {
                ASSERT(!iter.isLocalValid());
            }

        } else {
            // Second half should match buffer2
            int j = i - kQuadCount;
            assert_quad_eq(r, quadsB[j], iter.deviceQuad());
            assert_metadata_eq(r, {2 * j, 0.5f * j}, iter.metadata());
            if (j % 2 == 0) {
                ASSERT(!iter.isLocalValid());
            } else {
                ASSERT(iter.isLocalValid());
                assert_quad_eq(r, quadsA[j], iter.localQuad());
            }
        }

        i++;
    }
    ASSERTF(i == 2 * kQuadCount, "Expected %d iterations, got: %d",2 * kQuadCount, i);
}

TEST(Metadata) {
    static const int kQuadCount = 3;

    // This test doesn't really care about the quad coordinates (except that they aren't modified
    // when mutating the metadata)
    GrQuad quad(SkRect::MakeLTRB(1.f, 2.f, 3.f, 4.f));

    GrQuadBuffer<TestData> buffer;
    for (int i = 0; i < kQuadCount; ++i) {
        buffer.append(quad, {i, 2.f * i}, i % 2 == 0 ? &quad : nullptr);
    }

    // Iterate once using the metadata iterator, confirm the test data and rewrite
    int i = 0;
    auto meta = buffer.metadata();
    while(meta.next()) {
        // Confirm initial state
        assert_metadata_eq(r, {i, 2.f * i}, *meta);
        // Rewrite
        *meta = {2 * i, 0.5f * i};
        i++;
    }
    ASSERTF(i == kQuadCount, "Expected %d iterations, got: %d", kQuadCount, i);

    // Now that all metadata has been touched, read with regular iterator and confirm updated state
    // and that no quad coordinates have been changed.
    i = 0;
    auto iter = buffer.iterator();
    while(iter.next()) {
        // New metadata
        assert_metadata_eq(r, {2 * i, 0.5f * i}, iter.metadata());

        // Quad coordinates are unchanged
        assert_quad_eq(r, quad, iter.deviceQuad());
        if (i % 2 == 0) {
            ASSERT(iter.isLocalValid());
            assert_quad_eq(r, quad, iter.localQuad());
        } else {
            ASSERT(!iter.isLocalValid());
        }
        i++;
    }
    ASSERTF(i == kQuadCount, "Expected %d iterations, got: %d", kQuadCount, i);
}
