// Copyright 2022 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/bazel_test/client/gizmo.h"

#include <iostream>

int main(int argc, char** argv) {
	printf("Hello world\n");

	printf("Gizmo: %f\n", getGizmo());
}
