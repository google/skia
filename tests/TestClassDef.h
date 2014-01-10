/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*  This file is meant to be included by .cpp files, so it can spew out a
    customized class + global definition.

    e.g.
    #include "TestClassDef.h"

    DEF_TEST(TestName, reporter) {
        ...
        REPORTER_ASSERT(reporter, x == 15);
    }
*/

#define DEF_TEST(name, reporter)                                                     \
    static void name(skiatest::Reporter* reporter);                                  \
    namespace skiatest {                                                             \
        class name##Class : public Test {                                            \
        public:                                                                      \
            static Test* Factory(void*) { return SkNEW(name##Class); }               \
        protected:                                                                   \
            virtual void onGetName(SkString* name) SK_OVERRIDE { name->set(#name); } \
            virtual void onRun(Reporter* reporter) SK_OVERRIDE { name(reporter); }   \
        };                                                                           \
        static TestRegistry gReg_##name##Class(name##Class::Factory);                \
    }                                                                                \
    static void name(skiatest::Reporter* reporter)

#define DEF_GPUTEST(name, reporter, factory)                                         \
    static void name(skiatest::Reporter* reporter, GrContextFactory* factory);       \
    namespace skiatest {                                                             \
        class name##Class : public GpuTest {                                         \
        public:                                                                      \
            static Test* Factory(void*) { return SkNEW(name##Class); }               \
        protected:                                                                   \
            virtual void onGetName(SkString* name) SK_OVERRIDE { name->set(#name); } \
            virtual void onRun(Reporter* reporter) SK_OVERRIDE {                     \
                name(reporter, GetGrContextFactory());                               \
            }                                                                        \
        };                                                                           \
        static TestRegistry gReg_##name##Class(name##Class::Factory);                \
    }                                                                                \
    static void name(skiatest::Reporter* reporter, GrContextFactory* factory)
