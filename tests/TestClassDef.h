/*  This file is meant to be included by .cpp files, so it can spew out a
    customized class + global definition.

    e.g.
    #include "TestClassDef.h"
    DEFINE_TESTCLASS("MyTest", MyTestClass, MyTestFunction)

    where MyTestFunction is declared as

        void MyTestFunction(skiatest::Reporter*)
*/

#define DEFINE_TESTCLASS(uiname, classname, function)                       \
    namespace skiatest {                                                    \
        class classname : public Test {                                     \
        public:                                                             \
            static Test* Factory(void*) { return SkNEW(classname); }        \
        protected:                                                          \
            virtual void onGetName(SkString* name) { name->set(uiname); }   \
            virtual void onRun(Reporter* reporter) { function(reporter); }  \
        };                                                                  \
        static TestRegistry gReg(classname::Factory);                       \
    }

