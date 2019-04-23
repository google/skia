/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrix.h"
#include "SkRandom.h"
#include "SkString.h"
#include "Test.h"

#include "GrTRecorder.h"

////////////////////////////////////////////////////////////////////////////////

static int gActiveRecorderItems = 0;

class IntWrapper {
public:
    IntWrapper() {}
    IntWrapper(int value) : fValue(value) {}
    operator int() { return fValue; }
private:
    int fValue;
};

struct ExtraData {
    typedef GrTRecorder<ExtraData> Recorder;

    ExtraData(int i) : fData(i) {
        int* extraData = this->extraData();
        for (int j = 0; j < i; j++) {
            extraData[j] = i;
        }
        ++gActiveRecorderItems;
    }
    ~ExtraData() { --gActiveRecorderItems; }
    int* extraData() { return reinterpret_cast<int*>(this + 1); }
    int fData;
};

static void test_extra_data(skiatest::Reporter* reporter) {
    ExtraData::Recorder recorder(0);
    REPORTER_ASSERT(reporter, recorder.empty());
    for (int i = 0; i < 100; ++i) {
        recorder.emplaceWithData<ExtraData>(i * sizeof(int), i);
        REPORTER_ASSERT(reporter, !recorder.empty());
    }
    REPORTER_ASSERT(reporter, 100 == gActiveRecorderItems);

    auto iter = recorder.begin();
    for (int i = 0; i < 100; ++i, ++iter) {
        REPORTER_ASSERT(reporter, i == iter->fData);
        for (int j = 0; j < i; j++) {
            REPORTER_ASSERT(reporter, i == iter->extraData()[j]);
        }
    }
    REPORTER_ASSERT(reporter, iter == recorder.end());

    recorder.reset();
    REPORTER_ASSERT(reporter, 0 == gActiveRecorderItems);
    REPORTER_ASSERT(reporter, recorder.begin() == recorder.end());
    REPORTER_ASSERT(reporter, recorder.empty());
}

enum ClassType {
    kBase_ClassType,
    kSubclass_ClassType,
    kSubSubclass_ClassType,
    kSubclassExtraData_ClassType,
    kSubclassEmpty_ClassType,

    kNumClassTypes
};

class Base {
public:
    typedef GrTRecorder<Base> Recorder;

    Base() {
        fMatrix.reset();
        ++gActiveRecorderItems;
    }

    virtual ~Base() { --gActiveRecorderItems; }

    virtual ClassType getType() { return kBase_ClassType; }

    virtual void validate(skiatest::Reporter* reporter) const {
        REPORTER_ASSERT(reporter, fMatrix.isIdentity());
    }

private:
    SkMatrix fMatrix;
};

class Subclass : public Base {
public:
    Subclass() : fString("Lorem ipsum dolor sit amet") {}

    virtual ClassType getType() { return kSubclass_ClassType; }

    virtual void validate(skiatest::Reporter* reporter) const {
        Base::validate(reporter);
        REPORTER_ASSERT(reporter, !strcmp("Lorem ipsum dolor sit amet", fString.c_str()));
    }

private:
    SkString fString;
};

class SubSubclass : public Subclass {
public:
    SubSubclass() : fInt(1234), fFloat(1.234f) {}

    virtual ClassType getType() { return kSubSubclass_ClassType; }

    virtual void validate(skiatest::Reporter* reporter) const {
        Subclass::validate(reporter);
        REPORTER_ASSERT(reporter, 1234 == fInt);
        REPORTER_ASSERT(reporter, 1.234f == fFloat);
    }

private:
    int fInt;
    float fFloat;
};

class SubclassExtraData : public Base {
public:
    SubclassExtraData(int length) : fLength(length) {
        int* data = reinterpret_cast<int*>(this + 1);
        for (int i = 0; i < fLength; ++i) {
            data[i] = ValueAt(i);
        }
    }

    virtual ClassType getType() { return kSubclassExtraData_ClassType; }

    virtual void validate(skiatest::Reporter* reporter) const {
        Base::validate(reporter);
        const int* data = reinterpret_cast<const int*>(this + 1);
        for (int i = 0; i < fLength; ++i) {
            REPORTER_ASSERT(reporter, ValueAt(i) == data[i]);
        }
    }

private:
    static int ValueAt(uint64_t i) {
        return static_cast<int>((123456789 + 987654321 * i) & 0xFFFFFFFF);
    }
    int fLength;
};

class SubclassEmpty : public Base {
public:
    virtual ClassType getType() { return kSubclassEmpty_ClassType; }
};

class Order {
public:
    Order() { this->reset(); }
    void reset() { fCurrent = 0; }
    ClassType next() {
        fCurrent = 1664525 * fCurrent + 1013904223;
        return static_cast<ClassType>(fCurrent % kNumClassTypes);
    }
private:
    uint32_t fCurrent;
};

static void test_subclasses_iter(skiatest::Reporter*, Order&, Base::Recorder::iterator&, int = 0);

static void test_subclasses(skiatest::Reporter* reporter) {
    Base::Recorder recorder(1024);

    Order order;
    for (int i = 0; i < 1000; i++) {
        switch (order.next()) {
            case kBase_ClassType:
                recorder.emplace<Base>();
                break;

            case kSubclass_ClassType:
                recorder.emplace<Subclass>();
                break;

            case kSubSubclass_ClassType:
                recorder.emplace<SubSubclass>();
                break;

            case kSubclassExtraData_ClassType:
                recorder.emplaceWithData<SubclassExtraData>(sizeof(int) * i, i);
                break;

            case kSubclassEmpty_ClassType:
                recorder.emplace<SubclassEmpty>();
                break;

            default:
                ERRORF(reporter, "Invalid class type");
                break;
        }
    }
    REPORTER_ASSERT(reporter, 1000 == gActiveRecorderItems);

    order.reset();
    auto iter = recorder.begin();

    test_subclasses_iter(reporter, order, iter);

    REPORTER_ASSERT(reporter, iter == recorder.end());
    // Don't reset the recorder. It should automatically destruct all its items.
}
static void test_subclasses_iter(skiatest::Reporter* reporter, Order& order,
                                 Base::Recorder::iterator& iter, int i) {
    if (i >= 1000) {
        return;
    }

    ClassType classType = order.next();

    REPORTER_ASSERT(reporter, classType == iter->getType());
    iter->validate(reporter);

    ++iter;
    test_subclasses_iter(reporter, order, iter, i + 1);
}

struct AlignBase {
    AlignBase() { ++gActiveRecorderItems; }
    ~AlignBase() { --gActiveRecorderItems; }
    char fValue;
};
struct alignas(16) Align16 : public AlignBase {};
struct alignas(32) Align32 : public AlignBase {};
struct alignas(64) Align64 : public AlignBase {};
struct alignas(128) Align128 : public AlignBase {};

static void test_alignment(skiatest::Reporter* reporter) {
    GrTRecorder<AlignBase> recorder(0);
    SkTArray<size_t> expectedAlignments;
    SkRandom random;
    for (int i = 0; i < 100; ++i) {
        size_t dataSize = random.nextULessThan(20);
        switch (random.nextULessThan(5)) {
            case 0:
                recorder.emplaceWithData<AlignBase>(dataSize);
                expectedAlignments.push_back(alignof(AlignBase));
                break;
            case 1:
                recorder.emplaceWithData<Align16>(dataSize);
                expectedAlignments.push_back(16);
                break;
            case 2:
                recorder.emplaceWithData<Align32>(dataSize);
                expectedAlignments.push_back(32);
                break;
            case 3:
                recorder.emplaceWithData<Align64>(dataSize);
                expectedAlignments.push_back(64);
                break;
            case 4:
                recorder.emplaceWithData<Align128>(dataSize);
                expectedAlignments.push_back(128);
                break;
        }
        recorder.back().fValue = i;
    }
    int i = 0;
    for (const auto& x : recorder) {
        REPORTER_ASSERT(reporter, x.fValue == i);
        auto pointer = reinterpret_cast<uintptr_t>(&x);
        auto mask = static_cast<uintptr_t>(expectedAlignments[i]) - 1;
        REPORTER_ASSERT(reporter, !(pointer & mask));
        i++;
    }
    REPORTER_ASSERT(reporter, i == 100);
}

DEF_GPUTEST(GrTRecorder, reporter, /* options */) {
    test_extra_data(reporter);
    REPORTER_ASSERT(reporter, 0 == gActiveRecorderItems);  // test_extra_data should call reset().

    test_subclasses(reporter);
    REPORTER_ASSERT(reporter, 0 == gActiveRecorderItems);  // Ensure ~GrTRecorder invokes dtors.

    test_alignment(reporter);
    REPORTER_ASSERT(reporter, 0 == gActiveRecorderItems);  // Ensure ~GrTRecorder invokes dtors.
}
