/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkRasterPipeline.h"

// load needs two variants, one to load 4 values...
static void SK_VECTORCALL load(SkRasterPipeline::Stage* st, size_t x,
                               Sk4f v0, Sk4f v1, Sk4f v2, Sk4f v3,
                               Sk4f v4, Sk4f v5, Sk4f v6, Sk4f v7) {
    auto ptr = st->ctx<const float*>();
    v0 = Sk4f{ptr[x+0]};
    v1 = Sk4f{ptr[x+1]};
    v2 = Sk4f{ptr[x+2]};
    v3 = Sk4f{ptr[x+3]};

    st->next(x, v0,v1,v2,v3, v4,v5,v6,v7);
}

// ...and one to load a single value.
static void SK_VECTORCALL load_tail(SkRasterPipeline::Stage* st, size_t x,
                                    Sk4f v0, Sk4f v1, Sk4f v2, Sk4f v3,
                                    Sk4f v4, Sk4f v5, Sk4f v6, Sk4f v7) {
    auto ptr = st->ctx<const float*>();
    v0 = Sk4f{ptr[x]};

    st->next(x, v0,v1,v2,v3, v4,v5,v6,v7);
}

// square doesn't really care how many of its inputs are active, nor does it need a context.
static void SK_VECTORCALL square(SkRasterPipeline::Stage* st, size_t x,
                                 Sk4f v0, Sk4f v1, Sk4f v2, Sk4f v3,
                                 Sk4f v4, Sk4f v5, Sk4f v6, Sk4f v7) {
    v0 *= v0;
    v1 *= v1;
    v2 *= v2;
    v3 *= v3;
    st->next(x, v0,v1,v2,v3, v4,v5,v6,v7);
}

// Like load, store has a _tail variant.  It ends the pipeline by returning.
static void SK_VECTORCALL store(SkRasterPipeline::Stage* st, size_t x,
                                Sk4f v0, Sk4f v1, Sk4f v2, Sk4f v3,
                                Sk4f v4, Sk4f v5, Sk4f v6, Sk4f v7) {
    auto ptr = st->ctx<float*>();
    ptr[x+0] = v0[0];
    ptr[x+1] = v1[0];
    ptr[x+2] = v2[0];
    ptr[x+3] = v3[0];
}

static void SK_VECTORCALL store_tail(SkRasterPipeline::Stage* st, size_t x,
                                     Sk4f v0, Sk4f v1, Sk4f v2, Sk4f v3,
                                     Sk4f v4, Sk4f v5, Sk4f v6, Sk4f v7) {
    auto ptr = st->ctx<float*>();
    ptr[x+0] = v0[0];
}

DEF_TEST(SkRasterPipeline, r) {
    // We'll build up and run a simple pipeline that exercises the salient
    // mechanics of SkRasterPipeline:
    //    - context pointers
    //    - stages sensitive to the number of pixels
    //    - stages insensitive to the number of pixels
    //
    // This pipeline loads up some values, squares them, then writes them back to memory.

    const float src_vals[] = { 1,2,3,4,5 };
    float       dst_vals[] = { 0,0,0,0,0 };

    SkRasterPipeline p;
    p.append(load, load_tail, src_vals);
    p.append(square);
    p.append(store, store_tail, dst_vals);

    p.run(5);

    REPORTER_ASSERT(r, dst_vals[0] ==  1);
    REPORTER_ASSERT(r, dst_vals[1] ==  4);
    REPORTER_ASSERT(r, dst_vals[2] ==  9);
    REPORTER_ASSERT(r, dst_vals[3] == 16);
    REPORTER_ASSERT(r, dst_vals[4] == 25);
}
