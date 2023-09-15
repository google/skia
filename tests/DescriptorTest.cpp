/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

class SkDescriptorTestHelper {
public:
    static void SetLength(SkDescriptor* desc, size_t length) { desc->fLength = length; }
    static void SetCount(SkDescriptor* desc, uint32_t count) { desc->fCount = count; }
};

DEF_TEST(Descriptor_empty, r) {
    const size_t size = sizeof(SkDescriptor);

    auto desc = SkDescriptor::Alloc(size);
    REPORTER_ASSERT(r, desc->isValid());
    REPORTER_ASSERT(r, desc->getLength() == size);
}

DEF_TEST(Descriptor_valid_simple, r) {
    const size_t size =
            sizeof(SkDescriptor) + sizeof(SkDescriptor::Entry) + sizeof(SkScalerContextRec);

    auto desc = SkDescriptor::Alloc(size);
    SkScalerContextRec rec;
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
    REPORTER_ASSERT(r, desc->isValid());
    REPORTER_ASSERT(r, desc->getLength() == size);

    SkDescriptorTestHelper::SetLength(desc.get(), size - 4);
    REPORTER_ASSERT(r, !desc->isValid());
}

DEF_TEST(Descriptor_valid_simple_extra_space, r) {
    const size_t extra_space = 100;
    const size_t size =
            sizeof(SkDescriptor) + sizeof(SkDescriptor::Entry) + sizeof(SkScalerContextRec);

    auto desc = SkDescriptor::Alloc(size + extra_space);
    SkScalerContextRec rec;
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
    REPORTER_ASSERT(r, desc->isValid());
    REPORTER_ASSERT(r, desc->getLength() == size);

    SkDescriptorTestHelper::SetLength(desc.get(), size - 4);
    REPORTER_ASSERT(r, !desc->isValid());
}

DEF_TEST(Descriptor_valid_more_tags, r) {
    const size_t effectSize = 16;
    const size_t testSize = 32;
    const size_t size = sizeof(SkDescriptor) + 3 * sizeof(SkDescriptor::Entry) +
                        sizeof(SkScalerContextRec) + effectSize + testSize;

    auto desc = SkDescriptor::Alloc(size);
    SkScalerContextRec rec;
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
    desc->addEntry(kEffects_SkDescriptorTag, effectSize, nullptr);
    desc->addEntry(SkSetFourByteTag('t', 'e', 's', 't'), testSize, nullptr);
    REPORTER_ASSERT(r, desc->isValid());
    REPORTER_ASSERT(r, desc->getLength() == size);

    SkDescriptorTestHelper::SetLength(desc.get(), size - 4);
    REPORTER_ASSERT(r, !desc->isValid());
}

DEF_TEST(Descriptor_invalid_rec_size, r) {
    const size_t size =
            sizeof(SkDescriptor) + sizeof(SkDescriptor::Entry) + sizeof(SkScalerContextRec) - 4;

    auto desc = SkDescriptor::Alloc(size);
    SkScalerContextRec rec;
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec) - 4, &rec);
    REPORTER_ASSERT(r, desc->getLength() == size);
    REPORTER_ASSERT(r, !desc->isValid());
}

DEF_TEST(Descriptor_invalid_length, r) {
    const size_t size = sizeof(SkDescriptor) + sizeof(SkDescriptor::Entry);
    const size_t effect_size = 1000;

    auto desc = SkDescriptor::Alloc(size);
    desc->addEntry(kEffects_SkDescriptorTag, effect_size, nullptr);

    SkDescriptorTestHelper::SetLength(desc.get(), size);
    REPORTER_ASSERT(r, !desc->isValid());

    SkDescriptorTestHelper::SetLength(desc.get(), size + effect_size);
    REPORTER_ASSERT(r, desc->isValid());
}

DEF_TEST(Descriptor_entry_too_big, r) {
    const size_t size = sizeof(SkDescriptor) + sizeof(SkDescriptor::Entry) + 4;
    // Must be less than fLength, but big enough to be bigger then fLength when added.
    const size_t effect_size = sizeof(SkDescriptor) + sizeof(SkDescriptor::Entry);

    auto desc = SkDescriptor::Alloc(size);

    desc->addEntry(kEffects_SkDescriptorTag, effect_size, nullptr);

    SkDescriptorTestHelper::SetLength(desc.get(), size);
    SkDescriptorTestHelper::SetCount(desc.get(), 2);
    REPORTER_ASSERT(r, !desc->isValid());

    SkDescriptorTestHelper::SetLength(desc.get(), size);
    SkDescriptorTestHelper::SetCount(desc.get(), 1);
    REPORTER_ASSERT(r, !desc->isValid());
}

DEF_TEST(Descriptor_entry_over_end, r) {
    auto desc = SkDescriptor::Alloc(36);

    // Make the start of the Entry be in the SkDescriptor, but the second half falls out side the
    // SkDescriptor. So: 12 (for descriptor) + 8 (for entry) + 12 (for entry length) = 32. An
    // An Entry is 8 bytes, so 4 bytes are < 36 and 4 bytes > 36.
    desc->addEntry(kEffects_SkDescriptorTag, 12, nullptr);

    SkDescriptorTestHelper::SetLength(desc.get(), 36);
    SkDescriptorTestHelper::SetCount(desc.get(), 2);
    REPORTER_ASSERT(r, !desc->isValid());
}

DEF_TEST(Descriptor_flatten_unflatten, r) {
    {
        SkBinaryWriteBuffer writer({});
        auto desc = SkDescriptor::Alloc(sizeof(SkDescriptor));
        desc->computeChecksum();
        desc->flatten(writer);
        auto data = writer.snapshotAsData();
        SkReadBuffer reader{data->data(), data->size()};
        auto ad = SkAutoDescriptor::MakeFromBuffer(reader);
        REPORTER_ASSERT(r, ad.has_value());
        REPORTER_ASSERT(r, ad->getDesc()->isValid());
    }

    {  // broken header
        SkBinaryWriteBuffer writer({});
        writer.writeInt(0);  // fChecksum
        auto data = writer.snapshotAsData();
        SkReadBuffer reader{data->data(), data->size()};
        auto ad = SkAutoDescriptor::MakeFromBuffer(reader);
        REPORTER_ASSERT(r, !ad.has_value());
    }

    {  // length too big
        SkBinaryWriteBuffer writer({});
        // Simulate a broken header
        writer.writeInt(0);    // fChecksum
        writer.writeInt(4000); // fLength
        writer.writeInt(0);    // fCount
        auto data = writer.snapshotAsData();
        SkReadBuffer reader{data->data(), data->size()};
        auto ad = SkAutoDescriptor::MakeFromBuffer(reader);
        REPORTER_ASSERT(r, !ad.has_value());
    }

    {  // length too small
        SkBinaryWriteBuffer writer({});
        // Simulate a broken header
        writer.writeInt(0);    // fChecksum
        writer.writeInt(3);    // fLength
        writer.writeInt(0);    // fCount
        auto data = writer.snapshotAsData();
        SkReadBuffer reader{data->data(), data->size()};
        auto ad = SkAutoDescriptor::MakeFromBuffer(reader);
        REPORTER_ASSERT(r, !ad.has_value());
    }

    {  // garbage in count
        SkBinaryWriteBuffer writer({});
        // Simulate a broken header
        writer.writeInt(0);    // fChecksum
        writer.writeInt(20);   // fLength
        writer.writeInt(10);   // fCount
        writer.writeInt(0);
        writer.writeInt(0);
        auto data = writer.snapshotAsData();
        SkReadBuffer reader{data->data(), data->size()};
        auto ad = SkAutoDescriptor::MakeFromBuffer(reader);
        REPORTER_ASSERT(r, !ad.has_value());
    }
}
