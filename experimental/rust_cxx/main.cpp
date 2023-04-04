// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// namespace hype_train
#include "experimental/rust_cxx/gen/hype-bridge.rs.h"

#include <stdio.h>
#include <string>


int main(int argc, char** argv) {
    printf("Hello C++\n");

    std::string words = "it works";

    hype_train::HypeOutput result = hype_train::hypeify(words, 3);

    printf("%s\n", result.output.c_str());
    printf("new len: %lu\n", result.new_len);

    return 0;
}
