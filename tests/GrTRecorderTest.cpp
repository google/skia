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

static int activeRecorderItems = 0;

class IntWrapper {
public:
    IntWrapper() {}
    IntWrapper(int value) : fValue(value) {}
    operator int() { return fValue; }
private:
    int fValue;
};

static void test_empty_back_and_pop(skiatest::Reporter* reporter) {
    SkRandom rand;
    for (int data = 0; data < 2; ++data) {
        // Do this with different starting sizes to have different alignment between blocks and pops.
        // pops. We want to test poping the first guy off, guys in the middle of the block, and the
        // first guy on a non-head block.
        for (int j = 0; j < 8; ++j) {
            GrTRecorder<IntWrapper> recorder(j);

            REPORTER_ASSERT(reporter, recorder.empty());

            for (int i = 0; i < 100; ++i) {
                if (data) {
                    REPORTER_ASSERT(reporter, i == recorder.emplaceWithData<IntWrapper>(
                                                           rand.nextULessThan(10), i));
                } else {
                    REPORTER_ASSERT(reporter, i == recorder.emplace<IntWrapper>(i));
                }
                REPORTER_ASSERT(reporter, !recorder.empty());
                REPORTER_ASSERT(reporter, i == recorder.back());
                if (i != recorder.back()) {
                    SkDebugf("stuff");
                }
                if (0 == (i % 7)) {
                    recorder.pop_back();
                    if (i > 0) {
                        REPORTER_ASSERT(reporter, !recorder.empty());
                        REPORTER_ASSERT(reporter, i-1 == recorder.back());
                    }
                }
            }
            REPORTER_ASSERT(reporter, !recorder.empty());
            recorder.reset();
            REPORTER_ASSERT(reporter, recorder.empty());
        }
    }
}

struct ExtraData {
    typedef GrTRecorder<ExtraData> Recorder;

    ExtraData(int i) : fData(i) {
        int* extraData = this->extraData();
        for (int j = 0; j < i; j++) {
            extraData[j] = i;
        }
        ++activeRecorderItems;
    }
    ~ExtraData() {
        --activeRecorderItems;
    }
    int* extraData() { return reinterpret_cast<int*>(this + 1); }
    int fData;
};

static void test_extra_data(skiatest::Reporter* reporter) {
    ExtraData::Recorder recorder(0);
    for (int i = 0; i < 100; ++i) {
        recorder.emplaceWithData<ExtraData>(i * sizeof(int), i);
    }
    REPORTER_ASSERT(reporter, 100 == activeRecorderItems);

    auto iter = recorder.begin();
    for (int i = 0; i < 100; ++i, ++iter) {
        REPORTER_ASSERT(reporter, i == iter->fData);
        for (int j = 0; j < i; j++) {
            REPORTER_ASSERT(reporter, i == iter->extraData()[j]);
        }
    }
    REPORTER_ASSERT(reporter, iter == recorder.end());

    auto rIter = recorder.rbegin();
    for (int i = 99; i >= 0; --i, ++rIter) {
        REPORTER_ASSERT(reporter, i == rIter->fData);
        for (int j = 0; j < i; j++) {
            REPORTER_ASSERT(reporter, i == rIter->extraData()[j]);
        }
    }
    REPORTER_ASSERT(reporter, rIter == recorder.rend());

    recorder.reset();
    REPORTER_ASSERT(reporter, 0 == activeRecorderItems);
    REPORTER_ASSERT(reporter, recorder.begin() == recorder.end());
    REPORTER_ASSERT(reporter, recorder.rbegin() == recorder.rend());
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
        ++activeRecorderItems;
    }

    virtual ~Base() { --activeRecorderItems; }

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
static void test_subclasses_iters(skiatest::Reporter*, Order&, Base::Recorder::Iter&,
                                  Base::Recorder::RIter&, int = 0);
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
    REPORTER_ASSERT(reporter, 1000 == activeRecorderItems);

    order.reset();
    auto iter = recorder.begin();
    auto rIter = recorder.rbegin();

    test_subclasses_iters(reporter, order, iter, rIter);

    REPORTER_ASSERT(reporter, iter == recorder.end());
    REPORTER_ASSERT(reporter, rIter == recorder.rend());

    // Don't reset the recorder. It should automatically destruct all its items.
}
static void test_subclasses_iters(skiatest::Reporter* reporter, Order& order,
                                  Base::Recorder::Iter& iter,
                                  Base::Recorder::RIter& rIter, int i) {
    if (i >= 1000) {
        return;
    }

    ClassType classType = order.next();

    REPORTER_ASSERT(reporter, classType == iter->getType());
    iter->validate(reporter);

    ++iter;
    test_subclasses_iters(reporter, order, iter, rIter, i + 1);

    REPORTER_ASSERT(reporter, classType == rIter->getType());
    rIter->validate(reporter);
    rIter++;
}

DEF_GPUTEST(GrTRecorder, reporter, /* options */) {
    test_empty_back_and_pop(reporter);

    test_extra_data(reporter);
    REPORTER_ASSERT(reporter, 0 == activeRecorderItems); // test_extra_data should call reset().

    test_subclasses(reporter);
    REPORTER_ASSERT(reporter, 0 == activeRecorderItems); // Ensure ~GrTRecorder invokes dtors.
}
