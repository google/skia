/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRefDict.h"
#include "Test.h"

class TestRC : public SkRefCnt {
public:

private:
    typedef SkRefCnt INHERITED;
};

DEF_TEST(RefDict, reporter) {
    TestRC    data0, data1;
    SkRefDict dict;

    REPORTER_ASSERT(reporter, nullptr == dict.find(nullptr));
    REPORTER_ASSERT(reporter, nullptr == dict.find("foo"));
    REPORTER_ASSERT(reporter, nullptr == dict.find("bar"));

    dict.set("foo", &data0);
    REPORTER_ASSERT(reporter, &data0 == dict.find("foo"));
    REPORTER_ASSERT(reporter, !data0.unique());

    dict.set("foo", &data0);
    REPORTER_ASSERT(reporter, &data0 == dict.find("foo"));
    REPORTER_ASSERT(reporter, !data0.unique());

    dict.set("foo", &data1);
    REPORTER_ASSERT(reporter, &data1 == dict.find("foo"));
    REPORTER_ASSERT(reporter, data0.unique());
    REPORTER_ASSERT(reporter, !data1.unique());

    dict.set("foo", nullptr);
    REPORTER_ASSERT(reporter, nullptr == dict.find("foo"));
    REPORTER_ASSERT(reporter, data0.unique());
    REPORTER_ASSERT(reporter, data1.unique());

    dict.set("foo", &data0);
    dict.set("bar", &data1);
    REPORTER_ASSERT(reporter, &data0 == dict.find("foo"));
    REPORTER_ASSERT(reporter, &data1 == dict.find("bar"));
    REPORTER_ASSERT(reporter, !data0.unique());
    REPORTER_ASSERT(reporter, !data1.unique());

    dict.set("foo", &data1);
    REPORTER_ASSERT(reporter, &data1 == dict.find("foo"));
    REPORTER_ASSERT(reporter, &data1 == dict.find("bar"));
    REPORTER_ASSERT(reporter, data0.unique());
    REPORTER_ASSERT(reporter, !data1.unique());

    dict.removeAll();
    REPORTER_ASSERT(reporter, nullptr == dict.find("foo"));
    REPORTER_ASSERT(reporter, nullptr == dict.find("bar"));
    REPORTER_ASSERT(reporter, data0.unique());
    REPORTER_ASSERT(reporter, data1.unique());

    {
        SkRefDict d;
        REPORTER_ASSERT(reporter, nullptr == d.find("foo"));
        REPORTER_ASSERT(reporter, data0.unique());
        d.set("foo", &data0);
        REPORTER_ASSERT(reporter, &data0 == d.find("foo"));
        REPORTER_ASSERT(reporter, !data0.unique());
        // let d go out of scope still with a ref on data0
    }
    // be sure d's destructor lowered data0's owner count back to 1
    REPORTER_ASSERT(reporter, data0.unique());
}
