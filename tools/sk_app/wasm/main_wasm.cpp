/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstdio>

// The equivalent of an sk_app implementation for WASM would be a webpage that
// sets up its own "windowing" and calls this entrypoint from the browser. There
// is currently no such implementation.
int main(int argc, char**argv) {
    fprintf(stderr, "WASM does not have an sk_app::Application implementation\n");
    return 1;
}
