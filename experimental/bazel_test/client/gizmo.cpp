// Copyright 2022 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/bazel_test/client/gizmo.h"

#include "experimental/bazel_test/base/base.h"
#include "experimental/bazel_test/core/core.h"

#if defined(SOURCE_INCLUDES_TRANSITIVE_HEADER)
// This fails with
//   module //experimental/bazel_test/client:client_lib does not depend
//   on a module exporting 'experimental/bazel_test/base/base_priv.h'
// because the client_lib cc_library does not depend on the base_priv
// cc_library directly (and cannot, due to visibility restrictions)
#include "experimental/bazel_test/base/base_priv.h"
#endif

#if defined(SOURCE_INCLUDES_PRIVATE_HEADER)
// This fails with
//   error: use of private header from outside its module:
//   'experimental/bazel_test/core/core_priv.h' [-Wprivate-header]
// because the generated core module has that file listed as "private"
// (because it came from srcs).
#include "experimental/bazel_test/core/core_priv.h"
#endif

float getGizmo() {
	return PublicCoreConstant * getMagicNumber() + PublicBaseConstant;
}
