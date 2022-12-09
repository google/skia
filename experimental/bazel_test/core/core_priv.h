// Copyright 2022 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "experimental/bazel_test/base/base.h"
#include "experimental/bazel_test/base/base_priv.h"

constexpr float PrivateCoreConstant = float(PublicBaseConstant)/float(PrivateBaseConstant * PrivateBaseConstant);
