/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkRefDict.h"

class TestRC : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(TestRC)
private:
    typedef SkRefCnt INHERITED;
};

DEF_TEST(RefDict, reporter) {
    TestRC    data0, data1;
    SkRefDict dict;

    REPORTER_ASSERT(reporter, NULL == dict.find(NULL));
    REPORTER_ASSERT(reporter, NULL == dict.find("foo"));
    REPORTER_ASSERT(reporter, NULL == dict.find("bar"));

    dict.set("foo", &data0);
    REPORTER_ASSERT(reporter, &data0 == dict.find("foo"));
    REPORTER_ASSERT(reporter, 2 == data0.getRefCnt());

    dict.set("foo", &data0);
    REPORTER_ASSERT(reporter, &data0 == dict.find("foo"));
    REPORTER_ASSERT(reporter, 2 == data0.getRefCnt());

    dict.set("foo", &data1);
    REPORTER_ASSERT(reporter, &data1 == dict.find("foo"));
    REPORTER_ASSERT(reporter, 1 == data0.getRefCnt());
    REPORTER_ASSERT(reporter, 2 == data1.getRefCnt());

    dict.set("foo", NULL);
    REPORTER_ASSERT(reporter, NULL == dict.find("foo"));
    REPORTER_ASSERT(reporter, 1 == data0.getRefCnt());
    REPORTER_ASSERT(reporter, 1 == data1.getRefCnt());

    dict.set("foo", &data0);
    dict.set("bar", &data1);
    REPORTER_ASSERT(reporter, &data0 == dict.find("foo"));
    REPORTER_ASSERT(reporter, &data1 == dict.find("bar"));
    REPORTER_ASSERT(reporter, 2 == data0.getRefCnt());
    REPORTER_ASSERT(reporter, 2 == data1.getRefCnt());

    dict.set("foo", &data1);
    REPORTER_ASSERT(reporter, &data1 == dict.find("foo"));
    REPORTER_ASSERT(reporter, &data1 == dict.find("bar"));
    REPORTER_ASSERT(reporter, 1 == data0.getRefCnt());
    REPORTER_ASSERT(reporter, 3 == data1.getRefCnt());

    dict.removeAll();
    REPORTER_ASSERT(reporter, NULL == dict.find("foo"));
    REPORTER_ASSERT(reporter, NULL == dict.find("bar"));
    REPORTER_ASSERT(reporter, 1 == data0.getRefCnt());
    REPORTER_ASSERT(reporter, 1 == data1.getRefCnt());

    {
        SkRefDict d;
        REPORTER_ASSERT(reporter, NULL == d.find("foo"));
        REPORTER_ASSERT(reporter, 1 == data0.getRefCnt());
        d.set("foo", &data0);
        REPORTER_ASSERT(reporter, &data0 == d.find("foo"));
        REPORTER_ASSERT(reporter, 2 == data0.getRefCnt());
        // let d go out of scope still with a ref on data0
    }
    // be sure d's destructor lowered data0's owner count back to 1
    REPORTER_ASSERT(reporter, 1 == data0.getRefCnt());
}
