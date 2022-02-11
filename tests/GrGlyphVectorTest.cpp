/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
 */

#include "src/core/SkGlyph.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/text/GrGlyphVector.h"

#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkWriteBuffer.h"
#include "src/gpu/GrSubRunAllocator.h"
#include "tests/Test.h"

class TestingPeer {
public:
    static const SkDescriptor& GetDescriptor(const GrGlyphVector& v) {
        return v.fStrike->getDescriptor();
    }
    static SkSpan<GrGlyphVector::Variant> GetGlyphs(const GrGlyphVector& v) {
        return v.fGlyphs;
    }
};

DEF_TEST(GrGlyphVector_Serialization, r) {
    SkFont font;
    auto [strikeSpec, _] = SkStrikeSpec::MakeCanonicalized(font);

    GrSubRunAllocator alloc;

    SkBulkGlyphMetricsAndImages glyphFinder{strikeSpec};
    const int N = 10;
    SkGlyphVariant* glyphs = alloc.makePODArray<SkGlyphVariant>(N);
    for (int i = 0; i < N; i++) {
        glyphs[i] = glyphFinder.glyph(SkPackedGlyphID(SkTo<SkGlyphID>(i + 1)));
    }

    GrGlyphVector src = GrGlyphVector::Make(
            strikeSpec.findOrCreateStrike(), SkMakeSpan(glyphs, N), &alloc);

    SkBinaryWriteBuffer wBuffer;
    src.flatten(wBuffer);

    auto data = wBuffer.snapshotAsData();
    SkReadBuffer rBuffer{data->data(), data->size()};
    auto dst = GrGlyphVector::MakeFromBuffer(rBuffer, &alloc);
    REPORTER_ASSERT(r, dst.has_value());
    REPORTER_ASSERT(r, TestingPeer::GetDescriptor(src) == TestingPeer::GetDescriptor(*dst));

    auto srcGlyphs = TestingPeer::GetGlyphs(src);
    auto dstGlyphs = TestingPeer::GetGlyphs(*dst);
    for (auto [srcGlyphID, dstGlyphID] : SkMakeZip(srcGlyphs, dstGlyphs)) {
        REPORTER_ASSERT(r, srcGlyphID.packedGlyphID == dstGlyphID.packedGlyphID);
    }
}

DEF_TEST(GrGlyphVector_BadLengths, r) {
    {
        SkFont font;
        auto [strikeSpec, _] = SkStrikeSpec::MakeCanonicalized(font);

        // Make broken stream by hand - zero length
        SkBinaryWriteBuffer wBuffer;
        strikeSpec.descriptor().flatten(wBuffer);
        wBuffer.write32(0);  // length
        auto data = wBuffer.snapshotAsData();
        SkReadBuffer rBuffer{data->data(), data->size()};
        GrSubRunAllocator alloc;
        auto dst = GrGlyphVector::MakeFromBuffer(rBuffer, &alloc);
        REPORTER_ASSERT(r, !dst.has_value());
    }

    {
        SkFont font;
        auto [strikeSpec, _] = SkStrikeSpec::MakeCanonicalized(font);

        // Make broken stream by hand - stream is too short
        SkBinaryWriteBuffer wBuffer;
        strikeSpec.descriptor().flatten(wBuffer);
        wBuffer.write32(5);  // length
        wBuffer.writeUInt(12);  // random data
        wBuffer.writeUInt(12);  // random data
        wBuffer.writeUInt(12);  // random data
        auto data = wBuffer.snapshotAsData();
        SkReadBuffer rBuffer{data->data(), data->size()};
        GrSubRunAllocator alloc;
        auto dst = GrGlyphVector::MakeFromBuffer(rBuffer, &alloc);
        REPORTER_ASSERT(r, !dst.has_value());
    }

    {
        SkFont font;
        auto [strikeSpec, _] = SkStrikeSpec::MakeCanonicalized(font);

        // Make broken stream by hand - length out of range of safe calculations
        SkBinaryWriteBuffer wBuffer;
        strikeSpec.descriptor().flatten(wBuffer);
        wBuffer.write32(INT_MAX - 10);  // length
        wBuffer.writeUInt(12);  // random data
        wBuffer.writeUInt(12);  // random data
        wBuffer.writeUInt(12);  // random data
        auto data = wBuffer.snapshotAsData();
        SkReadBuffer rBuffer{data->data(), data->size()};
        GrSubRunAllocator alloc;
        auto dst = GrGlyphVector::MakeFromBuffer(rBuffer, &alloc);
        REPORTER_ASSERT(r, !dst.has_value());
    }
}
