/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRefDict.h"
#include "RefCntIs.h"
#include "Test.h"

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
    REPORTER_ASSERT(reporter, RefCntIs(data0, 2));

    dict.set("foo", &data0);
    REPORTER_ASSERT(reporter, &data0 == dict.find("foo"));
    REPORTER_ASSERT(reporter, RefCntIs(data0, 2));

    dict.set("foo", &data1);
    REPORTER_ASSERT(reporter, &data1 == dict.find("foo"));
    REPORTER_ASSERT(reporter, RefCntIs(data0, 1));
    REPORTER_ASSERT(reporter, RefCntIs(data1, 2));

    dict.set("foo", NULL);
    REPORTER_ASSERT(reporter, NULL == dict.find("foo"));
    REPORTER_ASSERT(reporter, RefCntIs(data0, 1));
    REPORTER_ASSERT(reporter, RefCntIs(data1, 1));

    dict.set("foo", &data0);
    dict.set("bar", &data1);
    REPORTER_ASSERT(reporter, &data0 == dict.find("foo"));
    REPORTER_ASSERT(reporter, &data1 == dict.find("bar"));
    REPORTER_ASSERT(reporter, RefCntIs(data0, 2));
    REPORTER_ASSERT(reporter, RefCntIs(data1, 2));

    dict.set("foo", &data1);
    REPORTER_ASSERT(reporter, &data1 == dict.find("foo"));
    REPORTER_ASSERT(reporter, &data1 == dict.find("bar"));
    REPORTER_ASSERT(reporter, RefCntIs(data0, 1));
    REPORTER_ASSERT(reporter, RefCntIs(data1, 3));

    dict.removeAll();
    REPORTER_ASSERT(reporter, NULL == dict.find("foo"));
    REPORTER_ASSERT(reporter, NULL == dict.find("bar"));
    REPORTER_ASSERT(reporter, RefCntIs(data0, 1));
    REPORTER_ASSERT(reporter, RefCntIs(data1, 1));

    {
        SkRefDict d;
        REPORTER_ASSERT(reporter, NULL == d.find("foo"));
        REPORTER_ASSERT(reporter, RefCntIs(data0, 1));
        d.set("foo", &data0);
        REPORTER_ASSERT(reporter, &data0 == d.find("foo"));
        REPORTER_ASSERT(reporter, RefCntIs(data0, 2));
        // let d go out of scope still with a ref on data0
    }
    // be sure d's destructor lowered data0's owner count back to 1
    REPORTER_ASSERT(reporter, RefCntIs(data0, 1));
}
