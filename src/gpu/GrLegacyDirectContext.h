// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef GrLegacyDirectContext_DEFINED
#define GrLegacyDirectContext_DEFINED

#include "include/gpu/GrContext.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
sk_sp<GrContext> GrMetalMakeLegacyDirectContext(const GrContextOptions&);
#endif

#endif  // GrLegacyDirectContext_DEFINED
