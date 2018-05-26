/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkJSON.h"
#include "SkStream.h"

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
        skjson::Dom dom(tst.in);
        const auto success = !dom.root().is<skjson::NullValue>();
        REPORTER_ASSERT(reporter, success == (tst.out != nullptr));
        if (!success) continue;

        SkDynamicMemoryWStream str;
        dom.write(&str);
        str.write8('\0');

        auto data = str.detachAsData();
        REPORTER_ASSERT(reporter, !strcmp(tst.out, static_cast<const char*>(data->data())));
    }

}
