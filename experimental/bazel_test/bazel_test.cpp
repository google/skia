// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include <png.h>
#include <ctime>
#include <iostream>

void print_localtime() {
    std::time_t result = std::time(nullptr);
    std::cout << std::asctime(std::localtime(&result));
}

int main(int argc, char** argv) {
    printf("Hello world\n");
    print_localtime();
    // https://docs.bazel.build/versions/main/test-encyclopedia.html#role-of-the-test-runner
    if (png_access_version_number() == 10638) {
        printf("PASS\n"); // This tells the human the test passed.
        return 0; // This tells Bazel the test passed.
    }
    printf("FAIL\n"); // This tells the human the test failed.
    return 1; // This tells Bazel the test failed.
}
