/*
 * Copyright 2016 Google, Inc. 
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

/*
 * Draw 50 semi-transparent circles with pseudo-random positions, radii, and
 * colors, using the default blend mode (SRC OVER). Use of SkRandom with 
 * default seed means results SHOULD be identical across multiple platforms. 
 */
DEF_SIMPLE_GM(bubbles, canvas, 512, 512) {
	canvas->clear(SK_ColorWHITE);

	SkPaint p;
	p.setAntiAlias(true);

	SkRandom rand;

	auto pos_min    = SkIntToScalar(0);
	auto pos_max    = SkIntToScalar(511);
	auto radius_min = SkIntToScalar(0);
	auto radius_max = SkIntToScalar(128);
	auto color_min  = SkIntToScalar(0);
	auto color_max  = SkIntToScalar(255);

	const int NUM_BUBBLES = 50;
	for (int i = 0; i < NUM_BUBBLES; i++) {
		auto cx     = rand.nextRangeScalar(pos_min, pos_max);
		auto cy     = rand.nextRangeScalar(pos_min, pos_max);
		auto radius = rand.nextRangeScalar(radius_min, radius_max);

		auto a = (U8CPU)128;
		auto r = (U8CPU)rand.nextRangeScalar(color_min, color_max);
		auto g = (U8CPU)rand.nextRangeScalar(color_min, color_max);
		auto b = (U8CPU)rand.nextRangeScalar(color_min, color_max);
		p.setColor(SkColorSetARGB(a, r, g, b));

		canvas->drawCircle(cx, cy, radius, p);
	}
}
