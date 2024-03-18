/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "src/base/SkArenaAlloc.h"
#include "src/utils/SkJSON.h"
#include "tests/Test.h"

#include <cstring>
#include <string_view>

using namespace skjson;

DEF_TEST(JSON_Parse, reporter) {
    static constexpr struct {
        const char* in;
        const char* out;
    } g_tests[] = {
        { ""     , nullptr },
        { "["    , nullptr },
        { "]"    , nullptr },
        { "[[]"  , nullptr },
        { "[]]"  , nullptr },
        { "[]f"  , nullptr },
        { "{"    , nullptr },
        { "}"    , nullptr },
        { "{{}"  , nullptr },
        { "{}}"  , nullptr },
        { "{}f"  , nullptr },
        { "{]"   , nullptr },
        { "[}"   , nullptr },
        { "{\"}" , nullptr },
        { "[\"]" , nullptr },
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

        { "{\"\":{}"                  , nullptr },
        { "{ null }"                  , nullptr },
        { "{ \"k\" : }"               , nullptr },
        { "{ : null }"                , nullptr },
        { "{ \"k\" : : null }"        , nullptr },
        { "{ \"k\" : null , }"        , nullptr },
        { "{ \"k\" : null \"k\" : 1 }", nullptr },

        {R"zzz(["\)zzz"      , nullptr},
        {R"zzz(["\])zzz"     , nullptr},
        {R"zzz(["\"])zzz"    , nullptr},
        {R"zzz(["\z"])zzz"   , nullptr},
        {R"zzz(["\u"])zzz"   , nullptr},
        {R"zzz(["\u0"])zzz"  , nullptr},
        {R"zzz(["\u00"])zzz" , nullptr},
        {R"zzz(["\u000"])zzz", nullptr},

        { "[]"                           , "[]" },
        { " \n\r\t [ \n\r\t ] \n\r\t "   , "[]" },
        { "[[]]"                         , "[[]]" },
        { "[ null ]"                     , "[null]" },
        { "[ true ]"                     , "[true]" },
        { "[ false ]"                    , "[false]" },
        { "[ 0 ]"                        , "[0]" },
        { "[ 1 ]"                        , "[1]" },
        { "[ 1.248 ]"                    , "[1.248]" },
        { "[ \"\" ]"                     , "[\"\"]" },
        { "[ \"foo{bar}baz\" ]"          , "[\"foo{bar}baz\"]" },
        { "[ \" f o o \" ]"              , "[\" f o o \"]" },
        { "[ \"123456\" ]"               , "[\"123456\"]" },
        { "[ \"1234567\" ]"              , "[\"1234567\"]" },
        { "[ \"12345678\" ]"             , "[\"12345678\"]" },
        { "[ \"123456789\" ]"            , "[\"123456789\"]" },
        { "[ null , true, false,0,12.8 ]", "[null,true,false,0,12.8]" },

        { "{}"                          , "{}" },
        { " \n\r\t { \n\r\t } \n\r\t "  , "{}" },
        { "{ \"k\" : null }"            , "{\"k\":null}" },
        { "{ \"foo{\" : \"bar}baz\" }"  , "{\"foo{\":\"bar}baz\"}" },
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

        {R"zzz(["\""])zzz"    , "[\"\"\"]"},
        {R"zzz(["\\"])zzz"    , "[\"\\\"]"},
        {R"zzz(["\/"])zzz"    , "[\"/\"]" },
        {R"zzz(["\b"])zzz"    , "[\"\b\"]"},
        {R"zzz(["\f"])zzz"    , "[\"\f\"]"},
        {R"zzz(["\n"])zzz"    , "[\"\n\"]"},
        {R"zzz(["\r"])zzz"    , "[\"\r\"]"},
        {R"zzz(["\t"])zzz"    , "[\"\t\"]"},
        {R"zzz(["\u1234"])zzz", "[\"\u1234\"]"},

        {R"zzz(["foo\"bar"])zzz"    , "[\"foo\"bar\"]"},
        {R"zzz(["foo\\bar"])zzz"    , "[\"foo\\bar\"]"},
        {R"zzz(["foo\/bar"])zzz"    , "[\"foo/bar\"]" },
        {R"zzz(["foo\bbar"])zzz"    , "[\"foo\bbar\"]"},
        {R"zzz(["foo\fbar"])zzz"    , "[\"foo\fbar\"]"},
        {R"zzz(["foo\nbar"])zzz"    , "[\"foo\nbar\"]"},
        {R"zzz(["foo\rbar"])zzz"    , "[\"foo\rbar\"]"},
        {R"zzz(["foo\tbar"])zzz"    , "[\"foo\tbar\"]"},
        {R"zzz(["foo\u1234bar"])zzz", "[\"foo\u1234bar\"]"},
    };

    for (const auto& tst : g_tests) {
        DOM dom(tst.in, strlen(tst.in));
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
    const VT* cast_t = v;
    REPORTER_ASSERT(reporter, (cast_t != nullptr) == is_type);

    if (is_type) {
        REPORTER_ASSERT(reporter, &v.as<VT>() == cast_t);
        REPORTER_ASSERT(reporter, *v.as<VT>() == pv);
    }
}

template <typename T>
static void check_vector(skiatest::Reporter* reporter, const Value& v, size_t expected_size,
                         bool is_vector) {
    REPORTER_ASSERT(reporter, v.is<T>() == is_vector);
    const T* cast_t = v;
    REPORTER_ASSERT(reporter, (cast_t != nullptr) == is_vector);

    if (is_vector) {
        const auto& vec = v.as<T>();
        REPORTER_ASSERT(reporter, &vec == cast_t);
        REPORTER_ASSERT(reporter, vec.size()  == expected_size);
        REPORTER_ASSERT(reporter, vec.begin() != nullptr);
        REPORTER_ASSERT(reporter, vec.end()   == vec.begin() + expected_size);
    }
}

static void check_string(skiatest::Reporter* reporter, const Value& v, const char* s) {
    check_vector<StringValue>(reporter, v, s ? strlen(s) : 0, !!s);
    if (s) {
        REPORTER_ASSERT(reporter, v.as<StringValue>().str() == s);
        REPORTER_ASSERT(reporter, !strcmp(v.as<StringValue>().begin(), s));
    }
}

DEF_TEST(JSON_DOM_visit, reporter) {
    static constexpr char json[] = "{     \n\
        \"k1\": null,                     \n\
        \"k2\": false,                    \n\
        \"k3\": true,                     \n\
        \"k4\": 42,                       \n\
        \"k5\": .75,                      \n\
        \"k6\": \"foo\",                  \n\
        \"k6b\": \"this string is long\", \n\
        \"k7\": [ 1, true, \"bar\" ],     \n\
        \"k8\": { \"kk1\": 2, \"kk2\": false, \"kk1\": \"baz\" } \n\
    }";

    DOM dom(json, strlen(json));

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
        const auto& v = jroot["k6b"];
        REPORTER_ASSERT(reporter, !v.is<NullValue>());

        check_primitive<bool, BoolValue>(reporter, v, false, false);
        check_primitive<float, NumberValue>(reporter, v, 0, false);

        check_string(reporter, v, "this string is long");
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
}

template <typename T>
void check_value(skiatest::Reporter* reporter, const Value& v, const char* expected_string) {
    REPORTER_ASSERT(reporter, v.is<T>());

    const T* cast_t = v;
    REPORTER_ASSERT(reporter, cast_t == &v.as<T>());

    const auto vstr = v.toString();
    REPORTER_ASSERT(reporter, 0 == strcmp(expected_string, vstr.c_str()));
}

DEF_TEST(JSON_DOM_build, reporter) {
    SkArenaAlloc alloc(4096);

    const auto v0  = NullValue();
    check_value<NullValue>(reporter, v0, "null");

    const auto v1  = BoolValue(true);
    check_value<BoolValue>(reporter, v1, "true");

    const auto v2  = BoolValue(false);
    check_value<BoolValue>(reporter, v2, "false");

    const auto v3  = NumberValue(0);
    check_value<NumberValue>(reporter, v3, "0");

    const auto v4  = NumberValue(42);
    check_value<NumberValue>(reporter, v4, "42");

    const auto v5  = NumberValue(42.75f);
    check_value<NumberValue>(reporter, v5, "42.75");

    const auto v6  = StringValue(nullptr, 0, alloc);
    check_value<StringValue>(reporter, v6, "\"\"");

    const auto v7  = StringValue(" foo ", 5, alloc);
    check_value<StringValue>(reporter, v7, "\" foo \"");

    const auto v8  = StringValue(" foo bar baz ", 13, alloc);
    check_value<StringValue>(reporter, v8, "\" foo bar baz \"");

    const auto v9  = ArrayValue(nullptr, 0, alloc);
    check_value<ArrayValue>(reporter, v9, "[]");

    const Value values0[] = { v0, v3, v9 };
    const auto v10 = ArrayValue(values0, std::size(values0), alloc);
    check_value<ArrayValue>(reporter, v10, "[null,0,[]]");

    const auto v11 = ObjectValue(nullptr, 0, alloc);
    check_value<ObjectValue>(reporter, v11, "{}");

    const Member members0[] = {
        { StringValue("key_0", 5, alloc), v1  },
        { StringValue("key_1", 5, alloc), v4  },
        { StringValue("key_2", 5, alloc), v11 },
    };
    const auto v12 = ObjectValue(members0, std::size(members0), alloc);
    check_value<ObjectValue>(reporter, v12, "{"
                                                "\"key_0\":true,"
                                                "\"key_1\":42,"
                                                "\"key_2\":{}"
                                            "}");

    const Value values1[] = { v2, v6, v12 };
    const auto v13 = ArrayValue(values1, std::size(values1), alloc);
    check_value<ArrayValue>(reporter, v13, "["
                                               "false,"
                                               "\"\","
                                               "{"
                                                   "\"key_0\":true,"
                                                   "\"key_1\":42,"
                                                   "\"key_2\":{}"
                                               "}"
                                           "]");

    const Member members1[] = {
        { StringValue("key_00", 6, alloc), v5  },
        { StringValue("key_01", 6, alloc), v7  },
        { StringValue("key_02", 6, alloc), v13 },
    };
    const auto v14 = ObjectValue(members1, std::size(members1), alloc);
    check_value<ObjectValue>(reporter, v14, "{"
                                                "\"key_00\":42.75,"
                                                "\"key_01\":\" foo \","
                                                "\"key_02\":["
                                                                "false,"
                                                                "\"\","
                                                                "{"
                                                                    "\"key_0\":true,"
                                                                    "\"key_1\":42,"
                                                                    "\"key_2\":{}"
                                                                "}"
                                                            "]"
                                            "}");
}

DEF_TEST(JSON_ParseNumber, reporter) {
    static constexpr struct {
        const char* string;
        SkScalar    value,
                    tolerance;
    } gTests[] = {
        { "0", 0, 0 },
        { "1", 1, 0 },

        { "00000000", 0, 0 },
        { "00000001", 1, 0 },

        { "0.001", 0.001f, 0 },
        { "1.001", 1.001f, 0 },

        { "0.000001"   ,    0.000001f, 0 },
        { "1.000001"   ,    1.000001f, 0 },
        { "1000.000001", 1000.000001f, 0 },

        { "0.0000000001"   ,    0.0000000001f, 0 },
        { "1.0000000001"   ,    1.0000000001f, 0 },
        { "1000.0000000001", 1000.0000000001f, 0 },

        { "20.001111814444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444473",
          20.001f, 0.001f },
    };

    for (const auto& test : gTests) {
        const auto json = SkStringPrintf("{ \"key\": %s }", test.string);
        const DOM dom(json.c_str(), json.size());
        const ObjectValue* jroot = dom.root();

        REPORTER_ASSERT(reporter, jroot);

        const NumberValue* jnumber = (*jroot)["key"];
        REPORTER_ASSERT(reporter, jnumber);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(**jnumber, test.value, test.tolerance));
    }
}

DEF_TEST(JSON_Lookup, r) {
    const char* json = R"({"foo": { "bar": { "baz": 100 }}})";
    const DOM dom(json, strlen(json));
    const Value& root = dom.root();

    REPORTER_ASSERT(r, root.is<ObjectValue>());
    REPORTER_ASSERT(r, root["foo"].is<ObjectValue>());
    REPORTER_ASSERT(r, root["foo"]["bar"].is<ObjectValue>());
    REPORTER_ASSERT(r, root["foo"]["bar"]["baz"].is<NumberValue>());

    REPORTER_ASSERT(r, root["foozz"].is<NullValue>());
    REPORTER_ASSERT(r, root["foozz"]["barzz"].is<NullValue>());
    REPORTER_ASSERT(r, root["foozz"]["barzz"]["bazzz"].is<NullValue>());
}

DEF_TEST(JSON_Writable, r) {
    const char* json = R"({"null": null, "num": 100})";
    const DOM dom(json, strlen(json));
    REPORTER_ASSERT(r, dom.root().is<ObjectValue>());
    const ObjectValue& root = dom.root().as<ObjectValue>();

    SkArenaAlloc alloc(4096);

    REPORTER_ASSERT(r, root["null"].is<NullValue>());
    Value& w1 = root.writable("null", alloc);
    REPORTER_ASSERT(r, w1.is<NullValue>());
    w1 = NumberValue(42);
    REPORTER_ASSERT(r, root["null"].is<NumberValue>());
    REPORTER_ASSERT(r, *root["null"].as<NumberValue>() == 42);

    REPORTER_ASSERT(r, root["num"].is<NumberValue>());
    Value& w2 = root.writable("num", alloc);
    REPORTER_ASSERT(r, w2.is<NumberValue>());
    w2 = StringValue("foo", 3, alloc);
    REPORTER_ASSERT(r, root["num"].is<StringValue>());
    REPORTER_ASSERT(r, root["num"].as<StringValue>().str() == "foo");

    // new/insert semantics
    REPORTER_ASSERT(r, root["new"].is<NullValue>());
    REPORTER_ASSERT(r, root.size() == 2u);
    Value& w3 = root.writable("new", alloc);
    REPORTER_ASSERT(r, w3.is<NullValue>());
    w3 = BoolValue(true);
    REPORTER_ASSERT(r, root.size() == 3u);
    REPORTER_ASSERT(r, root["new"].is<BoolValue>());
    REPORTER_ASSERT(r, *root["new"].as<BoolValue>());

    root.writable("newobj", alloc) = ObjectValue(nullptr, 0, alloc);
    REPORTER_ASSERT(r, root.size() == 4u);
    REPORTER_ASSERT(r, root["newobj"].is<ObjectValue>());
    const ObjectValue& newobj = root["newobj"].as<ObjectValue>();
    REPORTER_ASSERT(r, newobj.size() == 0u);

    newobj.writable("newprop", alloc) = NumberValue(-1);
    REPORTER_ASSERT(r, newobj.size() == 1u);
    REPORTER_ASSERT(r, root["newobj"]["newprop"].is<NumberValue>());
    REPORTER_ASSERT(r, *root["newobj"]["newprop"].as<NumberValue>() == -1);

    REPORTER_ASSERT(r, root.toString() ==
        SkString(R"({"null":42,"num":"foo","new":true,"newobj":{"newprop":-1}})"));
}
