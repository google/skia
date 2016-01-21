/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkToFromValue_DEFINED
#define SkToFromValue_DEFINED

#include "SkValue.h"

template <typename T>
SkValue SkToValue(const T&);

template <typename T>
SkValue SkToValue(const T*);

template <typename T>
bool SkFromValue(const SkValue&, T*);

#endif  // SkToFromValue_DEFINED
