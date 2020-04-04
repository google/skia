/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkScalerContext.h"
#include "tests/Test.h"

#include <memory>

class SkDescriptorTestHelper {
public:
    static void SetLength(SkDescriptor* desc, size_t length) { desc->fLength = length; }
    static void SetCount(SkDescriptor* desc, uint32_t count) { desc->fCount = count; }
};

DEF_TEST(Descriptor_empty, r) {
    const size_t size = sizeof(SkDescriptor);

    auto desc = SkDescriptor::Alloc(size);
    desc->init();
    REPORTER_ASSERT(r, desc->isValid());
    REPORTER_ASSERT(r, desc->getLength() == size);
}

DEF_TEST(Descriptor_valid_simple, r) {
    const size_t size =
            sizeof(SkDescriptor) + sizeof(SkDescriptor::Entry) + sizeof(SkScalerContextRec);

    auto desc = SkDescriptor::Alloc(size);
    desc->init();
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
    desc->init();
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
    desc->init();
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
    desc->init();
    SkScalerContextRec rec;
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec) - 4, &rec);
    REPORTER_ASSERT(r, desc->getLength() == size);
    REPORTER_ASSERT(r, !desc->isValid());
}

DEF_TEST(Descriptor_invalid_length, r) {
    const size_t size = sizeof(SkDescriptor) + sizeof(SkDescriptor::Entry);
    const size_t effect_size = 1000;

    auto desc = SkDescriptor::Alloc(size);
    desc->init();
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
    desc->init();

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
    desc->init();

    // Make the start of the Entry be in the SkDescriptor, but the second half falls out side the
    // SkDescriptor. So: 12 (for descriptor) + 8 (for entry) + 12 (for entry length) = 32. An
    // An Entry is 8 bytes, so 4 bytes are < 36 and 4 bytes > 36.
    desc->addEntry(kEffects_SkDescriptorTag, 12, nullptr);

    SkDescriptorTestHelper::SetLength(desc.get(), 36);
    SkDescriptorTestHelper::SetCount(desc.get(), 2);
    REPORTER_ASSERT(r, !desc->isValid());
}
