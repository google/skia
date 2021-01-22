/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeEffectPriv_DEFINED
#define SkRuntimeEffectPriv_DEFINED

/*
 * Controls how much inlining is performed when compiling SkSL for SkRuntimeEffect instances.
 * See also: SkSL::Program::Settings::fInlineThreshold
 */
void SkRuntimeEffect_SetInlineThreshold(int threshold);

#endif
