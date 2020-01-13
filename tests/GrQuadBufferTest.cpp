// /*
//  * Copyright 2019 Google LLC
//  *
//  * Use of this source code is governed by a BSD-style license that can be
//  * found in the LICENSE file.
//  */

// #include "tests/Test.h"

// #include "src/gpu/geometry/GrQuadBuffer.h"

// #include <vector>

// #define ASSERT(cond) REPORTER_ASSERT(r, cond)
// #define ASSERTF(cond, ...) REPORTER_ASSERT(r, cond, __VA_ARGS__)
// #define TEST(name) DEF_TEST(GrQuadBuffer##name, r)

// struct TestData {
//     int fItem1;
//     float fItem2;
// };

// static void assert_quad_eq(skiatest::Reporter* r, const GrQuad& expected, const GrQuad& actual) {
//     ASSERTF(expected.quadType() == actual.quadType(), "Expected type %d, got %d",
//             (int) expected.quadType(), (int) actual.quadType());
//     for (int i = 0; i < 4; ++i) {
//         ASSERTF(expected.x(i) == actual.x(i), "Expected x(%d) = %f, got %d",
//                 i, expected.x(i), actual.x(i));
//         ASSERTF(expected.y(i) == actual.y(i), "Expected y(%d) = %f, got %d",
//                 i, expected.y(i), actual.y(i));
//         ASSERTF(expected.w(i) == actual.w(i), "Expected w(%d) = %f, got %d",
//                 i, expected.w(i), actual.w(i));
//     }
// }

// static void assert_metadata_eq(skiatest::Reporter* r, const TestData& expected,
//                                const TestData& actual) {
//     ASSERTF(expected.fItem1 == actual.fItem1 && expected.fItem2 == actual.fItem2,
//             "Expected { %d, %f } for metadata, got: { %d %f }",
//             expected.fItem1, expected.fItem2, actual.fItem1, actual.fItem2);
// }

// static std::vector<GrQuad> generate_quads(float seed, int cnt, const GrQuad::Type types[]) {
//     // For convenience use matrix to derive each quad type, rely on different seed values to
//     // differentiate between quads of the same type
//     SkMatrix rotate;
//     rotate.setRotate(45.f);
//     SkMatrix skew;
//     skew.setSkew(0.5f, 0.5f);
//     SkMatrix perspective;
//     perspective.setPerspX(0.01f);
//     perspective.setPerspY(0.001f);

//     std::vector<GrQuad> quads;
//     SkRect rect = SkRect::MakeXYWH(seed, 2.f * seed, 2.f * seed, seed);
//     for (int i = 0; i < cnt; ++i) {
//         GrQuad quad;
//         switch(types[i]) {
//             case GrQuad::Type::kAxisAligned:
//                 quad = GrQuad(rect);
//                 break;
//             case GrQuad::Type::kRectilinear:
//                 quad = GrQuad::MakeFromRect(rect, rotate);
//                 break;
//             case GrQuad::Type::kGeneral:
//                 quad = GrQuad::MakeFromRect(rect, skew);
//                 break;
//             default:
//                 SkASSERT(types[i] == GrQuad::Type::kPerspective);
//                 quad = GrQuad::MakeFromRect(rect, perspective);
//                 break;
//         }

//         SkASSERT(quad.quadType() == types[i]); // sanity check
//         quads.push_back(quad);
//     }
//     return quads;
// }

// TEST(Append) {
//     // Generate test data, which includes all quad types out of enum-order and duplicates
//     static const int kQuadCount = 6;
//     static const GrQuad::Type kDeviceTypes[] = {
//         GrQuad::Type::kAxisAligned, GrQuad::Type::kRectilinear, GrQuad::Type::kGeneral,
//         GrQuad::Type::kPerspective, GrQuad::Type::kRectilinear, GrQuad::Type::kAxisAligned
//     };
//     // Odd indexed quads will be ignored and not stored in the buffer
//     static const GrQuad::Type kLocalTypes[] = {
//         GrQuad::Type::kGeneral, GrQuad::Type::kGeneral, GrQuad::Type::kRectilinear,
//         GrQuad::Type::kRectilinear, GrQuad::Type::kAxisAligned, GrQuad::Type::kAxisAligned
//     };
//     static_assert(SK_ARRAY_COUNT(kDeviceTypes) == kQuadCount, "device quad count");
//     static_assert(SK_ARRAY_COUNT(kLocalTypes) == kQuadCount, "local quad count");

//     std::vector<GrQuad> expectedDeviceQuads = generate_quads(1.f, kQuadCount, kDeviceTypes);
//     std::vector<GrQuad> expectedLocalQuads = generate_quads(2.f, kQuadCount, kLocalTypes);

//     // Fill in the buffer with the device quads, and a local quad if the index is even
//     SkSTArenaAlloc<512> arena;
//     GrQuadBuffer<TestData> buffer;
//     for (int i = 0; i < kQuadCount; ++i) {
//         buffer.append(&arena, expectedDeviceQuads[i],                   // device quad
//                       { 2 * i, 3.f * i },                              // metadata
//                       i % 2 == 0 ? &expectedLocalQuads[i] : nullptr);  // optional local quad
//     }

//     // Confirm the state of the buffer
//     ASSERT(kQuadCount == buffer.count());
//     ASSERT(GrQuad::Type::kPerspective == buffer.deviceQuadType());
//     ASSERT(GrQuad::Type::kGeneral == buffer.localQuadType());

//     int i = 0;
//     auto iter = buffer.iterator();
//     while(iter.next()) {
//         // Each entry always has the device quad
//         assert_quad_eq(r, expectedDeviceQuads[i], *iter.deviceQuad());
//         assert_metadata_eq(r, {2 * i, 3.f * i}, iter.metadata());

//         if (i % 2 == 0) {
//             // Confirm local quads included on even entries
//             ASSERT(iter.isLocalValid());
//             assert_quad_eq(r, expectedLocalQuads[i], *iter.localQuad());
//         } else {
//             // Should not have locals
//             ASSERT(!iter.isLocalValid());
//             ASSERT(!iter.localQuad());
//         }

//         i++;
//     }
//     ASSERTF(i == kQuadCount, "Expected %d iterations, got: %d", kQuadCount, i);
// }

// TEST(Concat) {
//     static const int kQuadCount = 2;
//     static const GrQuad::Type kTypesA[] = { GrQuad::Type::kAxisAligned, GrQuad::Type::kRectilinear };
//     static const GrQuad::Type kTypesB[] = { GrQuad::Type::kGeneral, GrQuad::Type::kPerspective };
//     static_assert(SK_ARRAY_COUNT(kTypesA) == kQuadCount, "quadsA count");
//     static_assert(SK_ARRAY_COUNT(kTypesB) == kQuadCount, "quadsB count");

//     std::vector<GrQuad> quadsA = generate_quads(1.f, kQuadCount, kTypesA);
//     std::vector<GrQuad> quadsB = generate_quads(2.f, kQuadCount, kTypesB);
//     // Make two buffers, the first uses 'quadsA' for device quads and 'quadsB' for local quads
//     // on even indices. The second uses 'quadsB' for device quads and 'quadsA' for local quads
//     // on odd indices.
//     SkSTArenaAlloc<512> arena;
//     GrQuadBuffer<TestData> buffer1;
//     GrQuadBuffer<TestData> buffer2;
//     for (int i = 0; i < kQuadCount; ++i) {
//         buffer1.append(&arena, quadsA[i], {i, 2.f * i}, i % 2 == 0 ? &quadsB[i] : nullptr);
//         buffer2.append(&arena, quadsB[i], {2 * i, 0.5f * i}, i % 2 == 0 ? nullptr : &quadsA[i]);
//     }

//     // Sanity check
//     ASSERT(kQuadCount == buffer1.count());
//     ASSERT(kQuadCount == buffer2.count());

//     // Perform the concatenation and then confirm the new state of buffer1
//     buffer1.concat(&arena, &buffer2);

//     ASSERT(2 * kQuadCount == buffer1.count());
//     int i = 0;
//     auto iter = buffer1.iterator();
//     while(iter.next()) {
//         if (i < kQuadCount) {
//             // First half should match original buffer1
//             assert_quad_eq(r, quadsA[i], *iter.deviceQuad());
//             assert_metadata_eq(r, {i, 2.f * i}, iter.metadata());
//             if (i % 2 == 0) {
//                 ASSERT(iter.isLocalValid());
//                 assert_quad_eq(r, quadsB[i], *iter.localQuad());
//             } else {
//                 ASSERT(!iter.isLocalValid());
//                 ASSERT(!iter.localQuad());
//             }

//         } else {
//             // Second half should match buffer2
//             int j = i - kQuadCount;
//             assert_quad_eq(r, quadsB[j], *iter.deviceQuad());
//             assert_metadata_eq(r, {2 * j, 0.5f * j}, iter.metadata());
//             if (j % 2 == 0) {
//                 ASSERT(!iter.isLocalValid());
//                 ASSERT(!iter.localQuad());
//             } else {
//                 ASSERT(iter.isLocalValid());
//                 assert_quad_eq(r, quadsA[j], *iter.localQuad());
//             }
//         }

//         i++;
//     }
//     ASSERTF(i == 2 * kQuadCount, "Expected %d iterations, got: %d",2 * kQuadCount, i);
// }

// TEST(Metadata) {
//     static const int kQuadCount = 3;

//     // This test doesn't really care about the quad coordinates (except that they aren't modified
//     // when mutating the metadata)
//     GrQuad quad(SkRect::MakeLTRB(1.f, 2.f, 3.f, 4.f));

//     SkSTArenaAlloc<512> arena;
//     GrQuadBuffer<TestData> buffer;
//     for (int i = 0; i < kQuadCount; ++i) {
//         buffer.append(&arena, quad, {i, 2.f * i}, i % 2 == 0 ? &quad : nullptr);
//     }

//     // Iterate once using the metadata iterator, confirm the test data and rewrite
//     int i = 0;
//     auto meta = buffer.metadata();
//     while(meta.next()) {
//         // Confirm initial state
//         assert_metadata_eq(r, {i, 2.f * i}, *meta);
//         // Rewrite
//         *meta = {2 * i, 0.5f * i};
//         i++;
//     }
//     ASSERTF(i == kQuadCount, "Expected %d iterations, got: %d", kQuadCount, i);

//     // Now that all metadata has been touched, read with regular iterator and confirm updated state
//     // and that no quad coordinates have been changed.
//     i = 0;
//     auto iter = buffer.iterator();
//     while(iter.next()) {
//         // New metadata
//         assert_metadata_eq(r, {2 * i, 0.5f * i}, iter.metadata());

//         // Quad coordinates are unchanged
//         assert_quad_eq(r, quad, *iter.deviceQuad());
//         if (i % 2 == 0) {
//             ASSERT(iter.isLocalValid());
//             assert_quad_eq(r, quad, *iter.localQuad());
//         } else {
//             ASSERT(!iter.isLocalValid());
//             ASSERT(!iter.localQuad());
//         }
//         i++;
//     }
//     ASSERTF(i == kQuadCount, "Expected %d iterations, got: %d", kQuadCount, i);
// }

// static int get_expected_block_count(int entryCount) {
//     int f0 = 0;
//     int f1 = 1;
//     int totalEntries = 0;
//     int blockCount = 0;
//     while(totalEntries < entryCount) {
//         int n = f0 + f1;
//         f0 = f1;
//         f1 = n;
//         totalEntries += n;
//         blockCount++;
//         SkDebugf("n: %d total: %d limit: %d ct: %d\n", n, totalEntries, entryCount, blockCount);
//     }
//     return blockCount;
// }

// // FIXME maybe just pass in the expected block count that is assumed to already incorporate the
// // effects of any partial allocations?
// static int get_block_count(skiatest::Reporter* r, GrQuadBuffer<TestData>* buffer, size_t entrySize,
//                            int expectedBlockCount, int partialAllocationLimit) {
//     int blockCount = 0;
//     int partialBlockAllocations = 0;

//     // Tracks expected entry count per block, following Fibonacci growth sequence of GruadBuffer
//     int f0 = 0;
//     int f1 = 1;
//     // Depending on how the buffer blocks map on to arena blocks, the actual entry count per block
//     // will be less than the expected.
//     int actualEntryCount = 0;

//     TestData* lastMetadataPtr = nullptr;
//     auto iter = buffer->metadata();
//     while(iter.next()) {
//         TestData* entryMetadataPtr = &(*iter);
//         if (lastMetadataPtr) {
//             // The "next" entry could actually located earlier in the arena due to the alternating
//             // appends in phase 2, followed by a concat.
//             intptr_t metadataDiff = reinterpret_cast<intptr_t>(entryMetadataPtr) -
//                                     reinterpret_cast<intptr_t>(lastMetadataPtr);
//             bool compact = ((intptr_t) entrySize == metadataDiff);
//             if (!compact) {
//                 // When it's not compact, we've moved to a new buffer block
//                 blockCount++;

//                 int expectedEntryCount = f0 + f1;
//                 ASSERTF(actualEntryCount > 0 && actualEntryCount <= expectedEntryCount,
//                         "Unexpected block entry count %d, should be between 1 and %d",
//                         actualEntryCount, expectedEntryCount);
//                 if (actualEntryCount < expectedEntryCount) {
//                     partialBlockAllocations++;
//                 }

//                 // The growth sequence stays the same, regardless of partial allocations
//                 f0 = f1;
//                 f1 = expectedEntryCount;
//                 actualEntryCount = 0;
//             }
//         } else {
//             // First entry, which must be inside a block
//             blockCount = 1;
//         }

//         lastMetadataPtr = entryMetadataPtr;
//         actualEntryCount++;
//     }

//     // At this point, actualBlockSize represents the current number of entries in the tail block
//     // of the buffer. Can detect whether or not the tail was partially allocated by querying the
//     // remaining number of bytes from the GrQuadBuffer.
//     size_t expectedDataSize = (f0 + f1) * entrySize;
//     size_t actualDataSize = actualEntryCount * entrySize + buffer->remaining();
//     ASSERTF(actualDataSize >= entrySize && actualDataSize <= expectedDataSize,
//             "Unexpected tail block size %d, should be between %d and %d",
//             actualDataSize, entrySize, expectedDataSize);
//     if (actualDataSize < expectedDataSize) {
//         partialBlockAllocations++;
//     }

//     // Partial block allocations occur when the arena's pages don't align with the growth sequence
//     // of the GrQuadBuffer. It's acceptable to encounter them, particularly in the append/concat
//     // abuse these tests put it through. However, it should still be relatively infrequent
//     // (this upper bound is picked manually to be reasonable given this test case).
//     ASSERTF(partialBlockAllocations <= partialAllocationLimit,
//             "Too many partial block allocations: %d, expected up to %d",
//             partialBlockAllocations, partialAllocationLimit);
//     // int expectedBlockCount = get_expected_block_count(buffer->count());
//     ASSERTF(blockCount == expectedBlockCount,
//             "Expected block count: %d, got %d",
//             expectedBlockCount, blockCount);
//     return blockCount;
// }

// /*static void dumpBufferDetails(const char* name, GrQuadBuffer<TestData>* buffer, int entrySize) {
//     TestData* lastMetadataPtr = nullptr;
//     int jumpCount = 0;
//     int blockSize = 0;

//     SkDebugf("%s\n", name);
//     buffer->dumpInfo();

//     int f0 = 0;
//     int f1 = 1;

//     auto iter = buffer->metadata();
//     while(iter.next()) {
//         TestData* entryMetadataPtr = &(*iter);
//         if (lastMetadataPtr) {
//             // The "next" entry could actually located earlier in the arena due to the alternating
//             // appends in phase 2, followed by a concat.
//             intptr_t metadataDiff = reinterpret_cast<intptr_t>(entryMetadataPtr) -
//                                     reinterpret_cast<intptr_t>(lastMetadataPtr);
//             bool compact = (entrySize == metadataDiff);
//             if (!compact) {
//                 jumpCount++;
//                 SkDebugf("Block size: %d, expected block size: %d, estimated data size: %d\n", blockSize, f0 + f1, blockSize * entrySize);
//                 int n = f0 + f1;
//                 f0 = f1;
//                 f1 = n;
//                 blockSize = 0;
//             }
//         }
//         lastMetadataPtr = entryMetadataPtr;
//         blockSize++;
//     }
//     SkDebugf("Block size: %d, estimated data size: %d, remaining: %d\n", blockSize, blockSize * entrySize, buffer->remaining());
//     SkDebugf("jump count = %d\n", jumpCount);
// }*/

// TEST(CompactAppendConcat) {
//     static const int kEntrySize = sizeof(TestData) + 16 * sizeof(float) + 4;

//     SkSTArenaAlloc<512> arena;

//     GrQuadBuffer<TestData> buffer1;
//     GrQuadBuffer<TestData> buffer2;

//     // Always use an axis-aligned quad for device and local coords so that it's possible to
//     // "know" where the metadata will be placed and detect compact entries and block jumps.
//     GrQuad quad(SkRect::MakeLTRB(1, 2, 3, 4));

//     // Adding quads will be done in 4 phases to stress interference with the arena:
//     // 1. 300 quads added to buffer1.
//     // 2. 300 quads added, alternating between buffer1 and buffer2.
//     // 3. 300 quads added to buffer2, followed by concatenating buffer2 to buffer1
//     // 4. 300 quads added to a new buffer and immediately concatenating that buffer to buffer1
//     for (int i = 0; i < 300; i++) {
//         buffer1.append(&arena, quad, {i, i + 0.5f}, &quad);
//     }
//     int actualBlockCount = get_block_count(r, &buffer1, kEntrySize, get_expected_block_count(300) + 1, 1);
//     SkDebugf("buffer1 @ 300 has %d blocks\n", actualBlockCount);

//     // dumpBufferDetails("buffer1 @ 300", &buffer1, kEntrySize);
//     for (int i = 0; i < 300; i++) {
//         if (i % 2 == 0) {
//             buffer1.append(&arena, quad, {300 + i, i + 300.5f}, &quad);
//         } else {
//             buffer2.append(&arena, quad, {300 + i, i + 300.5f}, &quad);
//         }
//     }
//     actualBlockCount = get_block_count(r, &buffer1, kEntrySize, get_expected_block_count(450) + 2, 3);
//     SkDebugf("buffer1 @ 450 has %d blocks\n", actualBlockCount);
//     actualBlockCount = get_block_count(r, &buffer2, kEntrySize, get_expected_block_count(150), 0);
//     SkDebugf("buffer2 @ 150 has %d blocks\n", actualBlockCount);
//     // dumpBufferDetails("buffer1 @ 450", &buffer1, kEntrySize);
//     // dumpBufferDetails("buffer2 @ 150", &buffer2, kEntrySize);
//     for (int i = 0; i < 300; i++) {
//         buffer2.append(&arena, quad, {600 + i, i + 600.5f}, &quad);
//     }
//     actualBlockCount = get_block_count(r, &buffer2, kEntrySize, get_expected_block_count(450), 1);
//     SkDebugf("buffer2 @ 450 has %d blocks\n", actualBlockCount);

//     ASSERTF(buffer1.count() == 450, "Expected 450 entries in buffer1, got: %d", buffer1.count());
//     ASSERTF(buffer2.count() == 450, "Expected 450 entries in buffer2, got: %d", buffer2.count());
//     buffer1.concat(&arena, &buffer2);
//     ASSERTF(buffer1.count() == 900, "Expected 900 entries after concat, got: %d", buffer1.count());
//     // dumpBufferDetails("buffer1 @ 900", &buffer1, kEntrySize);
//     // FIXME to actually do this, should get the # remaining in buffer1 and subtract off blocks
//     // from b2 until filled, then sum remaining b2's count to b1's count.
//     // FIXME currently linked blocks from b2 get detected as partial allocations even though they
//     // aren't really. Maybe we don't bother tracking them at all? Perhaps we change the partial
//     // allocation assert to exact equality (although it is dependent on arena initialization size).
//     // And then, we track buffer1's expectations for 300 and 450, then update it after the concat
//     // by adding the linked block count.
//     // FIXME worth considering if get_expected_block_count(900) matches what we'd get by analyzing
//     // the remaining number of bytes. If so, perhaps we can just remove the test-only remainder function
//     SkDebugf("expected(900) = %d, 1200 = %d\n", get_expected_block_count(900), get_expected_block_count(1200));
//     actualBlockCount = get_block_count(r, &buffer1, kEntrySize, 16, 6);
//     SkDebugf("buffer1 @ 900 has %d blocks\n", actualBlockCount);
//     for (int i = 0; i < 300; i++) {
//         GrQuadBuffer<TestData> buffer3;
//         buffer3.append(&arena, quad, {900 + i, i + 900.5f}, &quad);
//         buffer1.concat(&arena, &buffer3);
//     }
//     actualBlockCount = get_block_count(r, &buffer1, kEntrySize, 17, 7);
//     SkDebugf("buffer1 @ 1200 has %d blocks\n", actualBlockCount);
//     // dumpBufferDetails("buffer1 @ 1200", &buffer1, kEntrySize);

//     // Here the test gets a little tricky because GrQuadBuffer doesn't expose its allocation
//     // details and it is a blocked linked list on top of an arena allocator. This means compaction
//     // and block sizes are tied to arena allocation pattern. Because every append to buffer1 and
//     // buffer2 used the same 2D quad for device and local coords, every entry will be
//     // sizeof(TestData) + 48 (4 sets of 4 floats) + 4 byte header. The MetadataIter provides access
//     // to the actual address of the TestData embedded in the underlying arena, so we can use that
//     // to detect when consecutive logical entries are packed by the difference between their
//     // metadata pointers.
//     // TestData* lastMetadataPtr = nullptr;
//     // int jumpCount = 0;
//     // int blockSize = 0;
//     // buffer1.dumpInfo();

//     // auto iter = buffer1.metadata();
//     // while(iter.next()) {
//     //     TestData* entryMetadataPtr = &(*iter);
//     //     if (lastMetadataPtr) {
//     //         // The "next" entry could actually located earlier in the arena due to the alternating
//     //         // appends in phase 2, followed by a concat.
//     //         intptr_t metadataDiff = reinterpret_cast<intptr_t>(entryMetadataPtr) -
//     //                                 reinterpret_cast<intptr_t>(lastMetadataPtr);
//     //         bool compact = (kEntrySize == metadataDiff);
//     //         if (!compact) {
//     //             jumpCount++;
//     //             SkDebugf("Block size: %d, estimated data size: %d\n", blockSize, blockSize * kEntrySize);
//     //             blockSize = 0;
//     //         }
//     //     }
//     //     lastMetadataPtr = entryMetadataPtr;
//     //     blockSize++;
//     // }
//     // SkDebugf("Block size: %d, estimated data size: %d\n", blockSize, blockSize * kEntrySize);
//     // SkDebugf("jump count = %d\n", jumpCount);
// }
