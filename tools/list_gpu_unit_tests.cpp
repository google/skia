/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "Test.h"

int main() {
    std::vector<std::string> tests;
    for (const skiatest::TestRegistry* r = skiatest::TestRegistry::Head(); r; r = r->next()) {
        const skiatest::Test& test = r->factory();
        if (test.needsGpu) {
            tests.push_back(std::string(test.name));
        }
    }
    std::sort(tests.begin(), tests.end());
    for (const std::string& test : tests) {
        std::cout << test << '\n';
    }
}
