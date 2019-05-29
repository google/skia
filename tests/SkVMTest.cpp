/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorPriv.h"
#include "include/private/SkColorData.h"
#include "src/core/SkVM.h"
#include "tests/Test.h"

enum Fmt { A8, G8, RGBA_8888 };
const char* fmt_name(Fmt fmt) {
    switch (fmt) {
        case A8:        return "A8";
        case G8:        return "G8";
        case RGBA_8888: return "RGBA_8888";
    }
    return "";
}

// Here's a cute little trick that avoids the need to explicitly thread
// and skvm::Builder* through and make a lot of builder->foo() calls.
// Instead the builder becomes this, with this-> omitted for clarity.
//
// Think of this as
//    static void srcover(skvm::Builder*, Fmt srcFmt, Fmt dstFmt) { ... }
//
// Some parts of this builder code are written less fluently than possible,
// to avoid any ambiguity of function argument evaluation order.  This lets
// our golden tests (kExpected) work portably.  In general there's no reason
// to fear nesting calls to Builder routines.

struct SrcoverBuilder : public skvm::Builder {
    SrcoverBuilder(Fmt srcFmt, Fmt dstFmt) {
        skvm::Arg src = arg(0),
                  dst = arg(1);

        auto byte_to_f32 = [&](skvm::Val byte) {
            skvm::Val _1_255 = splat(1/255.0f);
            return mul_f32(_1_255, to_f32(byte));
        };

        auto load = [&](skvm::Arg ptr, Fmt fmt,
                        skvm::Val* r, skvm::Val* g, skvm::Val* b, skvm::Val* a) {
            switch (fmt) {
                case A8: {
                    *r = *g = *b = splat(0.0f);
                    *a = byte_to_f32(load8(ptr));
                } break;

                case G8: {
                    *r = *g = *b = byte_to_f32(load8(ptr));
                    *a = splat(1.0f);
                } break;

                case RGBA_8888: {
                    skvm::Val rgba = load32(ptr),
                              _255 = splat(255);
                    *r = byte_to_f32(bit_and(    rgba     , _255));
                    *g = byte_to_f32(bit_and(shr(rgba,  8), _255));
                    *b = byte_to_f32(bit_and(shr(rgba, 16), _255));
                    *a = byte_to_f32(        shr(rgba, 24)       );
                } break;
            }
        };

        skvm::Val r,g,b,a;
        load(src, srcFmt, &r,&g,&b,&a);

        skvm::Val dr,dg,db,da;
        load(dst, dstFmt, &dr,&dg,&db,&da);

        skvm::Val invA = sub_f32(splat(1.0f), a);
        r = mad_f32(dr, invA, r);
        g = mad_f32(dg, invA, g);
        b = mad_f32(db, invA, b);
        a = mad_f32(da, invA, a);

        auto f32_to_byte = [&](skvm::Val f32) {
            skvm::Val _255 = splat(255.0f),
                      _0_5 = splat(0.5f);
            return to_i32(mad_f32(f32, _255, _0_5));
        };
        switch (dstFmt) {
            case A8: {
                store8(dst, f32_to_byte(a));
            } break;

            case G8: {
                skvm::Val _2126 = splat(0.2126f),
                          _7152 = splat(0.7152f),
                          _0722 = splat(0.0722f);
                store8(dst, f32_to_byte(mad_f32(r, _2126,
                                        mad_f32(g, _7152,
                                        mul_f32(b, _0722)))));
            } break;

            case RGBA_8888: {
                r =     f32_to_byte(r)     ;
                g = shl(f32_to_byte(g),  8);
                b = shl(f32_to_byte(b), 16);
                a = shl(f32_to_byte(a), 24);

                r = bit_or(r,g);
                r = bit_or(r,b);
                r = bit_or(r,a);

                store32(dst, r);
            } break;
        }
    }
};

static const char* kExpected[] = {
R"(☠️v1 = splat 0 (0)
 v2 = load8 arg(0)
 v3 = splat 3B808081 (0.0039215689)
 v4 = to_f32 v2
 v5 = mul_f32 v3 v4
 v6 = load8 arg(1)
 v7 = to_f32 v6
 v8 = mul_f32 v3 v7
 v9 = splat 3F800000 (1)
 v10 = sub_f32 v9 v5
☠️v11 = mad_f32 v1 v10 v1
 v12 = mad_f32 v8 v10 v5
 v13 = splat 437F0000 (255)
 v14 = splat 3F000000 (0.5)
 v15 = mad_f32 v12 v13 v14
 v16 = to_i32 v15
 store8 arg(1) v16
)",
R"( v1 = splat 0 (0)
 v2 = load8 arg(0)
 v3 = splat 3B808081 (0.0039215689)
 v4 = to_f32 v2
 v5 = mul_f32 v3 v4
 v6 = load8 arg(1)
 v7 = to_f32 v6
 v8 = mul_f32 v3 v7
 v9 = splat 3F800000 (1)
 v10 = sub_f32 v9 v5
 v11 = mad_f32 v8 v10 v1
☠️v12 = mad_f32 v9 v10 v5
 v13 = splat 3E59B3D0 (0.21259999)
 v14 = splat 3F371759 (0.71520001)
 v15 = splat 3D93DD98 (0.0722)
 v16 = mul_f32 v11 v15
 v17 = mad_f32 v11 v14 v16
 v18 = mad_f32 v11 v13 v17
 v19 = splat 437F0000 (255)
 v20 = splat 3F000000 (0.5)
 v21 = mad_f32 v18 v19 v20
 v22 = to_i32 v21
 store8 arg(1) v22
)",
R"( v1 = splat 0 (0)
 v2 = load8 arg(0)
 v3 = splat 3B808081 (0.0039215689)
 v4 = to_f32 v2
 v5 = mul_f32 v3 v4
 v6 = load32 arg(1)
 v7 = splat FF (3.5733111e-43)
 v8 = bit_and v6 v7
 v9 = to_f32 v8
 v10 = mul_f32 v3 v9
 v11 = shr v6 8 (1.1210388e-44)
 v12 = bit_and v11 v7
 v13 = to_f32 v12
 v14 = mul_f32 v3 v13
 v15 = shr v6 10 (2.2420775e-44)
 v16 = bit_and v15 v7
 v17 = to_f32 v16
 v18 = mul_f32 v3 v17
 v19 = shr v6 18 (3.3631163e-44)
 v20 = to_f32 v19
 v21 = mul_f32 v3 v20
 v22 = splat 3F800000 (1)
 v23 = sub_f32 v22 v5
 v24 = mad_f32 v10 v23 v1
 v25 = mad_f32 v14 v23 v1
 v26 = mad_f32 v18 v23 v1
 v27 = mad_f32 v21 v23 v5
 v28 = splat 437F0000 (255)
 v29 = splat 3F000000 (0.5)
 v30 = mad_f32 v24 v28 v29
 v31 = to_i32 v30
 v32 = mad_f32 v25 v28 v29
 v33 = to_i32 v32
 v34 = shl v33 8 (1.1210388e-44)
 v35 = mad_f32 v26 v28 v29
 v36 = to_i32 v35
 v37 = shl v36 10 (2.2420775e-44)
 v38 = mad_f32 v27 v28 v29
 v39 = to_i32 v38
 v40 = shl v39 18 (3.3631163e-44)
 v41 = bit_or v31 v34
 v42 = bit_or v41 v37
 v43 = bit_or v42 v40
 store32 arg(1) v43
)",
R"(☠️v1 = load8 arg(0)
 v2 = splat 3B808081 (0.0039215689)
☠️v3 = to_f32 v1
☠️v4 = mul_f32 v2 v3
 v5 = splat 3F800000 (1)
☠️v6 = splat 0 (0)
 v7 = load8 arg(1)
 v8 = to_f32 v7
 v9 = mul_f32 v2 v8
 v10 = sub_f32 v5 v5
☠️v11 = mad_f32 v6 v10 v4
 v12 = mad_f32 v9 v10 v5
 v13 = splat 437F0000 (255)
 v14 = splat 3F000000 (0.5)
 v15 = mad_f32 v12 v13 v14
 v16 = to_i32 v15
 store8 arg(1) v16
)",
R"( v1 = load8 arg(0)
 v2 = splat 3B808081 (0.0039215689)
 v3 = to_f32 v1
 v4 = mul_f32 v2 v3
 v5 = splat 3F800000 (1)
 v6 = load8 arg(1)
 v7 = to_f32 v6
 v8 = mul_f32 v2 v7
 v9 = sub_f32 v5 v5
 v10 = mad_f32 v8 v9 v4
☠️v11 = mad_f32 v5 v9 v5
 v12 = splat 3E59B3D0 (0.21259999)
 v13 = splat 3F371759 (0.71520001)
 v14 = splat 3D93DD98 (0.0722)
 v15 = mul_f32 v10 v14
 v16 = mad_f32 v10 v13 v15
 v17 = mad_f32 v10 v12 v16
 v18 = splat 437F0000 (255)
 v19 = splat 3F000000 (0.5)
 v20 = mad_f32 v17 v18 v19
 v21 = to_i32 v20
 store8 arg(1) v21
)",
R"( v1 = load8 arg(0)
 v2 = splat 3B808081 (0.0039215689)
 v3 = to_f32 v1
 v4 = mul_f32 v2 v3
 v5 = splat 3F800000 (1)
 v6 = load32 arg(1)
 v7 = splat FF (3.5733111e-43)
 v8 = bit_and v6 v7
 v9 = to_f32 v8
 v10 = mul_f32 v2 v9
 v11 = shr v6 8 (1.1210388e-44)
 v12 = bit_and v11 v7
 v13 = to_f32 v12
 v14 = mul_f32 v2 v13
 v15 = shr v6 10 (2.2420775e-44)
 v16 = bit_and v15 v7
 v17 = to_f32 v16
 v18 = mul_f32 v2 v17
 v19 = shr v6 18 (3.3631163e-44)
 v20 = to_f32 v19
 v21 = mul_f32 v2 v20
 v22 = sub_f32 v5 v5
 v23 = mad_f32 v10 v22 v4
 v24 = mad_f32 v14 v22 v4
 v25 = mad_f32 v18 v22 v4
 v26 = mad_f32 v21 v22 v5
 v27 = splat 437F0000 (255)
 v28 = splat 3F000000 (0.5)
 v29 = mad_f32 v23 v27 v28
 v30 = to_i32 v29
 v31 = mad_f32 v24 v27 v28
 v32 = to_i32 v31
 v33 = shl v32 8 (1.1210388e-44)
 v34 = mad_f32 v25 v27 v28
 v35 = to_i32 v34
 v36 = shl v35 10 (2.2420775e-44)
 v37 = mad_f32 v26 v27 v28
 v38 = to_i32 v37
 v39 = shl v38 18 (3.3631163e-44)
 v40 = bit_or v30 v33
 v41 = bit_or v40 v36
 v42 = bit_or v41 v39
 store32 arg(1) v42
)",
R"( v1 = load32 arg(0)
☠️v2 = splat FF (3.5733111e-43)
☠️v3 = bit_and v1 v2
 v4 = splat 3B808081 (0.0039215689)
☠️v5 = to_f32 v3
☠️v6 = mul_f32 v4 v5
☠️v7 = shr v1 8 (1.1210388e-44)
☠️v8 = bit_and v7 v2
☠️v9 = to_f32 v8
☠️v10 = mul_f32 v4 v9
☠️v11 = shr v1 10 (2.2420775e-44)
☠️v12 = bit_and v11 v2
☠️v13 = to_f32 v12
☠️v14 = mul_f32 v4 v13
 v15 = shr v1 18 (3.3631163e-44)
 v16 = to_f32 v15
 v17 = mul_f32 v4 v16
☠️v18 = splat 0 (0)
 v19 = load8 arg(1)
 v20 = to_f32 v19
 v21 = mul_f32 v4 v20
 v22 = splat 3F800000 (1)
 v23 = sub_f32 v22 v17
☠️v24 = mad_f32 v18 v23 v6
☠️v25 = mad_f32 v18 v23 v10
☠️v26 = mad_f32 v18 v23 v14
 v27 = mad_f32 v21 v23 v17
 v28 = splat 437F0000 (255)
 v29 = splat 3F000000 (0.5)
 v30 = mad_f32 v27 v28 v29
 v31 = to_i32 v30
 store8 arg(1) v31
)",
R"( v1 = load32 arg(0)
 v2 = splat FF (3.5733111e-43)
 v3 = bit_and v1 v2
 v4 = splat 3B808081 (0.0039215689)
 v5 = to_f32 v3
 v6 = mul_f32 v4 v5
 v7 = shr v1 8 (1.1210388e-44)
 v8 = bit_and v7 v2
 v9 = to_f32 v8
 v10 = mul_f32 v4 v9
 v11 = shr v1 10 (2.2420775e-44)
 v12 = bit_and v11 v2
 v13 = to_f32 v12
 v14 = mul_f32 v4 v13
 v15 = shr v1 18 (3.3631163e-44)
 v16 = to_f32 v15
 v17 = mul_f32 v4 v16
 v18 = load8 arg(1)
 v19 = to_f32 v18
 v20 = mul_f32 v4 v19
 v21 = splat 3F800000 (1)
 v22 = sub_f32 v21 v17
 v23 = mad_f32 v20 v22 v6
 v24 = mad_f32 v20 v22 v10
 v25 = mad_f32 v20 v22 v14
☠️v26 = mad_f32 v21 v22 v17
 v27 = splat 3E59B3D0 (0.21259999)
 v28 = splat 3F371759 (0.71520001)
 v29 = splat 3D93DD98 (0.0722)
 v30 = mul_f32 v25 v29
 v31 = mad_f32 v24 v28 v30
 v32 = mad_f32 v23 v27 v31
 v33 = splat 437F0000 (255)
 v34 = splat 3F000000 (0.5)
 v35 = mad_f32 v32 v33 v34
 v36 = to_i32 v35
 store8 arg(1) v36
)",
R"( v1 = load32 arg(0)
 v2 = splat FF (3.5733111e-43)
 v3 = bit_and v1 v2
 v4 = splat 3B808081 (0.0039215689)
 v5 = to_f32 v3
 v6 = mul_f32 v4 v5
 v7 = shr v1 8 (1.1210388e-44)
 v8 = bit_and v7 v2
 v9 = to_f32 v8
 v10 = mul_f32 v4 v9
 v11 = shr v1 10 (2.2420775e-44)
 v12 = bit_and v11 v2
 v13 = to_f32 v12
 v14 = mul_f32 v4 v13
 v15 = shr v1 18 (3.3631163e-44)
 v16 = to_f32 v15
 v17 = mul_f32 v4 v16
 v18 = load32 arg(1)
 v19 = bit_and v18 v2
 v20 = to_f32 v19
 v21 = mul_f32 v4 v20
 v22 = shr v18 8 (1.1210388e-44)
 v23 = bit_and v22 v2
 v24 = to_f32 v23
 v25 = mul_f32 v4 v24
 v26 = shr v18 10 (2.2420775e-44)
 v27 = bit_and v26 v2
 v28 = to_f32 v27
 v29 = mul_f32 v4 v28
 v30 = shr v18 18 (3.3631163e-44)
 v31 = to_f32 v30
 v32 = mul_f32 v4 v31
 v33 = splat 3F800000 (1)
 v34 = sub_f32 v33 v17
 v35 = mad_f32 v21 v34 v6
 v36 = mad_f32 v25 v34 v10
 v37 = mad_f32 v29 v34 v14
 v38 = mad_f32 v32 v34 v17
 v39 = splat 437F0000 (255)
 v40 = splat 3F000000 (0.5)
 v41 = mad_f32 v35 v39 v40
 v42 = to_i32 v41
 v43 = mad_f32 v36 v39 v40
 v44 = to_i32 v43
 v45 = shl v44 8 (1.1210388e-44)
 v46 = mad_f32 v37 v39 v40
 v47 = to_i32 v46
 v48 = shl v47 10 (2.2420775e-44)
 v49 = mad_f32 v38 v39 v40
 v50 = to_i32 v49
 v51 = shl v50 18 (3.3631163e-44)
 v52 = bit_or v42 v45
 v53 = bit_or v52 v48
 v54 = bit_or v53 v51
 store32 arg(1) v54
)",
};

DEF_TEST(SkVM, r) {
    for (int s = 0; s < 3; s++)
    for (int d = 0; d < 3; d++) {
        auto srcFmt = (Fmt)s,
             dstFmt = (Fmt)d;
        skvm::Program program = SrcoverBuilder{srcFmt, dstFmt}.done();

        SkDynamicMemoryWStream buf;
        program.dump(&buf);
        sk_sp<SkData> blob = buf.detachAsData();

        bool train = false;
        if (train) {
            SkDebugf("R\"(%.*s)\",\n", blob->size(), blob->data());
        } else if (0 != memcmp(kExpected[3*s+d], blob->data(), blob->size())) {
            ERRORF(r, "SkVMTest needs retraining.\n");
        }
    }

    {
        skvm::Program program = SrcoverBuilder{RGBA_8888, RGBA_8888}.done();

        uint32_t src = 0xbb007733,
                 dst = 0xffaaccee;
        SkPMColor want = SkPMSrcOver(src, dst);  // 0xff2dad73

        program.eval(1, &src, &dst);

        // dst is probably 0xff2dad72.
        for (int i = 0; i < 4; i++) {
            uint8_t d = dst,
                    w = want;
            REPORTER_ASSERT(r, abs(d-w) < 2);
            dst  >>= 8;
            want >>= 8;
        }
    }

    {
        skvm::Program program = SrcoverBuilder{RGBA_8888, G8}.done();

        uint32_t src = 0xbb007733;
        uint8_t dst = 0x42;
        SkPMColor over = SkPMSrcOver(SkPackARGB32(0xbb, 0x33, 0x77, 0x00), 0xff424242);

        uint8_t want = SkComputeLuminance(SkGetPackedR32(over),
                                          SkGetPackedG32(over),
                                          SkGetPackedB32(over));
        program.eval(1, &src, &dst);

        REPORTER_ASSERT(r, abs(dst-want) < 3);
    }

    {
        skvm::Program program = SrcoverBuilder{A8, A8}.done();

        uint8_t src[256],
                dst[256];
        for (int i = 0; i < 256; i++) {
            src[i] = 255 - i;
            dst[i] = i;
        }

        program.eval(256, src, dst);

        for (int i = 0; i < 256; i++) {
            uint8_t want = SkGetPackedA32(SkPMSrcOver(SkPackARGB32(src[i], 0,0,0),
                                                      SkPackARGB32(     i, 0,0,0)));
            REPORTER_ASSERT(r, abs(dst[i]-want) < 2);
        }
    }
}
