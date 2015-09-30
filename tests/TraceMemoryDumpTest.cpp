/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTraceMemoryDump.h"

#include "Test.h"

/*
 * Build test for SkTraceMemoryDump.
 */
class TestSkTraceMemoryDump : public SkTraceMemoryDump {
public:
    TestSkTraceMemoryDump() { }
    ~TestSkTraceMemoryDump() override { }

    void dumpNumericValue(const char* dumpName, const char* valueName, const char* units,
                          uint64_t value) override { }
    void setMemoryBacking(const char* dumpName, const char* backingType,
                          const char* backingObjectId) override { }
    void setDiscardableMemoryBacking(
        const char* dumpName,
        const SkDiscardableMemory& discardableMemoryObject) override { }
    LevelOfDetail getRequestedDetails() const override {
        return SkTraceMemoryDump::kObjectsBreakdowns_LevelOfDetail;
    }
};

DEF_TEST(SkTraceMemoryDump, reporter) {
    TestSkTraceMemoryDump x;
    x.dumpNumericValue("foobar", "size", "bytes", 42);
    if (x.getRequestedDetails() == SkTraceMemoryDump::kObjectsBreakdowns_LevelOfDetail) {
        x.dumpNumericValue("foobar/object1", "size", "bytes", 23);
    }
}
