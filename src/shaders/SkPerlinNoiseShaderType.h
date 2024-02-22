/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPerlinNoiseShaderType_DEFINED
#define SkPerlinNoiseShaderType_DEFINED

/**
 * About the noise types : the difference between the first two is just minor tweaks to the
 * algorithm; they're not two entirely different noises. The output looks different, but once the
 * noise is generated in the [1, -1] range, the output is brought back in the [0, 1] range by doing:
 *   kFractalNoise : noise * 0.5 + 0.5
 *   kTurbulence   : abs(noise)
 * Very little differs between the two types, although you can tell the difference visually.
 */
enum class SkPerlinNoiseShaderType { kFractalNoise, kTurbulence, kLast = kTurbulence };

#endif
