/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkJSON.h"
#include "SkStream.h"

using namespace skjson;

DEF_TEST(SkJSON_Parse, reporter) {
    static constexpr struct {
        const char* in;
        const char* out;
    } g_tests[] = {
        { ""     , nullptr },
        { "["    , nullptr },
        { "]"    , nullptr },
        { "{"    , nullptr },
        { "}"    , nullptr },
        { "{]"   , nullptr },
        { "[}"   , nullptr },
        { "1"    , nullptr },
        { "true" , nullptr },
        { "false", nullptr },
        { "null" , nullptr },

        { "[nulll]" , nullptr },
        { "[false2]", nullptr },
        { "[true:]" , nullptr },

        { "[1 2]"   , nullptr },
        { "[1,,2]"  , nullptr },
        { "[1,2,]"  , nullptr },
        { "[,1,2]"  , nullptr },

        { "[ \"foo"       , nullptr },
        { "[ \"fo\0o\" ]" , nullptr },

        { "{ null }"                  , nullptr },
        { "{ \"k\" : }"               , nullptr },
        { "{ : null }"                , nullptr },
        { "{ \"k\" : : null }"        , nullptr },
        { "{ \"k\" : null , }"        , nullptr },
        { "{ \"k\" : null \"k\" : 1 }", nullptr },


        { " \n\r\t [ \n\r\t ] \n\r\t "   , "[]" },
        { "[ null ]"                     , "[null]" },
        { "[ true ]"                     , "[true]" },
        { "[ false ]"                    , "[false]" },
        { "[ 0 ]"                        , "[0]" },
        { "[ 1 ]"                        , "[1]" },
        { "[ 1.248 ]"                    , "[1.248]" },
        { "[ \"\" ]"                     , "[\"\"]" },
        { "[ \" f o o \" ]"              , "[\" f o o \"]" },
        { "[ \"123456\" ]"               , "[\"123456\"]" },
        { "[ \"1234567\" ]"              , "[\"1234567\"]" },
        { "[ \"12345678\" ]"             , "[\"12345678\"]" },
        { "[ \"123456789\" ]"            , "[\"123456789\"]" },
        { "[ null , true, false,0,12.8 ]", "[null,true,false,0,12.8]" },

        { " \n\r\t { \n\r\t } \n\r\t "  , "{}" },
        { "{ \"k\" : null }"            , "{\"k\":null}" },
        { "{ \"k1\" : null, \"k2 \":0 }", "{\"k1\":null,\"k2 \":0}" },
        { "{ \"k1\" : null, \"k1\":0 }" , "{\"k1\":null,\"k1\":0}" },

        { "{ \"k1\" : null,                   \n\
             \"k2\" : 0,                      \n\
             \"k3\" : [                       \n\
                        true,                 \r\n\
                        { \"kk1\" : \"foo\" , \n\
                          \"kk2\" : \"bar\" , \n\
                          \"kk3\" : 1.28 ,    \n\
                          \"kk4\" : [ 42 ]    \n\
                        } ,                   \n\
                        \"boo\" ,             \n\
                        null                  \n\
                      ]                       \n\
           }",
          "{\"k1\":null,\"k2\":0,\"k3\":[true,"
              "{\"kk1\":\"foo\",\"kk2\":\"bar\",\"kk3\":1.28,\"kk4\":[42]},\"boo\",null]}" },
    };

    for (const auto& tst : g_tests) {
        DOM dom(tst.in);
        const auto success = !dom.root().is<NullValue>();
        REPORTER_ASSERT(reporter, success == (tst.out != nullptr));
        if (!success) continue;

        SkDynamicMemoryWStream str;
        dom.write(&str);
        str.write8('\0');

        auto data = str.detachAsData();
        REPORTER_ASSERT(reporter, !strcmp(tst.out, static_cast<const char*>(data->data())));
    }

}

template <typename T, typename VT>
static void check_primitive(skiatest::Reporter* reporter, const Value& v, T pv,
                            bool is_type) {

    REPORTER_ASSERT(reporter,  v.is<VT>() == is_type);
    REPORTER_ASSERT(reporter, *v.as<VT>() == pv);
}

template <typename T>
static void check_vector(skiatest::Reporter* reporter, const Value& v, size_t expected_size,
                         bool is_vector) {
    REPORTER_ASSERT(reporter, v.is<T>() == is_vector);

    const auto& vec = v.as<T>();
    REPORTER_ASSERT(reporter, vec.size() == expected_size);
    REPORTER_ASSERT(reporter, (vec.begin() != nullptr) == is_vector);
    REPORTER_ASSERT(reporter, vec.end()   == vec.begin() + expected_size);
}

static void check_string(skiatest::Reporter* reporter, const Value& v, const char* s) {
    check_vector<StringValue>(reporter, v, s ? strlen(s) : 0, !!s);
    if (s) {
        REPORTER_ASSERT(reporter, !strcmp(v.as<StringValue>().begin(), s));
    }
}

DEF_TEST(SkJSON_DOM, reporter) {
    static constexpr char json[] = "{ \n\
        \"k1\": null,                \n\
        \"k2\": false,               \n\
        \"k3\": true,                \n\
        \"k4\": 42,                  \n\
        \"k5\": .75,                 \n\
        \"k6\": \"foo\",             \n\
        \"k7\": [ 1, true, \"bar\" ], \n\
        \"k8\": { \"kk1\": 2, \"kk2\": false, \"kk1\": \"baz\" } \n\
    }";

    DOM dom(json);

    const auto& jroot = dom.root().as<ObjectValue>();
    REPORTER_ASSERT(reporter, jroot.is<ObjectValue>());

    {
        const auto& v = jroot["k1"];
        REPORTER_ASSERT(reporter,  v.is<NullValue>());

        check_primitive<bool, BoolValue>(reporter, v, false, false);
        check_primitive<float, NumberValue>(reporter, v, 0, false);

        check_string(reporter, v, nullptr);
        check_vector<ArrayValue >(reporter, v, 0, false);
        check_vector<ObjectValue>(reporter, v, 0, false);
    }

    {
        const auto& v = jroot["k2"];
        REPORTER_ASSERT(reporter, !v.is<NullValue>());

        check_primitive<bool, BoolValue>(reporter, v, false, true);
        check_primitive<float, NumberValue>(reporter, v, 0, false);

        check_string(reporter, v, nullptr);
        check_vector<ArrayValue >(reporter, v, 0, false);
        check_vector<ObjectValue>(reporter, v, 0, false);
    }

    {
        const auto& v = jroot["k3"];
        REPORTER_ASSERT(reporter, !v.is<NullValue>());

        check_primitive<bool, BoolValue>(reporter, v, true, true);
        check_primitive<float, NumberValue>(reporter, v, 0, false);

        check_string(reporter, v, nullptr);
        check_vector<ArrayValue >(reporter, v, 0, false);
        check_vector<ObjectValue>(reporter, v, 0, false);
    }

    {
        const auto& v = jroot["k4"];
        REPORTER_ASSERT(reporter, !v.is<NullValue>());

        check_primitive<bool, BoolValue>(reporter, v, false, false);
        check_primitive<float, NumberValue>(reporter, v, 42, true);

        check_string(reporter, v, nullptr);
        check_vector<ArrayValue >(reporter, v, 0, false);
        check_vector<ObjectValue>(reporter, v, 0, false);
    }

    {
        const auto& v = jroot["k5"];
        REPORTER_ASSERT(reporter, !v.is<NullValue>());

        check_primitive<bool, BoolValue>(reporter, v, false, false);
        check_primitive<float, NumberValue>(reporter, v, .75f, true);

        check_string(reporter, v, nullptr);
        check_vector<ArrayValue >(reporter, v, 0, false);
        check_vector<ObjectValue>(reporter, v, 0, false);
    }

    {
        const auto& v = jroot["k6"];
        REPORTER_ASSERT(reporter, !v.is<NullValue>());

        check_primitive<bool, BoolValue>(reporter, v, false, false);
        check_primitive<float, NumberValue>(reporter, v, 0, false);

        check_string(reporter, v, "foo");
        check_vector<ArrayValue >(reporter, v, 0, false);
        check_vector<ObjectValue>(reporter, v, 0, false);
    }

    {
        const auto& v = jroot["k7"];
        REPORTER_ASSERT(reporter, !v.is<NullValue>());

        check_primitive<bool, BoolValue>(reporter, v, false, false);
        check_primitive<float, NumberValue>(reporter, v, 0, false);

        check_string(reporter, v, nullptr);
        check_vector<ObjectValue>(reporter, v, 0, false);

        check_vector<ArrayValue >(reporter, v, 3, true);
        check_primitive<float, NumberValue>(reporter, v.as<ArrayValue>()[0], 1, true);
        check_primitive<bool, BoolValue>(reporter, v.as<ArrayValue>()[1], true, true);
        check_vector<StringValue>(reporter, v.as<ArrayValue>()[2], 3, true);
        REPORTER_ASSERT(reporter, v.as<ArrayValue>()[3].is<NullValue>());
    }

    {
        const auto& v = jroot["k8"];
        REPORTER_ASSERT(reporter, !v.is<NullValue>());

        check_primitive<bool, BoolValue>(reporter, v, false, false);
        check_primitive<float, NumberValue>(reporter, v, 0, false);

        check_string(reporter, v, nullptr);
        check_vector<ArrayValue >(reporter, v, 0, false);

        check_vector<ObjectValue>(reporter, v, 3, true);

        const auto& m0 = v.as<ObjectValue>().begin()[0];
        check_string(reporter, m0.fKey, "kk1");
        check_primitive<float, NumberValue>(reporter, m0.fValue, 2, true);

        const auto& m1 = v.as<ObjectValue>().begin()[1];
        check_string(reporter, m1.fKey, "kk2");
        check_primitive<bool, BoolValue>(reporter, m1.fValue, false, true);

        const auto& m2 = v.as<ObjectValue>().begin()[2];
        check_string(reporter, m2.fKey, "kk1");
        check_string(reporter, m2.fValue, "baz");

        REPORTER_ASSERT(reporter, v.as<ObjectValue>()[""].is<NullValue>());
        REPORTER_ASSERT(reporter, v.as<ObjectValue>()["nosuchkey"].is<NullValue>());
        check_string(reporter, v.as<ObjectValue>()["kk1"], "baz");
        check_primitive<bool, BoolValue>(reporter, v.as<ObjectValue>()["kk2"], false, true);
    }

    {
        const auto& v =
            jroot["foo"].as<ObjectValue>()["bar"].as<ObjectValue>()["baz"];
        REPORTER_ASSERT(reporter, v.is<NullValue>());
    }
}
