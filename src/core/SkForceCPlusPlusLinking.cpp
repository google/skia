/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This file is intentionally empty.  We add it to the dependencies of skia_lib
// so that GYP detects that libskia is a C++ library (implicitly depending on
// the standard library, -lm, etc.) from its file extension.
//
// If we didn't do this, GYP would link libskia.so as a C library and we'd get
// link-time failures for simple binaries that don't themselves depend on the
// C++ standard library.
//
// Even if we try hard not to depend on the standard library, say, never
// calling new or delete, the compiler can still insert calls on our behalf
// that make us depend on it anyway: a handler when we call a for a pure
// virtual, thread-safety guards around statics, probably other similar
// language constructs.
