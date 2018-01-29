/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef ResourceFactory_DEFINED
#define ResourceFactory_DEFINED

#include <SkData.h>

using SkResourceFactoryType = sk_sp<SkData> (*)(const char*);

extern SkResourceFactoryType gResourceFactory;

#endif  // ResourceFactory_DEFINED
