/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_SURFACE_CL_12_ONCE
#define SKC_SURFACE_CL_12_ONCE

//
// Unlike other object platform implementations, the surface object
// implementation needs to access the opaque platform-specfic outputs
// of the composition and styling objects.
//
//  Composition : { keys,   offsets, key_count, offset_count }
//  Styling     : { layers, groups,  commands                }
//
// With the OpenCL platform we'll handle this by simply exposing the
// argument value (void*) and its size (size_t).
//
// TODO: It might make sense in the future to support more complex
//       rendering jobs that simultaneously involve multiple surfaces,
//       compositions and stylings.
//

#endif

//
//
//
