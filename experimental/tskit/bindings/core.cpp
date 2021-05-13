/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <string>

#include "experimental/tskit/bindings/bindings.h"

class Something {
public:
    Something(std::string n): fName(n) {}

    const std::string getName() {
        return fName;
    }

    void setName(std::string name) {
        fName = name;
    }

private:
    std::string fName;
};

EMSCRIPTEN_BINDINGS(Core) {
    TS_PRIVATE_EXPORT("_privateFunction(x: number, y: number): number")
    function("_privateFunction", optional_override([](int x, int y)->size_t {
        return x * y;
    }));

    /**
     * This function does a public thing.
     * @param input an ice cream flavor
     */
    TS_EXPORT("publicFunction(input: string): void")
    function("publicFunction", optional_override([](std::string s)->void {
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
        TS_PRIVATE_EXPORT("setName(name: string): void")
        .function("_setName", &Something::setName);
}
