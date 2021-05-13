/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This is not a real C++ file, but used to test the parsing of gen_types
 */
#include <string>

#include "experimental/tskit/bindings/bindings.h"

EMSCRIPTEN_BINDINGS(Core) {
    TS_PRIVATE_EXPORT("_privateFunction2(x: number, y: number): number")
    function("_privateFunction2", optional_override([](int x, int y)->size_t {
        return x * y;
    }));

    /**
     * This function does a public thing.
     * @param input an ice cream flavor
     */
    TS_EXPORT("publicFunction2(input: string): void")
    function("publicFunction2", optional_override([](std::string s)->void {
        printf("Hello %s\n", s.c_str());
    }));

    /**
     * The Something class is quite something. See SkSomething.h for more.
     */
    class_<Something>("Something")
        /**
         * Returns a Something with the provided name.
         * @param name
         */
        TS_EXPORT("new(name: string): Something")
        .constructor<std::string>()
        /**
         * Returns the associated name.
         */
        TS_EXPORT("getName(): string")
        .function("getName", &Something::getName)
        TS_PRIVATE_EXPORT("_setName(name: string): void")
        .function("_setName", &Something::setName);

    /**
     * The AnotherClass class is another class.
     */
    class_<AnotherClass>("AnotherClass")
        TS_EXPORT("new(): AnotherClass")
        .constructor<>()
        /**
         * Returns a Something with the provided name.
         * @param name
         * @param thing - will be used, I promise.
         */
        TS_EXPORT("new(name: string, thing: Something): AnotherClass")
        .constructor<std::string,Something>()
        /**
         * Returns the associated thing.
         */
        TS_EXPORT("get(): Something")
        .function("get", &AnotherClass::get);

    value_object<SomeValueObject>("SomeValueObject")
        /**
            The number of columns that the frobulator needs.
            @type @optional number
         */
        .field("columns",   &SomeValueObject::columns)
        /**
         *   The object associated with the frobulator.
         *   @type AnotherClass
         */
        .field("object",    &SomeValueObject::object)
        /** @type string*/
        .field("name",      &SomeValueObject::slot)
        /**
          *  @type boolean
          */
        .field("isInteger", &SomeValueObject::isInteger);

    TS_PRIVATE_EXPORT("_privateFunction1(ptr: number): number;")
    function("_privateFunction1", &SkCanvas::whatever);

    /**
     * This function does another public thing.
     * @param input an ice cream flavor
     */
    TS_EXPORT("publicFunction1(): boolean;")
    function("publicFunction1", &SkCanvas::blerg);

    /**
     *  @type boolean
     */
    constant("hasBird", true);
    /**
     *  This is the flag which is a lot of fun.
     *  It is used in a variety of ways.
     *  #funwithflags
     *  @type number
     */
    constant("SOME_FLAG", 0x2);

#ifdef SK_EXTRA_FEATURE
    /**
     *  This is set if the extra feature is compiled in.
     *  @type @optional string
     */
    constant("optionalConst", "foo");
#endif
}
