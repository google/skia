/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJSONRenderer_DEFINED
#define SkJSONRenderer_DEFINED

#include "SkCanvas.h"

namespace SkJSONRenderer {
	/* 
	 * Takes a JSON document produced by SkJSONCanvas and issues its draw commands to the target
	 * canvas.
	 */
	void render(const char* json, SkCanvas* target);
}

#endif
