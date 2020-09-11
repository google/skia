// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include <png.h>

int main(int argc, char** argv) {
    return png_access_version_number() == 10637
        ? 0 : 1;
}
