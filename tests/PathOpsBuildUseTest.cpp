/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsTestCommon.h"

#define TEST(name) { name, #name }

static void build1_1(skiatest::Reporter* reporter, const char* filename) {
    SkOpBuilder builder;
    SkPath path;
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x436ae68e), SkBits2Float(0x43adff26));  // 234.901f, 347.993f
path.quadTo(SkBits2Float(0x436ae68e), SkBits2Float(0x43b32ca2), SkBits2Float(0x4363940a), SkBits2Float(0x43b6d5e4));  // 234.901f, 358.349f, 227.578f, 365.671f
path.quadTo(SkBits2Float(0x435c4186), SkBits2Float(0x43ba7f26), SkBits2Float(0x4351e68e), SkBits2Float(0x43ba7f26));  // 220.256f, 372.993f, 209.901f, 372.993f
path.quadTo(SkBits2Float(0x43478b96), SkBits2Float(0x43ba7f26), SkBits2Float(0x43403912), SkBits2Float(0x43b6d5e4));  // 199.545f, 372.993f, 192.223f, 365.671f
path.quadTo(SkBits2Float(0x4338e68e), SkBits2Float(0x43b32ca2), SkBits2Float(0x4338e68e), SkBits2Float(0x43adff26));  // 184.901f, 358.349f, 184.901f, 347.993f
path.quadTo(SkBits2Float(0x4338e68e), SkBits2Float(0x43a8d1aa), SkBits2Float(0x43403912), SkBits2Float(0x43a52868));  // 184.901f, 337.638f, 192.223f, 330.316f
path.quadTo(SkBits2Float(0x43478b96), SkBits2Float(0x43a17f26), SkBits2Float(0x4351e68e), SkBits2Float(0x43a17f26));  // 199.545f, 322.993f, 209.901f, 322.993f
path.quadTo(SkBits2Float(0x435c4186), SkBits2Float(0x43a17f26), SkBits2Float(0x4363940a), SkBits2Float(0x43a52868));  // 220.256f, 322.993f, 227.578f, 330.316f
path.quadTo(SkBits2Float(0x436ae68e), SkBits2Float(0x43a8d1aa), SkBits2Float(0x436ae68e), SkBits2Float(0x43adff26));  // 234.901f, 337.638f, 234.901f, 347.993f
path.close();
    SkPath path0(path);
    builder.add(path0, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ad0aca), SkBits2Float(0x432a0e2c));  // 346.084f, 170.055f
path.quadTo(SkBits2Float(0x43ad0aca), SkBits2Float(0x43346924), SkBits2Float(0x43a96188), SkBits2Float(0x433bbba8));  // 346.084f, 180.411f, 338.762f, 187.733f
path.quadTo(SkBits2Float(0x43a5b846), SkBits2Float(0x43430e2c), SkBits2Float(0x43a08aca), SkBits2Float(0x43430e2c));  // 331.44f, 195.055f, 321.084f, 195.055f
path.quadTo(SkBits2Float(0x439b5d4e), SkBits2Float(0x43430e2c), SkBits2Float(0x4397b40c), SkBits2Float(0x433bbba8));  // 310.729f, 195.055f, 303.407f, 187.733f
path.quadTo(SkBits2Float(0x43940aca), SkBits2Float(0x43346924), SkBits2Float(0x43940aca), SkBits2Float(0x432a0e2c));  // 296.084f, 180.411f, 296.084f, 170.055f
path.quadTo(SkBits2Float(0x43940aca), SkBits2Float(0x431fb334), SkBits2Float(0x4397b40c), SkBits2Float(0x431860b0));  // 296.084f, 159.7f, 303.407f, 152.378f
path.quadTo(SkBits2Float(0x439b5d4e), SkBits2Float(0x43110e2c), SkBits2Float(0x43a08aca), SkBits2Float(0x43110e2c));  // 310.729f, 145.055f, 321.084f, 145.055f
path.quadTo(SkBits2Float(0x43a5b846), SkBits2Float(0x43110e2c), SkBits2Float(0x43a96188), SkBits2Float(0x431860b0));  // 331.44f, 145.055f, 338.762f, 152.378f
path.quadTo(SkBits2Float(0x43ad0aca), SkBits2Float(0x431fb334), SkBits2Float(0x43ad0aca), SkBits2Float(0x432a0e2c));  // 346.084f, 159.7f, 346.084f, 170.055f
path.close();
    SkPath path1(path);
    builder.add(path1, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x431f14f9), SkBits2Float(0x433943fd));  // 159.082f, 185.266f
path.quadTo(SkBits2Float(0x431f14f9), SkBits2Float(0x43439ef4), SkBits2Float(0x4317c275), SkBits2Float(0x434af179));  // 159.082f, 195.621f, 151.76f, 202.943f
path.quadTo(SkBits2Float(0x43106ff0), SkBits2Float(0x435243fd), SkBits2Float(0x430614f9), SkBits2Float(0x435243fd));  // 144.437f, 210.266f, 134.082f, 210.266f
path.quadTo(SkBits2Float(0x42f77403), SkBits2Float(0x435243fd), SkBits2Float(0x42e8cefa), SkBits2Float(0x434af179));  // 123.727f, 210.266f, 116.404f, 202.943f
path.quadTo(SkBits2Float(0x42da29f2), SkBits2Float(0x43439ef4), SkBits2Float(0x42da29f2), SkBits2Float(0x433943fd));  // 109.082f, 195.621f, 109.082f, 185.266f
path.quadTo(SkBits2Float(0x42da29f2), SkBits2Float(0x432ee906), SkBits2Float(0x42e8cefa), SkBits2Float(0x43279681));  // 109.082f, 174.91f, 116.404f, 167.588f
path.quadTo(SkBits2Float(0x42f77403), SkBits2Float(0x432043fd), SkBits2Float(0x430614f9), SkBits2Float(0x432043fd));  // 123.727f, 160.266f, 134.082f, 160.266f
path.quadTo(SkBits2Float(0x43106ff0), SkBits2Float(0x432043fd), SkBits2Float(0x4317c275), SkBits2Float(0x43279681));  // 144.437f, 160.266f, 151.76f, 167.588f
path.quadTo(SkBits2Float(0x431f14f9), SkBits2Float(0x432ee906), SkBits2Float(0x431f14f9), SkBits2Float(0x433943fd));  // 159.082f, 174.91f, 159.082f, 185.266f
path.close();
    SkPath path2(path);
    builder.add(path2, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x433ad67a), SkBits2Float(0x43abd585));  // 186.838f, 343.668f
path.quadTo(SkBits2Float(0x433ad67a), SkBits2Float(0x43b10301), SkBits2Float(0x433383f6), SkBits2Float(0x43b4ac43));  // 186.838f, 354.023f, 179.515f, 361.346f
path.quadTo(SkBits2Float(0x432c3172), SkBits2Float(0x43b85585), SkBits2Float(0x4321d67a), SkBits2Float(0x43b85585));  // 172.193f, 368.668f, 161.838f, 368.668f
path.quadTo(SkBits2Float(0x43177b82), SkBits2Float(0x43b85585), SkBits2Float(0x431028fe), SkBits2Float(0x43b4ac43));  // 151.482f, 368.668f, 144.16f, 361.346f
path.quadTo(SkBits2Float(0x4308d67a), SkBits2Float(0x43b10301), SkBits2Float(0x4308d67a), SkBits2Float(0x43abd585));  // 136.838f, 354.023f, 136.838f, 343.668f
path.quadTo(SkBits2Float(0x4308d67a), SkBits2Float(0x43a6a809), SkBits2Float(0x431028fe), SkBits2Float(0x43a2fec7));  // 136.838f, 333.313f, 144.16f, 325.99f
path.quadTo(SkBits2Float(0x43177b82), SkBits2Float(0x439f5585), SkBits2Float(0x4321d67a), SkBits2Float(0x439f5585));  // 151.482f, 318.668f, 161.838f, 318.668f
path.quadTo(SkBits2Float(0x432c3172), SkBits2Float(0x439f5585), SkBits2Float(0x433383f6), SkBits2Float(0x43a2fec7));  // 172.193f, 318.668f, 179.515f, 325.99f
path.quadTo(SkBits2Float(0x433ad67a), SkBits2Float(0x43a6a809), SkBits2Float(0x433ad67a), SkBits2Float(0x43abd585));  // 186.838f, 333.313f, 186.838f, 343.668f
path.close();
    SkPath path3(path);
    builder.add(path3, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43bff91b), SkBits2Float(0x43973a57));  // 383.946f, 302.456f
path.quadTo(SkBits2Float(0x43bff91b), SkBits2Float(0x439c67d3), SkBits2Float(0x43bc4fd9), SkBits2Float(0x43a01115));  // 383.946f, 312.811f, 376.624f, 320.133f
path.quadTo(SkBits2Float(0x43b8a697), SkBits2Float(0x43a3ba57), SkBits2Float(0x43b3791b), SkBits2Float(0x43a3ba57));  // 369.301f, 327.456f, 358.946f, 327.456f
path.quadTo(SkBits2Float(0x43ae4b9f), SkBits2Float(0x43a3ba57), SkBits2Float(0x43aaa25d), SkBits2Float(0x43a01115));  // 348.591f, 327.456f, 341.268f, 320.133f
path.quadTo(SkBits2Float(0x43a6f91b), SkBits2Float(0x439c67d3), SkBits2Float(0x43a6f91b), SkBits2Float(0x43973a57));  // 333.946f, 312.811f, 333.946f, 302.456f
path.quadTo(SkBits2Float(0x43a6f91b), SkBits2Float(0x43920cdb), SkBits2Float(0x43aaa25d), SkBits2Float(0x438e6399));  // 333.946f, 292.1f, 341.268f, 284.778f
path.quadTo(SkBits2Float(0x43ae4b9f), SkBits2Float(0x438aba57), SkBits2Float(0x43b3791b), SkBits2Float(0x438aba57));  // 348.591f, 277.456f, 358.946f, 277.456f
path.quadTo(SkBits2Float(0x43b8a697), SkBits2Float(0x438aba57), SkBits2Float(0x43bc4fd9), SkBits2Float(0x438e6399));  // 369.301f, 277.456f, 376.624f, 284.778f
path.quadTo(SkBits2Float(0x43bff91b), SkBits2Float(0x43920cdb), SkBits2Float(0x43bff91b), SkBits2Float(0x43973a57));  // 383.946f, 292.1f, 383.946f, 302.456f
path.close();
    SkPath path4(path);
    builder.add(path4, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43374c2c), SkBits2Float(0x437e8a30));  // 183.298f, 254.54f
path.quadTo(SkBits2Float(0x43374c2c), SkBits2Float(0x43847294), SkBits2Float(0x432ff9a8), SkBits2Float(0x43881bd6));  // 183.298f, 264.895f, 175.975f, 272.217f
path.quadTo(SkBits2Float(0x4328a724), SkBits2Float(0x438bc518), SkBits2Float(0x431e4c2c), SkBits2Float(0x438bc518));  // 168.653f, 279.54f, 158.298f, 279.54f
path.quadTo(SkBits2Float(0x4313f134), SkBits2Float(0x438bc518), SkBits2Float(0x430c9eb0), SkBits2Float(0x43881bd6));  // 147.942f, 279.54f, 140.62f, 272.217f
path.quadTo(SkBits2Float(0x43054c2c), SkBits2Float(0x43847294), SkBits2Float(0x43054c2c), SkBits2Float(0x437e8a30));  // 133.298f, 264.895f, 133.298f, 254.54f
path.quadTo(SkBits2Float(0x43054c2c), SkBits2Float(0x43742f38), SkBits2Float(0x430c9eb0), SkBits2Float(0x436cdcb4));  // 133.298f, 244.184f, 140.62f, 236.862f
path.quadTo(SkBits2Float(0x4313f134), SkBits2Float(0x43658a30), SkBits2Float(0x431e4c2c), SkBits2Float(0x43658a30));  // 147.942f, 229.54f, 158.298f, 229.54f
path.quadTo(SkBits2Float(0x4328a724), SkBits2Float(0x43658a30), SkBits2Float(0x432ff9a8), SkBits2Float(0x436cdcb4));  // 168.653f, 229.54f, 175.975f, 236.862f
path.quadTo(SkBits2Float(0x43374c2c), SkBits2Float(0x43742f38), SkBits2Float(0x43374c2c), SkBits2Float(0x437e8a30));  // 183.298f, 244.184f, 183.298f, 254.54f
path.close();
    SkPath path5(path);
    builder.add(path5, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x430e01e8), SkBits2Float(0x435c8671));  // 142.007f, 220.525f
path.quadTo(SkBits2Float(0x430e01e8), SkBits2Float(0x4366e168), SkBits2Float(0x4306af64), SkBits2Float(0x436e33ed));  // 142.007f, 230.88f, 134.685f, 238.203f
path.quadTo(SkBits2Float(0x42feb9bf), SkBits2Float(0x43758671), SkBits2Float(0x42ea03d0), SkBits2Float(0x43758671));  // 127.363f, 245.525f, 117.007f, 245.525f
path.quadTo(SkBits2Float(0x42d54de1), SkBits2Float(0x43758671), SkBits2Float(0x42c6a8d8), SkBits2Float(0x436e33ed));  // 106.652f, 245.525f, 99.3298f, 238.203f
path.quadTo(SkBits2Float(0x42b803d0), SkBits2Float(0x4366e168), SkBits2Float(0x42b803d0), SkBits2Float(0x435c8671));  // 92.0074f, 230.88f, 92.0074f, 220.525f
path.quadTo(SkBits2Float(0x42b803d0), SkBits2Float(0x43522b7a), SkBits2Float(0x42c6a8d8), SkBits2Float(0x434ad8f5));  // 92.0074f, 210.17f, 99.3298f, 202.847f
path.quadTo(SkBits2Float(0x42d54de1), SkBits2Float(0x43438671), SkBits2Float(0x42ea03d0), SkBits2Float(0x43438671));  // 106.652f, 195.525f, 117.007f, 195.525f
path.quadTo(SkBits2Float(0x42feb9bf), SkBits2Float(0x43438671), SkBits2Float(0x4306af64), SkBits2Float(0x434ad8f5));  // 127.363f, 195.525f, 134.685f, 202.847f
path.quadTo(SkBits2Float(0x430e01e8), SkBits2Float(0x43522b7a), SkBits2Float(0x430e01e8), SkBits2Float(0x435c8671));  // 142.007f, 210.17f, 142.007f, 220.525f
path.close();
    SkPath path6(path);
    builder.add(path6, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x438b7062), SkBits2Float(0x42d54bf2));  // 278.878f, 106.648f
path.quadTo(SkBits2Float(0x438b7062), SkBits2Float(0x42ea01e1), SkBits2Float(0x4387c720), SkBits2Float(0x42f8a6ea));  // 278.878f, 117.004f, 271.556f, 124.326f
path.quadTo(SkBits2Float(0x43841dde), SkBits2Float(0x4303a5f9), SkBits2Float(0x437de0c4), SkBits2Float(0x4303a5f9));  // 264.233f, 131.648f, 253.878f, 131.648f
path.quadTo(SkBits2Float(0x437385cc), SkBits2Float(0x4303a5f9), SkBits2Float(0x436c3348), SkBits2Float(0x42f8a6ea));  // 243.523f, 131.648f, 236.2f, 124.326f
path.quadTo(SkBits2Float(0x4364e0c3), SkBits2Float(0x42ea01e1), SkBits2Float(0x4364e0c3), SkBits2Float(0x42d54bf2));  // 228.878f, 117.004f, 228.878f, 106.648f
path.quadTo(SkBits2Float(0x4364e0c3), SkBits2Float(0x42c09603), SkBits2Float(0x436c3348), SkBits2Float(0x42b1f0fa));  // 228.878f, 96.293f, 236.2f, 88.9707f
path.quadTo(SkBits2Float(0x437385cc), SkBits2Float(0x42a34bf2), SkBits2Float(0x437de0c4), SkBits2Float(0x42a34bf2));  // 243.523f, 81.6483f, 253.878f, 81.6483f
path.quadTo(SkBits2Float(0x43841dde), SkBits2Float(0x42a34bf2), SkBits2Float(0x4387c720), SkBits2Float(0x42b1f0fa));  // 264.233f, 81.6483f, 271.556f, 88.9707f
path.quadTo(SkBits2Float(0x438b7062), SkBits2Float(0x42c09603), SkBits2Float(0x438b7062), SkBits2Float(0x42d54bf2));  // 278.878f, 96.293f, 278.878f, 106.648f
path.close();
    SkPath path7(path);
    builder.add(path7, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43de3ff6), SkBits2Float(0x43963745));  // 444.5f, 300.432f
path.quadTo(SkBits2Float(0x43de3ff6), SkBits2Float(0x439b64c1), SkBits2Float(0x43da96b4), SkBits2Float(0x439f0e03));  // 444.5f, 310.787f, 437.177f, 318.109f
path.quadTo(SkBits2Float(0x43d6ed72), SkBits2Float(0x43a2b745), SkBits2Float(0x43d1bff6), SkBits2Float(0x43a2b745));  // 429.855f, 325.432f, 419.5f, 325.432f
path.quadTo(SkBits2Float(0x43cc927a), SkBits2Float(0x43a2b745), SkBits2Float(0x43c8e938), SkBits2Float(0x439f0e03));  // 409.144f, 325.432f, 401.822f, 318.109f
path.quadTo(SkBits2Float(0x43c53ff6), SkBits2Float(0x439b64c1), SkBits2Float(0x43c53ff6), SkBits2Float(0x43963745));  // 394.5f, 310.787f, 394.5f, 300.432f
path.quadTo(SkBits2Float(0x43c53ff6), SkBits2Float(0x439109c9), SkBits2Float(0x43c8e938), SkBits2Float(0x438d6087));  // 394.5f, 290.076f, 401.822f, 282.754f
path.quadTo(SkBits2Float(0x43cc927a), SkBits2Float(0x4389b745), SkBits2Float(0x43d1bff6), SkBits2Float(0x4389b745));  // 409.144f, 275.432f, 419.5f, 275.432f
path.quadTo(SkBits2Float(0x43d6ed72), SkBits2Float(0x4389b745), SkBits2Float(0x43da96b4), SkBits2Float(0x438d6087));  // 429.855f, 275.432f, 437.177f, 282.754f
path.quadTo(SkBits2Float(0x43de3ff6), SkBits2Float(0x439109c9), SkBits2Float(0x43de3ff6), SkBits2Float(0x43963745));  // 444.5f, 290.076f, 444.5f, 300.432f
path.close();
    SkPath path8(path);
    builder.add(path8, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43aae79c), SkBits2Float(0x438d0cbc));  // 341.809f, 282.099f
path.quadTo(SkBits2Float(0x43aae79c), SkBits2Float(0x43923a38), SkBits2Float(0x43a73e5a), SkBits2Float(0x4395e37a));  // 341.809f, 292.455f, 334.487f, 299.777f
path.quadTo(SkBits2Float(0x43a39518), SkBits2Float(0x43998cbc), SkBits2Float(0x439e679c), SkBits2Float(0x43998cbc));  // 327.165f, 307.099f, 316.809f, 307.099f
path.quadTo(SkBits2Float(0x43993a20), SkBits2Float(0x43998cbc), SkBits2Float(0x439590de), SkBits2Float(0x4395e37a));  // 306.454f, 307.099f, 299.132f, 299.777f
path.quadTo(SkBits2Float(0x4391e79c), SkBits2Float(0x43923a38), SkBits2Float(0x4391e79c), SkBits2Float(0x438d0cbc));  // 291.809f, 292.455f, 291.809f, 282.099f
path.quadTo(SkBits2Float(0x4391e79c), SkBits2Float(0x4387df40), SkBits2Float(0x439590de), SkBits2Float(0x438435fe));  // 291.809f, 271.744f, 299.132f, 264.422f
path.quadTo(SkBits2Float(0x43993a20), SkBits2Float(0x43808cbc), SkBits2Float(0x439e679c), SkBits2Float(0x43808cbc));  // 306.454f, 257.099f, 316.809f, 257.099f
path.quadTo(SkBits2Float(0x43a39518), SkBits2Float(0x43808cbc), SkBits2Float(0x43a73e5a), SkBits2Float(0x438435fe));  // 327.165f, 257.099f, 334.487f, 264.422f
path.quadTo(SkBits2Float(0x43aae79c), SkBits2Float(0x4387df40), SkBits2Float(0x43aae79c), SkBits2Float(0x438d0cbc));  // 341.809f, 271.744f, 341.809f, 282.099f
path.close();
    SkPath path9(path);
    builder.add(path9, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4354ce7d), SkBits2Float(0x43842ec9));  // 212.807f, 264.366f
path.quadTo(SkBits2Float(0x4354ce7d), SkBits2Float(0x43895c45), SkBits2Float(0x434d7bf9), SkBits2Float(0x438d0587));  // 212.807f, 274.721f, 205.484f, 282.043f
path.quadTo(SkBits2Float(0x43462974), SkBits2Float(0x4390aec9), SkBits2Float(0x433bce7d), SkBits2Float(0x4390aec9));  // 198.162f, 289.366f, 187.807f, 289.366f
path.quadTo(SkBits2Float(0x43317386), SkBits2Float(0x4390aec9), SkBits2Float(0x432a2101), SkBits2Float(0x438d0587));  // 177.451f, 289.366f, 170.129f, 282.043f
path.quadTo(SkBits2Float(0x4322ce7d), SkBits2Float(0x43895c45), SkBits2Float(0x4322ce7d), SkBits2Float(0x43842ec9));  // 162.807f, 274.721f, 162.807f, 264.366f
path.quadTo(SkBits2Float(0x4322ce7d), SkBits2Float(0x437e029a), SkBits2Float(0x432a2101), SkBits2Float(0x4376b016));  // 162.807f, 254.01f, 170.129f, 246.688f
path.quadTo(SkBits2Float(0x43317386), SkBits2Float(0x436f5d92), SkBits2Float(0x433bce7d), SkBits2Float(0x436f5d92));  // 177.451f, 239.366f, 187.807f, 239.366f
path.quadTo(SkBits2Float(0x43462974), SkBits2Float(0x436f5d92), SkBits2Float(0x434d7bf9), SkBits2Float(0x4376b016));  // 198.162f, 239.366f, 205.484f, 246.688f
path.quadTo(SkBits2Float(0x4354ce7d), SkBits2Float(0x437e029a), SkBits2Float(0x4354ce7d), SkBits2Float(0x43842ec9));  // 212.807f, 254.01f, 212.807f, 264.366f
path.close();
    SkPath path10(path);
    builder.add(path10, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a8c299), SkBits2Float(0x432fce08));  // 337.52f, 175.805f
path.quadTo(SkBits2Float(0x43a8c299), SkBits2Float(0x433a2900), SkBits2Float(0x43a51957), SkBits2Float(0x43417b84));  // 337.52f, 186.16f, 330.198f, 193.482f
path.quadTo(SkBits2Float(0x43a17015), SkBits2Float(0x4348ce08), SkBits2Float(0x439c4299), SkBits2Float(0x4348ce08));  // 322.876f, 200.805f, 312.52f, 200.805f
path.quadTo(SkBits2Float(0x4397151d), SkBits2Float(0x4348ce08), SkBits2Float(0x43936bdb), SkBits2Float(0x43417b84));  // 302.165f, 200.805f, 294.843f, 193.482f
path.quadTo(SkBits2Float(0x438fc299), SkBits2Float(0x433a2900), SkBits2Float(0x438fc299), SkBits2Float(0x432fce08));  // 287.52f, 186.16f, 287.52f, 175.805f
path.quadTo(SkBits2Float(0x438fc299), SkBits2Float(0x43257310), SkBits2Float(0x43936bdb), SkBits2Float(0x431e208c));  // 287.52f, 165.449f, 294.843f, 158.127f
path.quadTo(SkBits2Float(0x4397151d), SkBits2Float(0x4316ce08), SkBits2Float(0x439c4299), SkBits2Float(0x4316ce08));  // 302.165f, 150.805f, 312.52f, 150.805f
path.quadTo(SkBits2Float(0x43a17015), SkBits2Float(0x4316ce08), SkBits2Float(0x43a51957), SkBits2Float(0x431e208c));  // 322.876f, 150.805f, 330.198f, 158.127f
path.quadTo(SkBits2Float(0x43a8c299), SkBits2Float(0x43257310), SkBits2Float(0x43a8c299), SkBits2Float(0x432fce08));  // 337.52f, 165.449f, 337.52f, 175.805f
path.close();
    SkPath path11(path);
    builder.add(path11, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43d7486e), SkBits2Float(0x430ebc47));  // 430.566f, 142.735f
path.quadTo(SkBits2Float(0x43d7486e), SkBits2Float(0x4319173e), SkBits2Float(0x43d39f2c), SkBits2Float(0x432069c3));  // 430.566f, 153.091f, 423.244f, 160.413f
path.quadTo(SkBits2Float(0x43cff5ea), SkBits2Float(0x4327bc47), SkBits2Float(0x43cac86e), SkBits2Float(0x4327bc47));  // 415.921f, 167.735f, 405.566f, 167.735f
path.quadTo(SkBits2Float(0x43c59af2), SkBits2Float(0x4327bc47), SkBits2Float(0x43c1f1b0), SkBits2Float(0x432069c3));  // 395.211f, 167.735f, 387.888f, 160.413f
path.quadTo(SkBits2Float(0x43be486e), SkBits2Float(0x4319173e), SkBits2Float(0x43be486e), SkBits2Float(0x430ebc47));  // 380.566f, 153.091f, 380.566f, 142.735f
path.quadTo(SkBits2Float(0x43be486e), SkBits2Float(0x43046150), SkBits2Float(0x43c1f1b0), SkBits2Float(0x42fa1d96));  // 380.566f, 132.38f, 387.888f, 125.058f
path.quadTo(SkBits2Float(0x43c59af2), SkBits2Float(0x42eb788e), SkBits2Float(0x43cac86e), SkBits2Float(0x42eb788e));  // 395.211f, 117.735f, 405.566f, 117.735f
path.quadTo(SkBits2Float(0x43cff5ea), SkBits2Float(0x42eb788e), SkBits2Float(0x43d39f2c), SkBits2Float(0x42fa1d96));  // 415.921f, 117.735f, 423.244f, 125.058f
path.quadTo(SkBits2Float(0x43d7486e), SkBits2Float(0x43046150), SkBits2Float(0x43d7486e), SkBits2Float(0x430ebc47));  // 430.566f, 132.38f, 430.566f, 142.735f
path.close();
    SkPath path12(path);
    builder.add(path12, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43484ac4), SkBits2Float(0x43421f09));  // 200.292f, 194.121f
path.quadTo(SkBits2Float(0x43484ac4), SkBits2Float(0x434c7a00), SkBits2Float(0x4340f840), SkBits2Float(0x4353cc85));  // 200.292f, 204.477f, 192.97f, 211.799f
path.quadTo(SkBits2Float(0x4339a5bc), SkBits2Float(0x435b1f09), SkBits2Float(0x432f4ac4), SkBits2Float(0x435b1f09));  // 185.647f, 219.121f, 175.292f, 219.121f
path.quadTo(SkBits2Float(0x4324efcc), SkBits2Float(0x435b1f09), SkBits2Float(0x431d9d48), SkBits2Float(0x4353cc85));  // 164.937f, 219.121f, 157.614f, 211.799f
path.quadTo(SkBits2Float(0x43164ac4), SkBits2Float(0x434c7a00), SkBits2Float(0x43164ac4), SkBits2Float(0x43421f09));  // 150.292f, 204.477f, 150.292f, 194.121f
path.quadTo(SkBits2Float(0x43164ac4), SkBits2Float(0x4337c412), SkBits2Float(0x431d9d48), SkBits2Float(0x4330718d));  // 150.292f, 183.766f, 157.614f, 176.444f
path.quadTo(SkBits2Float(0x4324efcc), SkBits2Float(0x43291f09), SkBits2Float(0x432f4ac4), SkBits2Float(0x43291f09));  // 164.937f, 169.121f, 175.292f, 169.121f
path.quadTo(SkBits2Float(0x4339a5bc), SkBits2Float(0x43291f09), SkBits2Float(0x4340f840), SkBits2Float(0x4330718d));  // 185.647f, 169.121f, 192.97f, 176.444f
path.quadTo(SkBits2Float(0x43484ac4), SkBits2Float(0x4337c412), SkBits2Float(0x43484ac4), SkBits2Float(0x43421f09));  // 200.292f, 183.766f, 200.292f, 194.121f
path.close();
    SkPath path13(path);
    builder.add(path13, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4328883e), SkBits2Float(0x42fb0be0));  // 168.532f, 125.523f
path.quadTo(SkBits2Float(0x4328883e), SkBits2Float(0x4307e0e7), SkBits2Float(0x432135ba), SkBits2Float(0x430f336c));  // 168.532f, 135.879f, 161.21f, 143.201f
path.quadTo(SkBits2Float(0x4319e336), SkBits2Float(0x431685f0), SkBits2Float(0x430f883e), SkBits2Float(0x431685f0));  // 153.888f, 150.523f, 143.532f, 150.523f
path.quadTo(SkBits2Float(0x43052d46), SkBits2Float(0x431685f0), SkBits2Float(0x42fbb584), SkBits2Float(0x430f336c));  // 133.177f, 150.523f, 125.855f, 143.201f
path.quadTo(SkBits2Float(0x42ed107c), SkBits2Float(0x4307e0e7), SkBits2Float(0x42ed107c), SkBits2Float(0x42fb0be0));  // 118.532f, 135.879f, 118.532f, 125.523f
path.quadTo(SkBits2Float(0x42ed107c), SkBits2Float(0x42e655f1), SkBits2Float(0x42fbb584), SkBits2Float(0x42d7b0e9));  // 118.532f, 115.168f, 125.855f, 107.846f
path.quadTo(SkBits2Float(0x43052d46), SkBits2Float(0x42c90be1), SkBits2Float(0x430f883e), SkBits2Float(0x42c90be1));  // 133.177f, 100.523f, 143.532f, 100.523f
path.quadTo(SkBits2Float(0x4319e336), SkBits2Float(0x42c90be1), SkBits2Float(0x432135ba), SkBits2Float(0x42d7b0e9));  // 153.888f, 100.523f, 161.21f, 107.846f
path.quadTo(SkBits2Float(0x4328883e), SkBits2Float(0x42e655f1), SkBits2Float(0x4328883e), SkBits2Float(0x42fb0be0));  // 168.532f, 115.168f, 168.532f, 125.523f
path.close();
    SkPath path14(path);
    builder.add(path14, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43b2bff8), SkBits2Float(0x439bb140));  // 357.5f, 311.385f
path.quadTo(SkBits2Float(0x43b2bff8), SkBits2Float(0x43a0debc), SkBits2Float(0x43af16b6), SkBits2Float(0x43a487fe));  // 357.5f, 321.74f, 350.177f, 329.062f
path.quadTo(SkBits2Float(0x43ab6d74), SkBits2Float(0x43a83140), SkBits2Float(0x43a63ff8), SkBits2Float(0x43a83140));  // 342.855f, 336.385f, 332.5f, 336.385f
path.quadTo(SkBits2Float(0x43a1127c), SkBits2Float(0x43a83140), SkBits2Float(0x439d693a), SkBits2Float(0x43a487fe));  // 322.144f, 336.385f, 314.822f, 329.062f
path.quadTo(SkBits2Float(0x4399bff8), SkBits2Float(0x43a0debc), SkBits2Float(0x4399bff8), SkBits2Float(0x439bb140));  // 307.5f, 321.74f, 307.5f, 311.385f
path.quadTo(SkBits2Float(0x4399bff8), SkBits2Float(0x439683c4), SkBits2Float(0x439d693a), SkBits2Float(0x4392da82));  // 307.5f, 301.029f, 314.822f, 293.707f
path.quadTo(SkBits2Float(0x43a1127c), SkBits2Float(0x438f3140), SkBits2Float(0x43a63ff8), SkBits2Float(0x438f3140));  // 322.144f, 286.385f, 332.5f, 286.385f
path.quadTo(SkBits2Float(0x43ab6d74), SkBits2Float(0x438f3140), SkBits2Float(0x43af16b6), SkBits2Float(0x4392da82));  // 342.855f, 286.385f, 350.177f, 293.707f
path.quadTo(SkBits2Float(0x43b2bff8), SkBits2Float(0x439683c4), SkBits2Float(0x43b2bff8), SkBits2Float(0x439bb140));  // 357.5f, 301.029f, 357.5f, 311.385f
path.close();
    SkPath path15(path);
    builder.add(path15, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x435ae426), SkBits2Float(0x4341f066));  // 218.891f, 193.939f
path.quadTo(SkBits2Float(0x435ae426), SkBits2Float(0x434c4b5e), SkBits2Float(0x435391a2), SkBits2Float(0x43539de2));  // 218.891f, 204.294f, 211.569f, 211.617f
path.quadTo(SkBits2Float(0x434c3f1e), SkBits2Float(0x435af066), SkBits2Float(0x4341e426), SkBits2Float(0x435af066));  // 204.247f, 218.939f, 193.891f, 218.939f
path.quadTo(SkBits2Float(0x4337892e), SkBits2Float(0x435af066), SkBits2Float(0x433036aa), SkBits2Float(0x43539de2));  // 183.536f, 218.939f, 176.214f, 211.617f
path.quadTo(SkBits2Float(0x4328e426), SkBits2Float(0x434c4b5e), SkBits2Float(0x4328e426), SkBits2Float(0x4341f066));  // 168.891f, 204.294f, 168.891f, 193.939f
path.quadTo(SkBits2Float(0x4328e426), SkBits2Float(0x4337956e), SkBits2Float(0x433036aa), SkBits2Float(0x433042ea));  // 168.891f, 183.584f, 176.214f, 176.261f
path.quadTo(SkBits2Float(0x4337892e), SkBits2Float(0x4328f066), SkBits2Float(0x4341e426), SkBits2Float(0x4328f066));  // 183.536f, 168.939f, 193.891f, 168.939f
path.quadTo(SkBits2Float(0x434c3f1e), SkBits2Float(0x4328f066), SkBits2Float(0x435391a2), SkBits2Float(0x433042ea));  // 204.247f, 168.939f, 211.569f, 176.261f
path.quadTo(SkBits2Float(0x435ae426), SkBits2Float(0x4337956e), SkBits2Float(0x435ae426), SkBits2Float(0x4341f066));  // 218.891f, 183.584f, 218.891f, 193.939f
path.close();
    SkPath path16(path);
    builder.add(path16, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x439817ba), SkBits2Float(0x42e83ba4));  // 304.185f, 116.116f
path.quadTo(SkBits2Float(0x439817ba), SkBits2Float(0x42fcf193), SkBits2Float(0x43946e78), SkBits2Float(0x4305cb4e));  // 304.185f, 126.472f, 296.863f, 133.794f
path.quadTo(SkBits2Float(0x4390c536), SkBits2Float(0x430d1dd2), SkBits2Float(0x438b97ba), SkBits2Float(0x430d1dd2));  // 289.541f, 141.116f, 279.185f, 141.116f
path.quadTo(SkBits2Float(0x43866a3e), SkBits2Float(0x430d1dd2), SkBits2Float(0x4382c0fc), SkBits2Float(0x4305cb4e));  // 268.83f, 141.116f, 261.508f, 133.794f
path.quadTo(SkBits2Float(0x437e2f74), SkBits2Float(0x42fcf193), SkBits2Float(0x437e2f74), SkBits2Float(0x42e83ba4));  // 254.185f, 126.472f, 254.185f, 116.116f
path.quadTo(SkBits2Float(0x437e2f74), SkBits2Float(0x42d385b5), SkBits2Float(0x4382c0fc), SkBits2Float(0x42c4e0ac));  // 254.185f, 105.761f, 261.508f, 98.4388f
path.quadTo(SkBits2Float(0x43866a3e), SkBits2Float(0x42b63ba4), SkBits2Float(0x438b97ba), SkBits2Float(0x42b63ba4));  // 268.83f, 91.1165f, 279.185f, 91.1165f
path.quadTo(SkBits2Float(0x4390c536), SkBits2Float(0x42b63ba4), SkBits2Float(0x43946e78), SkBits2Float(0x42c4e0ac));  // 289.541f, 91.1165f, 296.863f, 98.4388f
path.quadTo(SkBits2Float(0x439817ba), SkBits2Float(0x42d385b5), SkBits2Float(0x439817ba), SkBits2Float(0x42e83ba4));  // 304.185f, 105.761f, 304.185f, 116.116f
path.close();
    SkPath path17(path);
    builder.add(path17, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4350558c), SkBits2Float(0x4382efb0));  // 208.334f, 261.873f
path.quadTo(SkBits2Float(0x4350558c), SkBits2Float(0x43881d2c), SkBits2Float(0x43490308), SkBits2Float(0x438bc66e));  // 208.334f, 272.228f, 201.012f, 279.55f
path.quadTo(SkBits2Float(0x4341b084), SkBits2Float(0x438f6fb0), SkBits2Float(0x4337558c), SkBits2Float(0x438f6fb0));  // 193.69f, 286.873f, 183.334f, 286.873f
path.quadTo(SkBits2Float(0x432cfa94), SkBits2Float(0x438f6fb0), SkBits2Float(0x4325a810), SkBits2Float(0x438bc66e));  // 172.979f, 286.873f, 165.656f, 279.55f
path.quadTo(SkBits2Float(0x431e558c), SkBits2Float(0x43881d2c), SkBits2Float(0x431e558c), SkBits2Float(0x4382efb0));  // 158.334f, 272.228f, 158.334f, 261.873f
path.quadTo(SkBits2Float(0x431e558c), SkBits2Float(0x437b8468), SkBits2Float(0x4325a810), SkBits2Float(0x437431e4));  // 158.334f, 251.517f, 165.656f, 244.195f
path.quadTo(SkBits2Float(0x432cfa94), SkBits2Float(0x436cdf60), SkBits2Float(0x4337558c), SkBits2Float(0x436cdf60));  // 172.979f, 236.873f, 183.334f, 236.873f
path.quadTo(SkBits2Float(0x4341b084), SkBits2Float(0x436cdf60), SkBits2Float(0x43490308), SkBits2Float(0x437431e4));  // 193.69f, 236.873f, 201.012f, 244.195f
path.quadTo(SkBits2Float(0x4350558c), SkBits2Float(0x437b8468), SkBits2Float(0x4350558c), SkBits2Float(0x4382efb0));  // 208.334f, 251.517f, 208.334f, 261.873f
path.close();
    SkPath path18(path);
    builder.add(path18, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a8ec1a), SkBits2Float(0x43a51083));  // 84.4611f, 330.129f
path.quadTo(SkBits2Float(0x42a8ec1a), SkBits2Float(0x43aa3dff), SkBits2Float(0x429a4711), SkBits2Float(0x43ade741));  // 84.4611f, 340.484f, 77.1388f, 347.807f
path.quadTo(SkBits2Float(0x428ba209), SkBits2Float(0x43b19083), SkBits2Float(0x426dd834), SkBits2Float(0x43b19083));  // 69.8165f, 355.129f, 59.4611f, 355.129f
path.quadTo(SkBits2Float(0x42446c56), SkBits2Float(0x43b19083), SkBits2Float(0x42272246), SkBits2Float(0x43ade741));  // 49.1058f, 355.129f, 41.7835f, 347.807f
path.quadTo(SkBits2Float(0x4209d835), SkBits2Float(0x43aa3dff), SkBits2Float(0x4209d835), SkBits2Float(0x43a51083));  // 34.4611f, 340.484f, 34.4611f, 330.129f
path.quadTo(SkBits2Float(0x4209d835), SkBits2Float(0x439fe307), SkBits2Float(0x42272246), SkBits2Float(0x439c39c5));  // 34.4611f, 319.774f, 41.7835f, 312.451f
path.quadTo(SkBits2Float(0x42446c56), SkBits2Float(0x43989083), SkBits2Float(0x426dd834), SkBits2Float(0x43989083));  // 49.1058f, 305.129f, 59.4611f, 305.129f
path.quadTo(SkBits2Float(0x428ba209), SkBits2Float(0x43989083), SkBits2Float(0x429a4711), SkBits2Float(0x439c39c5));  // 69.8165f, 305.129f, 77.1388f, 312.451f
path.quadTo(SkBits2Float(0x42a8ec1a), SkBits2Float(0x439fe307), SkBits2Float(0x42a8ec1a), SkBits2Float(0x43a51083));  // 84.4611f, 319.774f, 84.4611f, 330.129f
path.close();
    SkPath path19(path);
    builder.add(path19, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43aca5fa), SkBits2Float(0x438f0f1d));  // 345.297f, 286.118f
path.quadTo(SkBits2Float(0x43aca5fa), SkBits2Float(0x43943c99), SkBits2Float(0x43a8fcb8), SkBits2Float(0x4397e5db));  // 345.297f, 296.473f, 337.974f, 303.796f
path.quadTo(SkBits2Float(0x43a55376), SkBits2Float(0x439b8f1d), SkBits2Float(0x43a025fa), SkBits2Float(0x439b8f1d));  // 330.652f, 311.118f, 320.297f, 311.118f
path.quadTo(SkBits2Float(0x439af87e), SkBits2Float(0x439b8f1d), SkBits2Float(0x43974f3c), SkBits2Float(0x4397e5db));  // 309.941f, 311.118f, 302.619f, 303.796f
path.quadTo(SkBits2Float(0x4393a5fa), SkBits2Float(0x43943c99), SkBits2Float(0x4393a5fa), SkBits2Float(0x438f0f1d));  // 295.297f, 296.473f, 295.297f, 286.118f
path.quadTo(SkBits2Float(0x4393a5fa), SkBits2Float(0x4389e1a1), SkBits2Float(0x43974f3c), SkBits2Float(0x4386385f));  // 295.297f, 275.763f, 302.619f, 268.44f
path.quadTo(SkBits2Float(0x439af87e), SkBits2Float(0x43828f1d), SkBits2Float(0x43a025fa), SkBits2Float(0x43828f1d));  // 309.941f, 261.118f, 320.297f, 261.118f
path.quadTo(SkBits2Float(0x43a55376), SkBits2Float(0x43828f1d), SkBits2Float(0x43a8fcb8), SkBits2Float(0x4386385f));  // 330.652f, 261.118f, 337.974f, 268.44f
path.quadTo(SkBits2Float(0x43aca5fa), SkBits2Float(0x4389e1a1), SkBits2Float(0x43aca5fa), SkBits2Float(0x438f0f1d));  // 345.297f, 275.763f, 345.297f, 286.118f
path.close();
    SkPath path20(path);
    builder.add(path20, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43846cfc), SkBits2Float(0x431b61f0));  // 264.851f, 155.383f
path.quadTo(SkBits2Float(0x43846cfc), SkBits2Float(0x4325bce8), SkBits2Float(0x4380c3ba), SkBits2Float(0x432d0f6c));  // 264.851f, 165.738f, 257.529f, 173.06f
path.quadTo(SkBits2Float(0x437a34f0), SkBits2Float(0x433461f0), SkBits2Float(0x436fd9f8), SkBits2Float(0x433461f0));  // 250.207f, 180.383f, 239.851f, 180.383f
path.quadTo(SkBits2Float(0x43657f00), SkBits2Float(0x433461f0), SkBits2Float(0x435e2c7c), SkBits2Float(0x432d0f6c));  // 229.496f, 180.383f, 222.174f, 173.06f
path.quadTo(SkBits2Float(0x4356d9f7), SkBits2Float(0x4325bce8), SkBits2Float(0x4356d9f7), SkBits2Float(0x431b61f0));  // 214.851f, 165.738f, 214.851f, 155.383f
path.quadTo(SkBits2Float(0x4356d9f7), SkBits2Float(0x431106f8), SkBits2Float(0x435e2c7c), SkBits2Float(0x4309b474));  // 214.851f, 145.027f, 222.174f, 137.705f
path.quadTo(SkBits2Float(0x43657f00), SkBits2Float(0x430261f0), SkBits2Float(0x436fd9f8), SkBits2Float(0x430261f0));  // 229.496f, 130.383f, 239.851f, 130.383f
path.quadTo(SkBits2Float(0x437a34f0), SkBits2Float(0x430261f0), SkBits2Float(0x4380c3ba), SkBits2Float(0x4309b474));  // 250.207f, 130.383f, 257.529f, 137.705f
path.quadTo(SkBits2Float(0x43846cfc), SkBits2Float(0x431106f8), SkBits2Float(0x43846cfc), SkBits2Float(0x431b61f0));  // 264.851f, 145.027f, 264.851f, 155.383f
path.close();
    SkPath path21(path);
    builder.add(path21, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x439597be), SkBits2Float(0x438dc3e1));  // 299.185f, 283.53f
path.quadTo(SkBits2Float(0x439597be), SkBits2Float(0x4392f15d), SkBits2Float(0x4391ee7c), SkBits2Float(0x43969a9f));  // 299.185f, 293.886f, 291.863f, 301.208f
path.quadTo(SkBits2Float(0x438e453a), SkBits2Float(0x439a43e1), SkBits2Float(0x438917be), SkBits2Float(0x439a43e1));  // 284.541f, 308.53f, 274.185f, 308.53f
path.quadTo(SkBits2Float(0x4383ea42), SkBits2Float(0x439a43e1), SkBits2Float(0x43804100), SkBits2Float(0x43969a9f));  // 263.83f, 308.53f, 256.508f, 301.208f
path.quadTo(SkBits2Float(0x43792f7c), SkBits2Float(0x4392f15d), SkBits2Float(0x43792f7c), SkBits2Float(0x438dc3e1));  // 249.185f, 293.886f, 249.185f, 283.53f
path.quadTo(SkBits2Float(0x43792f7c), SkBits2Float(0x43889665), SkBits2Float(0x43804100), SkBits2Float(0x4384ed23));  // 249.185f, 273.175f, 256.508f, 265.853f
path.quadTo(SkBits2Float(0x4383ea42), SkBits2Float(0x438143e1), SkBits2Float(0x438917be), SkBits2Float(0x438143e1));  // 263.83f, 258.53f, 274.185f, 258.53f
path.quadTo(SkBits2Float(0x438e453a), SkBits2Float(0x438143e1), SkBits2Float(0x4391ee7c), SkBits2Float(0x4384ed23));  // 284.541f, 258.53f, 291.863f, 265.853f
path.quadTo(SkBits2Float(0x439597be), SkBits2Float(0x43889665), SkBits2Float(0x439597be), SkBits2Float(0x438dc3e1));  // 299.185f, 273.175f, 299.185f, 283.53f
path.close();
    SkPath path22(path);
    builder.add(path22, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42db7a2b), SkBits2Float(0x43699568));  // 109.739f, 233.584f
path.quadTo(SkBits2Float(0x42db7a2b), SkBits2Float(0x4373f05f), SkBits2Float(0x42ccd522), SkBits2Float(0x437b42e3));  // 109.739f, 243.939f, 102.416f, 251.261f
path.quadTo(SkBits2Float(0x42be301a), SkBits2Float(0x43814ab4), SkBits2Float(0x42a97a2b), SkBits2Float(0x43814ab4));  // 95.0939f, 258.584f, 84.7386f, 258.584f
path.quadTo(SkBits2Float(0x4294c43c), SkBits2Float(0x43814ab4), SkBits2Float(0x42861f34), SkBits2Float(0x437b42e3));  // 74.3833f, 258.584f, 67.0609f, 251.261f
path.quadTo(SkBits2Float(0x426ef456), SkBits2Float(0x4373f05f), SkBits2Float(0x426ef456), SkBits2Float(0x43699568));  // 59.7386f, 243.939f, 59.7386f, 233.584f
path.quadTo(SkBits2Float(0x426ef456), SkBits2Float(0x435f3a71), SkBits2Float(0x42861f34), SkBits2Float(0x4357e7ed));  // 59.7386f, 223.228f, 67.0609f, 215.906f
path.quadTo(SkBits2Float(0x4294c43c), SkBits2Float(0x43509569), SkBits2Float(0x42a97a2b), SkBits2Float(0x43509569));  // 74.3833f, 208.584f, 84.7386f, 208.584f
path.quadTo(SkBits2Float(0x42be301a), SkBits2Float(0x43509569), SkBits2Float(0x42ccd522), SkBits2Float(0x4357e7ed));  // 95.0939f, 208.584f, 102.416f, 215.906f
path.quadTo(SkBits2Float(0x42db7a2b), SkBits2Float(0x435f3a71), SkBits2Float(0x42db7a2b), SkBits2Float(0x43699568));  // 109.739f, 223.228f, 109.739f, 233.584f
path.close();
    SkPath path23(path);
    builder.add(path23, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x433bea80), SkBits2Float(0x43861662));  // 187.916f, 268.175f
path.quadTo(SkBits2Float(0x433bea80), SkBits2Float(0x438b43de), SkBits2Float(0x433497fc), SkBits2Float(0x438eed20));  // 187.916f, 278.53f, 180.594f, 285.853f
path.quadTo(SkBits2Float(0x432d4578), SkBits2Float(0x43929662), SkBits2Float(0x4322ea80), SkBits2Float(0x43929662));  // 173.271f, 293.175f, 162.916f, 293.175f
path.quadTo(SkBits2Float(0x43188f88), SkBits2Float(0x43929662), SkBits2Float(0x43113d04), SkBits2Float(0x438eed20));  // 152.561f, 293.175f, 145.238f, 285.853f
path.quadTo(SkBits2Float(0x4309ea80), SkBits2Float(0x438b43de), SkBits2Float(0x4309ea80), SkBits2Float(0x43861662));  // 137.916f, 278.53f, 137.916f, 268.175f
path.quadTo(SkBits2Float(0x4309ea80), SkBits2Float(0x4380e8e6), SkBits2Float(0x43113d04), SkBits2Float(0x437a7f48));  // 137.916f, 257.82f, 145.238f, 250.497f
path.quadTo(SkBits2Float(0x43188f88), SkBits2Float(0x43732cc4), SkBits2Float(0x4322ea80), SkBits2Float(0x43732cc4));  // 152.561f, 243.175f, 162.916f, 243.175f
path.quadTo(SkBits2Float(0x432d4578), SkBits2Float(0x43732cc4), SkBits2Float(0x433497fc), SkBits2Float(0x437a7f48));  // 173.271f, 243.175f, 180.594f, 250.497f
path.quadTo(SkBits2Float(0x433bea80), SkBits2Float(0x4380e8e6), SkBits2Float(0x433bea80), SkBits2Float(0x43861662));  // 187.916f, 257.82f, 187.916f, 268.175f
path.close();
    SkPath path24(path);
    builder.add(path24, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4386aaee), SkBits2Float(0x43991356));  // 269.335f, 306.151f
path.quadTo(SkBits2Float(0x4386aaee), SkBits2Float(0x439e40d2), SkBits2Float(0x438301ac), SkBits2Float(0x43a1ea14));  // 269.335f, 316.506f, 262.013f, 323.829f
path.quadTo(SkBits2Float(0x437eb0d4), SkBits2Float(0x43a59356), SkBits2Float(0x437455dc), SkBits2Float(0x43a59356));  // 254.691f, 331.151f, 244.335f, 331.151f
path.quadTo(SkBits2Float(0x4369fae4), SkBits2Float(0x43a59356), SkBits2Float(0x4362a860), SkBits2Float(0x43a1ea14));  // 233.98f, 331.151f, 226.658f, 323.829f
path.quadTo(SkBits2Float(0x435b55dc), SkBits2Float(0x439e40d2), SkBits2Float(0x435b55dc), SkBits2Float(0x43991356));  // 219.335f, 316.506f, 219.335f, 306.151f
path.quadTo(SkBits2Float(0x435b55dc), SkBits2Float(0x4393e5da), SkBits2Float(0x4362a860), SkBits2Float(0x43903c98));  // 219.335f, 295.796f, 226.658f, 288.473f
path.quadTo(SkBits2Float(0x4369fae4), SkBits2Float(0x438c9356), SkBits2Float(0x437455dc), SkBits2Float(0x438c9356));  // 233.98f, 281.151f, 244.335f, 281.151f
path.quadTo(SkBits2Float(0x437eb0d4), SkBits2Float(0x438c9356), SkBits2Float(0x438301ac), SkBits2Float(0x43903c98));  // 254.691f, 281.151f, 262.013f, 288.473f
path.quadTo(SkBits2Float(0x4386aaee), SkBits2Float(0x4393e5da), SkBits2Float(0x4386aaee), SkBits2Float(0x43991356));  // 269.335f, 295.796f, 269.335f, 306.151f
path.close();
    SkPath path25(path);
    builder.add(path25, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43b0a7ab), SkBits2Float(0x42d6b25c));  // 353.31f, 107.348f
path.quadTo(SkBits2Float(0x43b0a7ab), SkBits2Float(0x42eb684b), SkBits2Float(0x43acfe69), SkBits2Float(0x42fa0d53));  // 353.31f, 117.704f, 345.988f, 125.026f
path.quadTo(SkBits2Float(0x43a95527), SkBits2Float(0x4304592e), SkBits2Float(0x43a427ab), SkBits2Float(0x4304592e));  // 338.665f, 132.348f, 328.31f, 132.348f
path.quadTo(SkBits2Float(0x439efa2f), SkBits2Float(0x4304592e), SkBits2Float(0x439b50ed), SkBits2Float(0x42fa0d53));  // 317.955f, 132.348f, 310.632f, 125.026f
path.quadTo(SkBits2Float(0x4397a7ab), SkBits2Float(0x42eb684b), SkBits2Float(0x4397a7ab), SkBits2Float(0x42d6b25c));  // 303.31f, 117.704f, 303.31f, 107.348f
path.quadTo(SkBits2Float(0x4397a7ab), SkBits2Float(0x42c1fc6d), SkBits2Float(0x439b50ed), SkBits2Float(0x42b35765));  // 303.31f, 96.993f, 310.632f, 89.6707f
path.quadTo(SkBits2Float(0x439efa2f), SkBits2Float(0x42a4b25d), SkBits2Float(0x43a427ab), SkBits2Float(0x42a4b25d));  // 317.955f, 82.3484f, 328.31f, 82.3484f
path.quadTo(SkBits2Float(0x43a95527), SkBits2Float(0x42a4b25d), SkBits2Float(0x43acfe69), SkBits2Float(0x42b35765));  // 338.665f, 82.3484f, 345.988f, 89.6707f
path.quadTo(SkBits2Float(0x43b0a7ab), SkBits2Float(0x42c1fc6d), SkBits2Float(0x43b0a7ab), SkBits2Float(0x42d6b25c));  // 353.31f, 96.993f, 353.31f, 107.348f
path.close();
    SkPath path26(path);
    builder.add(path26, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43d478db), SkBits2Float(0x4301c45a));  // 424.944f, 129.767f
path.quadTo(SkBits2Float(0x43d478db), SkBits2Float(0x430c1f52), SkBits2Float(0x43d0cf99), SkBits2Float(0x431371d6));  // 424.944f, 140.122f, 417.622f, 147.445f
path.quadTo(SkBits2Float(0x43cd2657), SkBits2Float(0x431ac45a), SkBits2Float(0x43c7f8db), SkBits2Float(0x431ac45a));  // 410.3f, 154.767f, 399.944f, 154.767f
path.quadTo(SkBits2Float(0x43c2cb5f), SkBits2Float(0x431ac45a), SkBits2Float(0x43bf221d), SkBits2Float(0x431371d6));  // 389.589f, 154.767f, 382.267f, 147.445f
path.quadTo(SkBits2Float(0x43bb78db), SkBits2Float(0x430c1f52), SkBits2Float(0x43bb78db), SkBits2Float(0x4301c45a));  // 374.944f, 140.122f, 374.944f, 129.767f
path.quadTo(SkBits2Float(0x43bb78db), SkBits2Float(0x42eed2c5), SkBits2Float(0x43bf221d), SkBits2Float(0x42e02dbc));  // 374.944f, 119.412f, 382.267f, 112.089f
path.quadTo(SkBits2Float(0x43c2cb5f), SkBits2Float(0x42d188b4), SkBits2Float(0x43c7f8db), SkBits2Float(0x42d188b4));  // 389.589f, 104.767f, 399.944f, 104.767f
path.quadTo(SkBits2Float(0x43cd2657), SkBits2Float(0x42d188b4), SkBits2Float(0x43d0cf99), SkBits2Float(0x42e02dbc));  // 410.3f, 104.767f, 417.622f, 112.089f
path.quadTo(SkBits2Float(0x43d478db), SkBits2Float(0x42eed2c5), SkBits2Float(0x43d478db), SkBits2Float(0x4301c45a));  // 424.944f, 119.412f, 424.944f, 129.767f
path.close();
    SkPath path27(path);
    builder.add(path27, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4370d681), SkBits2Float(0x4375fc44));  // 240.838f, 245.985f
path.quadTo(SkBits2Float(0x4370d681), SkBits2Float(0x43802b9e), SkBits2Float(0x436983fd), SkBits2Float(0x4383d4e0));  // 240.838f, 256.341f, 233.516f, 263.663f
path.quadTo(SkBits2Float(0x43623178), SkBits2Float(0x43877e22), SkBits2Float(0x4357d681), SkBits2Float(0x43877e22));  // 226.193f, 270.985f, 215.838f, 270.985f
path.quadTo(SkBits2Float(0x434d7b8a), SkBits2Float(0x43877e22), SkBits2Float(0x43462905), SkBits2Float(0x4383d4e0));  // 205.483f, 270.985f, 198.16f, 263.663f
path.quadTo(SkBits2Float(0x433ed681), SkBits2Float(0x43802b9e), SkBits2Float(0x433ed681), SkBits2Float(0x4375fc44));  // 190.838f, 256.341f, 190.838f, 245.985f
path.quadTo(SkBits2Float(0x433ed681), SkBits2Float(0x436ba14d), SkBits2Float(0x43462905), SkBits2Float(0x43644ec9));  // 190.838f, 235.63f, 198.16f, 228.308f
path.quadTo(SkBits2Float(0x434d7b8a), SkBits2Float(0x435cfc45), SkBits2Float(0x4357d681), SkBits2Float(0x435cfc45));  // 205.483f, 220.985f, 215.838f, 220.985f
path.quadTo(SkBits2Float(0x43623178), SkBits2Float(0x435cfc45), SkBits2Float(0x436983fd), SkBits2Float(0x43644ec9));  // 226.193f, 220.985f, 233.516f, 228.308f
path.quadTo(SkBits2Float(0x4370d681), SkBits2Float(0x436ba14d), SkBits2Float(0x4370d681), SkBits2Float(0x4375fc44));  // 240.838f, 235.63f, 240.838f, 245.985f
path.close();
    SkPath path28(path);
    builder.add(path28, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43883c1d), SkBits2Float(0x438a9227));  // 272.47f, 277.142f
path.quadTo(SkBits2Float(0x43883c1d), SkBits2Float(0x438fbfa3), SkBits2Float(0x438492db), SkBits2Float(0x439368e5));  // 272.47f, 287.497f, 265.147f, 294.819f
path.quadTo(SkBits2Float(0x4380e999), SkBits2Float(0x43971227), SkBits2Float(0x4377783a), SkBits2Float(0x43971227));  // 257.825f, 302.142f, 247.47f, 302.142f
path.quadTo(SkBits2Float(0x436d1d42), SkBits2Float(0x43971227), SkBits2Float(0x4365cabe), SkBits2Float(0x439368e5));  // 237.114f, 302.142f, 229.792f, 294.819f
path.quadTo(SkBits2Float(0x435e783a), SkBits2Float(0x438fbfa3), SkBits2Float(0x435e783a), SkBits2Float(0x438a9227));  // 222.47f, 287.497f, 222.47f, 277.142f
path.quadTo(SkBits2Float(0x435e783a), SkBits2Float(0x438564ab), SkBits2Float(0x4365cabe), SkBits2Float(0x4381bb69));  // 222.47f, 266.786f, 229.792f, 259.464f
path.quadTo(SkBits2Float(0x436d1d42), SkBits2Float(0x437c244e), SkBits2Float(0x4377783a), SkBits2Float(0x437c244e));  // 237.114f, 252.142f, 247.47f, 252.142f
path.quadTo(SkBits2Float(0x4380e999), SkBits2Float(0x437c244e), SkBits2Float(0x438492db), SkBits2Float(0x4381bb69));  // 257.825f, 252.142f, 265.147f, 259.464f
path.quadTo(SkBits2Float(0x43883c1d), SkBits2Float(0x438564ab), SkBits2Float(0x43883c1d), SkBits2Float(0x438a9227));  // 272.47f, 266.786f, 272.47f, 277.142f
path.close();
    SkPath path29(path);
    builder.add(path29, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43b5b3a4), SkBits2Float(0x43a2dbb0));  // 363.403f, 325.716f
path.quadTo(SkBits2Float(0x43b5b3a4), SkBits2Float(0x43a8092c), SkBits2Float(0x43b20a62), SkBits2Float(0x43abb26e));  // 363.403f, 336.072f, 356.081f, 343.394f
path.quadTo(SkBits2Float(0x43ae6120), SkBits2Float(0x43af5bb0), SkBits2Float(0x43a933a4), SkBits2Float(0x43af5bb0));  // 348.759f, 350.716f, 338.403f, 350.716f
path.quadTo(SkBits2Float(0x43a40628), SkBits2Float(0x43af5bb0), SkBits2Float(0x43a05ce6), SkBits2Float(0x43abb26e));  // 328.048f, 350.716f, 320.726f, 343.394f
path.quadTo(SkBits2Float(0x439cb3a4), SkBits2Float(0x43a8092c), SkBits2Float(0x439cb3a4), SkBits2Float(0x43a2dbb0));  // 313.403f, 336.072f, 313.403f, 325.716f
path.quadTo(SkBits2Float(0x439cb3a4), SkBits2Float(0x439dae34), SkBits2Float(0x43a05ce6), SkBits2Float(0x439a04f2));  // 313.403f, 315.361f, 320.726f, 308.039f
path.quadTo(SkBits2Float(0x43a40628), SkBits2Float(0x43965bb0), SkBits2Float(0x43a933a4), SkBits2Float(0x43965bb0));  // 328.048f, 300.716f, 338.403f, 300.716f
path.quadTo(SkBits2Float(0x43ae6120), SkBits2Float(0x43965bb0), SkBits2Float(0x43b20a62), SkBits2Float(0x439a04f2));  // 348.759f, 300.716f, 356.081f, 308.039f
path.quadTo(SkBits2Float(0x43b5b3a4), SkBits2Float(0x439dae34), SkBits2Float(0x43b5b3a4), SkBits2Float(0x43a2dbb0));  // 363.403f, 315.361f, 363.403f, 325.716f
path.close();
    SkPath path30(path);
    builder.add(path30, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a81cf4), SkBits2Float(0x431b2abc));  // 336.226f, 155.167f
path.quadTo(SkBits2Float(0x43a81cf4), SkBits2Float(0x432585b4), SkBits2Float(0x43a473b2), SkBits2Float(0x432cd838));  // 336.226f, 165.522f, 328.904f, 172.845f
path.quadTo(SkBits2Float(0x43a0ca70), SkBits2Float(0x43342abc), SkBits2Float(0x439b9cf4), SkBits2Float(0x43342abc));  // 321.582f, 180.167f, 311.226f, 180.167f
path.quadTo(SkBits2Float(0x43966f78), SkBits2Float(0x43342abc), SkBits2Float(0x4392c636), SkBits2Float(0x432cd838));  // 300.871f, 180.167f, 293.549f, 172.845f
path.quadTo(SkBits2Float(0x438f1cf4), SkBits2Float(0x432585b4), SkBits2Float(0x438f1cf4), SkBits2Float(0x431b2abc));  // 286.226f, 165.522f, 286.226f, 155.167f
path.quadTo(SkBits2Float(0x438f1cf4), SkBits2Float(0x4310cfc4), SkBits2Float(0x4392c636), SkBits2Float(0x43097d40));  // 286.226f, 144.812f, 293.549f, 137.489f
path.quadTo(SkBits2Float(0x43966f78), SkBits2Float(0x43022abc), SkBits2Float(0x439b9cf4), SkBits2Float(0x43022abc));  // 300.871f, 130.167f, 311.226f, 130.167f
path.quadTo(SkBits2Float(0x43a0ca70), SkBits2Float(0x43022abc), SkBits2Float(0x43a473b2), SkBits2Float(0x43097d40));  // 321.582f, 130.167f, 328.904f, 137.489f
path.quadTo(SkBits2Float(0x43a81cf4), SkBits2Float(0x4310cfc4), SkBits2Float(0x43a81cf4), SkBits2Float(0x431b2abc));  // 336.226f, 144.812f, 336.226f, 155.167f
path.close();
    SkPath path31(path);
    builder.add(path31, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x435e092f), SkBits2Float(0x43698168));  // 222.036f, 233.505f
path.quadTo(SkBits2Float(0x435e092f), SkBits2Float(0x4373dc5f), SkBits2Float(0x4356b6ab), SkBits2Float(0x437b2ee3));  // 222.036f, 243.861f, 214.714f, 251.183f
path.quadTo(SkBits2Float(0x434f6426), SkBits2Float(0x438140b4), SkBits2Float(0x4345092f), SkBits2Float(0x438140b4));  // 207.391f, 258.505f, 197.036f, 258.505f
path.quadTo(SkBits2Float(0x433aae38), SkBits2Float(0x438140b4), SkBits2Float(0x43335bb3), SkBits2Float(0x437b2ee3));  // 186.681f, 258.505f, 179.358f, 251.183f
path.quadTo(SkBits2Float(0x432c092f), SkBits2Float(0x4373dc5f), SkBits2Float(0x432c092f), SkBits2Float(0x43698168));  // 172.036f, 243.861f, 172.036f, 233.505f
path.quadTo(SkBits2Float(0x432c092f), SkBits2Float(0x435f2671), SkBits2Float(0x43335bb3), SkBits2Float(0x4357d3ed));  // 172.036f, 223.15f, 179.358f, 215.828f
path.quadTo(SkBits2Float(0x433aae38), SkBits2Float(0x43508169), SkBits2Float(0x4345092f), SkBits2Float(0x43508169));  // 186.681f, 208.506f, 197.036f, 208.506f
path.quadTo(SkBits2Float(0x434f6426), SkBits2Float(0x43508169), SkBits2Float(0x4356b6ab), SkBits2Float(0x4357d3ed));  // 207.391f, 208.506f, 214.714f, 215.828f
path.quadTo(SkBits2Float(0x435e092f), SkBits2Float(0x435f2671), SkBits2Float(0x435e092f), SkBits2Float(0x43698168));  // 222.036f, 223.15f, 222.036f, 233.505f
path.close();
    SkPath path32(path);
    builder.add(path32, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43b29d51), SkBits2Float(0x434f504b));  // 357.229f, 207.314f
path.quadTo(SkBits2Float(0x43b29d51), SkBits2Float(0x4359ab42), SkBits2Float(0x43aef40f), SkBits2Float(0x4360fdc7));  // 357.229f, 217.669f, 349.907f, 224.991f
path.quadTo(SkBits2Float(0x43ab4acd), SkBits2Float(0x4368504b), SkBits2Float(0x43a61d51), SkBits2Float(0x4368504b));  // 342.584f, 232.314f, 332.229f, 232.314f
path.quadTo(SkBits2Float(0x43a0efd5), SkBits2Float(0x4368504b), SkBits2Float(0x439d4693), SkBits2Float(0x4360fdc7));  // 321.874f, 232.314f, 314.551f, 224.991f
path.quadTo(SkBits2Float(0x43999d51), SkBits2Float(0x4359ab42), SkBits2Float(0x43999d51), SkBits2Float(0x434f504b));  // 307.229f, 217.669f, 307.229f, 207.314f
path.quadTo(SkBits2Float(0x43999d51), SkBits2Float(0x4344f554), SkBits2Float(0x439d4693), SkBits2Float(0x433da2cf));  // 307.229f, 196.958f, 314.551f, 189.636f
path.quadTo(SkBits2Float(0x43a0efd5), SkBits2Float(0x4336504b), SkBits2Float(0x43a61d51), SkBits2Float(0x4336504b));  // 321.874f, 182.314f, 332.229f, 182.314f
path.quadTo(SkBits2Float(0x43ab4acd), SkBits2Float(0x4336504b), SkBits2Float(0x43aef40f), SkBits2Float(0x433da2cf));  // 342.584f, 182.314f, 349.907f, 189.636f
path.quadTo(SkBits2Float(0x43b29d51), SkBits2Float(0x4344f554), SkBits2Float(0x43b29d51), SkBits2Float(0x434f504b));  // 357.229f, 196.958f, 357.229f, 207.314f
path.close();
    SkPath path33(path);
    builder.add(path33, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x439022b6), SkBits2Float(0x434132a3));  // 288.271f, 193.198f
path.quadTo(SkBits2Float(0x439022b6), SkBits2Float(0x434b8d9a), SkBits2Float(0x438c7974), SkBits2Float(0x4352e01f));  // 288.271f, 203.553f, 280.949f, 210.875f
path.quadTo(SkBits2Float(0x4388d032), SkBits2Float(0x435a32a3), SkBits2Float(0x4383a2b6), SkBits2Float(0x435a32a3));  // 273.627f, 218.198f, 263.271f, 218.198f
path.quadTo(SkBits2Float(0x437cea74), SkBits2Float(0x435a32a3), SkBits2Float(0x437597f0), SkBits2Float(0x4352e01f));  // 252.916f, 218.198f, 245.594f, 210.875f
path.quadTo(SkBits2Float(0x436e456c), SkBits2Float(0x434b8d9a), SkBits2Float(0x436e456c), SkBits2Float(0x434132a3));  // 238.271f, 203.553f, 238.271f, 193.198f
path.quadTo(SkBits2Float(0x436e456c), SkBits2Float(0x4336d7ac), SkBits2Float(0x437597f0), SkBits2Float(0x432f8527));  // 238.271f, 182.842f, 245.594f, 175.52f
path.quadTo(SkBits2Float(0x437cea74), SkBits2Float(0x432832a3), SkBits2Float(0x4383a2b6), SkBits2Float(0x432832a3));  // 252.916f, 168.198f, 263.271f, 168.198f
path.quadTo(SkBits2Float(0x4388d032), SkBits2Float(0x432832a3), SkBits2Float(0x438c7974), SkBits2Float(0x432f8527));  // 273.627f, 168.198f, 280.949f, 175.52f
path.quadTo(SkBits2Float(0x439022b6), SkBits2Float(0x4336d7ac), SkBits2Float(0x439022b6), SkBits2Float(0x434132a3));  // 288.271f, 182.842f, 288.271f, 193.198f
path.close();
    SkPath path34(path);
    builder.add(path34, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x434c6e1b), SkBits2Float(0x4386bd38));  // 204.43f, 269.478f
path.quadTo(SkBits2Float(0x434c6e1b), SkBits2Float(0x438beab4), SkBits2Float(0x43451b97), SkBits2Float(0x438f93f6));  // 204.43f, 279.834f, 197.108f, 287.156f
path.quadTo(SkBits2Float(0x433dc912), SkBits2Float(0x43933d38), SkBits2Float(0x43336e1b), SkBits2Float(0x43933d38));  // 189.785f, 294.478f, 179.43f, 294.478f
path.quadTo(SkBits2Float(0x43291324), SkBits2Float(0x43933d38), SkBits2Float(0x4321c09f), SkBits2Float(0x438f93f6));  // 169.075f, 294.478f, 161.752f, 287.156f
path.quadTo(SkBits2Float(0x431a6e1b), SkBits2Float(0x438beab4), SkBits2Float(0x431a6e1b), SkBits2Float(0x4386bd38));  // 154.43f, 279.834f, 154.43f, 269.478f
path.quadTo(SkBits2Float(0x431a6e1b), SkBits2Float(0x43818fbc), SkBits2Float(0x4321c09f), SkBits2Float(0x437bccf4));  // 154.43f, 259.123f, 161.752f, 251.801f
path.quadTo(SkBits2Float(0x43291324), SkBits2Float(0x43747a70), SkBits2Float(0x43336e1b), SkBits2Float(0x43747a70));  // 169.075f, 244.478f, 179.43f, 244.478f
path.quadTo(SkBits2Float(0x433dc912), SkBits2Float(0x43747a70), SkBits2Float(0x43451b97), SkBits2Float(0x437bccf4));  // 189.785f, 244.478f, 197.108f, 251.801f
path.quadTo(SkBits2Float(0x434c6e1b), SkBits2Float(0x43818fbc), SkBits2Float(0x434c6e1b), SkBits2Float(0x4386bd38));  // 204.43f, 259.123f, 204.43f, 269.478f
path.close();
    SkPath path35(path);
    builder.add(path35, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43926b36), SkBits2Float(0x43b08773));  // 292.838f, 353.058f
path.quadTo(SkBits2Float(0x43926b36), SkBits2Float(0x43b5b4ef), SkBits2Float(0x438ec1f4), SkBits2Float(0x43b95e31));  // 292.838f, 363.414f, 285.515f, 370.736f
path.quadTo(SkBits2Float(0x438b18b2), SkBits2Float(0x43bd0773), SkBits2Float(0x4385eb36), SkBits2Float(0x43bd0773));  // 278.193f, 378.058f, 267.838f, 378.058f
path.quadTo(SkBits2Float(0x4380bdba), SkBits2Float(0x43bd0773), SkBits2Float(0x437a28f0), SkBits2Float(0x43b95e31));  // 257.482f, 378.058f, 250.16f, 370.736f
path.quadTo(SkBits2Float(0x4372d66c), SkBits2Float(0x43b5b4ef), SkBits2Float(0x4372d66c), SkBits2Float(0x43b08773));  // 242.838f, 363.414f, 242.838f, 353.058f
path.quadTo(SkBits2Float(0x4372d66c), SkBits2Float(0x43ab59f7), SkBits2Float(0x437a28f0), SkBits2Float(0x43a7b0b5));  // 242.838f, 342.703f, 250.16f, 335.381f
path.quadTo(SkBits2Float(0x4380bdba), SkBits2Float(0x43a40773), SkBits2Float(0x4385eb36), SkBits2Float(0x43a40773));  // 257.482f, 328.058f, 267.838f, 328.058f
path.quadTo(SkBits2Float(0x438b18b2), SkBits2Float(0x43a40773), SkBits2Float(0x438ec1f4), SkBits2Float(0x43a7b0b5));  // 278.193f, 328.058f, 285.515f, 335.381f
path.quadTo(SkBits2Float(0x43926b36), SkBits2Float(0x43ab59f7), SkBits2Float(0x43926b36), SkBits2Float(0x43b08773));  // 292.838f, 342.703f, 292.838f, 353.058f
path.close();
    SkPath path36(path);
    builder.add(path36, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ea874d), SkBits2Float(0x4382542c));  // 469.057f, 260.658f
path.quadTo(SkBits2Float(0x43ea874d), SkBits2Float(0x438781a8), SkBits2Float(0x43e6de0b), SkBits2Float(0x438b2aea));  // 469.057f, 271.013f, 461.735f, 278.335f
path.quadTo(SkBits2Float(0x43e334c9), SkBits2Float(0x438ed42c), SkBits2Float(0x43de074d), SkBits2Float(0x438ed42c));  // 454.412f, 285.658f, 444.057f, 285.658f
path.quadTo(SkBits2Float(0x43d8d9d1), SkBits2Float(0x438ed42c), SkBits2Float(0x43d5308f), SkBits2Float(0x438b2aea));  // 433.702f, 285.658f, 426.379f, 278.335f
path.quadTo(SkBits2Float(0x43d1874d), SkBits2Float(0x438781a8), SkBits2Float(0x43d1874d), SkBits2Float(0x4382542c));  // 419.057f, 271.013f, 419.057f, 260.658f
path.quadTo(SkBits2Float(0x43d1874d), SkBits2Float(0x437a4d60), SkBits2Float(0x43d5308f), SkBits2Float(0x4372fadc));  // 419.057f, 250.302f, 426.379f, 242.98f
path.quadTo(SkBits2Float(0x43d8d9d1), SkBits2Float(0x436ba858), SkBits2Float(0x43de074d), SkBits2Float(0x436ba858));  // 433.702f, 235.658f, 444.057f, 235.658f
path.quadTo(SkBits2Float(0x43e334c9), SkBits2Float(0x436ba858), SkBits2Float(0x43e6de0b), SkBits2Float(0x4372fadc));  // 454.412f, 235.658f, 461.735f, 242.98f
path.quadTo(SkBits2Float(0x43ea874d), SkBits2Float(0x437a4d60), SkBits2Float(0x43ea874d), SkBits2Float(0x4382542c));  // 469.057f, 250.302f, 469.057f, 260.658f
path.close();
    SkPath path37(path);
    builder.add(path37, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42e7d715), SkBits2Float(0x436cecfc));  // 115.92f, 236.926f
path.quadTo(SkBits2Float(0x42e7d715), SkBits2Float(0x437747f3), SkBits2Float(0x42d9320c), SkBits2Float(0x437e9a77));  // 115.92f, 247.281f, 108.598f, 254.603f
path.quadTo(SkBits2Float(0x42ca8d04), SkBits2Float(0x4382f67e), SkBits2Float(0x42b5d715), SkBits2Float(0x4382f67e));  // 101.275f, 261.926f, 90.9201f, 261.926f
path.quadTo(SkBits2Float(0x42a12126), SkBits2Float(0x4382f67e), SkBits2Float(0x42927c1e), SkBits2Float(0x437e9a77));  // 80.5647f, 261.926f, 73.2424f, 254.603f
path.quadTo(SkBits2Float(0x4283d715), SkBits2Float(0x437747f3), SkBits2Float(0x4283d715), SkBits2Float(0x436cecfc));  // 65.9201f, 247.281f, 65.9201f, 236.926f
path.quadTo(SkBits2Float(0x4283d715), SkBits2Float(0x43629205), SkBits2Float(0x42927c1e), SkBits2Float(0x435b3f81));  // 65.9201f, 226.57f, 73.2424f, 219.248f
path.quadTo(SkBits2Float(0x42a12126), SkBits2Float(0x4353ecfd), SkBits2Float(0x42b5d715), SkBits2Float(0x4353ecfd));  // 80.5647f, 211.926f, 90.9201f, 211.926f
path.quadTo(SkBits2Float(0x42ca8d04), SkBits2Float(0x4353ecfd), SkBits2Float(0x42d9320c), SkBits2Float(0x435b3f81));  // 101.275f, 211.926f, 108.598f, 219.248f
path.quadTo(SkBits2Float(0x42e7d715), SkBits2Float(0x43629205), SkBits2Float(0x42e7d715), SkBits2Float(0x436cecfc));  // 115.92f, 226.57f, 115.92f, 236.926f
path.close();
    SkPath path38(path);
    builder.add(path38, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43545413), SkBits2Float(0x4333cfcf));  // 212.328f, 179.812f
path.quadTo(SkBits2Float(0x43545413), SkBits2Float(0x433e2ac6), SkBits2Float(0x434d018f), SkBits2Float(0x43457d4b));  // 212.328f, 190.167f, 205.006f, 197.489f
path.quadTo(SkBits2Float(0x4345af0a), SkBits2Float(0x434ccfcf), SkBits2Float(0x433b5413), SkBits2Float(0x434ccfcf));  // 197.684f, 204.812f, 187.328f, 204.812f
path.quadTo(SkBits2Float(0x4330f91c), SkBits2Float(0x434ccfcf), SkBits2Float(0x4329a697), SkBits2Float(0x43457d4b));  // 176.973f, 204.812f, 169.651f, 197.489f
path.quadTo(SkBits2Float(0x43225413), SkBits2Float(0x433e2ac6), SkBits2Float(0x43225413), SkBits2Float(0x4333cfcf));  // 162.328f, 190.167f, 162.328f, 179.812f
path.quadTo(SkBits2Float(0x43225413), SkBits2Float(0x432974d8), SkBits2Float(0x4329a697), SkBits2Float(0x43222253));  // 162.328f, 169.456f, 169.651f, 162.134f
path.quadTo(SkBits2Float(0x4330f91c), SkBits2Float(0x431acfcf), SkBits2Float(0x433b5413), SkBits2Float(0x431acfcf));  // 176.973f, 154.812f, 187.328f, 154.812f
path.quadTo(SkBits2Float(0x4345af0a), SkBits2Float(0x431acfcf), SkBits2Float(0x434d018f), SkBits2Float(0x43222253));  // 197.684f, 154.812f, 205.006f, 162.134f
path.quadTo(SkBits2Float(0x43545413), SkBits2Float(0x432974d8), SkBits2Float(0x43545413), SkBits2Float(0x4333cfcf));  // 212.328f, 169.456f, 212.328f, 179.812f
path.close();
    SkPath path39(path);
    builder.add(path39, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43c9b16c), SkBits2Float(0x41823be6));  // 403.386f, 16.2792f
path.quadTo(SkBits2Float(0x43c9b16c), SkBits2Float(0x41d513a2), SkBits2Float(0x43c6082a), SkBits2Float(0x4207d3e2));  // 403.386f, 26.6346f, 396.064f, 33.9569f
path.quadTo(SkBits2Float(0x43c25ee8), SkBits2Float(0x42251df3), SkBits2Float(0x43bd316c), SkBits2Float(0x42251df3));  // 388.741f, 41.2792f, 378.386f, 41.2792f
path.quadTo(SkBits2Float(0x43b803f0), SkBits2Float(0x42251df3), SkBits2Float(0x43b45aae), SkBits2Float(0x4207d3e2));  // 368.031f, 41.2792f, 360.708f, 33.9569f
path.quadTo(SkBits2Float(0x43b0b16c), SkBits2Float(0x41d513a2), SkBits2Float(0x43b0b16c), SkBits2Float(0x41823be6));  // 353.386f, 26.6346f, 353.386f, 16.2792f
path.quadTo(SkBits2Float(0x43b0b16c), SkBits2Float(0x40bd90a8), SkBits2Float(0x43b45aae), SkBits2Float(0xbfb2ff80));  // 353.386f, 5.92391f, 360.708f, -1.39842f
path.quadTo(SkBits2Float(0x43b803f0), SkBits2Float(0xc10b8834), SkBits2Float(0x43bd316c), SkBits2Float(0xc10b8834));  // 368.031f, -8.72075f, 378.386f, -8.72075f
path.quadTo(SkBits2Float(0x43c25ee8), SkBits2Float(0xc10b8834), SkBits2Float(0x43c6082a), SkBits2Float(0xbfb2ff80));  // 388.741f, -8.72075f, 396.064f, -1.39842f
path.quadTo(SkBits2Float(0x43c9b16c), SkBits2Float(0x40bd90a8), SkBits2Float(0x43c9b16c), SkBits2Float(0x41823be6));  // 403.386f, 5.92391f, 403.386f, 16.2792f
path.close();
    SkPath path40(path);
    builder.add(path40, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43747fcb), SkBits2Float(0x43805e9d));  // 244.499f, 256.739f
path.quadTo(SkBits2Float(0x43747fcb), SkBits2Float(0x43858c19), SkBits2Float(0x436d2d47), SkBits2Float(0x4389355b));  // 244.499f, 267.095f, 237.177f, 274.417f
path.quadTo(SkBits2Float(0x4365dac2), SkBits2Float(0x438cde9d), SkBits2Float(0x435b7fcb), SkBits2Float(0x438cde9d));  // 229.855f, 281.739f, 219.499f, 281.739f
path.quadTo(SkBits2Float(0x435124d4), SkBits2Float(0x438cde9d), SkBits2Float(0x4349d24f), SkBits2Float(0x4389355b));  // 209.144f, 281.739f, 201.822f, 274.417f
path.quadTo(SkBits2Float(0x43427fcb), SkBits2Float(0x43858c19), SkBits2Float(0x43427fcb), SkBits2Float(0x43805e9d));  // 194.499f, 267.095f, 194.499f, 256.739f
path.quadTo(SkBits2Float(0x43427fcb), SkBits2Float(0x43766242), SkBits2Float(0x4349d24f), SkBits2Float(0x436f0fbe));  // 194.499f, 246.384f, 201.822f, 239.061f
path.quadTo(SkBits2Float(0x435124d4), SkBits2Float(0x4367bd3a), SkBits2Float(0x435b7fcb), SkBits2Float(0x4367bd3a));  // 209.144f, 231.739f, 219.499f, 231.739f
path.quadTo(SkBits2Float(0x4365dac2), SkBits2Float(0x4367bd3a), SkBits2Float(0x436d2d47), SkBits2Float(0x436f0fbe));  // 229.855f, 231.739f, 237.177f, 239.061f
path.quadTo(SkBits2Float(0x43747fcb), SkBits2Float(0x43766242), SkBits2Float(0x43747fcb), SkBits2Float(0x43805e9d));  // 244.499f, 246.384f, 244.499f, 256.739f
path.close();
    SkPath path41(path);
    builder.add(path41, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43910318), SkBits2Float(0x43826a1e));  // 290.024f, 260.829f
path.quadTo(SkBits2Float(0x43910318), SkBits2Float(0x4387979a), SkBits2Float(0x438d59d6), SkBits2Float(0x438b40dc));  // 290.024f, 271.184f, 282.702f, 278.507f
path.quadTo(SkBits2Float(0x4389b094), SkBits2Float(0x438eea1e), SkBits2Float(0x43848318), SkBits2Float(0x438eea1e));  // 275.38f, 285.829f, 265.024f, 285.829f
path.quadTo(SkBits2Float(0x437eab38), SkBits2Float(0x438eea1e), SkBits2Float(0x437758b4), SkBits2Float(0x438b40dc));  // 254.669f, 285.829f, 247.346f, 278.507f
path.quadTo(SkBits2Float(0x43700630), SkBits2Float(0x4387979a), SkBits2Float(0x43700630), SkBits2Float(0x43826a1e));  // 240.024f, 271.184f, 240.024f, 260.829f
path.quadTo(SkBits2Float(0x43700630), SkBits2Float(0x437a7944), SkBits2Float(0x437758b4), SkBits2Float(0x437326c0));  // 240.024f, 250.474f, 247.346f, 243.151f
path.quadTo(SkBits2Float(0x437eab38), SkBits2Float(0x436bd43c), SkBits2Float(0x43848318), SkBits2Float(0x436bd43c));  // 254.669f, 235.829f, 265.024f, 235.829f
path.quadTo(SkBits2Float(0x4389b094), SkBits2Float(0x436bd43c), SkBits2Float(0x438d59d6), SkBits2Float(0x437326c0));  // 275.38f, 235.829f, 282.702f, 243.151f
path.quadTo(SkBits2Float(0x43910318), SkBits2Float(0x437a7944), SkBits2Float(0x43910318), SkBits2Float(0x43826a1e));  // 290.024f, 250.474f, 290.024f, 260.829f
path.close();
    SkPath path42(path);
    builder.add(path42, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41c80000), SkBits2Float(0x436edb04));  // 25, 238.856f
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x437935fb), SkBits2Float(0x418d6bde), SkBits2Float(0x43804440));  // 25, 249.211f, 17.6777f, 256.533f
path.quadTo(SkBits2Float(0x4125af78), SkBits2Float(0x4383ed82), SkBits2Float(0x00000000), SkBits2Float(0x4383ed82));  // 10.3553f, 263.856f, 0, 263.856f
path.quadTo(SkBits2Float(0xc125af78), SkBits2Float(0x4383ed82), SkBits2Float(0xc18d6bde), SkBits2Float(0x43804440));  // -10.3553f, 263.856f, -17.6777f, 256.533f
path.quadTo(SkBits2Float(0xc1c80000), SkBits2Float(0x437935fb), SkBits2Float(0xc1c80000), SkBits2Float(0x436edb04));  // -25, 249.211f, -25, 238.856f
path.quadTo(SkBits2Float(0xc1c80000), SkBits2Float(0x4364800d), SkBits2Float(0xc18d6bde), SkBits2Float(0x435d2d89));  // -25, 228.5f, -17.6777f, 221.178f
path.quadTo(SkBits2Float(0xc125af78), SkBits2Float(0x4355db05), SkBits2Float(0x00000000), SkBits2Float(0x4355db05));  // -10.3553f, 213.856f, 0, 213.856f
path.quadTo(SkBits2Float(0x4125af78), SkBits2Float(0x4355db05), SkBits2Float(0x418d6bde), SkBits2Float(0x435d2d89));  // 10.3553f, 213.856f, 17.6777f, 221.178f
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4364800d), SkBits2Float(0x41c80000), SkBits2Float(0x436edb04));  // 25, 228.5f, 25, 238.856f
path.close();
    SkPath path43(path);
    builder.add(path43, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x435d07bd), SkBits2Float(0x4395fbb5));  // 221.03f, 299.966f
path.quadTo(SkBits2Float(0x435d07bd), SkBits2Float(0x439b2931), SkBits2Float(0x4355b539), SkBits2Float(0x439ed273));  // 221.03f, 310.322f, 213.708f, 317.644f
path.quadTo(SkBits2Float(0x434e62b4), SkBits2Float(0x43a27bb5), SkBits2Float(0x434407bd), SkBits2Float(0x43a27bb5));  // 206.386f, 324.966f, 196.03f, 324.966f
path.quadTo(SkBits2Float(0x4339acc6), SkBits2Float(0x43a27bb5), SkBits2Float(0x43325a41), SkBits2Float(0x439ed273));  // 185.675f, 324.966f, 178.353f, 317.644f
path.quadTo(SkBits2Float(0x432b07bd), SkBits2Float(0x439b2931), SkBits2Float(0x432b07bd), SkBits2Float(0x4395fbb5));  // 171.03f, 310.322f, 171.03f, 299.966f
path.quadTo(SkBits2Float(0x432b07bd), SkBits2Float(0x4390ce39), SkBits2Float(0x43325a41), SkBits2Float(0x438d24f7));  // 171.03f, 289.611f, 178.353f, 282.289f
path.quadTo(SkBits2Float(0x4339acc6), SkBits2Float(0x43897bb5), SkBits2Float(0x434407bd), SkBits2Float(0x43897bb5));  // 185.675f, 274.966f, 196.03f, 274.966f
path.quadTo(SkBits2Float(0x434e62b4), SkBits2Float(0x43897bb5), SkBits2Float(0x4355b539), SkBits2Float(0x438d24f7));  // 206.386f, 274.966f, 213.708f, 282.289f
path.quadTo(SkBits2Float(0x435d07bd), SkBits2Float(0x4390ce39), SkBits2Float(0x435d07bd), SkBits2Float(0x4395fbb5));  // 221.03f, 289.611f, 221.03f, 299.966f
path.close();
    SkPath path44(path);
    builder.add(path44, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a3ec29), SkBits2Float(0x434ac5a3));  // 327.845f, 202.772f
path.quadTo(SkBits2Float(0x43a3ec29), SkBits2Float(0x4355209a), SkBits2Float(0x43a042e7), SkBits2Float(0x435c731f));  // 327.845f, 213.127f, 320.523f, 220.45f
path.quadTo(SkBits2Float(0x439c99a5), SkBits2Float(0x4363c5a3), SkBits2Float(0x43976c29), SkBits2Float(0x4363c5a3));  // 313.2f, 227.772f, 302.845f, 227.772f
path.quadTo(SkBits2Float(0x43923ead), SkBits2Float(0x4363c5a3), SkBits2Float(0x438e956b), SkBits2Float(0x435c731f));  // 292.49f, 227.772f, 285.167f, 220.45f
path.quadTo(SkBits2Float(0x438aec29), SkBits2Float(0x4355209a), SkBits2Float(0x438aec29), SkBits2Float(0x434ac5a3));  // 277.845f, 213.127f, 277.845f, 202.772f
path.quadTo(SkBits2Float(0x438aec29), SkBits2Float(0x43406aac), SkBits2Float(0x438e956b), SkBits2Float(0x43391827));  // 277.845f, 192.417f, 285.167f, 185.094f
path.quadTo(SkBits2Float(0x43923ead), SkBits2Float(0x4331c5a3), SkBits2Float(0x43976c29), SkBits2Float(0x4331c5a3));  // 292.49f, 177.772f, 302.845f, 177.772f
path.quadTo(SkBits2Float(0x439c99a5), SkBits2Float(0x4331c5a3), SkBits2Float(0x43a042e7), SkBits2Float(0x43391827));  // 313.2f, 177.772f, 320.523f, 185.094f
path.quadTo(SkBits2Float(0x43a3ec29), SkBits2Float(0x43406aac), SkBits2Float(0x43a3ec29), SkBits2Float(0x434ac5a3));  // 327.845f, 192.417f, 327.845f, 202.772f
path.close();
    SkPath path45(path);
    builder.add(path45, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4380585e), SkBits2Float(0x43199f0e));  // 256.69f, 153.621f
path.quadTo(SkBits2Float(0x4380585e), SkBits2Float(0x4323fa06), SkBits2Float(0x43795e38), SkBits2Float(0x432b4c8a));  // 256.69f, 163.977f, 249.368f, 171.299f
path.quadTo(SkBits2Float(0x43720bb4), SkBits2Float(0x43329f0e), SkBits2Float(0x4367b0bc), SkBits2Float(0x43329f0e));  // 242.046f, 178.621f, 231.69f, 178.621f
path.quadTo(SkBits2Float(0x435d55c4), SkBits2Float(0x43329f0e), SkBits2Float(0x43560340), SkBits2Float(0x432b4c8a));  // 221.335f, 178.621f, 214.013f, 171.299f
path.quadTo(SkBits2Float(0x434eb0bc), SkBits2Float(0x4323fa06), SkBits2Float(0x434eb0bc), SkBits2Float(0x43199f0e));  // 206.69f, 163.977f, 206.69f, 153.621f
path.quadTo(SkBits2Float(0x434eb0bc), SkBits2Float(0x430f4416), SkBits2Float(0x43560340), SkBits2Float(0x4307f192));  // 206.69f, 143.266f, 214.013f, 135.944f
path.quadTo(SkBits2Float(0x435d55c4), SkBits2Float(0x43009f0e), SkBits2Float(0x4367b0bc), SkBits2Float(0x43009f0e));  // 221.335f, 128.621f, 231.69f, 128.621f
path.quadTo(SkBits2Float(0x43720bb4), SkBits2Float(0x43009f0e), SkBits2Float(0x43795e38), SkBits2Float(0x4307f192));  // 242.046f, 128.621f, 249.368f, 135.944f
path.quadTo(SkBits2Float(0x4380585e), SkBits2Float(0x430f4416), SkBits2Float(0x4380585e), SkBits2Float(0x43199f0e));  // 256.69f, 143.266f, 256.69f, 153.621f
path.close();
    SkPath path46(path);
    builder.add(path46, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43599e4b), SkBits2Float(0x43c5e452));  // 217.618f, 395.784f
path.quadTo(SkBits2Float(0x43599e4b), SkBits2Float(0x43cb11ce), SkBits2Float(0x43524bc7), SkBits2Float(0x43cebb10));  // 217.618f, 406.139f, 210.296f, 413.461f
path.quadTo(SkBits2Float(0x434af942), SkBits2Float(0x43d26452), SkBits2Float(0x43409e4b), SkBits2Float(0x43d26452));  // 202.974f, 420.784f, 192.618f, 420.784f
path.quadTo(SkBits2Float(0x43364354), SkBits2Float(0x43d26452), SkBits2Float(0x432ef0cf), SkBits2Float(0x43cebb10));  // 182.263f, 420.784f, 174.941f, 413.461f
path.quadTo(SkBits2Float(0x43279e4b), SkBits2Float(0x43cb11ce), SkBits2Float(0x43279e4b), SkBits2Float(0x43c5e452));  // 167.618f, 406.139f, 167.618f, 395.784f
path.quadTo(SkBits2Float(0x43279e4b), SkBits2Float(0x43c0b6d6), SkBits2Float(0x432ef0cf), SkBits2Float(0x43bd0d94));  // 167.618f, 385.428f, 174.941f, 378.106f
path.quadTo(SkBits2Float(0x43364354), SkBits2Float(0x43b96452), SkBits2Float(0x43409e4b), SkBits2Float(0x43b96452));  // 182.263f, 370.784f, 192.618f, 370.784f
path.quadTo(SkBits2Float(0x434af942), SkBits2Float(0x43b96452), SkBits2Float(0x43524bc7), SkBits2Float(0x43bd0d94));  // 202.974f, 370.784f, 210.296f, 378.106f
path.quadTo(SkBits2Float(0x43599e4b), SkBits2Float(0x43c0b6d6), SkBits2Float(0x43599e4b), SkBits2Float(0x43c5e452));  // 217.618f, 385.428f, 217.618f, 395.784f
path.close();
    SkPath path47(path);
    builder.add(path47, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x435e28dc), SkBits2Float(0x43a32a69));  // 222.16f, 326.331f
path.quadTo(SkBits2Float(0x435e28dc), SkBits2Float(0x43a857e5), SkBits2Float(0x4356d658), SkBits2Float(0x43ac0127));  // 222.16f, 336.687f, 214.837f, 344.009f
path.quadTo(SkBits2Float(0x434f83d4), SkBits2Float(0x43afaa69), SkBits2Float(0x434528dc), SkBits2Float(0x43afaa69));  // 207.515f, 351.331f, 197.16f, 351.331f
path.quadTo(SkBits2Float(0x433acde4), SkBits2Float(0x43afaa69), SkBits2Float(0x43337b60), SkBits2Float(0x43ac0127));  // 186.804f, 351.331f, 179.482f, 344.009f
path.quadTo(SkBits2Float(0x432c28dc), SkBits2Float(0x43a857e5), SkBits2Float(0x432c28dc), SkBits2Float(0x43a32a69));  // 172.16f, 336.687f, 172.16f, 326.331f
path.quadTo(SkBits2Float(0x432c28dc), SkBits2Float(0x439dfced), SkBits2Float(0x43337b60), SkBits2Float(0x439a53ab));  // 172.16f, 315.976f, 179.482f, 308.654f
path.quadTo(SkBits2Float(0x433acde4), SkBits2Float(0x4396aa69), SkBits2Float(0x434528dc), SkBits2Float(0x4396aa69));  // 186.804f, 301.331f, 197.16f, 301.331f
path.quadTo(SkBits2Float(0x434f83d4), SkBits2Float(0x4396aa69), SkBits2Float(0x4356d658), SkBits2Float(0x439a53ab));  // 207.515f, 301.331f, 214.837f, 308.654f
path.quadTo(SkBits2Float(0x435e28dc), SkBits2Float(0x439dfced), SkBits2Float(0x435e28dc), SkBits2Float(0x43a32a69));  // 222.16f, 315.976f, 222.16f, 326.331f
path.close();
    SkPath path48(path);
    builder.add(path48, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x433b4ff9), SkBits2Float(0x438034ac));  // 187.312f, 256.411f
path.quadTo(SkBits2Float(0x433b4ff9), SkBits2Float(0x43856228), SkBits2Float(0x4333fd75), SkBits2Float(0x43890b6a));  // 187.312f, 266.767f, 179.99f, 274.089f
path.quadTo(SkBits2Float(0x432caaf0), SkBits2Float(0x438cb4ac), SkBits2Float(0x43224ff9), SkBits2Float(0x438cb4ac));  // 172.668f, 281.411f, 162.312f, 281.411f
path.quadTo(SkBits2Float(0x4317f502), SkBits2Float(0x438cb4ac), SkBits2Float(0x4310a27d), SkBits2Float(0x43890b6a));  // 151.957f, 281.411f, 144.635f, 274.089f
path.quadTo(SkBits2Float(0x43094ff9), SkBits2Float(0x43856228), SkBits2Float(0x43094ff9), SkBits2Float(0x438034ac));  // 137.312f, 266.767f, 137.312f, 256.411f
path.quadTo(SkBits2Float(0x43094ff9), SkBits2Float(0x43760e60), SkBits2Float(0x4310a27d), SkBits2Float(0x436ebbdc));  // 137.312f, 246.056f, 144.635f, 238.734f
path.quadTo(SkBits2Float(0x4317f502), SkBits2Float(0x43676958), SkBits2Float(0x43224ff9), SkBits2Float(0x43676958));  // 151.957f, 231.411f, 162.312f, 231.411f
path.quadTo(SkBits2Float(0x432caaf0), SkBits2Float(0x43676958), SkBits2Float(0x4333fd75), SkBits2Float(0x436ebbdc));  // 172.668f, 231.411f, 179.99f, 238.734f
path.quadTo(SkBits2Float(0x433b4ff9), SkBits2Float(0x43760e60), SkBits2Float(0x433b4ff9), SkBits2Float(0x438034ac));  // 187.312f, 246.056f, 187.312f, 256.411f
path.close();
    SkPath path49(path);
    builder.add(path49, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4346c2ee), SkBits2Float(0x435b284b));  // 198.761f, 219.157f
path.quadTo(SkBits2Float(0x4346c2ee), SkBits2Float(0x43658342), SkBits2Float(0x433f706a), SkBits2Float(0x436cd5c7));  // 198.761f, 229.513f, 191.439f, 236.835f
path.quadTo(SkBits2Float(0x43381de6), SkBits2Float(0x4374284b), SkBits2Float(0x432dc2ee), SkBits2Float(0x4374284b));  // 184.117f, 244.157f, 173.761f, 244.157f
path.quadTo(SkBits2Float(0x432367f6), SkBits2Float(0x4374284b), SkBits2Float(0x431c1572), SkBits2Float(0x436cd5c7));  // 163.406f, 244.157f, 156.084f, 236.835f
path.quadTo(SkBits2Float(0x4314c2ee), SkBits2Float(0x43658342), SkBits2Float(0x4314c2ee), SkBits2Float(0x435b284b));  // 148.761f, 229.513f, 148.761f, 219.157f
path.quadTo(SkBits2Float(0x4314c2ee), SkBits2Float(0x4350cd54), SkBits2Float(0x431c1572), SkBits2Float(0x43497acf));  // 148.761f, 208.802f, 156.084f, 201.48f
path.quadTo(SkBits2Float(0x432367f6), SkBits2Float(0x4342284b), SkBits2Float(0x432dc2ee), SkBits2Float(0x4342284b));  // 163.406f, 194.157f, 173.761f, 194.157f
path.quadTo(SkBits2Float(0x43381de6), SkBits2Float(0x4342284b), SkBits2Float(0x433f706a), SkBits2Float(0x43497acf));  // 184.117f, 194.157f, 191.439f, 201.48f
path.quadTo(SkBits2Float(0x4346c2ee), SkBits2Float(0x4350cd54), SkBits2Float(0x4346c2ee), SkBits2Float(0x435b284b));  // 198.761f, 208.802f, 198.761f, 219.157f
path.close();
    SkPath path50(path);
    builder.add(path50, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43fb0cf6), SkBits2Float(0x438812a5));  // 502.101f, 272.146f
path.quadTo(SkBits2Float(0x43fb0cf6), SkBits2Float(0x438d4021), SkBits2Float(0x43f763b4), SkBits2Float(0x4390e963));  // 502.101f, 282.501f, 494.779f, 289.823f
path.quadTo(SkBits2Float(0x43f3ba72), SkBits2Float(0x439492a5), SkBits2Float(0x43ee8cf6), SkBits2Float(0x439492a5));  // 487.457f, 297.146f, 477.101f, 297.146f
path.quadTo(SkBits2Float(0x43e95f7a), SkBits2Float(0x439492a5), SkBits2Float(0x43e5b638), SkBits2Float(0x4390e963));  // 466.746f, 297.146f, 459.424f, 289.823f
path.quadTo(SkBits2Float(0x43e20cf6), SkBits2Float(0x438d4021), SkBits2Float(0x43e20cf6), SkBits2Float(0x438812a5));  // 452.101f, 282.501f, 452.101f, 272.146f
path.quadTo(SkBits2Float(0x43e20cf6), SkBits2Float(0x4382e529), SkBits2Float(0x43e5b638), SkBits2Float(0x437e77ce));  // 452.101f, 261.79f, 459.424f, 254.468f
path.quadTo(SkBits2Float(0x43e95f7a), SkBits2Float(0x4377254a), SkBits2Float(0x43ee8cf6), SkBits2Float(0x4377254a));  // 466.746f, 247.146f, 477.101f, 247.146f
path.quadTo(SkBits2Float(0x43f3ba72), SkBits2Float(0x4377254a), SkBits2Float(0x43f763b4), SkBits2Float(0x437e77ce));  // 487.457f, 247.146f, 494.779f, 254.468f
path.quadTo(SkBits2Float(0x43fb0cf6), SkBits2Float(0x4382e529), SkBits2Float(0x43fb0cf6), SkBits2Float(0x438812a5));  // 502.101f, 261.79f, 502.101f, 272.146f
path.close();
    SkPath path51(path);
    builder.add(path51, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x438f42d6), SkBits2Float(0x435a09d4));  // 286.522f, 218.038f
path.quadTo(SkBits2Float(0x438f42d6), SkBits2Float(0x436464cc), SkBits2Float(0x438b9994), SkBits2Float(0x436bb750));  // 286.522f, 228.394f, 279.2f, 235.716f
path.quadTo(SkBits2Float(0x4387f052), SkBits2Float(0x437309d4), SkBits2Float(0x4382c2d6), SkBits2Float(0x437309d4));  // 271.878f, 243.038f, 261.522f, 243.038f
path.quadTo(SkBits2Float(0x437b2ab4), SkBits2Float(0x437309d4), SkBits2Float(0x4373d830), SkBits2Float(0x436bb750));  // 251.167f, 243.038f, 243.844f, 235.716f
path.quadTo(SkBits2Float(0x436c85ac), SkBits2Float(0x436464cc), SkBits2Float(0x436c85ac), SkBits2Float(0x435a09d4));  // 236.522f, 228.394f, 236.522f, 218.038f
path.quadTo(SkBits2Float(0x436c85ac), SkBits2Float(0x434faedc), SkBits2Float(0x4373d830), SkBits2Float(0x43485c58));  // 236.522f, 207.683f, 243.844f, 200.361f
path.quadTo(SkBits2Float(0x437b2ab4), SkBits2Float(0x434109d4), SkBits2Float(0x4382c2d6), SkBits2Float(0x434109d4));  // 251.167f, 193.038f, 261.522f, 193.038f
path.quadTo(SkBits2Float(0x4387f052), SkBits2Float(0x434109d4), SkBits2Float(0x438b9994), SkBits2Float(0x43485c58));  // 271.878f, 193.038f, 279.2f, 200.361f
path.quadTo(SkBits2Float(0x438f42d6), SkBits2Float(0x434faedc), SkBits2Float(0x438f42d6), SkBits2Float(0x435a09d4));  // 286.522f, 207.683f, 286.522f, 218.038f
path.close();
    SkPath path52(path);
    builder.add(path52, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ac18fb), SkBits2Float(0x43378440));  // 344.195f, 183.517f
path.quadTo(SkBits2Float(0x43ac18fb), SkBits2Float(0x4341df38), SkBits2Float(0x43a86fb9), SkBits2Float(0x434931bc));  // 344.195f, 193.872f, 336.873f, 201.194f
path.quadTo(SkBits2Float(0x43a4c677), SkBits2Float(0x43508440), SkBits2Float(0x439f98fb), SkBits2Float(0x43508440));  // 329.551f, 208.517f, 319.195f, 208.517f
path.quadTo(SkBits2Float(0x439a6b7f), SkBits2Float(0x43508440), SkBits2Float(0x4396c23d), SkBits2Float(0x434931bc));  // 308.84f, 208.517f, 301.517f, 201.194f
path.quadTo(SkBits2Float(0x439318fb), SkBits2Float(0x4341df38), SkBits2Float(0x439318fb), SkBits2Float(0x43378440));  // 294.195f, 193.872f, 294.195f, 183.517f
path.quadTo(SkBits2Float(0x439318fb), SkBits2Float(0x432d2948), SkBits2Float(0x4396c23d), SkBits2Float(0x4325d6c4));  // 294.195f, 173.161f, 301.517f, 165.839f
path.quadTo(SkBits2Float(0x439a6b7f), SkBits2Float(0x431e8440), SkBits2Float(0x439f98fb), SkBits2Float(0x431e8440));  // 308.84f, 158.517f, 319.195f, 158.517f
path.quadTo(SkBits2Float(0x43a4c677), SkBits2Float(0x431e8440), SkBits2Float(0x43a86fb9), SkBits2Float(0x4325d6c4));  // 329.551f, 158.517f, 336.873f, 165.839f
path.quadTo(SkBits2Float(0x43ac18fb), SkBits2Float(0x432d2948), SkBits2Float(0x43ac18fb), SkBits2Float(0x43378440));  // 344.195f, 173.161f, 344.195f, 183.517f
path.close();
    SkPath path53(path);
    builder.add(path53, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42ef12a3), SkBits2Float(0x430c5faa));  // 119.536f, 140.374f
path.quadTo(SkBits2Float(0x42ef12a3), SkBits2Float(0x4316baa2), SkBits2Float(0x42e06d9a), SkBits2Float(0x431e0d26));  // 119.536f, 150.729f, 112.214f, 158.051f
path.quadTo(SkBits2Float(0x42d1c892), SkBits2Float(0x43255faa), SkBits2Float(0x42bd12a3), SkBits2Float(0x43255faa));  // 104.892f, 165.374f, 94.5364f, 165.374f
path.quadTo(SkBits2Float(0x42a85cb4), SkBits2Float(0x43255faa), SkBits2Float(0x4299b7ac), SkBits2Float(0x431e0d26));  // 84.1811f, 165.374f, 76.8587f, 158.051f
path.quadTo(SkBits2Float(0x428b12a3), SkBits2Float(0x4316baa2), SkBits2Float(0x428b12a3), SkBits2Float(0x430c5faa));  // 69.5364f, 150.729f, 69.5364f, 140.374f
path.quadTo(SkBits2Float(0x428b12a3), SkBits2Float(0x430204b2), SkBits2Float(0x4299b7ac), SkBits2Float(0x42f5645c));  // 69.5364f, 130.018f, 76.8587f, 122.696f
path.quadTo(SkBits2Float(0x42a85cb4), SkBits2Float(0x42e6bf54), SkBits2Float(0x42bd12a3), SkBits2Float(0x42e6bf54));  // 84.1811f, 115.374f, 94.5364f, 115.374f
path.quadTo(SkBits2Float(0x42d1c892), SkBits2Float(0x42e6bf54), SkBits2Float(0x42e06d9a), SkBits2Float(0x42f5645c));  // 104.892f, 115.374f, 112.214f, 122.696f
path.quadTo(SkBits2Float(0x42ef12a3), SkBits2Float(0x430204b2), SkBits2Float(0x42ef12a3), SkBits2Float(0x430c5faa));  // 119.536f, 130.018f, 119.536f, 140.374f
path.close();
    SkPath path54(path);
    builder.add(path54, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43f569c1), SkBits2Float(0x43463314));  // 490.826f, 198.2f
path.quadTo(SkBits2Float(0x43f569c1), SkBits2Float(0x43508e0c), SkBits2Float(0x43f1c07f), SkBits2Float(0x4357e090));  // 490.826f, 208.555f, 483.504f, 215.877f
path.quadTo(SkBits2Float(0x43ee173d), SkBits2Float(0x435f3314), SkBits2Float(0x43e8e9c1), SkBits2Float(0x435f3314));  // 476.182f, 223.2f, 465.826f, 223.2f
path.quadTo(SkBits2Float(0x43e3bc45), SkBits2Float(0x435f3314), SkBits2Float(0x43e01303), SkBits2Float(0x4357e090));  // 455.471f, 223.2f, 448.149f, 215.877f
path.quadTo(SkBits2Float(0x43dc69c1), SkBits2Float(0x43508e0c), SkBits2Float(0x43dc69c1), SkBits2Float(0x43463314));  // 440.826f, 208.555f, 440.826f, 198.2f
path.quadTo(SkBits2Float(0x43dc69c1), SkBits2Float(0x433bd81c), SkBits2Float(0x43e01303), SkBits2Float(0x43348598));  // 440.826f, 187.844f, 448.149f, 180.522f
path.quadTo(SkBits2Float(0x43e3bc45), SkBits2Float(0x432d3314), SkBits2Float(0x43e8e9c1), SkBits2Float(0x432d3314));  // 455.471f, 173.2f, 465.826f, 173.2f
path.quadTo(SkBits2Float(0x43ee173d), SkBits2Float(0x432d3314), SkBits2Float(0x43f1c07f), SkBits2Float(0x43348598));  // 476.182f, 173.2f, 483.504f, 180.522f
path.quadTo(SkBits2Float(0x43f569c1), SkBits2Float(0x433bd81c), SkBits2Float(0x43f569c1), SkBits2Float(0x43463314));  // 490.826f, 187.844f, 490.826f, 198.2f
path.close();
    SkPath path55(path);
    builder.add(path55, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4346ee50), SkBits2Float(0x4386bdd6));  // 198.931f, 269.483f
path.quadTo(SkBits2Float(0x4346ee50), SkBits2Float(0x438beb52), SkBits2Float(0x433f9bcc), SkBits2Float(0x438f9494));  // 198.931f, 279.838f, 191.609f, 287.161f
path.quadTo(SkBits2Float(0x43384948), SkBits2Float(0x43933dd6), SkBits2Float(0x432dee50), SkBits2Float(0x43933dd6));  // 184.286f, 294.483f, 173.931f, 294.483f
path.quadTo(SkBits2Float(0x43239358), SkBits2Float(0x43933dd6), SkBits2Float(0x431c40d4), SkBits2Float(0x438f9494));  // 163.576f, 294.483f, 156.253f, 287.161f
path.quadTo(SkBits2Float(0x4314ee50), SkBits2Float(0x438beb52), SkBits2Float(0x4314ee50), SkBits2Float(0x4386bdd6));  // 148.931f, 279.838f, 148.931f, 269.483f
path.quadTo(SkBits2Float(0x4314ee50), SkBits2Float(0x4381905a), SkBits2Float(0x431c40d4), SkBits2Float(0x437bce30));  // 148.931f, 259.128f, 156.253f, 251.805f
path.quadTo(SkBits2Float(0x43239358), SkBits2Float(0x43747bac), SkBits2Float(0x432dee50), SkBits2Float(0x43747bac));  // 163.576f, 244.483f, 173.931f, 244.483f
path.quadTo(SkBits2Float(0x43384948), SkBits2Float(0x43747bac), SkBits2Float(0x433f9bcc), SkBits2Float(0x437bce30));  // 184.286f, 244.483f, 191.609f, 251.805f
path.quadTo(SkBits2Float(0x4346ee50), SkBits2Float(0x4381905a), SkBits2Float(0x4346ee50), SkBits2Float(0x4386bdd6));  // 198.931f, 259.128f, 198.931f, 269.483f
path.close();
    SkPath path56(path);
    builder.add(path56, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4403bd60), SkBits2Float(0x438666a7));  // 526.959f, 268.802f
path.quadTo(SkBits2Float(0x4403bd60), SkBits2Float(0x438b9423), SkBits2Float(0x4401e8bf), SkBits2Float(0x438f3d65));  // 526.959f, 279.157f, 519.637f, 286.48f
path.quadTo(SkBits2Float(0x4400141e), SkBits2Float(0x4392e6a7), SkBits2Float(0x43fafac0), SkBits2Float(0x4392e6a7));  // 512.314f, 293.802f, 501.959f, 293.802f
path.quadTo(SkBits2Float(0x43f5cd44), SkBits2Float(0x4392e6a7), SkBits2Float(0x43f22402), SkBits2Float(0x438f3d65));  // 491.604f, 293.802f, 484.281f, 286.48f
path.quadTo(SkBits2Float(0x43ee7ac1), SkBits2Float(0x438b9423), SkBits2Float(0x43ee7ac1), SkBits2Float(0x438666a7));  // 476.959f, 279.157f, 476.959f, 268.802f
path.quadTo(SkBits2Float(0x43ee7ac1), SkBits2Float(0x4381392b), SkBits2Float(0x43f22402), SkBits2Float(0x437b1fd2));  // 476.959f, 258.447f, 484.281f, 251.124f
path.quadTo(SkBits2Float(0x43f5cd44), SkBits2Float(0x4373cd4e), SkBits2Float(0x43fafac0), SkBits2Float(0x4373cd4e));  // 491.604f, 243.802f, 501.959f, 243.802f
path.quadTo(SkBits2Float(0x4400141e), SkBits2Float(0x4373cd4e), SkBits2Float(0x4401e8bf), SkBits2Float(0x437b1fd2));  // 512.314f, 243.802f, 519.637f, 251.124f
path.quadTo(SkBits2Float(0x4403bd60), SkBits2Float(0x4381392b), SkBits2Float(0x4403bd60), SkBits2Float(0x438666a7));  // 526.959f, 258.447f, 526.959f, 268.802f
path.close();
    SkPath path57(path);
    builder.add(path57, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x433c6aff), SkBits2Float(0x439bf9f9));  // 188.418f, 311.953f
path.quadTo(SkBits2Float(0x433c6aff), SkBits2Float(0x43a12775), SkBits2Float(0x4335187b), SkBits2Float(0x43a4d0b7));  // 188.418f, 322.308f, 181.096f, 329.631f
path.quadTo(SkBits2Float(0x432dc5f6), SkBits2Float(0x43a879f9), SkBits2Float(0x43236aff), SkBits2Float(0x43a879f9));  // 173.773f, 336.953f, 163.418f, 336.953f
path.quadTo(SkBits2Float(0x43191008), SkBits2Float(0x43a879f9), SkBits2Float(0x4311bd83), SkBits2Float(0x43a4d0b7));  // 153.063f, 336.953f, 145.74f, 329.631f
path.quadTo(SkBits2Float(0x430a6aff), SkBits2Float(0x43a12775), SkBits2Float(0x430a6aff), SkBits2Float(0x439bf9f9));  // 138.418f, 322.308f, 138.418f, 311.953f
path.quadTo(SkBits2Float(0x430a6aff), SkBits2Float(0x4396cc7d), SkBits2Float(0x4311bd83), SkBits2Float(0x4393233b));  // 138.418f, 301.598f, 145.74f, 294.275f
path.quadTo(SkBits2Float(0x43191008), SkBits2Float(0x438f79f9), SkBits2Float(0x43236aff), SkBits2Float(0x438f79f9));  // 153.063f, 286.953f, 163.418f, 286.953f
path.quadTo(SkBits2Float(0x432dc5f6), SkBits2Float(0x438f79f9), SkBits2Float(0x4335187b), SkBits2Float(0x4393233b));  // 173.773f, 286.953f, 181.096f, 294.275f
path.quadTo(SkBits2Float(0x433c6aff), SkBits2Float(0x4396cc7d), SkBits2Float(0x433c6aff), SkBits2Float(0x439bf9f9));  // 188.418f, 301.598f, 188.418f, 311.953f
path.close();
    SkPath path58(path);
    builder.add(path58, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4397b4f8), SkBits2Float(0x43598b8a));  // 303.414f, 217.545f
path.quadTo(SkBits2Float(0x4397b4f8), SkBits2Float(0x4363e682), SkBits2Float(0x43940bb6), SkBits2Float(0x436b3906));  // 303.414f, 227.9f, 296.091f, 235.223f
path.quadTo(SkBits2Float(0x43906274), SkBits2Float(0x43728b8a), SkBits2Float(0x438b34f8), SkBits2Float(0x43728b8a));  // 288.769f, 242.545f, 278.414f, 242.545f
path.quadTo(SkBits2Float(0x4386077c), SkBits2Float(0x43728b8a), SkBits2Float(0x43825e3a), SkBits2Float(0x436b3906));  // 268.058f, 242.545f, 260.736f, 235.223f
path.quadTo(SkBits2Float(0x437d69f0), SkBits2Float(0x4363e682), SkBits2Float(0x437d69f0), SkBits2Float(0x43598b8a));  // 253.414f, 227.9f, 253.414f, 217.545f
path.quadTo(SkBits2Float(0x437d69f0), SkBits2Float(0x434f3092), SkBits2Float(0x43825e3a), SkBits2Float(0x4347de0e));  // 253.414f, 207.19f, 260.736f, 199.867f
path.quadTo(SkBits2Float(0x4386077c), SkBits2Float(0x43408b8a), SkBits2Float(0x438b34f8), SkBits2Float(0x43408b8a));  // 268.058f, 192.545f, 278.414f, 192.545f
path.quadTo(SkBits2Float(0x43906274), SkBits2Float(0x43408b8a), SkBits2Float(0x43940bb6), SkBits2Float(0x4347de0e));  // 288.769f, 192.545f, 296.091f, 199.867f
path.quadTo(SkBits2Float(0x4397b4f8), SkBits2Float(0x434f3092), SkBits2Float(0x4397b4f8), SkBits2Float(0x43598b8a));  // 303.414f, 207.19f, 303.414f, 217.545f
path.close();
    SkPath path59(path);
    builder.add(path59, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x430d7c0c), SkBits2Float(0x435ebbfb));  // 141.485f, 222.734f
path.quadTo(SkBits2Float(0x430d7c0c), SkBits2Float(0x436916f2), SkBits2Float(0x43062988), SkBits2Float(0x43706977));  // 141.485f, 233.09f, 134.162f, 240.412f
path.quadTo(SkBits2Float(0x42fdae07), SkBits2Float(0x4377bbfb), SkBits2Float(0x42e8f818), SkBits2Float(0x4377bbfb));  // 126.84f, 247.734f, 116.485f, 247.734f
path.quadTo(SkBits2Float(0x42d44229), SkBits2Float(0x4377bbfb), SkBits2Float(0x42c59d20), SkBits2Float(0x43706977));  // 106.129f, 247.734f, 98.8069f, 240.412f
path.quadTo(SkBits2Float(0x42b6f818), SkBits2Float(0x436916f2), SkBits2Float(0x42b6f818), SkBits2Float(0x435ebbfb));  // 91.4846f, 233.09f, 91.4846f, 222.734f
path.quadTo(SkBits2Float(0x42b6f818), SkBits2Float(0x43546104), SkBits2Float(0x42c59d20), SkBits2Float(0x434d0e7f));  // 91.4846f, 212.379f, 98.8069f, 205.057f
path.quadTo(SkBits2Float(0x42d44229), SkBits2Float(0x4345bbfb), SkBits2Float(0x42e8f818), SkBits2Float(0x4345bbfb));  // 106.129f, 197.734f, 116.485f, 197.734f
path.quadTo(SkBits2Float(0x42fdae07), SkBits2Float(0x4345bbfb), SkBits2Float(0x43062988), SkBits2Float(0x434d0e7f));  // 126.84f, 197.734f, 134.162f, 205.057f
path.quadTo(SkBits2Float(0x430d7c0c), SkBits2Float(0x43546104), SkBits2Float(0x430d7c0c), SkBits2Float(0x435ebbfb));  // 141.485f, 212.379f, 141.485f, 222.734f
path.close();
    SkPath path60(path);
    builder.add(path60, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43b7303b), SkBits2Float(0x42e664c0));  // 366.377f, 115.197f
path.quadTo(SkBits2Float(0x43b7303b), SkBits2Float(0x42fb1aaf), SkBits2Float(0x43b386f9), SkBits2Float(0x4304dfdc));  // 366.377f, 125.552f, 359.054f, 132.874f
path.quadTo(SkBits2Float(0x43afddb7), SkBits2Float(0x430c3260), SkBits2Float(0x43aab03b), SkBits2Float(0x430c3260));  // 351.732f, 140.197f, 341.377f, 140.197f
path.quadTo(SkBits2Float(0x43a582bf), SkBits2Float(0x430c3260), SkBits2Float(0x43a1d97d), SkBits2Float(0x4304dfdc));  // 331.021f, 140.197f, 323.699f, 132.874f
path.quadTo(SkBits2Float(0x439e303b), SkBits2Float(0x42fb1aaf), SkBits2Float(0x439e303b), SkBits2Float(0x42e664c0));  // 316.377f, 125.552f, 316.377f, 115.197f
path.quadTo(SkBits2Float(0x439e303b), SkBits2Float(0x42d1aed1), SkBits2Float(0x43a1d97d), SkBits2Float(0x42c309c8));  // 316.377f, 104.841f, 323.699f, 97.5191f
path.quadTo(SkBits2Float(0x43a582bf), SkBits2Float(0x42b464bf), SkBits2Float(0x43aab03b), SkBits2Float(0x42b464bf));  // 331.021f, 90.1968f, 341.377f, 90.1968f
path.quadTo(SkBits2Float(0x43afddb7), SkBits2Float(0x42b464bf), SkBits2Float(0x43b386f9), SkBits2Float(0x42c309c8));  // 351.732f, 90.1968f, 359.054f, 97.5191f
path.quadTo(SkBits2Float(0x43b7303b), SkBits2Float(0x42d1aed1), SkBits2Float(0x43b7303b), SkBits2Float(0x42e664c0));  // 366.377f, 104.841f, 366.377f, 115.197f
path.close();
    SkPath path61(path);
    builder.add(path61, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ded748), SkBits2Float(0x43786398));  // 445.682f, 248.389f
path.quadTo(SkBits2Float(0x43ded748), SkBits2Float(0x43815f48), SkBits2Float(0x43db2e06), SkBits2Float(0x4385088a));  // 445.682f, 258.744f, 438.36f, 266.067f
path.quadTo(SkBits2Float(0x43d784c4), SkBits2Float(0x4388b1cc), SkBits2Float(0x43d25748), SkBits2Float(0x4388b1cc));  // 431.037f, 273.389f, 420.682f, 273.389f
path.quadTo(SkBits2Float(0x43cd29cc), SkBits2Float(0x4388b1cc), SkBits2Float(0x43c9808a), SkBits2Float(0x4385088a));  // 410.327f, 273.389f, 403.004f, 266.067f
path.quadTo(SkBits2Float(0x43c5d748), SkBits2Float(0x43815f48), SkBits2Float(0x43c5d748), SkBits2Float(0x43786398));  // 395.682f, 258.744f, 395.682f, 248.389f
path.quadTo(SkBits2Float(0x43c5d748), SkBits2Float(0x436e08a1), SkBits2Float(0x43c9808a), SkBits2Float(0x4366b61d));  // 395.682f, 238.034f, 403.004f, 230.711f
path.quadTo(SkBits2Float(0x43cd29cc), SkBits2Float(0x435f6399), SkBits2Float(0x43d25748), SkBits2Float(0x435f6399));  // 410.327f, 223.389f, 420.682f, 223.389f
path.quadTo(SkBits2Float(0x43d784c4), SkBits2Float(0x435f6399), SkBits2Float(0x43db2e06), SkBits2Float(0x4366b61d));  // 431.037f, 223.389f, 438.36f, 230.711f
path.quadTo(SkBits2Float(0x43ded748), SkBits2Float(0x436e08a1), SkBits2Float(0x43ded748), SkBits2Float(0x43786398));  // 445.682f, 238.034f, 445.682f, 248.389f
path.close();
    SkPath path62(path);
    builder.add(path62, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43bbf04d), SkBits2Float(0x4397997d));  // 375.877f, 303.199f
path.quadTo(SkBits2Float(0x43bbf04d), SkBits2Float(0x439cc6f9), SkBits2Float(0x43b8470b), SkBits2Float(0x43a0703b));  // 375.877f, 313.554f, 368.555f, 320.877f
path.quadTo(SkBits2Float(0x43b49dc9), SkBits2Float(0x43a4197d), SkBits2Float(0x43af704d), SkBits2Float(0x43a4197d));  // 361.233f, 328.199f, 350.877f, 328.199f
path.quadTo(SkBits2Float(0x43aa42d1), SkBits2Float(0x43a4197d), SkBits2Float(0x43a6998f), SkBits2Float(0x43a0703b));  // 340.522f, 328.199f, 333.2f, 320.877f
path.quadTo(SkBits2Float(0x43a2f04d), SkBits2Float(0x439cc6f9), SkBits2Float(0x43a2f04d), SkBits2Float(0x4397997d));  // 325.877f, 313.554f, 325.877f, 303.199f
path.quadTo(SkBits2Float(0x43a2f04d), SkBits2Float(0x43926c01), SkBits2Float(0x43a6998f), SkBits2Float(0x438ec2bf));  // 325.877f, 292.844f, 333.2f, 285.521f
path.quadTo(SkBits2Float(0x43aa42d1), SkBits2Float(0x438b197d), SkBits2Float(0x43af704d), SkBits2Float(0x438b197d));  // 340.522f, 278.199f, 350.877f, 278.199f
path.quadTo(SkBits2Float(0x43b49dc9), SkBits2Float(0x438b197d), SkBits2Float(0x43b8470b), SkBits2Float(0x438ec2bf));  // 361.233f, 278.199f, 368.555f, 285.521f
path.quadTo(SkBits2Float(0x43bbf04d), SkBits2Float(0x43926c01), SkBits2Float(0x43bbf04d), SkBits2Float(0x4397997d));  // 375.877f, 292.844f, 375.877f, 303.199f
path.close();
    SkPath path63(path);
    builder.add(path63, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43b62e97), SkBits2Float(0x4313ec95));  // 364.364f, 147.924f
path.quadTo(SkBits2Float(0x43b62e97), SkBits2Float(0x431e478c), SkBits2Float(0x43b28555), SkBits2Float(0x43259a11));  // 364.364f, 158.279f, 357.042f, 165.602f
path.quadTo(SkBits2Float(0x43aedc13), SkBits2Float(0x432cec95), SkBits2Float(0x43a9ae97), SkBits2Float(0x432cec95));  // 349.719f, 172.924f, 339.364f, 172.924f
path.quadTo(SkBits2Float(0x43a4811b), SkBits2Float(0x432cec95), SkBits2Float(0x43a0d7d9), SkBits2Float(0x43259a11));  // 329.009f, 172.924f, 321.686f, 165.602f
path.quadTo(SkBits2Float(0x439d2e97), SkBits2Float(0x431e478c), SkBits2Float(0x439d2e97), SkBits2Float(0x4313ec95));  // 314.364f, 158.279f, 314.364f, 147.924f
path.quadTo(SkBits2Float(0x439d2e97), SkBits2Float(0x4309919e), SkBits2Float(0x43a0d7d9), SkBits2Float(0x43023f19));  // 314.364f, 137.569f, 321.686f, 130.246f
path.quadTo(SkBits2Float(0x43a4811b), SkBits2Float(0x42f5d92a), SkBits2Float(0x43a9ae97), SkBits2Float(0x42f5d92a));  // 329.009f, 122.924f, 339.364f, 122.924f
path.quadTo(SkBits2Float(0x43aedc13), SkBits2Float(0x42f5d92a), SkBits2Float(0x43b28555), SkBits2Float(0x43023f19));  // 349.719f, 122.924f, 357.042f, 130.246f
path.quadTo(SkBits2Float(0x43b62e97), SkBits2Float(0x4309919e), SkBits2Float(0x43b62e97), SkBits2Float(0x4313ec95));  // 364.364f, 137.569f, 364.364f, 147.924f
path.close();
    SkPath path64(path);
    builder.add(path64, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ec86f7), SkBits2Float(0x43b37791));  // 473.054f, 358.934f
path.quadTo(SkBits2Float(0x43ec86f7), SkBits2Float(0x43b8a50d), SkBits2Float(0x43e8ddb5), SkBits2Float(0x43bc4e4f));  // 473.054f, 369.289f, 465.732f, 376.612f
path.quadTo(SkBits2Float(0x43e53473), SkBits2Float(0x43bff791), SkBits2Float(0x43e006f7), SkBits2Float(0x43bff791));  // 458.41f, 383.934f, 448.054f, 383.934f
path.quadTo(SkBits2Float(0x43dad97b), SkBits2Float(0x43bff791), SkBits2Float(0x43d73039), SkBits2Float(0x43bc4e4f));  // 437.699f, 383.934f, 430.377f, 376.612f
path.quadTo(SkBits2Float(0x43d386f7), SkBits2Float(0x43b8a50d), SkBits2Float(0x43d386f7), SkBits2Float(0x43b37791));  // 423.054f, 369.289f, 423.054f, 358.934f
path.quadTo(SkBits2Float(0x43d386f7), SkBits2Float(0x43ae4a15), SkBits2Float(0x43d73039), SkBits2Float(0x43aaa0d3));  // 423.054f, 348.579f, 430.377f, 341.256f
path.quadTo(SkBits2Float(0x43dad97b), SkBits2Float(0x43a6f791), SkBits2Float(0x43e006f7), SkBits2Float(0x43a6f791));  // 437.699f, 333.934f, 448.054f, 333.934f
path.quadTo(SkBits2Float(0x43e53473), SkBits2Float(0x43a6f791), SkBits2Float(0x43e8ddb5), SkBits2Float(0x43aaa0d3));  // 458.41f, 333.934f, 465.732f, 341.256f
path.quadTo(SkBits2Float(0x43ec86f7), SkBits2Float(0x43ae4a15), SkBits2Float(0x43ec86f7), SkBits2Float(0x43b37791));  // 473.054f, 348.579f, 473.054f, 358.934f
path.close();
    SkPath path65(path);
    builder.add(path65, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43840826), SkBits2Float(0x43927b24));  // 264.064f, 292.962f
path.quadTo(SkBits2Float(0x43840826), SkBits2Float(0x4397a8a0), SkBits2Float(0x43805ee4), SkBits2Float(0x439b51e2));  // 264.064f, 303.317f, 256.741f, 310.64f
path.quadTo(SkBits2Float(0x43796b44), SkBits2Float(0x439efb24), SkBits2Float(0x436f104c), SkBits2Float(0x439efb24));  // 249.419f, 317.962f, 239.064f, 317.962f
path.quadTo(SkBits2Float(0x4364b554), SkBits2Float(0x439efb24), SkBits2Float(0x435d62d0), SkBits2Float(0x439b51e2));  // 228.708f, 317.962f, 221.386f, 310.64f
path.quadTo(SkBits2Float(0x4356104c), SkBits2Float(0x4397a8a0), SkBits2Float(0x4356104c), SkBits2Float(0x43927b24));  // 214.064f, 303.317f, 214.064f, 292.962f
path.quadTo(SkBits2Float(0x4356104c), SkBits2Float(0x438d4da8), SkBits2Float(0x435d62d0), SkBits2Float(0x4389a466));  // 214.064f, 282.607f, 221.386f, 275.284f
path.quadTo(SkBits2Float(0x4364b554), SkBits2Float(0x4385fb24), SkBits2Float(0x436f104c), SkBits2Float(0x4385fb24));  // 228.708f, 267.962f, 239.064f, 267.962f
path.quadTo(SkBits2Float(0x43796b44), SkBits2Float(0x4385fb24), SkBits2Float(0x43805ee4), SkBits2Float(0x4389a466));  // 249.419f, 267.962f, 256.741f, 275.284f
path.quadTo(SkBits2Float(0x43840826), SkBits2Float(0x438d4da8), SkBits2Float(0x43840826), SkBits2Float(0x43927b24));  // 264.064f, 282.607f, 264.064f, 292.962f
path.close();
    SkPath path66(path);
    builder.add(path66, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4387c794), SkBits2Float(0x43567a78));  // 271.559f, 214.478f
path.quadTo(SkBits2Float(0x4387c794), SkBits2Float(0x4360d570), SkBits2Float(0x43841e52), SkBits2Float(0x436827f4));  // 271.559f, 224.834f, 264.237f, 232.156f
path.quadTo(SkBits2Float(0x43807510), SkBits2Float(0x436f7a78), SkBits2Float(0x43768f28), SkBits2Float(0x436f7a78));  // 256.915f, 239.478f, 246.559f, 239.478f
path.quadTo(SkBits2Float(0x436c3430), SkBits2Float(0x436f7a78), SkBits2Float(0x4364e1ac), SkBits2Float(0x436827f4));  // 236.204f, 239.478f, 228.882f, 232.156f
path.quadTo(SkBits2Float(0x435d8f27), SkBits2Float(0x4360d570), SkBits2Float(0x435d8f27), SkBits2Float(0x43567a78));  // 221.559f, 224.834f, 221.559f, 214.478f
path.quadTo(SkBits2Float(0x435d8f27), SkBits2Float(0x434c1f80), SkBits2Float(0x4364e1ac), SkBits2Float(0x4344ccfc));  // 221.559f, 204.123f, 228.882f, 196.801f
path.quadTo(SkBits2Float(0x436c3430), SkBits2Float(0x433d7a78), SkBits2Float(0x43768f28), SkBits2Float(0x433d7a78));  // 236.204f, 189.478f, 246.559f, 189.478f
path.quadTo(SkBits2Float(0x43807510), SkBits2Float(0x433d7a78), SkBits2Float(0x43841e52), SkBits2Float(0x4344ccfc));  // 256.915f, 189.478f, 264.237f, 196.801f
path.quadTo(SkBits2Float(0x4387c794), SkBits2Float(0x434c1f80), SkBits2Float(0x4387c794), SkBits2Float(0x43567a78));  // 271.559f, 204.123f, 271.559f, 214.478f
path.close();
    SkPath path67(path);
    builder.add(path67, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43cfc71f), SkBits2Float(0x4314ea3c));  // 415.556f, 148.915f
path.quadTo(SkBits2Float(0x43cfc71f), SkBits2Float(0x431f4534), SkBits2Float(0x43cc1ddd), SkBits2Float(0x432697b8));  // 415.556f, 159.27f, 408.233f, 166.593f
path.quadTo(SkBits2Float(0x43c8749b), SkBits2Float(0x432dea3c), SkBits2Float(0x43c3471f), SkBits2Float(0x432dea3c));  // 400.911f, 173.915f, 390.556f, 173.915f
path.quadTo(SkBits2Float(0x43be19a3), SkBits2Float(0x432dea3c), SkBits2Float(0x43ba7061), SkBits2Float(0x432697b8));  // 380.2f, 173.915f, 372.878f, 166.593f
path.quadTo(SkBits2Float(0x43b6c71f), SkBits2Float(0x431f4534), SkBits2Float(0x43b6c71f), SkBits2Float(0x4314ea3c));  // 365.556f, 159.27f, 365.556f, 148.915f
path.quadTo(SkBits2Float(0x43b6c71f), SkBits2Float(0x430a8f44), SkBits2Float(0x43ba7061), SkBits2Float(0x43033cc0));  // 365.556f, 138.56f, 372.878f, 131.237f
path.quadTo(SkBits2Float(0x43be19a3), SkBits2Float(0x42f7d478), SkBits2Float(0x43c3471f), SkBits2Float(0x42f7d478));  // 380.2f, 123.915f, 390.556f, 123.915f
path.quadTo(SkBits2Float(0x43c8749b), SkBits2Float(0x42f7d478), SkBits2Float(0x43cc1ddd), SkBits2Float(0x43033cc0));  // 400.911f, 123.915f, 408.233f, 131.237f
path.quadTo(SkBits2Float(0x43cfc71f), SkBits2Float(0x430a8f44), SkBits2Float(0x43cfc71f), SkBits2Float(0x4314ea3c));  // 415.556f, 138.56f, 415.556f, 148.915f
path.close();
    SkPath path68(path);
    builder.add(path68, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a48336), SkBits2Float(0x4336f503));  // 329.025f, 182.957f
path.quadTo(SkBits2Float(0x43a48336), SkBits2Float(0x43414ffa), SkBits2Float(0x43a0d9f4), SkBits2Float(0x4348a27f));  // 329.025f, 193.312f, 321.703f, 200.635f
path.quadTo(SkBits2Float(0x439d30b2), SkBits2Float(0x434ff503), SkBits2Float(0x43980336), SkBits2Float(0x434ff503));  // 314.38f, 207.957f, 304.025f, 207.957f
path.quadTo(SkBits2Float(0x4392d5ba), SkBits2Float(0x434ff503), SkBits2Float(0x438f2c78), SkBits2Float(0x4348a27f));  // 293.67f, 207.957f, 286.347f, 200.635f
path.quadTo(SkBits2Float(0x438b8336), SkBits2Float(0x43414ffa), SkBits2Float(0x438b8336), SkBits2Float(0x4336f503));  // 279.025f, 193.312f, 279.025f, 182.957f
path.quadTo(SkBits2Float(0x438b8336), SkBits2Float(0x432c9a0c), SkBits2Float(0x438f2c78), SkBits2Float(0x43254787));  // 279.025f, 172.602f, 286.347f, 165.279f
path.quadTo(SkBits2Float(0x4392d5ba), SkBits2Float(0x431df503), SkBits2Float(0x43980336), SkBits2Float(0x431df503));  // 293.67f, 157.957f, 304.025f, 157.957f
path.quadTo(SkBits2Float(0x439d30b2), SkBits2Float(0x431df503), SkBits2Float(0x43a0d9f4), SkBits2Float(0x43254787));  // 314.38f, 157.957f, 321.703f, 165.279f
path.quadTo(SkBits2Float(0x43a48336), SkBits2Float(0x432c9a0c), SkBits2Float(0x43a48336), SkBits2Float(0x4336f503));  // 329.025f, 172.602f, 329.025f, 182.957f
path.close();
    SkPath path69(path);
    builder.add(path69, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x439a774e), SkBits2Float(0x43a57837));  // 308.932f, 330.939f
path.quadTo(SkBits2Float(0x439a774e), SkBits2Float(0x43aaa5b3), SkBits2Float(0x4396ce0c), SkBits2Float(0x43ae4ef5));  // 308.932f, 341.295f, 301.61f, 348.617f
path.quadTo(SkBits2Float(0x439324ca), SkBits2Float(0x43b1f837), SkBits2Float(0x438df74e), SkBits2Float(0x43b1f837));  // 294.287f, 355.939f, 283.932f, 355.939f
path.quadTo(SkBits2Float(0x4388c9d2), SkBits2Float(0x43b1f837), SkBits2Float(0x43852090), SkBits2Float(0x43ae4ef5));  // 273.577f, 355.939f, 266.254f, 348.617f
path.quadTo(SkBits2Float(0x4381774e), SkBits2Float(0x43aaa5b3), SkBits2Float(0x4381774e), SkBits2Float(0x43a57837));  // 258.932f, 341.295f, 258.932f, 330.939f
path.quadTo(SkBits2Float(0x4381774e), SkBits2Float(0x43a04abb), SkBits2Float(0x43852090), SkBits2Float(0x439ca179));  // 258.932f, 320.584f, 266.254f, 313.262f
path.quadTo(SkBits2Float(0x4388c9d2), SkBits2Float(0x4398f837), SkBits2Float(0x438df74e), SkBits2Float(0x4398f837));  // 273.577f, 305.939f, 283.932f, 305.939f
path.quadTo(SkBits2Float(0x439324ca), SkBits2Float(0x4398f837), SkBits2Float(0x4396ce0c), SkBits2Float(0x439ca179));  // 294.287f, 305.939f, 301.61f, 313.262f
path.quadTo(SkBits2Float(0x439a774e), SkBits2Float(0x43a04abb), SkBits2Float(0x439a774e), SkBits2Float(0x43a57837));  // 308.932f, 320.584f, 308.932f, 330.939f
path.close();
    SkPath path70(path);
    builder.add(path70, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x439be624), SkBits2Float(0x438cec6f));  // 311.798f, 281.847f
path.quadTo(SkBits2Float(0x439be624), SkBits2Float(0x439219eb), SkBits2Float(0x43983ce2), SkBits2Float(0x4395c32d));  // 311.798f, 292.202f, 304.476f, 299.525f
path.quadTo(SkBits2Float(0x439493a0), SkBits2Float(0x43996c6f), SkBits2Float(0x438f6624), SkBits2Float(0x43996c6f));  // 297.153f, 306.847f, 286.798f, 306.847f
path.quadTo(SkBits2Float(0x438a38a8), SkBits2Float(0x43996c6f), SkBits2Float(0x43868f66), SkBits2Float(0x4395c32d));  // 276.443f, 306.847f, 269.12f, 299.525f
path.quadTo(SkBits2Float(0x4382e624), SkBits2Float(0x439219eb), SkBits2Float(0x4382e624), SkBits2Float(0x438cec6f));  // 261.798f, 292.202f, 261.798f, 281.847f
path.quadTo(SkBits2Float(0x4382e624), SkBits2Float(0x4387bef3), SkBits2Float(0x43868f66), SkBits2Float(0x438415b1));  // 261.798f, 271.492f, 269.12f, 264.169f
path.quadTo(SkBits2Float(0x438a38a8), SkBits2Float(0x43806c6f), SkBits2Float(0x438f6624), SkBits2Float(0x43806c6f));  // 276.443f, 256.847f, 286.798f, 256.847f
path.quadTo(SkBits2Float(0x439493a0), SkBits2Float(0x43806c6f), SkBits2Float(0x43983ce2), SkBits2Float(0x438415b1));  // 297.153f, 256.847f, 304.476f, 264.169f
path.quadTo(SkBits2Float(0x439be624), SkBits2Float(0x4387bef3), SkBits2Float(0x439be624), SkBits2Float(0x438cec6f));  // 311.798f, 271.492f, 311.798f, 281.847f
path.close();
    SkPath path71(path);
    builder.add(path71, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43714851), SkBits2Float(0x43aff2c0));  // 241.282f, 351.896f
path.quadTo(SkBits2Float(0x43714851), SkBits2Float(0x43b5203c), SkBits2Float(0x4369f5cd), SkBits2Float(0x43b8c97e));  // 241.282f, 362.252f, 233.96f, 369.574f
path.quadTo(SkBits2Float(0x4362a348), SkBits2Float(0x43bc72c0), SkBits2Float(0x43584851), SkBits2Float(0x43bc72c0));  // 226.638f, 376.896f, 216.282f, 376.896f
path.quadTo(SkBits2Float(0x434ded5a), SkBits2Float(0x43bc72c0), SkBits2Float(0x43469ad5), SkBits2Float(0x43b8c97e));  // 205.927f, 376.896f, 198.605f, 369.574f
path.quadTo(SkBits2Float(0x433f4851), SkBits2Float(0x43b5203c), SkBits2Float(0x433f4851), SkBits2Float(0x43aff2c0));  // 191.282f, 362.252f, 191.282f, 351.896f
path.quadTo(SkBits2Float(0x433f4851), SkBits2Float(0x43aac544), SkBits2Float(0x43469ad5), SkBits2Float(0x43a71c02));  // 191.282f, 341.541f, 198.605f, 334.219f
path.quadTo(SkBits2Float(0x434ded5a), SkBits2Float(0x43a372c0), SkBits2Float(0x43584851), SkBits2Float(0x43a372c0));  // 205.927f, 326.896f, 216.282f, 326.896f
path.quadTo(SkBits2Float(0x4362a348), SkBits2Float(0x43a372c0), SkBits2Float(0x4369f5cd), SkBits2Float(0x43a71c02));  // 226.638f, 326.896f, 233.96f, 334.219f
path.quadTo(SkBits2Float(0x43714851), SkBits2Float(0x43aac544), SkBits2Float(0x43714851), SkBits2Float(0x43aff2c0));  // 241.282f, 341.541f, 241.282f, 351.896f
path.close();
    SkPath path72(path);
    builder.add(path72, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43e644d3), SkBits2Float(0x43041b70));  // 460.538f, 132.107f
path.quadTo(SkBits2Float(0x43e644d3), SkBits2Float(0x430e7668), SkBits2Float(0x43e29b91), SkBits2Float(0x4315c8ec));  // 460.538f, 142.463f, 453.215f, 149.785f
path.quadTo(SkBits2Float(0x43def24f), SkBits2Float(0x431d1b70), SkBits2Float(0x43d9c4d3), SkBits2Float(0x431d1b70));  // 445.893f, 157.107f, 435.538f, 157.107f
path.quadTo(SkBits2Float(0x43d49757), SkBits2Float(0x431d1b70), SkBits2Float(0x43d0ee15), SkBits2Float(0x4315c8ec));  // 425.182f, 157.107f, 417.86f, 149.785f
path.quadTo(SkBits2Float(0x43cd44d3), SkBits2Float(0x430e7668), SkBits2Float(0x43cd44d3), SkBits2Float(0x43041b70));  // 410.538f, 142.463f, 410.538f, 132.107f
path.quadTo(SkBits2Float(0x43cd44d3), SkBits2Float(0x42f380f1), SkBits2Float(0x43d0ee15), SkBits2Float(0x42e4dbe8));  // 410.538f, 121.752f, 417.86f, 114.43f
path.quadTo(SkBits2Float(0x43d49757), SkBits2Float(0x42d636e0), SkBits2Float(0x43d9c4d3), SkBits2Float(0x42d636e0));  // 425.182f, 107.107f, 435.538f, 107.107f
path.quadTo(SkBits2Float(0x43def24f), SkBits2Float(0x42d636e0), SkBits2Float(0x43e29b91), SkBits2Float(0x42e4dbe8));  // 445.893f, 107.107f, 453.215f, 114.43f
path.quadTo(SkBits2Float(0x43e644d3), SkBits2Float(0x42f380f1), SkBits2Float(0x43e644d3), SkBits2Float(0x43041b70));  // 460.538f, 121.752f, 460.538f, 132.107f
path.close();
    SkPath path73(path);
    builder.add(path73, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43c29be7), SkBits2Float(0x4366bcd5));  // 389.218f, 230.738f
path.quadTo(SkBits2Float(0x43c29be7), SkBits2Float(0x437117cc), SkBits2Float(0x43bef2a5), SkBits2Float(0x43786a51));  // 389.218f, 241.093f, 381.896f, 248.415f
path.quadTo(SkBits2Float(0x43bb4963), SkBits2Float(0x437fbcd5), SkBits2Float(0x43b61be7), SkBits2Float(0x437fbcd5));  // 374.573f, 255.738f, 364.218f, 255.738f
path.quadTo(SkBits2Float(0x43b0ee6b), SkBits2Float(0x437fbcd5), SkBits2Float(0x43ad4529), SkBits2Float(0x43786a51));  // 353.863f, 255.738f, 346.54f, 248.415f
path.quadTo(SkBits2Float(0x43a99be7), SkBits2Float(0x437117cc), SkBits2Float(0x43a99be7), SkBits2Float(0x4366bcd5));  // 339.218f, 241.093f, 339.218f, 230.738f
path.quadTo(SkBits2Float(0x43a99be7), SkBits2Float(0x435c61de), SkBits2Float(0x43ad4529), SkBits2Float(0x43550f59));  // 339.218f, 220.382f, 346.54f, 213.06f
path.quadTo(SkBits2Float(0x43b0ee6b), SkBits2Float(0x434dbcd5), SkBits2Float(0x43b61be7), SkBits2Float(0x434dbcd5));  // 353.863f, 205.738f, 364.218f, 205.738f
path.quadTo(SkBits2Float(0x43bb4963), SkBits2Float(0x434dbcd5), SkBits2Float(0x43bef2a5), SkBits2Float(0x43550f59));  // 374.573f, 205.738f, 381.896f, 213.06f
path.quadTo(SkBits2Float(0x43c29be7), SkBits2Float(0x435c61de), SkBits2Float(0x43c29be7), SkBits2Float(0x4366bcd5));  // 389.218f, 220.382f, 389.218f, 230.738f
path.close();
    SkPath path74(path);
    builder.add(path74, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43b8637d), SkBits2Float(0x435f209d));  // 368.777f, 223.127f
path.quadTo(SkBits2Float(0x43b8637d), SkBits2Float(0x43697b94), SkBits2Float(0x43b4ba3b), SkBits2Float(0x4370ce19));  // 368.777f, 233.483f, 361.455f, 240.805f
path.quadTo(SkBits2Float(0x43b110f9), SkBits2Float(0x4378209d), SkBits2Float(0x43abe37d), SkBits2Float(0x4378209d));  // 354.133f, 248.127f, 343.777f, 248.127f
path.quadTo(SkBits2Float(0x43a6b601), SkBits2Float(0x4378209d), SkBits2Float(0x43a30cbf), SkBits2Float(0x4370ce19));  // 333.422f, 248.127f, 326.1f, 240.805f
path.quadTo(SkBits2Float(0x439f637d), SkBits2Float(0x43697b94), SkBits2Float(0x439f637d), SkBits2Float(0x435f209d));  // 318.777f, 233.483f, 318.777f, 223.127f
path.quadTo(SkBits2Float(0x439f637d), SkBits2Float(0x4354c5a6), SkBits2Float(0x43a30cbf), SkBits2Float(0x434d7321));  // 318.777f, 212.772f, 326.1f, 205.45f
path.quadTo(SkBits2Float(0x43a6b601), SkBits2Float(0x4346209d), SkBits2Float(0x43abe37d), SkBits2Float(0x4346209d));  // 333.422f, 198.127f, 343.777f, 198.127f
path.quadTo(SkBits2Float(0x43b110f9), SkBits2Float(0x4346209d), SkBits2Float(0x43b4ba3b), SkBits2Float(0x434d7321));  // 354.133f, 198.127f, 361.455f, 205.45f
path.quadTo(SkBits2Float(0x43b8637d), SkBits2Float(0x4354c5a6), SkBits2Float(0x43b8637d), SkBits2Float(0x435f209d));  // 368.777f, 212.772f, 368.777f, 223.127f
path.close();
    SkPath path75(path);
    builder.add(path75, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a3ec8f), SkBits2Float(0x435dba35));  // 327.848f, 221.727f
path.quadTo(SkBits2Float(0x43a3ec8f), SkBits2Float(0x4368152c), SkBits2Float(0x43a0434d), SkBits2Float(0x436f67b1));  // 327.848f, 232.083f, 320.526f, 239.405f
path.quadTo(SkBits2Float(0x439c9a0b), SkBits2Float(0x4376ba35), SkBits2Float(0x43976c8f), SkBits2Float(0x4376ba35));  // 313.203f, 246.727f, 302.848f, 246.727f
path.quadTo(SkBits2Float(0x43923f13), SkBits2Float(0x4376ba35), SkBits2Float(0x438e95d1), SkBits2Float(0x436f67b1));  // 292.493f, 246.727f, 285.17f, 239.405f
path.quadTo(SkBits2Float(0x438aec8f), SkBits2Float(0x4368152c), SkBits2Float(0x438aec8f), SkBits2Float(0x435dba35));  // 277.848f, 232.083f, 277.848f, 221.727f
path.quadTo(SkBits2Float(0x438aec8f), SkBits2Float(0x43535f3e), SkBits2Float(0x438e95d1), SkBits2Float(0x434c0cb9));  // 277.848f, 211.372f, 285.17f, 204.05f
path.quadTo(SkBits2Float(0x43923f13), SkBits2Float(0x4344ba35), SkBits2Float(0x43976c8f), SkBits2Float(0x4344ba35));  // 292.493f, 196.727f, 302.848f, 196.727f
path.quadTo(SkBits2Float(0x439c9a0b), SkBits2Float(0x4344ba35), SkBits2Float(0x43a0434d), SkBits2Float(0x434c0cb9));  // 313.203f, 196.727f, 320.526f, 204.05f
path.quadTo(SkBits2Float(0x43a3ec8f), SkBits2Float(0x43535f3e), SkBits2Float(0x43a3ec8f), SkBits2Float(0x435dba35));  // 327.848f, 211.372f, 327.848f, 221.727f
path.close();
    SkPath path76(path);
    builder.add(path76, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4408ede5), SkBits2Float(0x436af388));  // 547.717f, 234.951f
path.quadTo(SkBits2Float(0x4408ede5), SkBits2Float(0x43754e7f), SkBits2Float(0x44071944), SkBits2Float(0x437ca103));  // 547.717f, 245.307f, 540.395f, 252.629f
path.quadTo(SkBits2Float(0x440544a3), SkBits2Float(0x4381f9c4), SkBits2Float(0x4402ade5), SkBits2Float(0x4381f9c4));  // 533.072f, 259.951f, 522.717f, 259.951f
path.quadTo(SkBits2Float(0x44001727), SkBits2Float(0x4381f9c4), SkBits2Float(0x43fc850c), SkBits2Float(0x437ca103));  // 512.362f, 259.951f, 505.039f, 252.629f
path.quadTo(SkBits2Float(0x43f8dbca), SkBits2Float(0x43754e7f), SkBits2Float(0x43f8dbca), SkBits2Float(0x436af388));  // 497.717f, 245.307f, 497.717f, 234.951f
path.quadTo(SkBits2Float(0x43f8dbca), SkBits2Float(0x43609891), SkBits2Float(0x43fc850c), SkBits2Float(0x4359460d));  // 497.717f, 224.596f, 505.039f, 217.274f
path.quadTo(SkBits2Float(0x44001727), SkBits2Float(0x4351f389), SkBits2Float(0x4402ade5), SkBits2Float(0x4351f389));  // 512.362f, 209.951f, 522.717f, 209.951f
path.quadTo(SkBits2Float(0x440544a3), SkBits2Float(0x4351f389), SkBits2Float(0x44071944), SkBits2Float(0x4359460d));  // 533.072f, 209.951f, 540.395f, 217.274f
path.quadTo(SkBits2Float(0x4408ede5), SkBits2Float(0x43609891), SkBits2Float(0x4408ede5), SkBits2Float(0x436af388));  // 547.717f, 224.596f, 547.717f, 234.951f
path.close();
    SkPath path77(path);
    builder.add(path77, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a06718), SkBits2Float(0x43848d47));  // 320.805f, 265.104f
path.quadTo(SkBits2Float(0x43a06718), SkBits2Float(0x4389bac3), SkBits2Float(0x439cbdd6), SkBits2Float(0x438d6405));  // 320.805f, 275.459f, 313.483f, 282.781f
path.quadTo(SkBits2Float(0x43991494), SkBits2Float(0x43910d47), SkBits2Float(0x4393e718), SkBits2Float(0x43910d47));  // 306.161f, 290.104f, 295.805f, 290.104f
path.quadTo(SkBits2Float(0x438eb99c), SkBits2Float(0x43910d47), SkBits2Float(0x438b105a), SkBits2Float(0x438d6405));  // 285.45f, 290.104f, 278.128f, 282.781f
path.quadTo(SkBits2Float(0x43876718), SkBits2Float(0x4389bac3), SkBits2Float(0x43876718), SkBits2Float(0x43848d47));  // 270.805f, 275.459f, 270.805f, 265.104f
path.quadTo(SkBits2Float(0x43876718), SkBits2Float(0x437ebf96), SkBits2Float(0x438b105a), SkBits2Float(0x43776d12));  // 270.805f, 254.748f, 278.128f, 247.426f
path.quadTo(SkBits2Float(0x438eb99c), SkBits2Float(0x43701a8e), SkBits2Float(0x4393e718), SkBits2Float(0x43701a8e));  // 285.45f, 240.104f, 295.805f, 240.104f
path.quadTo(SkBits2Float(0x43991494), SkBits2Float(0x43701a8e), SkBits2Float(0x439cbdd6), SkBits2Float(0x43776d12));  // 306.161f, 240.104f, 313.483f, 247.426f
path.quadTo(SkBits2Float(0x43a06718), SkBits2Float(0x437ebf96), SkBits2Float(0x43a06718), SkBits2Float(0x43848d47));  // 320.805f, 254.748f, 320.805f, 265.104f
path.close();
    SkPath path78(path);
    builder.add(path78, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a4f721), SkBits2Float(0x439dea2f));  // 329.931f, 315.83f
path.quadTo(SkBits2Float(0x43a4f721), SkBits2Float(0x43a317ab), SkBits2Float(0x43a14ddf), SkBits2Float(0x43a6c0ed));  // 329.931f, 326.185f, 322.608f, 333.507f
path.quadTo(SkBits2Float(0x439da49d), SkBits2Float(0x43aa6a2f), SkBits2Float(0x43987721), SkBits2Float(0x43aa6a2f));  // 315.286f, 340.83f, 304.931f, 340.83f
path.quadTo(SkBits2Float(0x439349a5), SkBits2Float(0x43aa6a2f), SkBits2Float(0x438fa063), SkBits2Float(0x43a6c0ed));  // 294.575f, 340.83f, 287.253f, 333.507f
path.quadTo(SkBits2Float(0x438bf721), SkBits2Float(0x43a317ab), SkBits2Float(0x438bf721), SkBits2Float(0x439dea2f));  // 279.931f, 326.185f, 279.931f, 315.83f
path.quadTo(SkBits2Float(0x438bf721), SkBits2Float(0x4398bcb3), SkBits2Float(0x438fa063), SkBits2Float(0x43951371));  // 279.931f, 305.474f, 287.253f, 298.152f
path.quadTo(SkBits2Float(0x439349a5), SkBits2Float(0x43916a2f), SkBits2Float(0x43987721), SkBits2Float(0x43916a2f));  // 294.575f, 290.83f, 304.931f, 290.83f
path.quadTo(SkBits2Float(0x439da49d), SkBits2Float(0x43916a2f), SkBits2Float(0x43a14ddf), SkBits2Float(0x43951371));  // 315.286f, 290.83f, 322.608f, 298.152f
path.quadTo(SkBits2Float(0x43a4f721), SkBits2Float(0x4398bcb3), SkBits2Float(0x43a4f721), SkBits2Float(0x439dea2f));  // 329.931f, 305.474f, 329.931f, 315.83f
path.close();
    SkPath path79(path);
    builder.add(path79, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4343f4a9), SkBits2Float(0x434a2b20));  // 195.956f, 202.168f
path.quadTo(SkBits2Float(0x4343f4a9), SkBits2Float(0x43548618), SkBits2Float(0x433ca225), SkBits2Float(0x435bd89c));  // 195.956f, 212.524f, 188.633f, 219.846f
path.quadTo(SkBits2Float(0x43354fa0), SkBits2Float(0x43632b20), SkBits2Float(0x432af4a9), SkBits2Float(0x43632b20));  // 181.311f, 227.168f, 170.956f, 227.168f
path.quadTo(SkBits2Float(0x432099b2), SkBits2Float(0x43632b20), SkBits2Float(0x4319472d), SkBits2Float(0x435bd89c));  // 160.6f, 227.168f, 153.278f, 219.846f
path.quadTo(SkBits2Float(0x4311f4a9), SkBits2Float(0x43548618), SkBits2Float(0x4311f4a9), SkBits2Float(0x434a2b20));  // 145.956f, 212.524f, 145.956f, 202.168f
path.quadTo(SkBits2Float(0x4311f4a9), SkBits2Float(0x433fd028), SkBits2Float(0x4319472d), SkBits2Float(0x43387da4));  // 145.956f, 191.813f, 153.278f, 184.491f
path.quadTo(SkBits2Float(0x432099b2), SkBits2Float(0x43312b20), SkBits2Float(0x432af4a9), SkBits2Float(0x43312b20));  // 160.6f, 177.168f, 170.956f, 177.168f
path.quadTo(SkBits2Float(0x43354fa0), SkBits2Float(0x43312b20), SkBits2Float(0x433ca225), SkBits2Float(0x43387da4));  // 181.311f, 177.168f, 188.633f, 184.491f
path.quadTo(SkBits2Float(0x4343f4a9), SkBits2Float(0x433fd028), SkBits2Float(0x4343f4a9), SkBits2Float(0x434a2b20));  // 195.956f, 191.813f, 195.956f, 202.168f
path.close();
    SkPath path80(path);
    builder.add(path80, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4281a60e), SkBits2Float(0x42f78d6e));  // 64.8243f, 123.776f
path.quadTo(SkBits2Float(0x4281a60e), SkBits2Float(0x430621ae), SkBits2Float(0x4266020c), SkBits2Float(0x430d7433));  // 64.8243f, 134.132f, 57.502f, 141.454f
path.quadTo(SkBits2Float(0x4248b7fa), SkBits2Float(0x4314c6b7), SkBits2Float(0x421f4c1c), SkBits2Float(0x4314c6b7));  // 50.1797f, 148.776f, 39.8243f, 148.776f
path.quadTo(SkBits2Float(0x41ebc07c), SkBits2Float(0x4314c6b7), SkBits2Float(0x41b12c59), SkBits2Float(0x430d7433));  // 29.469f, 148.776f, 22.1467f, 141.454f
path.quadTo(SkBits2Float(0x416d306c), SkBits2Float(0x430621ae), SkBits2Float(0x416d306c), SkBits2Float(0x42f78d6e));  // 14.8243f, 134.132f, 14.8243f, 123.776f
path.quadTo(SkBits2Float(0x416d306c), SkBits2Float(0x42e2d77f), SkBits2Float(0x41b12c59), SkBits2Float(0x42d43276));  // 14.8243f, 113.421f, 22.1467f, 106.099f
path.quadTo(SkBits2Float(0x41ebc07c), SkBits2Float(0x42c58d6e), SkBits2Float(0x421f4c1c), SkBits2Float(0x42c58d6e));  // 29.469f, 98.7762f, 39.8243f, 98.7762f
path.quadTo(SkBits2Float(0x4248b7fa), SkBits2Float(0x42c58d6e), SkBits2Float(0x4266020c), SkBits2Float(0x42d43276));  // 50.1797f, 98.7762f, 57.502f, 106.099f
path.quadTo(SkBits2Float(0x4281a60e), SkBits2Float(0x42e2d77f), SkBits2Float(0x4281a60e), SkBits2Float(0x42f78d6e));  // 64.8243f, 113.421f, 64.8243f, 123.776f
path.close();
    SkPath path81(path);
    builder.add(path81, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43740113), SkBits2Float(0x4363dcf0));  // 244.004f, 227.863f
path.quadTo(SkBits2Float(0x43740113), SkBits2Float(0x436e37e8), SkBits2Float(0x436cae8f), SkBits2Float(0x43758a6c));  // 244.004f, 238.218f, 236.682f, 245.541f
path.quadTo(SkBits2Float(0x43655c0a), SkBits2Float(0x437cdcf0), SkBits2Float(0x435b0113), SkBits2Float(0x437cdcf0));  // 229.36f, 252.863f, 219.004f, 252.863f
path.quadTo(SkBits2Float(0x4350a61c), SkBits2Float(0x437cdcf0), SkBits2Float(0x43495397), SkBits2Float(0x43758a6c));  // 208.649f, 252.863f, 201.327f, 245.541f
path.quadTo(SkBits2Float(0x43420113), SkBits2Float(0x436e37e8), SkBits2Float(0x43420113), SkBits2Float(0x4363dcf0));  // 194.004f, 238.218f, 194.004f, 227.863f
path.quadTo(SkBits2Float(0x43420113), SkBits2Float(0x435981f8), SkBits2Float(0x43495397), SkBits2Float(0x43522f74));  // 194.004f, 217.508f, 201.327f, 210.185f
path.quadTo(SkBits2Float(0x4350a61c), SkBits2Float(0x434adcf0), SkBits2Float(0x435b0113), SkBits2Float(0x434adcf0));  // 208.649f, 202.863f, 219.004f, 202.863f
path.quadTo(SkBits2Float(0x43655c0a), SkBits2Float(0x434adcf0), SkBits2Float(0x436cae8f), SkBits2Float(0x43522f74));  // 229.36f, 202.863f, 236.682f, 210.185f
path.quadTo(SkBits2Float(0x43740113), SkBits2Float(0x435981f8), SkBits2Float(0x43740113), SkBits2Float(0x4363dcf0));  // 244.004f, 217.508f, 244.004f, 227.863f
path.close();
    SkPath path82(path);
    builder.add(path82, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43bf0464), SkBits2Float(0x431e17ca));  // 382.034f, 158.093f
path.quadTo(SkBits2Float(0x43bf0464), SkBits2Float(0x432872c2), SkBits2Float(0x43bb5b22), SkBits2Float(0x432fc546));  // 382.034f, 168.448f, 374.712f, 175.771f
path.quadTo(SkBits2Float(0x43b7b1e0), SkBits2Float(0x433717ca), SkBits2Float(0x43b28464), SkBits2Float(0x433717ca));  // 367.39f, 183.093f, 357.034f, 183.093f
path.quadTo(SkBits2Float(0x43ad56e8), SkBits2Float(0x433717ca), SkBits2Float(0x43a9ada6), SkBits2Float(0x432fc546));  // 346.679f, 183.093f, 339.357f, 175.771f
path.quadTo(SkBits2Float(0x43a60464), SkBits2Float(0x432872c2), SkBits2Float(0x43a60464), SkBits2Float(0x431e17ca));  // 332.034f, 168.448f, 332.034f, 158.093f
path.quadTo(SkBits2Float(0x43a60464), SkBits2Float(0x4313bcd2), SkBits2Float(0x43a9ada6), SkBits2Float(0x430c6a4e));  // 332.034f, 147.738f, 339.357f, 140.415f
path.quadTo(SkBits2Float(0x43ad56e8), SkBits2Float(0x430517ca), SkBits2Float(0x43b28464), SkBits2Float(0x430517ca));  // 346.679f, 133.093f, 357.034f, 133.093f
path.quadTo(SkBits2Float(0x43b7b1e0), SkBits2Float(0x430517ca), SkBits2Float(0x43bb5b22), SkBits2Float(0x430c6a4e));  // 367.39f, 133.093f, 374.712f, 140.415f
path.quadTo(SkBits2Float(0x43bf0464), SkBits2Float(0x4313bcd2), SkBits2Float(0x43bf0464), SkBits2Float(0x431e17ca));  // 382.034f, 147.738f, 382.034f, 158.093f
path.close();
    SkPath path83(path);
    builder.add(path83, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a72678), SkBits2Float(0x438273e0));  // 334.301f, 260.905f
path.quadTo(SkBits2Float(0x43a72678), SkBits2Float(0x4387a15c), SkBits2Float(0x43a37d36), SkBits2Float(0x438b4a9e));  // 334.301f, 271.261f, 326.978f, 278.583f
path.quadTo(SkBits2Float(0x439fd3f4), SkBits2Float(0x438ef3e0), SkBits2Float(0x439aa678), SkBits2Float(0x438ef3e0));  // 319.656f, 285.905f, 309.301f, 285.905f
path.quadTo(SkBits2Float(0x439578fc), SkBits2Float(0x438ef3e0), SkBits2Float(0x4391cfba), SkBits2Float(0x438b4a9e));  // 298.945f, 285.905f, 291.623f, 278.583f
path.quadTo(SkBits2Float(0x438e2678), SkBits2Float(0x4387a15c), SkBits2Float(0x438e2678), SkBits2Float(0x438273e0));  // 284.301f, 271.261f, 284.301f, 260.905f
path.quadTo(SkBits2Float(0x438e2678), SkBits2Float(0x437a8cc8), SkBits2Float(0x4391cfba), SkBits2Float(0x43733a44));  // 284.301f, 250.55f, 291.623f, 243.228f
path.quadTo(SkBits2Float(0x439578fc), SkBits2Float(0x436be7c0), SkBits2Float(0x439aa678), SkBits2Float(0x436be7c0));  // 298.945f, 235.905f, 309.301f, 235.905f
path.quadTo(SkBits2Float(0x439fd3f4), SkBits2Float(0x436be7c0), SkBits2Float(0x43a37d36), SkBits2Float(0x43733a44));  // 319.656f, 235.905f, 326.978f, 243.228f
path.quadTo(SkBits2Float(0x43a72678), SkBits2Float(0x437a8cc8), SkBits2Float(0x43a72678), SkBits2Float(0x438273e0));  // 334.301f, 250.55f, 334.301f, 260.905f
path.close();
    SkPath path84(path);
    builder.add(path84, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4361bf09), SkBits2Float(0x43464372));  // 225.746f, 198.263f
path.quadTo(SkBits2Float(0x4361bf09), SkBits2Float(0x43509e6a), SkBits2Float(0x435a6c85), SkBits2Float(0x4357f0ee));  // 225.746f, 208.619f, 218.424f, 215.941f
path.quadTo(SkBits2Float(0x43531a00), SkBits2Float(0x435f4372), SkBits2Float(0x4348bf09), SkBits2Float(0x435f4372));  // 211.102f, 223.263f, 200.746f, 223.263f
path.quadTo(SkBits2Float(0x433e6412), SkBits2Float(0x435f4372), SkBits2Float(0x4337118d), SkBits2Float(0x4357f0ee));  // 190.391f, 223.263f, 183.069f, 215.941f
path.quadTo(SkBits2Float(0x432fbf09), SkBits2Float(0x43509e6a), SkBits2Float(0x432fbf09), SkBits2Float(0x43464372));  // 175.746f, 208.619f, 175.746f, 198.263f
path.quadTo(SkBits2Float(0x432fbf09), SkBits2Float(0x433be87a), SkBits2Float(0x4337118d), SkBits2Float(0x433495f6));  // 175.746f, 187.908f, 183.069f, 180.586f
path.quadTo(SkBits2Float(0x433e6412), SkBits2Float(0x432d4372), SkBits2Float(0x4348bf09), SkBits2Float(0x432d4372));  // 190.391f, 173.263f, 200.746f, 173.263f
path.quadTo(SkBits2Float(0x43531a00), SkBits2Float(0x432d4372), SkBits2Float(0x435a6c85), SkBits2Float(0x433495f6));  // 211.102f, 173.263f, 218.424f, 180.586f
path.quadTo(SkBits2Float(0x4361bf09), SkBits2Float(0x433be87a), SkBits2Float(0x4361bf09), SkBits2Float(0x43464372));  // 225.746f, 187.908f, 225.746f, 198.263f
path.close();
    SkPath path85(path);
    builder.add(path85, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4377220b), SkBits2Float(0x4392653c));  // 247.133f, 292.791f
path.quadTo(SkBits2Float(0x4377220b), SkBits2Float(0x439792b8), SkBits2Float(0x436fcf87), SkBits2Float(0x439b3bfa));  // 247.133f, 303.146f, 239.811f, 310.469f
path.quadTo(SkBits2Float(0x43687d02), SkBits2Float(0x439ee53c), SkBits2Float(0x435e220b), SkBits2Float(0x439ee53c));  // 232.488f, 317.791f, 222.133f, 317.791f
path.quadTo(SkBits2Float(0x4353c714), SkBits2Float(0x439ee53c), SkBits2Float(0x434c748f), SkBits2Float(0x439b3bfa));  // 211.778f, 317.791f, 204.455f, 310.469f
path.quadTo(SkBits2Float(0x4345220b), SkBits2Float(0x439792b8), SkBits2Float(0x4345220b), SkBits2Float(0x4392653c));  // 197.133f, 303.146f, 197.133f, 292.791f
path.quadTo(SkBits2Float(0x4345220b), SkBits2Float(0x438d37c0), SkBits2Float(0x434c748f), SkBits2Float(0x43898e7e));  // 197.133f, 282.436f, 204.455f, 275.113f
path.quadTo(SkBits2Float(0x4353c714), SkBits2Float(0x4385e53c), SkBits2Float(0x435e220b), SkBits2Float(0x4385e53c));  // 211.778f, 267.791f, 222.133f, 267.791f
path.quadTo(SkBits2Float(0x43687d02), SkBits2Float(0x4385e53c), SkBits2Float(0x436fcf87), SkBits2Float(0x43898e7e));  // 232.488f, 267.791f, 239.811f, 275.113f
path.quadTo(SkBits2Float(0x4377220b), SkBits2Float(0x438d37c0), SkBits2Float(0x4377220b), SkBits2Float(0x4392653c));  // 247.133f, 282.436f, 247.133f, 292.791f
path.close();
    SkPath path86(path);
    builder.add(path86, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4402a250), SkBits2Float(0x4331ae72));  // 522.536f, 177.681f
path.quadTo(SkBits2Float(0x4402a250), SkBits2Float(0x433c096a), SkBits2Float(0x4400cdaf), SkBits2Float(0x43435bee));  // 522.536f, 188.037f, 515.214f, 195.359f
path.quadTo(SkBits2Float(0x43fdf21c), SkBits2Float(0x434aae72), SkBits2Float(0x43f8c4a0), SkBits2Float(0x434aae72));  // 507.891f, 202.681f, 497.536f, 202.681f
path.quadTo(SkBits2Float(0x43f39724), SkBits2Float(0x434aae72), SkBits2Float(0x43efede2), SkBits2Float(0x43435bee));  // 487.181f, 202.681f, 479.858f, 195.359f
path.quadTo(SkBits2Float(0x43ec44a0), SkBits2Float(0x433c096a), SkBits2Float(0x43ec44a0), SkBits2Float(0x4331ae72));  // 472.536f, 188.037f, 472.536f, 177.681f
path.quadTo(SkBits2Float(0x43ec44a0), SkBits2Float(0x4327537a), SkBits2Float(0x43efede2), SkBits2Float(0x432000f6));  // 472.536f, 167.326f, 479.858f, 160.004f
path.quadTo(SkBits2Float(0x43f39724), SkBits2Float(0x4318ae72), SkBits2Float(0x43f8c4a0), SkBits2Float(0x4318ae72));  // 487.181f, 152.681f, 497.536f, 152.681f
path.quadTo(SkBits2Float(0x43fdf21c), SkBits2Float(0x4318ae72), SkBits2Float(0x4400cdaf), SkBits2Float(0x432000f6));  // 507.891f, 152.681f, 515.214f, 160.004f
path.quadTo(SkBits2Float(0x4402a250), SkBits2Float(0x4327537a), SkBits2Float(0x4402a250), SkBits2Float(0x4331ae72));  // 522.536f, 167.326f, 522.536f, 177.681f
path.close();
    SkPath path87(path);
    builder.add(path87, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x439cfd65), SkBits2Float(0x4331c20a));  // 313.98f, 177.758f
path.quadTo(SkBits2Float(0x439cfd65), SkBits2Float(0x433c1d02), SkBits2Float(0x43995423), SkBits2Float(0x43436f86));  // 313.98f, 188.113f, 306.657f, 195.436f
path.quadTo(SkBits2Float(0x4395aae1), SkBits2Float(0x434ac20a), SkBits2Float(0x43907d65), SkBits2Float(0x434ac20a));  // 299.335f, 202.758f, 288.98f, 202.758f
path.quadTo(SkBits2Float(0x438b4fe9), SkBits2Float(0x434ac20a), SkBits2Float(0x4387a6a7), SkBits2Float(0x43436f86));  // 278.624f, 202.758f, 271.302f, 195.436f
path.quadTo(SkBits2Float(0x4383fd65), SkBits2Float(0x433c1d02), SkBits2Float(0x4383fd65), SkBits2Float(0x4331c20a));  // 263.98f, 188.113f, 263.98f, 177.758f
path.quadTo(SkBits2Float(0x4383fd65), SkBits2Float(0x43276712), SkBits2Float(0x4387a6a7), SkBits2Float(0x4320148e));  // 263.98f, 167.403f, 271.302f, 160.08f
path.quadTo(SkBits2Float(0x438b4fe9), SkBits2Float(0x4318c20a), SkBits2Float(0x43907d65), SkBits2Float(0x4318c20a));  // 278.624f, 152.758f, 288.98f, 152.758f
path.quadTo(SkBits2Float(0x4395aae1), SkBits2Float(0x4318c20a), SkBits2Float(0x43995423), SkBits2Float(0x4320148e));  // 299.335f, 152.758f, 306.657f, 160.08f
path.quadTo(SkBits2Float(0x439cfd65), SkBits2Float(0x43276712), SkBits2Float(0x439cfd65), SkBits2Float(0x4331c20a));  // 313.98f, 167.403f, 313.98f, 177.758f
path.close();
    SkPath path88(path);
    builder.add(path88, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43c6cc0f), SkBits2Float(0x430c1343));  // 397.594f, 140.075f
path.quadTo(SkBits2Float(0x43c6cc0f), SkBits2Float(0x43166e3a), SkBits2Float(0x43c322cd), SkBits2Float(0x431dc0bf));  // 397.594f, 150.431f, 390.272f, 157.753f
path.quadTo(SkBits2Float(0x43bf798b), SkBits2Float(0x43251343), SkBits2Float(0x43ba4c0f), SkBits2Float(0x43251343));  // 382.95f, 165.075f, 372.594f, 165.075f
path.quadTo(SkBits2Float(0x43b51e93), SkBits2Float(0x43251343), SkBits2Float(0x43b17551), SkBits2Float(0x431dc0bf));  // 362.239f, 165.075f, 354.917f, 157.753f
path.quadTo(SkBits2Float(0x43adcc0f), SkBits2Float(0x43166e3a), SkBits2Float(0x43adcc0f), SkBits2Float(0x430c1343));  // 347.594f, 150.431f, 347.594f, 140.075f
path.quadTo(SkBits2Float(0x43adcc0f), SkBits2Float(0x4301b84c), SkBits2Float(0x43b17551), SkBits2Float(0x42f4cb8e));  // 347.594f, 129.72f, 354.917f, 122.398f
path.quadTo(SkBits2Float(0x43b51e93), SkBits2Float(0x42e62686), SkBits2Float(0x43ba4c0f), SkBits2Float(0x42e62686));  // 362.239f, 115.075f, 372.594f, 115.075f
path.quadTo(SkBits2Float(0x43bf798b), SkBits2Float(0x42e62686), SkBits2Float(0x43c322cd), SkBits2Float(0x42f4cb8e));  // 382.95f, 115.075f, 390.272f, 122.398f
path.quadTo(SkBits2Float(0x43c6cc0f), SkBits2Float(0x4301b84c), SkBits2Float(0x43c6cc0f), SkBits2Float(0x430c1343));  // 397.594f, 129.72f, 397.594f, 140.075f
path.close();
    SkPath path89(path);
    builder.add(path89, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43453a6b), SkBits2Float(0x438ed8e3));  // 197.228f, 285.694f
path.quadTo(SkBits2Float(0x43453a6b), SkBits2Float(0x4394065f), SkBits2Float(0x433de7e7), SkBits2Float(0x4397afa1));  // 197.228f, 296.05f, 189.906f, 303.372f
path.quadTo(SkBits2Float(0x43369562), SkBits2Float(0x439b58e3), SkBits2Float(0x432c3a6b), SkBits2Float(0x439b58e3));  // 182.584f, 310.694f, 172.228f, 310.694f
path.quadTo(SkBits2Float(0x4321df74), SkBits2Float(0x439b58e3), SkBits2Float(0x431a8cef), SkBits2Float(0x4397afa1));  // 161.873f, 310.694f, 154.551f, 303.372f
path.quadTo(SkBits2Float(0x43133a6b), SkBits2Float(0x4394065f), SkBits2Float(0x43133a6b), SkBits2Float(0x438ed8e3));  // 147.228f, 296.05f, 147.228f, 285.694f
path.quadTo(SkBits2Float(0x43133a6b), SkBits2Float(0x4389ab67), SkBits2Float(0x431a8cef), SkBits2Float(0x43860225));  // 147.228f, 275.339f, 154.551f, 268.017f
path.quadTo(SkBits2Float(0x4321df74), SkBits2Float(0x438258e3), SkBits2Float(0x432c3a6b), SkBits2Float(0x438258e3));  // 161.873f, 260.694f, 172.228f, 260.694f
path.quadTo(SkBits2Float(0x43369562), SkBits2Float(0x438258e3), SkBits2Float(0x433de7e7), SkBits2Float(0x43860225));  // 182.584f, 260.694f, 189.906f, 268.017f
path.quadTo(SkBits2Float(0x43453a6b), SkBits2Float(0x4389ab67), SkBits2Float(0x43453a6b), SkBits2Float(0x438ed8e3));  // 197.228f, 275.339f, 197.228f, 285.694f
path.close();
    SkPath path90(path);
    builder.add(path90, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a452ad), SkBits2Float(0x434c38a8));  // 328.646f, 204.221f
path.quadTo(SkBits2Float(0x43a452ad), SkBits2Float(0x435693a0), SkBits2Float(0x43a0a96b), SkBits2Float(0x435de624));  // 328.646f, 214.577f, 321.324f, 221.899f
path.quadTo(SkBits2Float(0x439d0029), SkBits2Float(0x436538a8), SkBits2Float(0x4397d2ad), SkBits2Float(0x436538a8));  // 314.001f, 229.221f, 303.646f, 229.221f
path.quadTo(SkBits2Float(0x4392a531), SkBits2Float(0x436538a8), SkBits2Float(0x438efbef), SkBits2Float(0x435de624));  // 293.291f, 229.221f, 285.968f, 221.899f
path.quadTo(SkBits2Float(0x438b52ad), SkBits2Float(0x435693a0), SkBits2Float(0x438b52ad), SkBits2Float(0x434c38a8));  // 278.646f, 214.577f, 278.646f, 204.221f
path.quadTo(SkBits2Float(0x438b52ad), SkBits2Float(0x4341ddb0), SkBits2Float(0x438efbef), SkBits2Float(0x433a8b2c));  // 278.646f, 193.866f, 285.968f, 186.544f
path.quadTo(SkBits2Float(0x4392a531), SkBits2Float(0x433338a8), SkBits2Float(0x4397d2ad), SkBits2Float(0x433338a8));  // 293.291f, 179.221f, 303.646f, 179.221f
path.quadTo(SkBits2Float(0x439d0029), SkBits2Float(0x433338a8), SkBits2Float(0x43a0a96b), SkBits2Float(0x433a8b2c));  // 314.001f, 179.221f, 321.324f, 186.544f
path.quadTo(SkBits2Float(0x43a452ad), SkBits2Float(0x4341ddb0), SkBits2Float(0x43a452ad), SkBits2Float(0x434c38a8));  // 328.646f, 193.866f, 328.646f, 204.221f
path.close();
    SkPath path91(path);
    builder.add(path91, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x44002d54), SkBits2Float(0x4339e9e8));  // 512.708f, 185.914f
path.quadTo(SkBits2Float(0x44002d54), SkBits2Float(0x434444e0), SkBits2Float(0x43fcb166), SkBits2Float(0x434b9764));  // 512.708f, 196.269f, 505.386f, 203.591f
path.quadTo(SkBits2Float(0x43f90824), SkBits2Float(0x4352e9e8), SkBits2Float(0x43f3daa8), SkBits2Float(0x4352e9e8));  // 498.064f, 210.914f, 487.708f, 210.914f
path.quadTo(SkBits2Float(0x43eead2c), SkBits2Float(0x4352e9e8), SkBits2Float(0x43eb03ea), SkBits2Float(0x434b9764));  // 477.353f, 210.914f, 470.031f, 203.591f
path.quadTo(SkBits2Float(0x43e75aa7), SkBits2Float(0x434444e0), SkBits2Float(0x43e75aa7), SkBits2Float(0x4339e9e8));  // 462.708f, 196.269f, 462.708f, 185.914f
path.quadTo(SkBits2Float(0x43e75aa7), SkBits2Float(0x432f8ef0), SkBits2Float(0x43eb03ea), SkBits2Float(0x43283c6c));  // 462.708f, 175.558f, 470.031f, 168.236f
path.quadTo(SkBits2Float(0x43eead2c), SkBits2Float(0x4320e9e8), SkBits2Float(0x43f3daa8), SkBits2Float(0x4320e9e8));  // 477.353f, 160.914f, 487.708f, 160.914f
path.quadTo(SkBits2Float(0x43f90824), SkBits2Float(0x4320e9e8), SkBits2Float(0x43fcb166), SkBits2Float(0x43283c6c));  // 498.064f, 160.914f, 505.386f, 168.236f
path.quadTo(SkBits2Float(0x44002d54), SkBits2Float(0x432f8ef0), SkBits2Float(0x44002d54), SkBits2Float(0x4339e9e8));  // 512.708f, 175.558f, 512.708f, 185.914f
path.close();
    SkPath path92(path);
    builder.add(path92, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4385d99d), SkBits2Float(0x4302a8cd));  // 267.7f, 130.659f
path.quadTo(SkBits2Float(0x4385d99d), SkBits2Float(0x430d03c4), SkBits2Float(0x4382305b), SkBits2Float(0x43145649));  // 267.7f, 141.015f, 260.378f, 148.337f
path.quadTo(SkBits2Float(0x437d0e32), SkBits2Float(0x431ba8cd), SkBits2Float(0x4372b33a), SkBits2Float(0x431ba8cd));  // 253.055f, 155.659f, 242.7f, 155.659f
path.quadTo(SkBits2Float(0x43685842), SkBits2Float(0x431ba8cd), SkBits2Float(0x436105be), SkBits2Float(0x43145649));  // 232.345f, 155.659f, 225.022f, 148.337f
path.quadTo(SkBits2Float(0x4359b33a), SkBits2Float(0x430d03c4), SkBits2Float(0x4359b33a), SkBits2Float(0x4302a8cd));  // 217.7f, 141.015f, 217.7f, 130.659f
path.quadTo(SkBits2Float(0x4359b33a), SkBits2Float(0x42f09bab), SkBits2Float(0x436105be), SkBits2Float(0x42e1f6a2));  // 217.7f, 120.304f, 225.022f, 112.982f
path.quadTo(SkBits2Float(0x43685842), SkBits2Float(0x42d3519a), SkBits2Float(0x4372b33a), SkBits2Float(0x42d3519a));  // 232.345f, 105.659f, 242.7f, 105.659f
path.quadTo(SkBits2Float(0x437d0e32), SkBits2Float(0x42d3519a), SkBits2Float(0x4382305b), SkBits2Float(0x42e1f6a2));  // 253.055f, 105.659f, 260.378f, 112.982f
path.quadTo(SkBits2Float(0x4385d99d), SkBits2Float(0x42f09bab), SkBits2Float(0x4385d99d), SkBits2Float(0x4302a8cd));  // 267.7f, 120.304f, 267.7f, 130.659f
path.close();
    SkPath path93(path);
    builder.add(path93, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42f92820), SkBits2Float(0x435b1722));  // 124.578f, 219.09f
path.quadTo(SkBits2Float(0x42f92820), SkBits2Float(0x4365721a), SkBits2Float(0x42ea8318), SkBits2Float(0x436cc49e));  // 124.578f, 229.446f, 117.256f, 236.768f
path.quadTo(SkBits2Float(0x42dbde0f), SkBits2Float(0x43741722), SkBits2Float(0x42c72820), SkBits2Float(0x43741722));  // 109.934f, 244.09f, 99.5784f, 244.09f
path.quadTo(SkBits2Float(0x42b27231), SkBits2Float(0x43741722), SkBits2Float(0x42a3cd28), SkBits2Float(0x436cc49e));  // 89.223f, 244.09f, 81.9007f, 236.768f
path.quadTo(SkBits2Float(0x42952820), SkBits2Float(0x4365721a), SkBits2Float(0x42952820), SkBits2Float(0x435b1722));  // 74.5784f, 229.446f, 74.5784f, 219.09f
path.quadTo(SkBits2Float(0x42952820), SkBits2Float(0x4350bc2a), SkBits2Float(0x42a3cd28), SkBits2Float(0x434969a6));  // 74.5784f, 208.735f, 81.9007f, 201.413f
path.quadTo(SkBits2Float(0x42b27231), SkBits2Float(0x43421722), SkBits2Float(0x42c72820), SkBits2Float(0x43421722));  // 89.223f, 194.09f, 99.5784f, 194.09f
path.quadTo(SkBits2Float(0x42dbde0f), SkBits2Float(0x43421722), SkBits2Float(0x42ea8318), SkBits2Float(0x434969a6));  // 109.934f, 194.09f, 117.256f, 201.413f
path.quadTo(SkBits2Float(0x42f92820), SkBits2Float(0x4350bc2a), SkBits2Float(0x42f92820), SkBits2Float(0x435b1722));  // 124.578f, 208.735f, 124.578f, 219.09f
path.close();
    SkPath path94(path);
    builder.add(path94, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43b70d86), SkBits2Float(0x435cf621));  // 366.106f, 220.961f
path.quadTo(SkBits2Float(0x43b70d86), SkBits2Float(0x43675118), SkBits2Float(0x43b36444), SkBits2Float(0x436ea39d));  // 366.106f, 231.317f, 358.783f, 238.639f
path.quadTo(SkBits2Float(0x43afbb02), SkBits2Float(0x4375f621), SkBits2Float(0x43aa8d86), SkBits2Float(0x4375f621));  // 351.461f, 245.961f, 341.106f, 245.961f
path.quadTo(SkBits2Float(0x43a5600a), SkBits2Float(0x4375f621), SkBits2Float(0x43a1b6c8), SkBits2Float(0x436ea39d));  // 330.75f, 245.961f, 323.428f, 238.639f
path.quadTo(SkBits2Float(0x439e0d86), SkBits2Float(0x43675118), SkBits2Float(0x439e0d86), SkBits2Float(0x435cf621));  // 316.106f, 231.317f, 316.106f, 220.961f
path.quadTo(SkBits2Float(0x439e0d86), SkBits2Float(0x43529b2a), SkBits2Float(0x43a1b6c8), SkBits2Float(0x434b48a5));  // 316.106f, 210.606f, 323.428f, 203.284f
path.quadTo(SkBits2Float(0x43a5600a), SkBits2Float(0x4343f621), SkBits2Float(0x43aa8d86), SkBits2Float(0x4343f621));  // 330.75f, 195.961f, 341.106f, 195.961f
path.quadTo(SkBits2Float(0x43afbb02), SkBits2Float(0x4343f621), SkBits2Float(0x43b36444), SkBits2Float(0x434b48a5));  // 351.461f, 195.961f, 358.783f, 203.284f
path.quadTo(SkBits2Float(0x43b70d86), SkBits2Float(0x43529b2a), SkBits2Float(0x43b70d86), SkBits2Float(0x435cf621));  // 366.106f, 210.606f, 366.106f, 220.961f
path.close();
    SkPath path95(path);
    builder.add(path95, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x436d80e1), SkBits2Float(0x43885efa));  // 237.503f, 272.742f
path.quadTo(SkBits2Float(0x436d80e1), SkBits2Float(0x438d8c76), SkBits2Float(0x43662e5d), SkBits2Float(0x439135b8));  // 237.503f, 283.097f, 230.181f, 290.42f
path.quadTo(SkBits2Float(0x435edbd8), SkBits2Float(0x4394defa), SkBits2Float(0x435480e1), SkBits2Float(0x4394defa));  // 222.859f, 297.742f, 212.503f, 297.742f
path.quadTo(SkBits2Float(0x434a25ea), SkBits2Float(0x4394defa), SkBits2Float(0x4342d365), SkBits2Float(0x439135b8));  // 202.148f, 297.742f, 194.826f, 290.42f
path.quadTo(SkBits2Float(0x433b80e1), SkBits2Float(0x438d8c76), SkBits2Float(0x433b80e1), SkBits2Float(0x43885efa));  // 187.503f, 283.097f, 187.503f, 272.742f
path.quadTo(SkBits2Float(0x433b80e1), SkBits2Float(0x4383317e), SkBits2Float(0x4342d365), SkBits2Float(0x437f1078));  // 187.503f, 262.387f, 194.826f, 255.064f
path.quadTo(SkBits2Float(0x434a25ea), SkBits2Float(0x4377bdf4), SkBits2Float(0x435480e1), SkBits2Float(0x4377bdf4));  // 202.148f, 247.742f, 212.503f, 247.742f
path.quadTo(SkBits2Float(0x435edbd8), SkBits2Float(0x4377bdf4), SkBits2Float(0x43662e5d), SkBits2Float(0x437f1078));  // 222.859f, 247.742f, 230.181f, 255.064f
path.quadTo(SkBits2Float(0x436d80e1), SkBits2Float(0x4383317e), SkBits2Float(0x436d80e1), SkBits2Float(0x43885efa));  // 237.503f, 262.387f, 237.503f, 272.742f
path.close();
    SkPath path96(path);
    builder.add(path96, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43e54635), SkBits2Float(0x43794ba0));  // 458.548f, 249.295f
path.quadTo(SkBits2Float(0x43e54635), SkBits2Float(0x4381d34c), SkBits2Float(0x43e19cf3), SkBits2Float(0x43857c8e));  // 458.548f, 259.651f, 451.226f, 266.973f
path.quadTo(SkBits2Float(0x43ddf3b1), SkBits2Float(0x438925d0), SkBits2Float(0x43d8c635), SkBits2Float(0x438925d0));  // 443.904f, 274.295f, 433.548f, 274.295f
path.quadTo(SkBits2Float(0x43d398b9), SkBits2Float(0x438925d0), SkBits2Float(0x43cfef77), SkBits2Float(0x43857c8e));  // 423.193f, 274.295f, 415.871f, 266.973f
path.quadTo(SkBits2Float(0x43cc4635), SkBits2Float(0x4381d34c), SkBits2Float(0x43cc4635), SkBits2Float(0x43794ba0));  // 408.548f, 259.651f, 408.548f, 249.295f
path.quadTo(SkBits2Float(0x43cc4635), SkBits2Float(0x436ef0a8), SkBits2Float(0x43cfef77), SkBits2Float(0x43679e24));  // 408.548f, 238.94f, 415.871f, 231.618f
path.quadTo(SkBits2Float(0x43d398b9), SkBits2Float(0x43604ba0), SkBits2Float(0x43d8c635), SkBits2Float(0x43604ba0));  // 423.193f, 224.295f, 433.548f, 224.295f
path.quadTo(SkBits2Float(0x43ddf3b1), SkBits2Float(0x43604ba0), SkBits2Float(0x43e19cf3), SkBits2Float(0x43679e24));  // 443.904f, 224.295f, 451.226f, 231.618f
path.quadTo(SkBits2Float(0x43e54635), SkBits2Float(0x436ef0a8), SkBits2Float(0x43e54635), SkBits2Float(0x43794ba0));  // 458.548f, 238.94f, 458.548f, 249.295f
path.close();
    SkPath path97(path);
    builder.add(path97, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42596ac0), SkBits2Float(0x42f09a24));  // 54.3542f, 120.301f
path.quadTo(SkBits2Float(0x42596ac0), SkBits2Float(0x4302a809), SkBits2Float(0x423c20af), SkBits2Float(0x4309fa8e));  // 54.3542f, 130.656f, 47.0319f, 137.979f
path.quadTo(SkBits2Float(0x421ed69e), SkBits2Float(0x43114d12), SkBits2Float(0x41ead580), SkBits2Float(0x43114d12));  // 39.7096f, 145.301f, 29.3542f, 145.301f
path.quadTo(SkBits2Float(0x4197fdc4), SkBits2Float(0x43114d12), SkBits2Float(0x413ad344), SkBits2Float(0x4309fa8e));  // 18.9989f, 145.301f, 11.6766f, 137.979f
path.quadTo(SkBits2Float(0x408b5604), SkBits2Float(0x4302a809), SkBits2Float(0x408b5604), SkBits2Float(0x42f09a24));  // 4.35425f, 130.656f, 4.35425f, 120.301f
path.quadTo(SkBits2Float(0x408b5604), SkBits2Float(0x42dbe435), SkBits2Float(0x413ad344), SkBits2Float(0x42cd3f2d));  // 4.35425f, 109.946f, 11.6766f, 102.623f
path.quadTo(SkBits2Float(0x4197fdc4), SkBits2Float(0x42be9a25), SkBits2Float(0x41ead580), SkBits2Float(0x42be9a25));  // 18.9989f, 95.3011f, 29.3542f, 95.3011f
path.quadTo(SkBits2Float(0x421ed69e), SkBits2Float(0x42be9a25), SkBits2Float(0x423c20af), SkBits2Float(0x42cd3f2d));  // 39.7096f, 95.3011f, 47.0319f, 102.623f
path.quadTo(SkBits2Float(0x42596ac0), SkBits2Float(0x42dbe435), SkBits2Float(0x42596ac0), SkBits2Float(0x42f09a24));  // 54.3542f, 109.946f, 54.3542f, 120.301f
path.close();
    SkPath path98(path);
    builder.add(path98, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ab48a8), SkBits2Float(0x438703b7));  // 342.568f, 270.029f
path.quadTo(SkBits2Float(0x43ab48a8), SkBits2Float(0x438c3133), SkBits2Float(0x43a79f66), SkBits2Float(0x438fda75));  // 342.568f, 280.384f, 335.245f, 287.707f
path.quadTo(SkBits2Float(0x43a3f624), SkBits2Float(0x439383b7), SkBits2Float(0x439ec8a8), SkBits2Float(0x439383b7));  // 327.923f, 295.029f, 317.568f, 295.029f
path.quadTo(SkBits2Float(0x43999b2c), SkBits2Float(0x439383b7), SkBits2Float(0x4395f1ea), SkBits2Float(0x438fda75));  // 307.212f, 295.029f, 299.89f, 287.707f
path.quadTo(SkBits2Float(0x439248a8), SkBits2Float(0x438c3133), SkBits2Float(0x439248a8), SkBits2Float(0x438703b7));  // 292.568f, 280.384f, 292.568f, 270.029f
path.quadTo(SkBits2Float(0x439248a8), SkBits2Float(0x4381d63b), SkBits2Float(0x4395f1ea), SkBits2Float(0x437c59f2));  // 292.568f, 259.674f, 299.89f, 252.351f
path.quadTo(SkBits2Float(0x43999b2c), SkBits2Float(0x4375076e), SkBits2Float(0x439ec8a8), SkBits2Float(0x4375076e));  // 307.212f, 245.029f, 317.568f, 245.029f
path.quadTo(SkBits2Float(0x43a3f624), SkBits2Float(0x4375076e), SkBits2Float(0x43a79f66), SkBits2Float(0x437c59f2));  // 327.923f, 245.029f, 335.245f, 252.351f
path.quadTo(SkBits2Float(0x43ab48a8), SkBits2Float(0x4381d63b), SkBits2Float(0x43ab48a8), SkBits2Float(0x438703b7));  // 342.568f, 259.674f, 342.568f, 270.029f
path.close();
    SkPath path99(path);
    builder.add(path99, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43b3d091), SkBits2Float(0x4394a1fa));  // 359.629f, 297.265f
path.quadTo(SkBits2Float(0x43b3d091), SkBits2Float(0x4399cf76), SkBits2Float(0x43b0274f), SkBits2Float(0x439d78b8));  // 359.629f, 307.621f, 352.307f, 314.943f
path.quadTo(SkBits2Float(0x43ac7e0d), SkBits2Float(0x43a121fa), SkBits2Float(0x43a75091), SkBits2Float(0x43a121fa));  // 344.985f, 322.265f, 334.629f, 322.265f
path.quadTo(SkBits2Float(0x43a22315), SkBits2Float(0x43a121fa), SkBits2Float(0x439e79d3), SkBits2Float(0x439d78b8));  // 324.274f, 322.265f, 316.952f, 314.943f
path.quadTo(SkBits2Float(0x439ad091), SkBits2Float(0x4399cf76), SkBits2Float(0x439ad091), SkBits2Float(0x4394a1fa));  // 309.629f, 307.621f, 309.629f, 297.265f
path.quadTo(SkBits2Float(0x439ad091), SkBits2Float(0x438f747e), SkBits2Float(0x439e79d3), SkBits2Float(0x438bcb3c));  // 309.629f, 286.91f, 316.952f, 279.588f
path.quadTo(SkBits2Float(0x43a22315), SkBits2Float(0x438821fa), SkBits2Float(0x43a75091), SkBits2Float(0x438821fa));  // 324.274f, 272.265f, 334.629f, 272.265f
path.quadTo(SkBits2Float(0x43ac7e0d), SkBits2Float(0x438821fa), SkBits2Float(0x43b0274f), SkBits2Float(0x438bcb3c));  // 344.985f, 272.265f, 352.307f, 279.588f
path.quadTo(SkBits2Float(0x43b3d091), SkBits2Float(0x438f747e), SkBits2Float(0x43b3d091), SkBits2Float(0x4394a1fa));  // 359.629f, 286.91f, 359.629f, 297.265f
path.close();
    SkPath path100(path);
    builder.add(path100, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a153aa), SkBits2Float(0x4387c2b8));  // 80.6634f, 271.521f
path.quadTo(SkBits2Float(0x42a153aa), SkBits2Float(0x438cf034), SkBits2Float(0x4292aea2), SkBits2Float(0x43909976));  // 80.6634f, 281.877f, 73.3411f, 289.199f
path.quadTo(SkBits2Float(0x42840999), SkBits2Float(0x439442b8), SkBits2Float(0x425ea754), SkBits2Float(0x439442b8));  // 66.0187f, 296.521f, 55.6634f, 296.521f
path.quadTo(SkBits2Float(0x42353b76), SkBits2Float(0x439442b8), SkBits2Float(0x4217f165), SkBits2Float(0x43909976));  // 45.3081f, 296.521f, 37.9857f, 289.199f
path.quadTo(SkBits2Float(0x41f54ea8), SkBits2Float(0x438cf034), SkBits2Float(0x41f54ea8), SkBits2Float(0x4387c2b8));  // 30.6634f, 281.877f, 30.6634f, 271.521f
path.quadTo(SkBits2Float(0x41f54ea8), SkBits2Float(0x4382953c), SkBits2Float(0x4217f165), SkBits2Float(0x437dd7f4));  // 30.6634f, 261.166f, 37.9857f, 253.844f
path.quadTo(SkBits2Float(0x42353b76), SkBits2Float(0x43768570), SkBits2Float(0x425ea754), SkBits2Float(0x43768570));  // 45.3081f, 246.521f, 55.6634f, 246.521f
path.quadTo(SkBits2Float(0x42840999), SkBits2Float(0x43768570), SkBits2Float(0x4292aea2), SkBits2Float(0x437dd7f4));  // 66.0187f, 246.521f, 73.3411f, 253.844f
path.quadTo(SkBits2Float(0x42a153aa), SkBits2Float(0x4382953c), SkBits2Float(0x42a153aa), SkBits2Float(0x4387c2b8));  // 80.6634f, 261.166f, 80.6634f, 271.521f
path.close();
    SkPath path101(path);
    builder.add(path101, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x436da8c1), SkBits2Float(0x434950e1));  // 237.659f, 201.316f
path.quadTo(SkBits2Float(0x436da8c1), SkBits2Float(0x4353abd8), SkBits2Float(0x4366563d), SkBits2Float(0x435afe5d));  // 237.659f, 211.671f, 230.337f, 218.994f
path.quadTo(SkBits2Float(0x435f03b8), SkBits2Float(0x436250e1), SkBits2Float(0x4354a8c1), SkBits2Float(0x436250e1));  // 223.015f, 226.316f, 212.659f, 226.316f
path.quadTo(SkBits2Float(0x434a4dca), SkBits2Float(0x436250e1), SkBits2Float(0x4342fb45), SkBits2Float(0x435afe5d));  // 202.304f, 226.316f, 194.982f, 218.994f
path.quadTo(SkBits2Float(0x433ba8c1), SkBits2Float(0x4353abd8), SkBits2Float(0x433ba8c1), SkBits2Float(0x434950e1));  // 187.659f, 211.671f, 187.659f, 201.316f
path.quadTo(SkBits2Float(0x433ba8c1), SkBits2Float(0x433ef5ea), SkBits2Float(0x4342fb45), SkBits2Float(0x4337a365));  // 187.659f, 190.961f, 194.982f, 183.638f
path.quadTo(SkBits2Float(0x434a4dca), SkBits2Float(0x433050e1), SkBits2Float(0x4354a8c1), SkBits2Float(0x433050e1));  // 202.304f, 176.316f, 212.659f, 176.316f
path.quadTo(SkBits2Float(0x435f03b8), SkBits2Float(0x433050e1), SkBits2Float(0x4366563d), SkBits2Float(0x4337a365));  // 223.015f, 176.316f, 230.337f, 183.638f
path.quadTo(SkBits2Float(0x436da8c1), SkBits2Float(0x433ef5ea), SkBits2Float(0x436da8c1), SkBits2Float(0x434950e1));  // 237.659f, 190.961f, 237.659f, 201.316f
path.close();
    SkPath path102(path);
    builder.add(path102, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ba530a), SkBits2Float(0x4376baae));  // 372.649f, 246.729f
path.quadTo(SkBits2Float(0x43ba530a), SkBits2Float(0x43808ad3), SkBits2Float(0x43b6a9c8), SkBits2Float(0x43843415));  // 372.649f, 257.085f, 365.326f, 264.407f
path.quadTo(SkBits2Float(0x43b30086), SkBits2Float(0x4387dd57), SkBits2Float(0x43add30a), SkBits2Float(0x4387dd57));  // 358.004f, 271.729f, 347.649f, 271.729f
path.quadTo(SkBits2Float(0x43a8a58e), SkBits2Float(0x4387dd57), SkBits2Float(0x43a4fc4c), SkBits2Float(0x43843415));  // 337.293f, 271.729f, 329.971f, 264.407f
path.quadTo(SkBits2Float(0x43a1530a), SkBits2Float(0x43808ad3), SkBits2Float(0x43a1530a), SkBits2Float(0x4376baae));  // 322.649f, 257.085f, 322.649f, 246.729f
path.quadTo(SkBits2Float(0x43a1530a), SkBits2Float(0x436c5fb6), SkBits2Float(0x43a4fc4c), SkBits2Float(0x43650d32));  // 322.649f, 236.374f, 329.971f, 229.052f
path.quadTo(SkBits2Float(0x43a8a58e), SkBits2Float(0x435dbaae), SkBits2Float(0x43add30a), SkBits2Float(0x435dbaae));  // 337.293f, 221.729f, 347.649f, 221.729f
path.quadTo(SkBits2Float(0x43b30086), SkBits2Float(0x435dbaae), SkBits2Float(0x43b6a9c8), SkBits2Float(0x43650d32));  // 358.004f, 221.729f, 365.326f, 229.052f
path.quadTo(SkBits2Float(0x43ba530a), SkBits2Float(0x436c5fb6), SkBits2Float(0x43ba530a), SkBits2Float(0x4376baae));  // 372.649f, 236.374f, 372.649f, 246.729f
path.close();
    SkPath path103(path);
    builder.add(path103, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4399286a), SkBits2Float(0x435887a5));  // 306.316f, 216.53f
path.quadTo(SkBits2Float(0x4399286a), SkBits2Float(0x4362e29c), SkBits2Float(0x43957f28), SkBits2Float(0x436a3521));  // 306.316f, 226.885f, 298.993f, 234.208f
path.quadTo(SkBits2Float(0x4391d5e6), SkBits2Float(0x437187a5), SkBits2Float(0x438ca86a), SkBits2Float(0x437187a5));  // 291.671f, 241.53f, 281.316f, 241.53f
path.quadTo(SkBits2Float(0x43877aee), SkBits2Float(0x437187a5), SkBits2Float(0x4383d1ac), SkBits2Float(0x436a3521));  // 270.96f, 241.53f, 263.638f, 234.208f
path.quadTo(SkBits2Float(0x4380286a), SkBits2Float(0x4362e29c), SkBits2Float(0x4380286a), SkBits2Float(0x435887a5));  // 256.316f, 226.885f, 256.316f, 216.53f
path.quadTo(SkBits2Float(0x4380286a), SkBits2Float(0x434e2cae), SkBits2Float(0x4383d1ac), SkBits2Float(0x4346da29));  // 256.316f, 206.175f, 263.638f, 198.852f
path.quadTo(SkBits2Float(0x43877aee), SkBits2Float(0x433f87a5), SkBits2Float(0x438ca86a), SkBits2Float(0x433f87a5));  // 270.96f, 191.53f, 281.316f, 191.53f
path.quadTo(SkBits2Float(0x4391d5e6), SkBits2Float(0x433f87a5), SkBits2Float(0x43957f28), SkBits2Float(0x4346da29));  // 291.671f, 191.53f, 298.993f, 198.852f
path.quadTo(SkBits2Float(0x4399286a), SkBits2Float(0x434e2cae), SkBits2Float(0x4399286a), SkBits2Float(0x435887a5));  // 306.316f, 206.175f, 306.316f, 216.53f
path.close();
    SkPath path104(path);
    builder.add(path104, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a08755), SkBits2Float(0x43120238));  // 321.057f, 146.009f
path.quadTo(SkBits2Float(0x43a08755), SkBits2Float(0x431c5d30), SkBits2Float(0x439cde13), SkBits2Float(0x4323afb4));  // 321.057f, 156.364f, 313.735f, 163.686f
path.quadTo(SkBits2Float(0x439934d1), SkBits2Float(0x432b0238), SkBits2Float(0x43940755), SkBits2Float(0x432b0238));  // 306.413f, 171.009f, 296.057f, 171.009f
path.quadTo(SkBits2Float(0x438ed9d9), SkBits2Float(0x432b0238), SkBits2Float(0x438b3097), SkBits2Float(0x4323afb4));  // 285.702f, 171.009f, 278.38f, 163.686f
path.quadTo(SkBits2Float(0x43878755), SkBits2Float(0x431c5d30), SkBits2Float(0x43878755), SkBits2Float(0x43120238));  // 271.057f, 156.364f, 271.057f, 146.009f
path.quadTo(SkBits2Float(0x43878755), SkBits2Float(0x4307a740), SkBits2Float(0x438b3097), SkBits2Float(0x430054bc));  // 271.057f, 135.653f, 278.38f, 128.331f
path.quadTo(SkBits2Float(0x438ed9d9), SkBits2Float(0x42f20470), SkBits2Float(0x43940755), SkBits2Float(0x42f20470));  // 285.702f, 121.009f, 296.057f, 121.009f
path.quadTo(SkBits2Float(0x439934d1), SkBits2Float(0x42f20470), SkBits2Float(0x439cde13), SkBits2Float(0x430054bc));  // 306.413f, 121.009f, 313.735f, 128.331f
path.quadTo(SkBits2Float(0x43a08755), SkBits2Float(0x4307a740), SkBits2Float(0x43a08755), SkBits2Float(0x43120238));  // 321.057f, 135.653f, 321.057f, 146.009f
path.close();
    SkPath path105(path);
    builder.add(path105, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43e31764), SkBits2Float(0x438e240b));  // 454.183f, 284.282f
path.quadTo(SkBits2Float(0x43e31764), SkBits2Float(0x43935187), SkBits2Float(0x43df6e22), SkBits2Float(0x4396fac9));  // 454.183f, 294.637f, 446.86f, 301.959f
path.quadTo(SkBits2Float(0x43dbc4e0), SkBits2Float(0x439aa40b), SkBits2Float(0x43d69764), SkBits2Float(0x439aa40b));  // 439.538f, 309.282f, 429.183f, 309.282f
path.quadTo(SkBits2Float(0x43d169e8), SkBits2Float(0x439aa40b), SkBits2Float(0x43cdc0a6), SkBits2Float(0x4396fac9));  // 418.827f, 309.282f, 411.505f, 301.959f
path.quadTo(SkBits2Float(0x43ca1764), SkBits2Float(0x43935187), SkBits2Float(0x43ca1764), SkBits2Float(0x438e240b));  // 404.183f, 294.637f, 404.183f, 284.282f
path.quadTo(SkBits2Float(0x43ca1764), SkBits2Float(0x4388f68f), SkBits2Float(0x43cdc0a6), SkBits2Float(0x43854d4d));  // 404.183f, 273.926f, 411.505f, 266.604f
path.quadTo(SkBits2Float(0x43d169e8), SkBits2Float(0x4381a40b), SkBits2Float(0x43d69764), SkBits2Float(0x4381a40b));  // 418.827f, 259.282f, 429.183f, 259.282f
path.quadTo(SkBits2Float(0x43dbc4e0), SkBits2Float(0x4381a40b), SkBits2Float(0x43df6e22), SkBits2Float(0x43854d4d));  // 439.538f, 259.282f, 446.86f, 266.604f
path.quadTo(SkBits2Float(0x43e31764), SkBits2Float(0x4388f68f), SkBits2Float(0x43e31764), SkBits2Float(0x438e240b));  // 454.183f, 273.926f, 454.183f, 284.282f
path.close();
    SkPath path106(path);
    builder.add(path106, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43430483), SkBits2Float(0x4345764d));  // 195.018f, 197.462f
path.quadTo(SkBits2Float(0x43430483), SkBits2Float(0x434fd144), SkBits2Float(0x433bb1ff), SkBits2Float(0x435723c9));  // 195.018f, 207.817f, 187.695f, 215.14f
path.quadTo(SkBits2Float(0x43345f7a), SkBits2Float(0x435e764d), SkBits2Float(0x432a0483), SkBits2Float(0x435e764d));  // 180.373f, 222.462f, 170.018f, 222.462f
path.quadTo(SkBits2Float(0x431fa98c), SkBits2Float(0x435e764d), SkBits2Float(0x43185707), SkBits2Float(0x435723c9));  // 159.662f, 222.462f, 152.34f, 215.14f
path.quadTo(SkBits2Float(0x43110483), SkBits2Float(0x434fd144), SkBits2Float(0x43110483), SkBits2Float(0x4345764d));  // 145.018f, 207.817f, 145.018f, 197.462f
path.quadTo(SkBits2Float(0x43110483), SkBits2Float(0x433b1b56), SkBits2Float(0x43185707), SkBits2Float(0x4333c8d1));  // 145.018f, 187.107f, 152.34f, 179.784f
path.quadTo(SkBits2Float(0x431fa98c), SkBits2Float(0x432c764d), SkBits2Float(0x432a0483), SkBits2Float(0x432c764d));  // 159.662f, 172.462f, 170.018f, 172.462f
path.quadTo(SkBits2Float(0x43345f7a), SkBits2Float(0x432c764d), SkBits2Float(0x433bb1ff), SkBits2Float(0x4333c8d1));  // 180.373f, 172.462f, 187.695f, 179.784f
path.quadTo(SkBits2Float(0x43430483), SkBits2Float(0x433b1b56), SkBits2Float(0x43430483), SkBits2Float(0x4345764d));  // 195.018f, 187.107f, 195.018f, 197.462f
path.close();
    SkPath path107(path);
    builder.add(path107, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x439a55b2), SkBits2Float(0x4370a1cc));  // 308.669f, 240.632f
path.quadTo(SkBits2Float(0x439a55b2), SkBits2Float(0x437afcc3), SkBits2Float(0x4396ac70), SkBits2Float(0x438127a4));  // 308.669f, 250.987f, 301.347f, 258.31f
path.quadTo(SkBits2Float(0x4393032e), SkBits2Float(0x4384d0e6), SkBits2Float(0x438dd5b2), SkBits2Float(0x4384d0e6));  // 294.025f, 265.632f, 283.669f, 265.632f
path.quadTo(SkBits2Float(0x4388a836), SkBits2Float(0x4384d0e6), SkBits2Float(0x4384fef4), SkBits2Float(0x438127a4));  // 273.314f, 265.632f, 265.992f, 258.31f
path.quadTo(SkBits2Float(0x438155b2), SkBits2Float(0x437afcc3), SkBits2Float(0x438155b2), SkBits2Float(0x4370a1cc));  // 258.669f, 250.987f, 258.669f, 240.632f
path.quadTo(SkBits2Float(0x438155b2), SkBits2Float(0x436646d5), SkBits2Float(0x4384fef4), SkBits2Float(0x435ef451));  // 258.669f, 230.277f, 265.992f, 222.954f
path.quadTo(SkBits2Float(0x4388a836), SkBits2Float(0x4357a1cd), SkBits2Float(0x438dd5b2), SkBits2Float(0x4357a1cd));  // 273.314f, 215.632f, 283.669f, 215.632f
path.quadTo(SkBits2Float(0x4393032e), SkBits2Float(0x4357a1cd), SkBits2Float(0x4396ac70), SkBits2Float(0x435ef451));  // 294.025f, 215.632f, 301.347f, 222.954f
path.quadTo(SkBits2Float(0x439a55b2), SkBits2Float(0x436646d5), SkBits2Float(0x439a55b2), SkBits2Float(0x4370a1cc));  // 308.669f, 230.277f, 308.669f, 240.632f
path.close();
    SkPath path108(path);
    builder.add(path108, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42fcf02d), SkBits2Float(0x4327ee31));  // 126.469f, 167.93f
path.quadTo(SkBits2Float(0x42fcf02d), SkBits2Float(0x43324928), SkBits2Float(0x42ee4b24), SkBits2Float(0x43399bad));  // 126.469f, 178.286f, 119.147f, 185.608f
path.quadTo(SkBits2Float(0x42dfa61c), SkBits2Float(0x4340ee31), SkBits2Float(0x42caf02d), SkBits2Float(0x4340ee31));  // 111.824f, 192.93f, 101.469f, 192.93f
path.quadTo(SkBits2Float(0x42b63a3e), SkBits2Float(0x4340ee31), SkBits2Float(0x42a79536), SkBits2Float(0x43399bad));  // 91.1138f, 192.93f, 83.7914f, 185.608f
path.quadTo(SkBits2Float(0x4298f02d), SkBits2Float(0x43324928), SkBits2Float(0x4298f02d), SkBits2Float(0x4327ee31));  // 76.4691f, 178.286f, 76.4691f, 167.93f
path.quadTo(SkBits2Float(0x4298f02d), SkBits2Float(0x431d933a), SkBits2Float(0x42a79536), SkBits2Float(0x431640b5));  // 76.4691f, 157.575f, 83.7914f, 150.253f
path.quadTo(SkBits2Float(0x42b63a3e), SkBits2Float(0x430eee31), SkBits2Float(0x42caf02d), SkBits2Float(0x430eee31));  // 91.1138f, 142.93f, 101.469f, 142.93f
path.quadTo(SkBits2Float(0x42dfa61c), SkBits2Float(0x430eee31), SkBits2Float(0x42ee4b24), SkBits2Float(0x431640b5));  // 111.824f, 142.93f, 119.147f, 150.253f
path.quadTo(SkBits2Float(0x42fcf02d), SkBits2Float(0x431d933a), SkBits2Float(0x42fcf02d), SkBits2Float(0x4327ee31));  // 126.469f, 157.575f, 126.469f, 167.93f
path.close();
    SkPath path109(path);
    builder.add(path109, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4408a32e), SkBits2Float(0x438aadd7));  // 546.55f, 277.358f
path.quadTo(SkBits2Float(0x4408a32e), SkBits2Float(0x438fdb53), SkBits2Float(0x4406ce8d), SkBits2Float(0x43938495));  // 546.55f, 287.713f, 539.227f, 295.036f
path.quadTo(SkBits2Float(0x4404f9ec), SkBits2Float(0x43972dd7), SkBits2Float(0x4402632e), SkBits2Float(0x43972dd7));  // 531.905f, 302.358f, 521.55f, 302.358f
path.quadTo(SkBits2Float(0x43ff98e0), SkBits2Float(0x43972dd7), SkBits2Float(0x43fbef9e), SkBits2Float(0x43938495));  // 511.194f, 302.358f, 503.872f, 295.036f
path.quadTo(SkBits2Float(0x43f8465c), SkBits2Float(0x438fdb53), SkBits2Float(0x43f8465c), SkBits2Float(0x438aadd7));  // 496.55f, 287.713f, 496.55f, 277.358f
path.quadTo(SkBits2Float(0x43f8465c), SkBits2Float(0x4385805b), SkBits2Float(0x43fbef9e), SkBits2Float(0x4381d719));  // 496.55f, 267.003f, 503.872f, 259.68f
path.quadTo(SkBits2Float(0x43ff98e0), SkBits2Float(0x437c5bae), SkBits2Float(0x4402632e), SkBits2Float(0x437c5bae));  // 511.194f, 252.358f, 521.55f, 252.358f
path.quadTo(SkBits2Float(0x4404f9ec), SkBits2Float(0x437c5bae), SkBits2Float(0x4406ce8d), SkBits2Float(0x4381d719));  // 531.905f, 252.358f, 539.227f, 259.68f
path.quadTo(SkBits2Float(0x4408a32e), SkBits2Float(0x4385805b), SkBits2Float(0x4408a32e), SkBits2Float(0x438aadd7));  // 546.55f, 267.003f, 546.55f, 277.358f
path.close();
    SkPath path110(path);
    builder.add(path110, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ae6436), SkBits2Float(0x4399c25d));  // 348.783f, 307.518f
path.quadTo(SkBits2Float(0x43ae6436), SkBits2Float(0x439eefd9), SkBits2Float(0x43aabaf4), SkBits2Float(0x43a2991b));  // 348.783f, 317.874f, 341.461f, 325.196f
path.quadTo(SkBits2Float(0x43a711b2), SkBits2Float(0x43a6425d), SkBits2Float(0x43a1e436), SkBits2Float(0x43a6425d));  // 334.138f, 332.518f, 323.783f, 332.518f
path.quadTo(SkBits2Float(0x439cb6ba), SkBits2Float(0x43a6425d), SkBits2Float(0x43990d78), SkBits2Float(0x43a2991b));  // 313.428f, 332.518f, 306.105f, 325.196f
path.quadTo(SkBits2Float(0x43956436), SkBits2Float(0x439eefd9), SkBits2Float(0x43956436), SkBits2Float(0x4399c25d));  // 298.783f, 317.874f, 298.783f, 307.518f
path.quadTo(SkBits2Float(0x43956436), SkBits2Float(0x439494e1), SkBits2Float(0x43990d78), SkBits2Float(0x4390eb9f));  // 298.783f, 297.163f, 306.105f, 289.841f
path.quadTo(SkBits2Float(0x439cb6ba), SkBits2Float(0x438d425d), SkBits2Float(0x43a1e436), SkBits2Float(0x438d425d));  // 313.428f, 282.518f, 323.783f, 282.518f
path.quadTo(SkBits2Float(0x43a711b2), SkBits2Float(0x438d425d), SkBits2Float(0x43aabaf4), SkBits2Float(0x4390eb9f));  // 334.138f, 282.518f, 341.461f, 289.841f
path.quadTo(SkBits2Float(0x43ae6436), SkBits2Float(0x439494e1), SkBits2Float(0x43ae6436), SkBits2Float(0x4399c25d));  // 348.783f, 297.163f, 348.783f, 307.518f
path.close();
    SkPath path111(path);
    builder.add(path111, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x437ef735), SkBits2Float(0x43a4d766));  // 254.966f, 329.683f
path.quadTo(SkBits2Float(0x437ef735), SkBits2Float(0x43aa04e2), SkBits2Float(0x4377a4b1), SkBits2Float(0x43adae24));  // 254.966f, 340.038f, 247.643f, 347.36f
path.quadTo(SkBits2Float(0x4370522c), SkBits2Float(0x43b15766), SkBits2Float(0x4365f735), SkBits2Float(0x43b15766));  // 240.321f, 354.683f, 229.966f, 354.683f
path.quadTo(SkBits2Float(0x435b9c3e), SkBits2Float(0x43b15766), SkBits2Float(0x435449b9), SkBits2Float(0x43adae24));  // 219.61f, 354.683f, 212.288f, 347.36f
path.quadTo(SkBits2Float(0x434cf735), SkBits2Float(0x43aa04e2), SkBits2Float(0x434cf735), SkBits2Float(0x43a4d766));  // 204.966f, 340.038f, 204.966f, 329.683f
path.quadTo(SkBits2Float(0x434cf735), SkBits2Float(0x439fa9ea), SkBits2Float(0x435449b9), SkBits2Float(0x439c00a8));  // 204.966f, 319.327f, 212.288f, 312.005f
path.quadTo(SkBits2Float(0x435b9c3e), SkBits2Float(0x43985766), SkBits2Float(0x4365f735), SkBits2Float(0x43985766));  // 219.61f, 304.683f, 229.966f, 304.683f
path.quadTo(SkBits2Float(0x4370522c), SkBits2Float(0x43985766), SkBits2Float(0x4377a4b1), SkBits2Float(0x439c00a8));  // 240.321f, 304.683f, 247.643f, 312.005f
path.quadTo(SkBits2Float(0x437ef735), SkBits2Float(0x439fa9ea), SkBits2Float(0x437ef735), SkBits2Float(0x43a4d766));  // 254.966f, 319.327f, 254.966f, 329.683f
path.close();
    SkPath path112(path);
    builder.add(path112, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4323b1dd), SkBits2Float(0x433e57b5));  // 163.695f, 190.343f
path.quadTo(SkBits2Float(0x4323b1dd), SkBits2Float(0x4348b2ac), SkBits2Float(0x431c5f59), SkBits2Float(0x43500531));  // 163.695f, 200.698f, 156.372f, 208.02f
path.quadTo(SkBits2Float(0x43150cd4), SkBits2Float(0x435757b5), SkBits2Float(0x430ab1dd), SkBits2Float(0x435757b5));  // 149.05f, 215.343f, 138.695f, 215.343f
path.quadTo(SkBits2Float(0x430056e6), SkBits2Float(0x435757b5), SkBits2Float(0x42f208c2), SkBits2Float(0x43500531));  // 128.339f, 215.343f, 121.017f, 208.02f
path.quadTo(SkBits2Float(0x42e363ba), SkBits2Float(0x4348b2ac), SkBits2Float(0x42e363ba), SkBits2Float(0x433e57b5));  // 113.695f, 200.698f, 113.695f, 190.343f
path.quadTo(SkBits2Float(0x42e363ba), SkBits2Float(0x4333fcbe), SkBits2Float(0x42f208c2), SkBits2Float(0x432caa39));  // 113.695f, 179.987f, 121.017f, 172.665f
path.quadTo(SkBits2Float(0x430056e6), SkBits2Float(0x432557b5), SkBits2Float(0x430ab1dd), SkBits2Float(0x432557b5));  // 128.339f, 165.343f, 138.695f, 165.343f
path.quadTo(SkBits2Float(0x43150cd4), SkBits2Float(0x432557b5), SkBits2Float(0x431c5f59), SkBits2Float(0x432caa39));  // 149.05f, 165.343f, 156.372f, 172.665f
path.quadTo(SkBits2Float(0x4323b1dd), SkBits2Float(0x4333fcbe), SkBits2Float(0x4323b1dd), SkBits2Float(0x433e57b5));  // 163.695f, 179.987f, 163.695f, 190.343f
path.close();
    SkPath path113(path);
    builder.add(path113, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x436e6f43), SkBits2Float(0x435d1aaa));  // 238.435f, 221.104f
path.quadTo(SkBits2Float(0x436e6f43), SkBits2Float(0x436775a2), SkBits2Float(0x43671cbf), SkBits2Float(0x436ec826));  // 238.435f, 231.46f, 231.112f, 238.782f
path.quadTo(SkBits2Float(0x435fca3a), SkBits2Float(0x43761aaa), SkBits2Float(0x43556f43), SkBits2Float(0x43761aaa));  // 223.79f, 246.104f, 213.435f, 246.104f
path.quadTo(SkBits2Float(0x434b144c), SkBits2Float(0x43761aaa), SkBits2Float(0x4343c1c7), SkBits2Float(0x436ec826));  // 203.079f, 246.104f, 195.757f, 238.782f
path.quadTo(SkBits2Float(0x433c6f43), SkBits2Float(0x436775a2), SkBits2Float(0x433c6f43), SkBits2Float(0x435d1aaa));  // 188.435f, 231.46f, 188.435f, 221.104f
path.quadTo(SkBits2Float(0x433c6f43), SkBits2Float(0x4352bfb2), SkBits2Float(0x4343c1c7), SkBits2Float(0x434b6d2e));  // 188.435f, 210.749f, 195.757f, 203.426f
path.quadTo(SkBits2Float(0x434b144c), SkBits2Float(0x43441aaa), SkBits2Float(0x43556f43), SkBits2Float(0x43441aaa));  // 203.079f, 196.104f, 213.435f, 196.104f
path.quadTo(SkBits2Float(0x435fca3a), SkBits2Float(0x43441aaa), SkBits2Float(0x43671cbf), SkBits2Float(0x434b6d2e));  // 223.79f, 196.104f, 231.112f, 203.426f
path.quadTo(SkBits2Float(0x436e6f43), SkBits2Float(0x4352bfb2), SkBits2Float(0x436e6f43), SkBits2Float(0x435d1aaa));  // 238.435f, 210.749f, 238.435f, 221.104f
path.close();
    SkPath path114(path);
    builder.add(path114, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43699f20), SkBits2Float(0x43b74967));  // 233.622f, 366.573f
path.quadTo(SkBits2Float(0x43699f20), SkBits2Float(0x43bc76e3), SkBits2Float(0x43624c9c), SkBits2Float(0x43c02025));  // 233.622f, 376.929f, 226.299f, 384.251f
path.quadTo(SkBits2Float(0x435afa18), SkBits2Float(0x43c3c967), SkBits2Float(0x43509f20), SkBits2Float(0x43c3c967));  // 218.977f, 391.573f, 208.622f, 391.573f
path.quadTo(SkBits2Float(0x43464428), SkBits2Float(0x43c3c967), SkBits2Float(0x433ef1a4), SkBits2Float(0x43c02025));  // 198.266f, 391.573f, 190.944f, 384.251f
path.quadTo(SkBits2Float(0x43379f20), SkBits2Float(0x43bc76e3), SkBits2Float(0x43379f20), SkBits2Float(0x43b74967));  // 183.622f, 376.929f, 183.622f, 366.573f
path.quadTo(SkBits2Float(0x43379f20), SkBits2Float(0x43b21beb), SkBits2Float(0x433ef1a4), SkBits2Float(0x43ae72a9));  // 183.622f, 356.218f, 190.944f, 348.896f
path.quadTo(SkBits2Float(0x43464428), SkBits2Float(0x43aac967), SkBits2Float(0x43509f20), SkBits2Float(0x43aac967));  // 198.266f, 341.573f, 208.622f, 341.573f
path.quadTo(SkBits2Float(0x435afa18), SkBits2Float(0x43aac967), SkBits2Float(0x43624c9c), SkBits2Float(0x43ae72a9));  // 218.977f, 341.573f, 226.299f, 348.896f
path.quadTo(SkBits2Float(0x43699f20), SkBits2Float(0x43b21beb), SkBits2Float(0x43699f20), SkBits2Float(0x43b74967));  // 233.622f, 356.218f, 233.622f, 366.573f
path.close();
    SkPath path115(path);
    builder.add(path115, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4372b5ce), SkBits2Float(0x434919ea));  // 242.71f, 201.101f
path.quadTo(SkBits2Float(0x4372b5ce), SkBits2Float(0x435374e2), SkBits2Float(0x436b634a), SkBits2Float(0x435ac766));  // 242.71f, 211.457f, 235.388f, 218.779f
path.quadTo(SkBits2Float(0x436410c6), SkBits2Float(0x436219ea), SkBits2Float(0x4359b5ce), SkBits2Float(0x436219ea));  // 228.066f, 226.101f, 217.71f, 226.101f
path.quadTo(SkBits2Float(0x434f5ad6), SkBits2Float(0x436219ea), SkBits2Float(0x43480852), SkBits2Float(0x435ac766));  // 207.355f, 226.101f, 200.033f, 218.779f
path.quadTo(SkBits2Float(0x4340b5ce), SkBits2Float(0x435374e2), SkBits2Float(0x4340b5ce), SkBits2Float(0x434919ea));  // 192.71f, 211.457f, 192.71f, 201.101f
path.quadTo(SkBits2Float(0x4340b5ce), SkBits2Float(0x433ebef2), SkBits2Float(0x43480852), SkBits2Float(0x43376c6e));  // 192.71f, 190.746f, 200.033f, 183.424f
path.quadTo(SkBits2Float(0x434f5ad6), SkBits2Float(0x433019ea), SkBits2Float(0x4359b5ce), SkBits2Float(0x433019ea));  // 207.355f, 176.101f, 217.71f, 176.101f
path.quadTo(SkBits2Float(0x436410c6), SkBits2Float(0x433019ea), SkBits2Float(0x436b634a), SkBits2Float(0x43376c6e));  // 228.066f, 176.101f, 235.388f, 183.424f
path.quadTo(SkBits2Float(0x4372b5ce), SkBits2Float(0x433ebef2), SkBits2Float(0x4372b5ce), SkBits2Float(0x434919ea));  // 242.71f, 190.746f, 242.71f, 201.101f
path.close();
    SkPath path116(path);
    builder.add(path116, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43212033), SkBits2Float(0x433c0771));  // 161.126f, 188.029f
path.quadTo(SkBits2Float(0x43212033), SkBits2Float(0x43466268), SkBits2Float(0x4319cdaf), SkBits2Float(0x434db4ed));  // 161.126f, 198.384f, 153.803f, 205.707f
path.quadTo(SkBits2Float(0x43127b2a), SkBits2Float(0x43550771), SkBits2Float(0x43082033), SkBits2Float(0x43550771));  // 146.481f, 213.029f, 136.126f, 213.029f
path.quadTo(SkBits2Float(0x42fb8a77), SkBits2Float(0x43550771), SkBits2Float(0x42ece56e), SkBits2Float(0x434db4ed));  // 125.77f, 213.029f, 118.448f, 205.707f
path.quadTo(SkBits2Float(0x42de4066), SkBits2Float(0x43466268), SkBits2Float(0x42de4066), SkBits2Float(0x433c0771));  // 111.126f, 198.384f, 111.126f, 188.029f
path.quadTo(SkBits2Float(0x42de4066), SkBits2Float(0x4331ac7a), SkBits2Float(0x42ece56e), SkBits2Float(0x432a59f5));  // 111.126f, 177.674f, 118.448f, 170.351f
path.quadTo(SkBits2Float(0x42fb8a77), SkBits2Float(0x43230771), SkBits2Float(0x43082033), SkBits2Float(0x43230771));  // 125.77f, 163.029f, 136.126f, 163.029f
path.quadTo(SkBits2Float(0x43127b2a), SkBits2Float(0x43230771), SkBits2Float(0x4319cdaf), SkBits2Float(0x432a59f5));  // 146.481f, 163.029f, 153.803f, 170.351f
path.quadTo(SkBits2Float(0x43212033), SkBits2Float(0x4331ac7a), SkBits2Float(0x43212033), SkBits2Float(0x433c0771));  // 161.126f, 177.674f, 161.126f, 188.029f
path.close();
    SkPath path117(path);
    builder.add(path117, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x434d1191), SkBits2Float(0x431c5cdf));  // 205.069f, 156.363f
path.quadTo(SkBits2Float(0x434d1191), SkBits2Float(0x4326b7d6), SkBits2Float(0x4345bf0d), SkBits2Float(0x432e0a5b));  // 205.069f, 166.718f, 197.746f, 174.04f
path.quadTo(SkBits2Float(0x433e6c88), SkBits2Float(0x43355cdf), SkBits2Float(0x43341191), SkBits2Float(0x43355cdf));  // 190.424f, 181.363f, 180.069f, 181.363f
path.quadTo(SkBits2Float(0x4329b69a), SkBits2Float(0x43355cdf), SkBits2Float(0x43226415), SkBits2Float(0x432e0a5b));  // 169.713f, 181.363f, 162.391f, 174.04f
path.quadTo(SkBits2Float(0x431b1191), SkBits2Float(0x4326b7d6), SkBits2Float(0x431b1191), SkBits2Float(0x431c5cdf));  // 155.069f, 166.718f, 155.069f, 156.363f
path.quadTo(SkBits2Float(0x431b1191), SkBits2Float(0x431201e8), SkBits2Float(0x43226415), SkBits2Float(0x430aaf63));  // 155.069f, 146.007f, 162.391f, 138.685f
path.quadTo(SkBits2Float(0x4329b69a), SkBits2Float(0x43035cdf), SkBits2Float(0x43341191), SkBits2Float(0x43035cdf));  // 169.713f, 131.363f, 180.069f, 131.363f
path.quadTo(SkBits2Float(0x433e6c88), SkBits2Float(0x43035cdf), SkBits2Float(0x4345bf0d), SkBits2Float(0x430aaf63));  // 190.424f, 131.363f, 197.746f, 138.685f
path.quadTo(SkBits2Float(0x434d1191), SkBits2Float(0x431201e8), SkBits2Float(0x434d1191), SkBits2Float(0x431c5cdf));  // 205.069f, 146.007f, 205.069f, 156.363f
path.close();
    SkPath path118(path);
    builder.add(path118, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43f7761c), SkBits2Float(0x43ba1964));  // 494.923f, 372.198f
path.quadTo(SkBits2Float(0x43f7761c), SkBits2Float(0x43bf46e0), SkBits2Float(0x43f3ccda), SkBits2Float(0x43c2f022));  // 494.923f, 382.554f, 487.6f, 389.876f
path.quadTo(SkBits2Float(0x43f02398), SkBits2Float(0x43c69964), SkBits2Float(0x43eaf61c), SkBits2Float(0x43c69964));  // 480.278f, 397.198f, 469.923f, 397.198f
path.quadTo(SkBits2Float(0x43e5c8a0), SkBits2Float(0x43c69964), SkBits2Float(0x43e21f5e), SkBits2Float(0x43c2f022));  // 459.567f, 397.198f, 452.245f, 389.876f
path.quadTo(SkBits2Float(0x43de761c), SkBits2Float(0x43bf46e0), SkBits2Float(0x43de761c), SkBits2Float(0x43ba1964));  // 444.923f, 382.554f, 444.923f, 372.198f
path.quadTo(SkBits2Float(0x43de761c), SkBits2Float(0x43b4ebe8), SkBits2Float(0x43e21f5e), SkBits2Float(0x43b142a6));  // 444.923f, 361.843f, 452.245f, 354.521f
path.quadTo(SkBits2Float(0x43e5c8a0), SkBits2Float(0x43ad9964), SkBits2Float(0x43eaf61c), SkBits2Float(0x43ad9964));  // 459.567f, 347.198f, 469.923f, 347.198f
path.quadTo(SkBits2Float(0x43f02398), SkBits2Float(0x43ad9964), SkBits2Float(0x43f3ccda), SkBits2Float(0x43b142a6));  // 480.278f, 347.198f, 487.6f, 354.521f
path.quadTo(SkBits2Float(0x43f7761c), SkBits2Float(0x43b4ebe8), SkBits2Float(0x43f7761c), SkBits2Float(0x43ba1964));  // 494.923f, 361.843f, 494.923f, 372.198f
path.close();
    SkPath path119(path);
    builder.add(path119, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4387b4f4), SkBits2Float(0x4382767e));  // 271.414f, 260.926f
path.quadTo(SkBits2Float(0x4387b4f4), SkBits2Float(0x4387a3fa), SkBits2Float(0x43840bb2), SkBits2Float(0x438b4d3c));  // 271.414f, 271.281f, 264.091f, 278.603f
path.quadTo(SkBits2Float(0x43806270), SkBits2Float(0x438ef67e), SkBits2Float(0x437669e8), SkBits2Float(0x438ef67e));  // 256.769f, 285.926f, 246.414f, 285.926f
path.quadTo(SkBits2Float(0x436c0ef1), SkBits2Float(0x438ef67e), SkBits2Float(0x4364bc6d), SkBits2Float(0x438b4d3c));  // 236.058f, 285.926f, 228.736f, 278.603f
path.quadTo(SkBits2Float(0x435d69e9), SkBits2Float(0x4387a3fa), SkBits2Float(0x435d69e9), SkBits2Float(0x4382767e));  // 221.414f, 271.281f, 221.414f, 260.926f
path.quadTo(SkBits2Float(0x435d69e9), SkBits2Float(0x437a9204), SkBits2Float(0x4364bc6d), SkBits2Float(0x43733f80));  // 221.414f, 250.57f, 228.736f, 243.248f
path.quadTo(SkBits2Float(0x436c0ef1), SkBits2Float(0x436becfc), SkBits2Float(0x437669e8), SkBits2Float(0x436becfc));  // 236.058f, 235.926f, 246.414f, 235.926f
path.quadTo(SkBits2Float(0x43806270), SkBits2Float(0x436becfc), SkBits2Float(0x43840bb2), SkBits2Float(0x43733f80));  // 256.769f, 235.926f, 264.091f, 243.248f
path.quadTo(SkBits2Float(0x4387b4f4), SkBits2Float(0x437a9204), SkBits2Float(0x4387b4f4), SkBits2Float(0x4382767e));  // 271.414f, 250.57f, 271.414f, 260.926f
path.close();
    SkPath path120(path);
    builder.add(path120, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43f176b1), SkBits2Float(0x437ef3b8));  // 482.927f, 254.952f
path.quadTo(SkBits2Float(0x43f176b1), SkBits2Float(0x4384a758), SkBits2Float(0x43edcd6f), SkBits2Float(0x4388509a));  // 482.927f, 265.307f, 475.605f, 272.63f
path.quadTo(SkBits2Float(0x43ea242d), SkBits2Float(0x438bf9dc), SkBits2Float(0x43e4f6b1), SkBits2Float(0x438bf9dc));  // 468.283f, 279.952f, 457.927f, 279.952f
path.quadTo(SkBits2Float(0x43dfc935), SkBits2Float(0x438bf9dc), SkBits2Float(0x43dc1ff3), SkBits2Float(0x4388509a));  // 447.572f, 279.952f, 440.25f, 272.63f
path.quadTo(SkBits2Float(0x43d876b1), SkBits2Float(0x4384a758), SkBits2Float(0x43d876b1), SkBits2Float(0x437ef3b8));  // 432.927f, 265.307f, 432.927f, 254.952f
path.quadTo(SkBits2Float(0x43d876b1), SkBits2Float(0x437498c0), SkBits2Float(0x43dc1ff3), SkBits2Float(0x436d463c));  // 432.927f, 244.597f, 440.25f, 237.274f
path.quadTo(SkBits2Float(0x43dfc935), SkBits2Float(0x4365f3b8), SkBits2Float(0x43e4f6b1), SkBits2Float(0x4365f3b8));  // 447.572f, 229.952f, 457.927f, 229.952f
path.quadTo(SkBits2Float(0x43ea242d), SkBits2Float(0x4365f3b8), SkBits2Float(0x43edcd6f), SkBits2Float(0x436d463c));  // 468.283f, 229.952f, 475.605f, 237.274f
path.quadTo(SkBits2Float(0x43f176b1), SkBits2Float(0x437498c0), SkBits2Float(0x43f176b1), SkBits2Float(0x437ef3b8));  // 482.927f, 244.597f, 482.927f, 254.952f
path.close();
    SkPath path121(path);
    builder.add(path121, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x437bd45b), SkBits2Float(0x4361cf52));  // 251.83f, 225.81f
path.quadTo(SkBits2Float(0x437bd45b), SkBits2Float(0x436c2a4a), SkBits2Float(0x437481d7), SkBits2Float(0x43737cce));  // 251.83f, 236.165f, 244.507f, 243.488f
path.quadTo(SkBits2Float(0x436d2f52), SkBits2Float(0x437acf52), SkBits2Float(0x4362d45b), SkBits2Float(0x437acf52));  // 237.185f, 250.81f, 226.83f, 250.81f
path.quadTo(SkBits2Float(0x43587964), SkBits2Float(0x437acf52), SkBits2Float(0x435126df), SkBits2Float(0x43737cce));  // 216.474f, 250.81f, 209.152f, 243.488f
path.quadTo(SkBits2Float(0x4349d45b), SkBits2Float(0x436c2a4a), SkBits2Float(0x4349d45b), SkBits2Float(0x4361cf52));  // 201.83f, 236.165f, 201.83f, 225.81f
path.quadTo(SkBits2Float(0x4349d45b), SkBits2Float(0x4357745a), SkBits2Float(0x435126df), SkBits2Float(0x435021d6));  // 201.83f, 215.454f, 209.152f, 208.132f
path.quadTo(SkBits2Float(0x43587964), SkBits2Float(0x4348cf52), SkBits2Float(0x4362d45b), SkBits2Float(0x4348cf52));  // 216.474f, 200.81f, 226.83f, 200.81f
path.quadTo(SkBits2Float(0x436d2f52), SkBits2Float(0x4348cf52), SkBits2Float(0x437481d7), SkBits2Float(0x435021d6));  // 237.185f, 200.81f, 244.507f, 208.132f
path.quadTo(SkBits2Float(0x437bd45b), SkBits2Float(0x4357745a), SkBits2Float(0x437bd45b), SkBits2Float(0x4361cf52));  // 251.83f, 215.454f, 251.83f, 225.81f
path.close();
    SkPath path122(path);
    builder.add(path122, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43bb783c), SkBits2Float(0x43d20000));  // 374.939f, 420
path.quadTo(SkBits2Float(0x43bb783c), SkBits2Float(0x43d72d7c), SkBits2Float(0x43b7cefa), SkBits2Float(0x43dad6be));  // 374.939f, 430.355f, 367.617f, 437.678f
path.quadTo(SkBits2Float(0x43b425b8), SkBits2Float(0x43de8000), SkBits2Float(0x43aef83c), SkBits2Float(0x43de8000));  // 360.295f, 445, 349.939f, 445
path.quadTo(SkBits2Float(0x43a9cac0), SkBits2Float(0x43de8000), SkBits2Float(0x43a6217e), SkBits2Float(0x43dad6be));  // 339.584f, 445, 332.262f, 437.678f
path.quadTo(SkBits2Float(0x43a2783c), SkBits2Float(0x43d72d7c), SkBits2Float(0x43a2783c), SkBits2Float(0x43d20000));  // 324.939f, 430.355f, 324.939f, 420
path.quadTo(SkBits2Float(0x43a2783c), SkBits2Float(0x43ccd284), SkBits2Float(0x43a6217e), SkBits2Float(0x43c92942));  // 324.939f, 409.645f, 332.262f, 402.322f
path.quadTo(SkBits2Float(0x43a9cac0), SkBits2Float(0x43c58000), SkBits2Float(0x43aef83c), SkBits2Float(0x43c58000));  // 339.584f, 395, 349.939f, 395
path.quadTo(SkBits2Float(0x43b425b8), SkBits2Float(0x43c58000), SkBits2Float(0x43b7cefa), SkBits2Float(0x43c92942));  // 360.295f, 395, 367.617f, 402.322f
path.quadTo(SkBits2Float(0x43bb783c), SkBits2Float(0x43ccd284), SkBits2Float(0x43bb783c), SkBits2Float(0x43d20000));  // 374.939f, 409.645f, 374.939f, 420
path.close();
    SkPath path123(path);
    builder.add(path123, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ca2436), SkBits2Float(0x435899a3));  // 404.283f, 216.6f
path.quadTo(SkBits2Float(0x43ca2436), SkBits2Float(0x4362f49a), SkBits2Float(0x43c67af4), SkBits2Float(0x436a471f));  // 404.283f, 226.955f, 396.961f, 234.278f
path.quadTo(SkBits2Float(0x43c2d1b2), SkBits2Float(0x437199a3), SkBits2Float(0x43bda436), SkBits2Float(0x437199a3));  // 389.638f, 241.6f, 379.283f, 241.6f
path.quadTo(SkBits2Float(0x43b876ba), SkBits2Float(0x437199a3), SkBits2Float(0x43b4cd78), SkBits2Float(0x436a471f));  // 368.928f, 241.6f, 361.605f, 234.278f
path.quadTo(SkBits2Float(0x43b12436), SkBits2Float(0x4362f49a), SkBits2Float(0x43b12436), SkBits2Float(0x435899a3));  // 354.283f, 226.955f, 354.283f, 216.6f
path.quadTo(SkBits2Float(0x43b12436), SkBits2Float(0x434e3eac), SkBits2Float(0x43b4cd78), SkBits2Float(0x4346ec27));  // 354.283f, 206.245f, 361.605f, 198.922f
path.quadTo(SkBits2Float(0x43b876ba), SkBits2Float(0x433f99a3), SkBits2Float(0x43bda436), SkBits2Float(0x433f99a3));  // 368.928f, 191.6f, 379.283f, 191.6f
path.quadTo(SkBits2Float(0x43c2d1b2), SkBits2Float(0x433f99a3), SkBits2Float(0x43c67af4), SkBits2Float(0x4346ec27));  // 389.638f, 191.6f, 396.961f, 198.922f
path.quadTo(SkBits2Float(0x43ca2436), SkBits2Float(0x434e3eac), SkBits2Float(0x43ca2436), SkBits2Float(0x435899a3));  // 404.283f, 206.245f, 404.283f, 216.6f
path.close();
    SkPath path124(path);
    builder.add(path124, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x440faed8), SkBits2Float(0x43474968));  // 574.732f, 199.287f
path.quadTo(SkBits2Float(0x440faed8), SkBits2Float(0x4351a460), SkBits2Float(0x440dda37), SkBits2Float(0x4358f6e4));  // 574.732f, 209.642f, 567.41f, 216.964f
path.quadTo(SkBits2Float(0x440c0596), SkBits2Float(0x43604968), SkBits2Float(0x44096ed8), SkBits2Float(0x43604968));  // 560.087f, 224.287f, 549.732f, 224.287f
path.quadTo(SkBits2Float(0x4406d81a), SkBits2Float(0x43604968), SkBits2Float(0x44050379), SkBits2Float(0x4358f6e4));  // 539.377f, 224.287f, 532.054f, 216.964f
path.quadTo(SkBits2Float(0x44032ed8), SkBits2Float(0x4351a460), SkBits2Float(0x44032ed8), SkBits2Float(0x43474968));  // 524.732f, 209.642f, 524.732f, 199.287f
path.quadTo(SkBits2Float(0x44032ed8), SkBits2Float(0x433cee70), SkBits2Float(0x44050379), SkBits2Float(0x43359bec));  // 524.732f, 188.931f, 532.054f, 181.609f
path.quadTo(SkBits2Float(0x4406d81a), SkBits2Float(0x432e4968), SkBits2Float(0x44096ed8), SkBits2Float(0x432e4968));  // 539.377f, 174.287f, 549.732f, 174.287f
path.quadTo(SkBits2Float(0x440c0596), SkBits2Float(0x432e4968), SkBits2Float(0x440dda37), SkBits2Float(0x43359bec));  // 560.087f, 174.287f, 567.41f, 181.609f
path.quadTo(SkBits2Float(0x440faed8), SkBits2Float(0x433cee70), SkBits2Float(0x440faed8), SkBits2Float(0x43474968));  // 574.732f, 188.931f, 574.732f, 199.287f
path.close();
    SkPath path125(path);
    builder.add(path125, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x437055b3), SkBits2Float(0x438ae764));  // 240.335f, 277.808f
path.quadTo(SkBits2Float(0x437055b3), SkBits2Float(0x439014e0), SkBits2Float(0x4369032f), SkBits2Float(0x4393be22));  // 240.335f, 288.163f, 233.012f, 295.485f
path.quadTo(SkBits2Float(0x4361b0aa), SkBits2Float(0x43976764), SkBits2Float(0x435755b3), SkBits2Float(0x43976764));  // 225.69f, 302.808f, 215.335f, 302.808f
path.quadTo(SkBits2Float(0x434cfabc), SkBits2Float(0x43976764), SkBits2Float(0x4345a837), SkBits2Float(0x4393be22));  // 204.979f, 302.808f, 197.657f, 295.485f
path.quadTo(SkBits2Float(0x433e55b3), SkBits2Float(0x439014e0), SkBits2Float(0x433e55b3), SkBits2Float(0x438ae764));  // 190.335f, 288.163f, 190.335f, 277.808f
path.quadTo(SkBits2Float(0x433e55b3), SkBits2Float(0x4385b9e8), SkBits2Float(0x4345a837), SkBits2Float(0x438210a6));  // 190.335f, 267.452f, 197.657f, 260.13f
path.quadTo(SkBits2Float(0x434cfabc), SkBits2Float(0x437ccec8), SkBits2Float(0x435755b3), SkBits2Float(0x437ccec8));  // 204.979f, 252.808f, 215.335f, 252.808f
path.quadTo(SkBits2Float(0x4361b0aa), SkBits2Float(0x437ccec8), SkBits2Float(0x4369032f), SkBits2Float(0x438210a6));  // 225.69f, 252.808f, 233.012f, 260.13f
path.quadTo(SkBits2Float(0x437055b3), SkBits2Float(0x4385b9e8), SkBits2Float(0x437055b3), SkBits2Float(0x438ae764));  // 240.335f, 267.452f, 240.335f, 277.808f
path.close();
    SkPath path126(path);
    builder.add(path126, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a201c9), SkBits2Float(0x43546e88));  // 324.014f, 212.432f
path.quadTo(SkBits2Float(0x43a201c9), SkBits2Float(0x435ec980), SkBits2Float(0x439e5887), SkBits2Float(0x43661c04));  // 324.014f, 222.787f, 316.692f, 230.109f
path.quadTo(SkBits2Float(0x439aaf45), SkBits2Float(0x436d6e88), SkBits2Float(0x439581c9), SkBits2Float(0x436d6e88));  // 309.369f, 237.432f, 299.014f, 237.432f
path.quadTo(SkBits2Float(0x4390544d), SkBits2Float(0x436d6e88), SkBits2Float(0x438cab0b), SkBits2Float(0x43661c04));  // 288.659f, 237.432f, 281.336f, 230.109f
path.quadTo(SkBits2Float(0x438901c9), SkBits2Float(0x435ec980), SkBits2Float(0x438901c9), SkBits2Float(0x43546e88));  // 274.014f, 222.787f, 274.014f, 212.432f
path.quadTo(SkBits2Float(0x438901c9), SkBits2Float(0x434a1390), SkBits2Float(0x438cab0b), SkBits2Float(0x4342c10c));  // 274.014f, 202.076f, 281.336f, 194.754f
path.quadTo(SkBits2Float(0x4390544d), SkBits2Float(0x433b6e88), SkBits2Float(0x439581c9), SkBits2Float(0x433b6e88));  // 288.659f, 187.432f, 299.014f, 187.432f
path.quadTo(SkBits2Float(0x439aaf45), SkBits2Float(0x433b6e88), SkBits2Float(0x439e5887), SkBits2Float(0x4342c10c));  // 309.369f, 187.432f, 316.692f, 194.754f
path.quadTo(SkBits2Float(0x43a201c9), SkBits2Float(0x434a1390), SkBits2Float(0x43a201c9), SkBits2Float(0x43546e88));  // 324.014f, 202.076f, 324.014f, 212.432f
path.close();
    SkPath path127(path);
    builder.add(path127, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42d10265), SkBits2Float(0x434da766));  // 104.505f, 205.654f
path.quadTo(SkBits2Float(0x42d10265), SkBits2Float(0x4358025e), SkBits2Float(0x42c25d5c), SkBits2Float(0x435f54e2));  // 104.505f, 216.009f, 97.1823f, 223.332f
path.quadTo(SkBits2Float(0x42b3b854), SkBits2Float(0x4366a766), SkBits2Float(0x429f0265), SkBits2Float(0x4366a766));  // 89.86f, 230.654f, 79.5047f, 230.654f
path.quadTo(SkBits2Float(0x428a4c76), SkBits2Float(0x4366a766), SkBits2Float(0x42774edb), SkBits2Float(0x435f54e2));  // 69.1493f, 230.654f, 61.827f, 223.332f
path.quadTo(SkBits2Float(0x425a04ca), SkBits2Float(0x4358025e), SkBits2Float(0x425a04ca), SkBits2Float(0x434da766));  // 54.5047f, 216.009f, 54.5047f, 205.654f
path.quadTo(SkBits2Float(0x425a04ca), SkBits2Float(0x43434c6e), SkBits2Float(0x42774edb), SkBits2Float(0x433bf9ea));  // 54.5047f, 195.299f, 61.827f, 187.976f
path.quadTo(SkBits2Float(0x428a4c76), SkBits2Float(0x4334a766), SkBits2Float(0x429f0265), SkBits2Float(0x4334a766));  // 69.1493f, 180.654f, 79.5047f, 180.654f
path.quadTo(SkBits2Float(0x42b3b854), SkBits2Float(0x4334a766), SkBits2Float(0x42c25d5c), SkBits2Float(0x433bf9ea));  // 89.86f, 180.654f, 97.1823f, 187.976f
path.quadTo(SkBits2Float(0x42d10265), SkBits2Float(0x43434c6e), SkBits2Float(0x42d10265), SkBits2Float(0x434da766));  // 104.505f, 195.299f, 104.505f, 205.654f
path.close();
    SkPath path128(path);
    builder.add(path128, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4396e5f4), SkBits2Float(0x432540f2));  // 301.797f, 165.254f
path.quadTo(SkBits2Float(0x4396e5f4), SkBits2Float(0x432f9bea), SkBits2Float(0x43933cb2), SkBits2Float(0x4336ee6e));  // 301.797f, 175.609f, 294.474f, 182.931f
path.quadTo(SkBits2Float(0x438f9370), SkBits2Float(0x433e40f2), SkBits2Float(0x438a65f4), SkBits2Float(0x433e40f2));  // 287.152f, 190.254f, 276.797f, 190.254f
path.quadTo(SkBits2Float(0x43853878), SkBits2Float(0x433e40f2), SkBits2Float(0x43818f36), SkBits2Float(0x4336ee6e));  // 266.441f, 190.254f, 259.119f, 182.931f
path.quadTo(SkBits2Float(0x437bcbe8), SkBits2Float(0x432f9bea), SkBits2Float(0x437bcbe8), SkBits2Float(0x432540f2));  // 251.797f, 175.609f, 251.797f, 165.254f
path.quadTo(SkBits2Float(0x437bcbe8), SkBits2Float(0x431ae5fa), SkBits2Float(0x43818f36), SkBits2Float(0x43139376));  // 251.797f, 154.898f, 259.119f, 147.576f
path.quadTo(SkBits2Float(0x43853878), SkBits2Float(0x430c40f2), SkBits2Float(0x438a65f4), SkBits2Float(0x430c40f2));  // 266.441f, 140.254f, 276.797f, 140.254f
path.quadTo(SkBits2Float(0x438f9370), SkBits2Float(0x430c40f2), SkBits2Float(0x43933cb2), SkBits2Float(0x43139376));  // 287.152f, 140.254f, 294.474f, 147.576f
path.quadTo(SkBits2Float(0x4396e5f4), SkBits2Float(0x431ae5fa), SkBits2Float(0x4396e5f4), SkBits2Float(0x432540f2));  // 301.797f, 154.898f, 301.797f, 165.254f
path.close();
    SkPath path129(path);
    builder.add(path129, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43759ab0), SkBits2Float(0x43329c4b));  // 245.604f, 178.611f
path.quadTo(SkBits2Float(0x43759ab0), SkBits2Float(0x433cf742), SkBits2Float(0x436e482c), SkBits2Float(0x434449c7));  // 245.604f, 188.966f, 238.282f, 196.288f
path.quadTo(SkBits2Float(0x4366f5a8), SkBits2Float(0x434b9c4b), SkBits2Float(0x435c9ab0), SkBits2Float(0x434b9c4b));  // 230.96f, 203.611f, 220.604f, 203.611f
path.quadTo(SkBits2Float(0x43523fb8), SkBits2Float(0x434b9c4b), SkBits2Float(0x434aed34), SkBits2Float(0x434449c7));  // 210.249f, 203.611f, 202.927f, 196.288f
path.quadTo(SkBits2Float(0x43439ab0), SkBits2Float(0x433cf742), SkBits2Float(0x43439ab0), SkBits2Float(0x43329c4b));  // 195.604f, 188.966f, 195.604f, 178.611f
path.quadTo(SkBits2Float(0x43439ab0), SkBits2Float(0x43284154), SkBits2Float(0x434aed34), SkBits2Float(0x4320eecf));  // 195.604f, 168.255f, 202.927f, 160.933f
path.quadTo(SkBits2Float(0x43523fb8), SkBits2Float(0x43199c4b), SkBits2Float(0x435c9ab0), SkBits2Float(0x43199c4b));  // 210.249f, 153.611f, 220.604f, 153.611f
path.quadTo(SkBits2Float(0x4366f5a8), SkBits2Float(0x43199c4b), SkBits2Float(0x436e482c), SkBits2Float(0x4320eecf));  // 230.96f, 153.611f, 238.282f, 160.933f
path.quadTo(SkBits2Float(0x43759ab0), SkBits2Float(0x43284154), SkBits2Float(0x43759ab0), SkBits2Float(0x43329c4b));  // 245.604f, 168.255f, 245.604f, 178.611f
path.close();
    SkPath path130(path);
    builder.add(path130, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43a6e3cc), SkBits2Float(0x43954ede));  // 333.78f, 298.616f
path.quadTo(SkBits2Float(0x43a6e3cc), SkBits2Float(0x439a7c5a), SkBits2Float(0x43a33a8a), SkBits2Float(0x439e259c));  // 333.78f, 308.971f, 326.457f, 316.294f
path.quadTo(SkBits2Float(0x439f9148), SkBits2Float(0x43a1cede), SkBits2Float(0x439a63cc), SkBits2Float(0x43a1cede));  // 319.135f, 323.616f, 308.78f, 323.616f
path.quadTo(SkBits2Float(0x43953650), SkBits2Float(0x43a1cede), SkBits2Float(0x43918d0e), SkBits2Float(0x439e259c));  // 298.424f, 323.616f, 291.102f, 316.294f
path.quadTo(SkBits2Float(0x438de3cc), SkBits2Float(0x439a7c5a), SkBits2Float(0x438de3cc), SkBits2Float(0x43954ede));  // 283.78f, 308.971f, 283.78f, 298.616f
path.quadTo(SkBits2Float(0x438de3cc), SkBits2Float(0x43902162), SkBits2Float(0x43918d0e), SkBits2Float(0x438c7820));  // 283.78f, 288.261f, 291.102f, 280.938f
path.quadTo(SkBits2Float(0x43953650), SkBits2Float(0x4388cede), SkBits2Float(0x439a63cc), SkBits2Float(0x4388cede));  // 298.424f, 273.616f, 308.78f, 273.616f
path.quadTo(SkBits2Float(0x439f9148), SkBits2Float(0x4388cede), SkBits2Float(0x43a33a8a), SkBits2Float(0x438c7820));  // 319.135f, 273.616f, 326.457f, 280.938f
path.quadTo(SkBits2Float(0x43a6e3cc), SkBits2Float(0x43902162), SkBits2Float(0x43a6e3cc), SkBits2Float(0x43954ede));  // 333.78f, 288.261f, 333.78f, 298.616f
path.close();
    SkPath path131(path);
    builder.add(path131, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x428e400e), SkBits2Float(0x43a02b61));  // 71.1251f, 320.339f
path.quadTo(SkBits2Float(0x428e400e), SkBits2Float(0x43a558dd), SkBits2Float(0x427f360c), SkBits2Float(0x43a9021f));  // 71.1251f, 330.694f, 63.8028f, 338.017f
path.quadTo(SkBits2Float(0x4261ebfa), SkBits2Float(0x43acab61), SkBits2Float(0x4238801c), SkBits2Float(0x43acab61));  // 56.4804f, 345.339f, 46.1251f, 345.339f
path.quadTo(SkBits2Float(0x420f143e), SkBits2Float(0x43acab61), SkBits2Float(0x41e39459), SkBits2Float(0x43a9021f));  // 35.7698f, 345.339f, 28.4474f, 338.017f
path.quadTo(SkBits2Float(0x41a90036), SkBits2Float(0x43a558dd), SkBits2Float(0x41a90036), SkBits2Float(0x43a02b61));  // 21.1251f, 330.694f, 21.1251f, 320.339f
path.quadTo(SkBits2Float(0x41a90036), SkBits2Float(0x439afde5), SkBits2Float(0x41e39459), SkBits2Float(0x439754a3));  // 21.1251f, 309.984f, 28.4474f, 302.661f
path.quadTo(SkBits2Float(0x420f143e), SkBits2Float(0x4393ab61), SkBits2Float(0x4238801c), SkBits2Float(0x4393ab61));  // 35.7698f, 295.339f, 46.1251f, 295.339f
path.quadTo(SkBits2Float(0x4261ebfa), SkBits2Float(0x4393ab61), SkBits2Float(0x427f360c), SkBits2Float(0x439754a3));  // 56.4804f, 295.339f, 63.8028f, 302.661f
path.quadTo(SkBits2Float(0x428e400e), SkBits2Float(0x439afde5), SkBits2Float(0x428e400e), SkBits2Float(0x43a02b61));  // 71.1251f, 309.984f, 71.1251f, 320.339f
path.close();
    SkPath path132(path);
    builder.add(path132, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4323b27c), SkBits2Float(0x4379f49c));  // 163.697f, 249.956f
path.quadTo(SkBits2Float(0x4323b27c), SkBits2Float(0x438227ca), SkBits2Float(0x431c5ff8), SkBits2Float(0x4385d10c));  // 163.697f, 260.311f, 156.375f, 267.633f
path.quadTo(SkBits2Float(0x43150d74), SkBits2Float(0x43897a4e), SkBits2Float(0x430ab27c), SkBits2Float(0x43897a4e));  // 149.053f, 274.956f, 138.697f, 274.956f
path.quadTo(SkBits2Float(0x43005784), SkBits2Float(0x43897a4e), SkBits2Float(0x42f20a00), SkBits2Float(0x4385d10c));  // 128.342f, 274.956f, 121.02f, 267.633f
path.quadTo(SkBits2Float(0x42e364f8), SkBits2Float(0x438227ca), SkBits2Float(0x42e364f8), SkBits2Float(0x4379f49c));  // 113.697f, 260.311f, 113.697f, 249.956f
path.quadTo(SkBits2Float(0x42e364f8), SkBits2Float(0x436f99a4), SkBits2Float(0x42f20a00), SkBits2Float(0x43684720));  // 113.697f, 239.6f, 121.02f, 232.278f
path.quadTo(SkBits2Float(0x43005784), SkBits2Float(0x4360f49c), SkBits2Float(0x430ab27c), SkBits2Float(0x4360f49c));  // 128.342f, 224.956f, 138.697f, 224.956f
path.quadTo(SkBits2Float(0x43150d74), SkBits2Float(0x4360f49c), SkBits2Float(0x431c5ff8), SkBits2Float(0x43684720));  // 149.053f, 224.956f, 156.375f, 232.278f
path.quadTo(SkBits2Float(0x4323b27c), SkBits2Float(0x436f99a4), SkBits2Float(0x4323b27c), SkBits2Float(0x4379f49c));  // 163.697f, 239.6f, 163.697f, 249.956f
path.close();
    SkPath path133(path);
    builder.add(path133, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43629af6), SkBits2Float(0x434de1f1));  // 226.605f, 205.883f
path.quadTo(SkBits2Float(0x43629af6), SkBits2Float(0x43583ce8), SkBits2Float(0x435b4872), SkBits2Float(0x435f8f6d));  // 226.605f, 216.238f, 219.283f, 223.56f
path.quadTo(SkBits2Float(0x4353f5ee), SkBits2Float(0x4366e1f1), SkBits2Float(0x43499af6), SkBits2Float(0x4366e1f1));  // 211.961f, 230.883f, 201.605f, 230.883f
path.quadTo(SkBits2Float(0x433f3ffe), SkBits2Float(0x4366e1f1), SkBits2Float(0x4337ed7a), SkBits2Float(0x435f8f6d));  // 191.25f, 230.883f, 183.928f, 223.56f
path.quadTo(SkBits2Float(0x43309af6), SkBits2Float(0x43583ce8), SkBits2Float(0x43309af6), SkBits2Float(0x434de1f1));  // 176.605f, 216.238f, 176.605f, 205.883f
path.quadTo(SkBits2Float(0x43309af6), SkBits2Float(0x434386fa), SkBits2Float(0x4337ed7a), SkBits2Float(0x433c3475));  // 176.605f, 195.527f, 183.928f, 188.205f
path.quadTo(SkBits2Float(0x433f3ffe), SkBits2Float(0x4334e1f1), SkBits2Float(0x43499af6), SkBits2Float(0x4334e1f1));  // 191.25f, 180.883f, 201.605f, 180.883f
path.quadTo(SkBits2Float(0x4353f5ee), SkBits2Float(0x4334e1f1), SkBits2Float(0x435b4872), SkBits2Float(0x433c3475));  // 211.961f, 180.883f, 219.283f, 188.205f
path.quadTo(SkBits2Float(0x43629af6), SkBits2Float(0x434386fa), SkBits2Float(0x43629af6), SkBits2Float(0x434de1f1));  // 226.605f, 195.527f, 226.605f, 205.883f
path.close();
    SkPath path134(path);
    builder.add(path134, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43969933), SkBits2Float(0x00000000));  // 301.197f, 0
path.quadTo(SkBits2Float(0x43969933), SkBits2Float(0x4125af78), SkBits2Float(0x4392eff1), SkBits2Float(0x418d6bde));  // 301.197f, 10.3553f, 293.875f, 17.6777f
path.quadTo(SkBits2Float(0x438f46af), SkBits2Float(0x41c80000), SkBits2Float(0x438a1933), SkBits2Float(0x41c80000));  // 286.552f, 25, 276.197f, 25
path.quadTo(SkBits2Float(0x4384ebb7), SkBits2Float(0x41c80000), SkBits2Float(0x43814275), SkBits2Float(0x418d6bde));  // 265.842f, 25, 258.519f, 17.6777f
path.quadTo(SkBits2Float(0x437b3266), SkBits2Float(0x4125af78), SkBits2Float(0x437b3266), SkBits2Float(0x00000000));  // 251.197f, 10.3553f, 251.197f, 0
path.quadTo(SkBits2Float(0x437b3266), SkBits2Float(0xc125af78), SkBits2Float(0x43814275), SkBits2Float(0xc18d6bde));  // 251.197f, -10.3553f, 258.519f, -17.6777f
path.quadTo(SkBits2Float(0x4384ebb7), SkBits2Float(0xc1c80000), SkBits2Float(0x438a1933), SkBits2Float(0xc1c80000));  // 265.842f, -25, 276.197f, -25
path.quadTo(SkBits2Float(0x438f46af), SkBits2Float(0xc1c80000), SkBits2Float(0x4392eff1), SkBits2Float(0xc18d6bde));  // 286.552f, -25, 293.875f, -17.6777f
path.quadTo(SkBits2Float(0x43969933), SkBits2Float(0xc125af78), SkBits2Float(0x43969933), SkBits2Float(0x00000000));  // 301.197f, -10.3553f, 301.197f, 0
path.close();
    SkPath path135(path);
    builder.add(path135, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43310d09), SkBits2Float(0x43ab2014));  // 177.051f, 342.251f
path.quadTo(SkBits2Float(0x43310d09), SkBits2Float(0x43b04d90), SkBits2Float(0x4329ba85), SkBits2Float(0x43b3f6d2));  // 177.051f, 352.606f, 169.729f, 359.928f
path.quadTo(SkBits2Float(0x43226800), SkBits2Float(0x43b7a014), SkBits2Float(0x43180d09), SkBits2Float(0x43b7a014));  // 162.406f, 367.251f, 152.051f, 367.251f
path.quadTo(SkBits2Float(0x430db212), SkBits2Float(0x43b7a014), SkBits2Float(0x43065f8d), SkBits2Float(0x43b3f6d2));  // 141.696f, 367.251f, 134.373f, 359.928f
path.quadTo(SkBits2Float(0x42fe1a12), SkBits2Float(0x43b04d90), SkBits2Float(0x42fe1a12), SkBits2Float(0x43ab2014));  // 127.051f, 352.606f, 127.051f, 342.251f
path.quadTo(SkBits2Float(0x42fe1a12), SkBits2Float(0x43a5f298), SkBits2Float(0x43065f8d), SkBits2Float(0x43a24956));  // 127.051f, 331.895f, 134.373f, 324.573f
path.quadTo(SkBits2Float(0x430db212), SkBits2Float(0x439ea014), SkBits2Float(0x43180d09), SkBits2Float(0x439ea014));  // 141.696f, 317.251f, 152.051f, 317.251f
path.quadTo(SkBits2Float(0x43226800), SkBits2Float(0x439ea014), SkBits2Float(0x4329ba85), SkBits2Float(0x43a24956));  // 162.406f, 317.251f, 169.729f, 324.573f
path.quadTo(SkBits2Float(0x43310d09), SkBits2Float(0x43a5f298), SkBits2Float(0x43310d09), SkBits2Float(0x43ab2014));  // 177.051f, 331.895f, 177.051f, 342.251f
path.close();
    SkPath path136(path);
    builder.add(path136, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43c19844), SkBits2Float(0x43818fc9));  // 387.19f, 259.123f
path.quadTo(SkBits2Float(0x43c19844), SkBits2Float(0x4386bd45), SkBits2Float(0x43bdef02), SkBits2Float(0x438a6687));  // 387.19f, 269.479f, 379.867f, 276.801f
path.quadTo(SkBits2Float(0x43ba45c0), SkBits2Float(0x438e0fc9), SkBits2Float(0x43b51844), SkBits2Float(0x438e0fc9));  // 372.545f, 284.123f, 362.19f, 284.123f
path.quadTo(SkBits2Float(0x43afeac8), SkBits2Float(0x438e0fc9), SkBits2Float(0x43ac4186), SkBits2Float(0x438a6687));  // 351.834f, 284.123f, 344.512f, 276.801f
path.quadTo(SkBits2Float(0x43a89844), SkBits2Float(0x4386bd45), SkBits2Float(0x43a89844), SkBits2Float(0x43818fc9));  // 337.19f, 269.479f, 337.19f, 259.123f
path.quadTo(SkBits2Float(0x43a89844), SkBits2Float(0x4378c49a), SkBits2Float(0x43ac4186), SkBits2Float(0x43717216));  // 337.19f, 248.768f, 344.512f, 241.446f
path.quadTo(SkBits2Float(0x43afeac8), SkBits2Float(0x436a1f92), SkBits2Float(0x43b51844), SkBits2Float(0x436a1f92));  // 351.834f, 234.123f, 362.19f, 234.123f
path.quadTo(SkBits2Float(0x43ba45c0), SkBits2Float(0x436a1f92), SkBits2Float(0x43bdef02), SkBits2Float(0x43717216));  // 372.545f, 234.123f, 379.867f, 241.446f
path.quadTo(SkBits2Float(0x43c19844), SkBits2Float(0x4378c49a), SkBits2Float(0x43c19844), SkBits2Float(0x43818fc9));  // 387.19f, 248.768f, 387.19f, 259.123f
path.close();
    SkPath path137(path);
    builder.add(path137, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43565ef5), SkBits2Float(0x42de6e20));  // 214.371f, 111.215f
path.quadTo(SkBits2Float(0x43565ef5), SkBits2Float(0x42f3240f), SkBits2Float(0x434f0c71), SkBits2Float(0x4300e48c));  // 214.371f, 121.57f, 207.049f, 128.893f
path.quadTo(SkBits2Float(0x4347b9ec), SkBits2Float(0x43083710), SkBits2Float(0x433d5ef5), SkBits2Float(0x43083710));  // 199.726f, 136.215f, 189.371f, 136.215f
path.quadTo(SkBits2Float(0x433303fe), SkBits2Float(0x43083710), SkBits2Float(0x432bb179), SkBits2Float(0x4300e48c));  // 179.016f, 136.215f, 171.693f, 128.893f
path.quadTo(SkBits2Float(0x43245ef5), SkBits2Float(0x42f3240f), SkBits2Float(0x43245ef5), SkBits2Float(0x42de6e20));  // 164.371f, 121.57f, 164.371f, 111.215f
path.quadTo(SkBits2Float(0x43245ef5), SkBits2Float(0x42c9b831), SkBits2Float(0x432bb179), SkBits2Float(0x42bb1329));  // 164.371f, 100.86f, 171.693f, 93.5374f
path.quadTo(SkBits2Float(0x433303fe), SkBits2Float(0x42ac6e21), SkBits2Float(0x433d5ef5), SkBits2Float(0x42ac6e21));  // 179.016f, 86.2151f, 189.371f, 86.2151f
path.quadTo(SkBits2Float(0x4347b9ec), SkBits2Float(0x42ac6e21), SkBits2Float(0x434f0c71), SkBits2Float(0x42bb1329));  // 199.726f, 86.2151f, 207.049f, 93.5374f
path.quadTo(SkBits2Float(0x43565ef5), SkBits2Float(0x42c9b831), SkBits2Float(0x43565ef5), SkBits2Float(0x42de6e20));  // 214.371f, 100.86f, 214.371f, 111.215f
path.close();
    SkPath path138(path);
    builder.add(path138, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43e8a73d), SkBits2Float(0x439f6574));  // 465.307f, 318.793f
path.quadTo(SkBits2Float(0x43e8a73d), SkBits2Float(0x43a492f0), SkBits2Float(0x43e4fdfb), SkBits2Float(0x43a83c32));  // 465.307f, 329.148f, 457.984f, 336.47f
path.quadTo(SkBits2Float(0x43e154b9), SkBits2Float(0x43abe574), SkBits2Float(0x43dc273d), SkBits2Float(0x43abe574));  // 450.662f, 343.793f, 440.307f, 343.793f
path.quadTo(SkBits2Float(0x43d6f9c1), SkBits2Float(0x43abe574), SkBits2Float(0x43d3507f), SkBits2Float(0x43a83c32));  // 429.951f, 343.793f, 422.629f, 336.47f
path.quadTo(SkBits2Float(0x43cfa73d), SkBits2Float(0x43a492f0), SkBits2Float(0x43cfa73d), SkBits2Float(0x439f6574));  // 415.307f, 329.148f, 415.307f, 318.793f
path.quadTo(SkBits2Float(0x43cfa73d), SkBits2Float(0x439a37f8), SkBits2Float(0x43d3507f), SkBits2Float(0x43968eb6));  // 415.307f, 308.437f, 422.629f, 301.115f
path.quadTo(SkBits2Float(0x43d6f9c1), SkBits2Float(0x4392e574), SkBits2Float(0x43dc273d), SkBits2Float(0x4392e574));  // 429.951f, 293.793f, 440.307f, 293.793f
path.quadTo(SkBits2Float(0x43e154b9), SkBits2Float(0x4392e574), SkBits2Float(0x43e4fdfb), SkBits2Float(0x43968eb6));  // 450.662f, 293.793f, 457.984f, 301.115f
path.quadTo(SkBits2Float(0x43e8a73d), SkBits2Float(0x439a37f8), SkBits2Float(0x43e8a73d), SkBits2Float(0x439f6574));  // 465.307f, 308.437f, 465.307f, 318.793f
path.close();
    SkPath path139(path);
    builder.add(path139, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43abcc05), SkBits2Float(0x42d3af00));  // 343.594f, 105.842f
path.quadTo(SkBits2Float(0x43abcc05), SkBits2Float(0x42e864ef), SkBits2Float(0x43a822c3), SkBits2Float(0x42f709f8));  // 343.594f, 116.197f, 336.272f, 123.519f
path.quadTo(SkBits2Float(0x43a47981), SkBits2Float(0x4302d780), SkBits2Float(0x439f4c05), SkBits2Float(0x4302d780));  // 328.949f, 130.842f, 318.594f, 130.842f
path.quadTo(SkBits2Float(0x439a1e89), SkBits2Float(0x4302d780), SkBits2Float(0x43967547), SkBits2Float(0x42f709f8));  // 308.239f, 130.842f, 300.916f, 123.519f
path.quadTo(SkBits2Float(0x4392cc05), SkBits2Float(0x42e864ef), SkBits2Float(0x4392cc05), SkBits2Float(0x42d3af00));  // 293.594f, 116.197f, 293.594f, 105.842f
path.quadTo(SkBits2Float(0x4392cc05), SkBits2Float(0x42bef911), SkBits2Float(0x43967547), SkBits2Float(0x42b05408));  // 293.594f, 95.4865f, 300.916f, 88.1641f
path.quadTo(SkBits2Float(0x439a1e89), SkBits2Float(0x42a1aeff), SkBits2Float(0x439f4c05), SkBits2Float(0x42a1aeff));  // 308.239f, 80.8418f, 318.594f, 80.8418f
path.quadTo(SkBits2Float(0x43a47981), SkBits2Float(0x42a1aeff), SkBits2Float(0x43a822c3), SkBits2Float(0x42b05408));  // 328.949f, 80.8418f, 336.272f, 88.1641f
path.quadTo(SkBits2Float(0x43abcc05), SkBits2Float(0x42bef911), SkBits2Float(0x43abcc05), SkBits2Float(0x42d3af00));  // 343.594f, 95.4865f, 343.594f, 105.842f
path.close();
    SkPath path140(path);
    builder.add(path140, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x438e8ffa), SkBits2Float(0x438bdd2b));  // 285.125f, 279.728f
path.quadTo(SkBits2Float(0x438e8ffa), SkBits2Float(0x43910aa7), SkBits2Float(0x438ae6b8), SkBits2Float(0x4394b3e9));  // 285.125f, 290.083f, 277.802f, 297.406f
path.quadTo(SkBits2Float(0x43873d76), SkBits2Float(0x43985d2b), SkBits2Float(0x43820ffa), SkBits2Float(0x43985d2b));  // 270.48f, 304.728f, 260.125f, 304.728f
path.quadTo(SkBits2Float(0x4379c4fc), SkBits2Float(0x43985d2b), SkBits2Float(0x43727278), SkBits2Float(0x4394b3e9));  // 249.769f, 304.728f, 242.447f, 297.406f
path.quadTo(SkBits2Float(0x436b1ff4), SkBits2Float(0x43910aa7), SkBits2Float(0x436b1ff4), SkBits2Float(0x438bdd2b));  // 235.125f, 290.083f, 235.125f, 279.728f
path.quadTo(SkBits2Float(0x436b1ff4), SkBits2Float(0x4386afaf), SkBits2Float(0x43727278), SkBits2Float(0x4383066d));  // 235.125f, 269.373f, 242.447f, 262.05f
path.quadTo(SkBits2Float(0x4379c4fc), SkBits2Float(0x437eba56), SkBits2Float(0x43820ffa), SkBits2Float(0x437eba56));  // 249.769f, 254.728f, 260.125f, 254.728f
path.quadTo(SkBits2Float(0x43873d76), SkBits2Float(0x437eba56), SkBits2Float(0x438ae6b8), SkBits2Float(0x4383066d));  // 270.48f, 254.728f, 277.802f, 262.05f
path.quadTo(SkBits2Float(0x438e8ffa), SkBits2Float(0x4386afaf), SkBits2Float(0x438e8ffa), SkBits2Float(0x438bdd2b));  // 285.125f, 269.373f, 285.125f, 279.728f
path.close();
    SkPath path141(path);
    builder.add(path141, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x441d8000), SkBits2Float(0x435e8072));  // 630, 222.502f
path.quadTo(SkBits2Float(0x441d8000), SkBits2Float(0x4368db6a), SkBits2Float(0x441bab5f), SkBits2Float(0x43702dee));  // 630, 232.857f, 622.678f, 240.179f
path.quadTo(SkBits2Float(0x4419d6be), SkBits2Float(0x43778072), SkBits2Float(0x44174000), SkBits2Float(0x43778072));  // 615.355f, 247.502f, 605, 247.502f
path.quadTo(SkBits2Float(0x4414a942), SkBits2Float(0x43778072), SkBits2Float(0x4412d4a1), SkBits2Float(0x43702dee));  // 594.645f, 247.502f, 587.322f, 240.179f
path.quadTo(SkBits2Float(0x44110000), SkBits2Float(0x4368db6a), SkBits2Float(0x44110000), SkBits2Float(0x435e8072));  // 580, 232.857f, 580, 222.502f
path.quadTo(SkBits2Float(0x44110000), SkBits2Float(0x4354257a), SkBits2Float(0x4412d4a1), SkBits2Float(0x434cd2f6));  // 580, 212.146f, 587.322f, 204.824f
path.quadTo(SkBits2Float(0x4414a942), SkBits2Float(0x43458072), SkBits2Float(0x44174000), SkBits2Float(0x43458072));  // 594.645f, 197.502f, 605, 197.502f
path.quadTo(SkBits2Float(0x4419d6be), SkBits2Float(0x43458072), SkBits2Float(0x441bab5f), SkBits2Float(0x434cd2f6));  // 615.355f, 197.502f, 622.678f, 204.824f
path.quadTo(SkBits2Float(0x441d8000), SkBits2Float(0x4354257a), SkBits2Float(0x441d8000), SkBits2Float(0x435e8072));  // 630, 212.146f, 630, 222.502f
path.close();
    SkPath path142(path);
    builder.add(path142, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43cab007), SkBits2Float(0x4370ffc2));  // 405.375f, 240.999f
path.quadTo(SkBits2Float(0x43cab007), SkBits2Float(0x437b5aba), SkBits2Float(0x43c706c5), SkBits2Float(0x4381569f));  // 405.375f, 251.354f, 398.053f, 258.677f
path.quadTo(SkBits2Float(0x43c35d83), SkBits2Float(0x4384ffe1), SkBits2Float(0x43be3007), SkBits2Float(0x4384ffe1));  // 390.731f, 265.999f, 380.375f, 265.999f
path.quadTo(SkBits2Float(0x43b9028b), SkBits2Float(0x4384ffe1), SkBits2Float(0x43b55949), SkBits2Float(0x4381569f));  // 370.02f, 265.999f, 362.698f, 258.677f
path.quadTo(SkBits2Float(0x43b1b007), SkBits2Float(0x437b5aba), SkBits2Float(0x43b1b007), SkBits2Float(0x4370ffc2));  // 355.375f, 251.354f, 355.375f, 240.999f
path.quadTo(SkBits2Float(0x43b1b007), SkBits2Float(0x4366a4ca), SkBits2Float(0x43b55949), SkBits2Float(0x435f5246));  // 355.375f, 230.644f, 362.698f, 223.321f
path.quadTo(SkBits2Float(0x43b9028b), SkBits2Float(0x4357ffc2), SkBits2Float(0x43be3007), SkBits2Float(0x4357ffc2));  // 370.02f, 215.999f, 380.375f, 215.999f
path.quadTo(SkBits2Float(0x43c35d83), SkBits2Float(0x4357ffc2), SkBits2Float(0x43c706c5), SkBits2Float(0x435f5246));  // 390.731f, 215.999f, 398.053f, 223.321f
path.quadTo(SkBits2Float(0x43cab007), SkBits2Float(0x4366a4ca), SkBits2Float(0x43cab007), SkBits2Float(0x4370ffc2));  // 405.375f, 230.644f, 405.375f, 240.999f
path.close();
    SkPath path143(path);
    builder.add(path143, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ae2f7d), SkBits2Float(0x43587ea0));  // 348.371f, 216.495f
path.quadTo(SkBits2Float(0x43ae2f7d), SkBits2Float(0x4362d998), SkBits2Float(0x43aa863b), SkBits2Float(0x436a2c1c));  // 348.371f, 226.85f, 341.049f, 234.172f
path.quadTo(SkBits2Float(0x43a6dcf9), SkBits2Float(0x43717ea0), SkBits2Float(0x43a1af7d), SkBits2Float(0x43717ea0));  // 333.726f, 241.495f, 323.371f, 241.495f
path.quadTo(SkBits2Float(0x439c8201), SkBits2Float(0x43717ea0), SkBits2Float(0x4398d8bf), SkBits2Float(0x436a2c1c));  // 313.016f, 241.495f, 305.693f, 234.172f
path.quadTo(SkBits2Float(0x43952f7d), SkBits2Float(0x4362d998), SkBits2Float(0x43952f7d), SkBits2Float(0x43587ea0));  // 298.371f, 226.85f, 298.371f, 216.495f
path.quadTo(SkBits2Float(0x43952f7d), SkBits2Float(0x434e23a8), SkBits2Float(0x4398d8bf), SkBits2Float(0x4346d124));  // 298.371f, 206.139f, 305.693f, 198.817f
path.quadTo(SkBits2Float(0x439c8201), SkBits2Float(0x433f7ea0), SkBits2Float(0x43a1af7d), SkBits2Float(0x433f7ea0));  // 313.016f, 191.495f, 323.371f, 191.495f
path.quadTo(SkBits2Float(0x43a6dcf9), SkBits2Float(0x433f7ea0), SkBits2Float(0x43aa863b), SkBits2Float(0x4346d124));  // 333.726f, 191.495f, 341.049f, 198.817f
path.quadTo(SkBits2Float(0x43ae2f7d), SkBits2Float(0x434e23a8), SkBits2Float(0x43ae2f7d), SkBits2Float(0x43587ea0));  // 348.371f, 206.139f, 348.371f, 216.495f
path.close();
    SkPath path144(path);
    builder.add(path144, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43add1a9), SkBits2Float(0x436897ac));  // 347.638f, 232.592f
path.quadTo(SkBits2Float(0x43add1a9), SkBits2Float(0x4372f2a3), SkBits2Float(0x43aa2867), SkBits2Float(0x437a4527));  // 347.638f, 242.948f, 340.316f, 250.27f
path.quadTo(SkBits2Float(0x43a67f25), SkBits2Float(0x4380cbd6), SkBits2Float(0x43a151a9), SkBits2Float(0x4380cbd6));  // 332.993f, 257.592f, 322.638f, 257.592f
path.quadTo(SkBits2Float(0x439c242d), SkBits2Float(0x4380cbd6), SkBits2Float(0x43987aeb), SkBits2Float(0x437a4527));  // 312.283f, 257.592f, 304.96f, 250.27f
path.quadTo(SkBits2Float(0x4394d1a9), SkBits2Float(0x4372f2a3), SkBits2Float(0x4394d1a9), SkBits2Float(0x436897ac));  // 297.638f, 242.948f, 297.638f, 232.592f
path.quadTo(SkBits2Float(0x4394d1a9), SkBits2Float(0x435e3cb5), SkBits2Float(0x43987aeb), SkBits2Float(0x4356ea31));  // 297.638f, 222.237f, 304.96f, 214.915f
path.quadTo(SkBits2Float(0x439c242d), SkBits2Float(0x434f97ad), SkBits2Float(0x43a151a9), SkBits2Float(0x434f97ad));  // 312.283f, 207.592f, 322.638f, 207.592f
path.quadTo(SkBits2Float(0x43a67f25), SkBits2Float(0x434f97ad), SkBits2Float(0x43aa2867), SkBits2Float(0x4356ea31));  // 332.993f, 207.592f, 340.316f, 214.915f
path.quadTo(SkBits2Float(0x43add1a9), SkBits2Float(0x435e3cb5), SkBits2Float(0x43add1a9), SkBits2Float(0x436897ac));  // 347.638f, 222.237f, 347.638f, 232.592f
path.close();
    SkPath path145(path);
    builder.add(path145, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43f37c5c), SkBits2Float(0x438cffd4));  // 486.972f, 281.999f
path.quadTo(SkBits2Float(0x43f37c5c), SkBits2Float(0x43922d50), SkBits2Float(0x43efd31a), SkBits2Float(0x4395d692));  // 486.972f, 292.354f, 479.649f, 299.676f
path.quadTo(SkBits2Float(0x43ec29d8), SkBits2Float(0x43997fd4), SkBits2Float(0x43e6fc5c), SkBits2Float(0x43997fd4));  // 472.327f, 306.999f, 461.972f, 306.999f
path.quadTo(SkBits2Float(0x43e1cee0), SkBits2Float(0x43997fd4), SkBits2Float(0x43de259e), SkBits2Float(0x4395d692));  // 451.616f, 306.999f, 444.294f, 299.676f
path.quadTo(SkBits2Float(0x43da7c5c), SkBits2Float(0x43922d50), SkBits2Float(0x43da7c5c), SkBits2Float(0x438cffd4));  // 436.972f, 292.354f, 436.972f, 281.999f
path.quadTo(SkBits2Float(0x43da7c5c), SkBits2Float(0x4387d258), SkBits2Float(0x43de259e), SkBits2Float(0x43842916));  // 436.972f, 271.643f, 444.294f, 264.321f
path.quadTo(SkBits2Float(0x43e1cee0), SkBits2Float(0x43807fd4), SkBits2Float(0x43e6fc5c), SkBits2Float(0x43807fd4));  // 451.616f, 256.999f, 461.972f, 256.999f
path.quadTo(SkBits2Float(0x43ec29d8), SkBits2Float(0x43807fd4), SkBits2Float(0x43efd31a), SkBits2Float(0x43842916));  // 472.327f, 256.999f, 479.649f, 264.321f
path.quadTo(SkBits2Float(0x43f37c5c), SkBits2Float(0x4387d258), SkBits2Float(0x43f37c5c), SkBits2Float(0x438cffd4));  // 486.972f, 271.643f, 486.972f, 281.999f
path.close();
    SkPath path146(path);
    builder.add(path146, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x434938a6), SkBits2Float(0x439b6682));  // 201.221f, 310.801f
path.quadTo(SkBits2Float(0x434938a6), SkBits2Float(0x43a093fe), SkBits2Float(0x4341e622), SkBits2Float(0x43a43d40));  // 201.221f, 321.156f, 193.899f, 328.479f
path.quadTo(SkBits2Float(0x433a939e), SkBits2Float(0x43a7e682), SkBits2Float(0x433038a6), SkBits2Float(0x43a7e682));  // 186.577f, 335.801f, 176.221f, 335.801f
path.quadTo(SkBits2Float(0x4325ddae), SkBits2Float(0x43a7e682), SkBits2Float(0x431e8b2a), SkBits2Float(0x43a43d40));  // 165.866f, 335.801f, 158.544f, 328.479f
path.quadTo(SkBits2Float(0x431738a6), SkBits2Float(0x43a093fe), SkBits2Float(0x431738a6), SkBits2Float(0x439b6682));  // 151.221f, 321.156f, 151.221f, 310.801f
path.quadTo(SkBits2Float(0x431738a6), SkBits2Float(0x43963906), SkBits2Float(0x431e8b2a), SkBits2Float(0x43928fc4));  // 151.221f, 300.445f, 158.544f, 293.123f
path.quadTo(SkBits2Float(0x4325ddae), SkBits2Float(0x438ee682), SkBits2Float(0x433038a6), SkBits2Float(0x438ee682));  // 165.866f, 285.801f, 176.221f, 285.801f
path.quadTo(SkBits2Float(0x433a939e), SkBits2Float(0x438ee682), SkBits2Float(0x4341e622), SkBits2Float(0x43928fc4));  // 186.577f, 285.801f, 193.899f, 293.123f
path.quadTo(SkBits2Float(0x434938a6), SkBits2Float(0x43963906), SkBits2Float(0x434938a6), SkBits2Float(0x439b6682));  // 201.221f, 300.445f, 201.221f, 310.801f
path.close();
    SkPath path147(path);
    builder.add(path147, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43957ad9), SkBits2Float(0x4340d6a9));  // 298.96f, 192.839f
path.quadTo(SkBits2Float(0x43957ad9), SkBits2Float(0x434b31a0), SkBits2Float(0x4391d197), SkBits2Float(0x43528425));  // 298.96f, 203.194f, 291.637f, 210.516f
path.quadTo(SkBits2Float(0x438e2855), SkBits2Float(0x4359d6a9), SkBits2Float(0x4388fad9), SkBits2Float(0x4359d6a9));  // 284.315f, 217.839f, 273.96f, 217.839f
path.quadTo(SkBits2Float(0x4383cd5d), SkBits2Float(0x4359d6a9), SkBits2Float(0x4380241b), SkBits2Float(0x43528425));  // 263.604f, 217.839f, 256.282f, 210.516f
path.quadTo(SkBits2Float(0x4378f5b2), SkBits2Float(0x434b31a0), SkBits2Float(0x4378f5b2), SkBits2Float(0x4340d6a9));  // 248.96f, 203.194f, 248.96f, 192.839f
path.quadTo(SkBits2Float(0x4378f5b2), SkBits2Float(0x43367bb2), SkBits2Float(0x4380241b), SkBits2Float(0x432f292d));  // 248.96f, 182.483f, 256.282f, 175.161f
path.quadTo(SkBits2Float(0x4383cd5d), SkBits2Float(0x4327d6a9), SkBits2Float(0x4388fad9), SkBits2Float(0x4327d6a9));  // 263.604f, 167.839f, 273.96f, 167.839f
path.quadTo(SkBits2Float(0x438e2855), SkBits2Float(0x4327d6a9), SkBits2Float(0x4391d197), SkBits2Float(0x432f292d));  // 284.315f, 167.839f, 291.637f, 175.161f
path.quadTo(SkBits2Float(0x43957ad9), SkBits2Float(0x43367bb2), SkBits2Float(0x43957ad9), SkBits2Float(0x4340d6a9));  // 298.96f, 182.483f, 298.96f, 192.839f
path.close();
    SkPath path148(path);
    builder.add(path148, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4359cdc3), SkBits2Float(0x431623e6));  // 217.804f, 150.14f
path.quadTo(SkBits2Float(0x4359cdc3), SkBits2Float(0x43207ede), SkBits2Float(0x43527b3f), SkBits2Float(0x4327d162));  // 217.804f, 160.496f, 210.481f, 167.818f
path.quadTo(SkBits2Float(0x434b28ba), SkBits2Float(0x432f23e6), SkBits2Float(0x4340cdc3), SkBits2Float(0x432f23e6));  // 203.159f, 175.14f, 192.804f, 175.14f
path.quadTo(SkBits2Float(0x433672cc), SkBits2Float(0x432f23e6), SkBits2Float(0x432f2047), SkBits2Float(0x4327d162));  // 182.448f, 175.14f, 175.126f, 167.818f
path.quadTo(SkBits2Float(0x4327cdc3), SkBits2Float(0x43207ede), SkBits2Float(0x4327cdc3), SkBits2Float(0x431623e6));  // 167.804f, 160.496f, 167.804f, 150.14f
path.quadTo(SkBits2Float(0x4327cdc3), SkBits2Float(0x430bc8ee), SkBits2Float(0x432f2047), SkBits2Float(0x4304766a));  // 167.804f, 139.785f, 175.126f, 132.463f
path.quadTo(SkBits2Float(0x433672cc), SkBits2Float(0x42fa47cc), SkBits2Float(0x4340cdc3), SkBits2Float(0x42fa47cc));  // 182.448f, 125.14f, 192.804f, 125.14f
path.quadTo(SkBits2Float(0x434b28ba), SkBits2Float(0x42fa47cc), SkBits2Float(0x43527b3f), SkBits2Float(0x4304766a));  // 203.159f, 125.14f, 210.481f, 132.463f
path.quadTo(SkBits2Float(0x4359cdc3), SkBits2Float(0x430bc8ee), SkBits2Float(0x4359cdc3), SkBits2Float(0x431623e6));  // 217.804f, 139.785f, 217.804f, 150.14f
path.close();
    SkPath path149(path);
    builder.add(path149, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4395d72b), SkBits2Float(0x43c5524a));  // 299.681f, 394.643f
path.quadTo(SkBits2Float(0x4395d72b), SkBits2Float(0x43ca7fc6), SkBits2Float(0x43922de9), SkBits2Float(0x43ce2908));  // 299.681f, 404.998f, 292.359f, 412.321f
path.quadTo(SkBits2Float(0x438e84a7), SkBits2Float(0x43d1d24a), SkBits2Float(0x4389572b), SkBits2Float(0x43d1d24a));  // 285.036f, 419.643f, 274.681f, 419.643f
path.quadTo(SkBits2Float(0x438429af), SkBits2Float(0x43d1d24a), SkBits2Float(0x4380806d), SkBits2Float(0x43ce2908));  // 264.326f, 419.643f, 257.003f, 412.321f
path.quadTo(SkBits2Float(0x4379ae56), SkBits2Float(0x43ca7fc6), SkBits2Float(0x4379ae56), SkBits2Float(0x43c5524a));  // 249.681f, 404.998f, 249.681f, 394.643f
path.quadTo(SkBits2Float(0x4379ae56), SkBits2Float(0x43c024ce), SkBits2Float(0x4380806d), SkBits2Float(0x43bc7b8c));  // 249.681f, 384.288f, 257.003f, 376.965f
path.quadTo(SkBits2Float(0x438429af), SkBits2Float(0x43b8d24a), SkBits2Float(0x4389572b), SkBits2Float(0x43b8d24a));  // 264.326f, 369.643f, 274.681f, 369.643f
path.quadTo(SkBits2Float(0x438e84a7), SkBits2Float(0x43b8d24a), SkBits2Float(0x43922de9), SkBits2Float(0x43bc7b8c));  // 285.036f, 369.643f, 292.359f, 376.965f
path.quadTo(SkBits2Float(0x4395d72b), SkBits2Float(0x43c024ce), SkBits2Float(0x4395d72b), SkBits2Float(0x43c5524a));  // 299.681f, 384.288f, 299.681f, 394.643f
path.close();
    SkPath path150(path);
    builder.add(path150, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43884f5e), SkBits2Float(0x436c6388));  // 272.62f, 236.389f
path.quadTo(SkBits2Float(0x43884f5e), SkBits2Float(0x4376be7f), SkBits2Float(0x4384a61c), SkBits2Float(0x437e1103));  // 272.62f, 246.744f, 265.298f, 254.066f
path.quadTo(SkBits2Float(0x4380fcda), SkBits2Float(0x4382b1c4), SkBits2Float(0x43779ebc), SkBits2Float(0x4382b1c4));  // 257.975f, 261.389f, 247.62f, 261.389f
path.quadTo(SkBits2Float(0x436d43c5), SkBits2Float(0x4382b1c4), SkBits2Float(0x4365f141), SkBits2Float(0x437e1103));  // 237.265f, 261.389f, 229.942f, 254.066f
path.quadTo(SkBits2Float(0x435e9ebd), SkBits2Float(0x4376be7f), SkBits2Float(0x435e9ebd), SkBits2Float(0x436c6388));  // 222.62f, 246.744f, 222.62f, 236.389f
path.quadTo(SkBits2Float(0x435e9ebd), SkBits2Float(0x43620891), SkBits2Float(0x4365f141), SkBits2Float(0x435ab60d));  // 222.62f, 226.033f, 229.942f, 218.711f
path.quadTo(SkBits2Float(0x436d43c5), SkBits2Float(0x43536389), SkBits2Float(0x43779ebc), SkBits2Float(0x43536389));  // 237.265f, 211.389f, 247.62f, 211.389f
path.quadTo(SkBits2Float(0x4380fcda), SkBits2Float(0x43536389), SkBits2Float(0x4384a61c), SkBits2Float(0x435ab60d));  // 257.975f, 211.389f, 265.298f, 218.711f
path.quadTo(SkBits2Float(0x43884f5e), SkBits2Float(0x43620891), SkBits2Float(0x43884f5e), SkBits2Float(0x436c6388));  // 272.62f, 226.033f, 272.62f, 236.389f
path.close();
    SkPath path151(path);
    builder.add(path151, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ac3331), SkBits2Float(0x4330c484));  // 344.4f, 176.768f
path.quadTo(SkBits2Float(0x43ac3331), SkBits2Float(0x433b1f7c), SkBits2Float(0x43a889ef), SkBits2Float(0x43427200));  // 344.4f, 187.123f, 337.078f, 194.445f
path.quadTo(SkBits2Float(0x43a4e0ad), SkBits2Float(0x4349c484), SkBits2Float(0x439fb331), SkBits2Float(0x4349c484));  // 329.755f, 201.768f, 319.4f, 201.768f
path.quadTo(SkBits2Float(0x439a85b5), SkBits2Float(0x4349c484), SkBits2Float(0x4396dc73), SkBits2Float(0x43427200));  // 309.045f, 201.768f, 301.722f, 194.445f
path.quadTo(SkBits2Float(0x43933331), SkBits2Float(0x433b1f7c), SkBits2Float(0x43933331), SkBits2Float(0x4330c484));  // 294.4f, 187.123f, 294.4f, 176.768f
path.quadTo(SkBits2Float(0x43933331), SkBits2Float(0x4326698c), SkBits2Float(0x4396dc73), SkBits2Float(0x431f1708));  // 294.4f, 166.412f, 301.722f, 159.09f
path.quadTo(SkBits2Float(0x439a85b5), SkBits2Float(0x4317c484), SkBits2Float(0x439fb331), SkBits2Float(0x4317c484));  // 309.045f, 151.768f, 319.4f, 151.768f
path.quadTo(SkBits2Float(0x43a4e0ad), SkBits2Float(0x4317c484), SkBits2Float(0x43a889ef), SkBits2Float(0x431f1708));  // 329.755f, 151.768f, 337.078f, 159.09f
path.quadTo(SkBits2Float(0x43ac3331), SkBits2Float(0x4326698c), SkBits2Float(0x43ac3331), SkBits2Float(0x4330c484));  // 344.4f, 166.412f, 344.4f, 176.768f
path.close();
    SkPath path152(path);
    builder.add(path152, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x435659f0), SkBits2Float(0x438a1820));  // 214.351f, 276.188f
path.quadTo(SkBits2Float(0x435659f0), SkBits2Float(0x438f459c), SkBits2Float(0x434f076c), SkBits2Float(0x4392eede));  // 214.351f, 286.544f, 207.029f, 293.866f
path.quadTo(SkBits2Float(0x4347b4e8), SkBits2Float(0x43969820), SkBits2Float(0x433d59f0), SkBits2Float(0x43969820));  // 199.707f, 301.188f, 189.351f, 301.188f
path.quadTo(SkBits2Float(0x4332fef8), SkBits2Float(0x43969820), SkBits2Float(0x432bac74), SkBits2Float(0x4392eede));  // 178.996f, 301.188f, 171.674f, 293.866f
path.quadTo(SkBits2Float(0x432459f0), SkBits2Float(0x438f459c), SkBits2Float(0x432459f0), SkBits2Float(0x438a1820));  // 164.351f, 286.544f, 164.351f, 276.188f
path.quadTo(SkBits2Float(0x432459f0), SkBits2Float(0x4384eaa4), SkBits2Float(0x432bac74), SkBits2Float(0x43814162));  // 164.351f, 265.833f, 171.674f, 258.511f
path.quadTo(SkBits2Float(0x4332fef8), SkBits2Float(0x437b3040), SkBits2Float(0x433d59f0), SkBits2Float(0x437b3040));  // 178.996f, 251.188f, 189.351f, 251.188f
path.quadTo(SkBits2Float(0x4347b4e8), SkBits2Float(0x437b3040), SkBits2Float(0x434f076c), SkBits2Float(0x43814162));  // 199.707f, 251.188f, 207.029f, 258.511f
path.quadTo(SkBits2Float(0x435659f0), SkBits2Float(0x4384eaa4), SkBits2Float(0x435659f0), SkBits2Float(0x438a1820));  // 214.351f, 265.833f, 214.351f, 276.188f
path.close();
    SkPath path153(path);
    builder.add(path153, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x437aa20e), SkBits2Float(0x434e714e));  // 250.633f, 206.443f
path.quadTo(SkBits2Float(0x437aa20e), SkBits2Float(0x4358cc46), SkBits2Float(0x43734f8a), SkBits2Float(0x43601eca));  // 250.633f, 216.798f, 243.311f, 224.12f
path.quadTo(SkBits2Float(0x436bfd06), SkBits2Float(0x4367714e), SkBits2Float(0x4361a20e), SkBits2Float(0x4367714e));  // 235.988f, 231.443f, 225.633f, 231.443f
path.quadTo(SkBits2Float(0x43574716), SkBits2Float(0x4367714e), SkBits2Float(0x434ff492), SkBits2Float(0x43601eca));  // 215.278f, 231.443f, 207.955f, 224.12f
path.quadTo(SkBits2Float(0x4348a20e), SkBits2Float(0x4358cc46), SkBits2Float(0x4348a20e), SkBits2Float(0x434e714e));  // 200.633f, 216.798f, 200.633f, 206.443f
path.quadTo(SkBits2Float(0x4348a20e), SkBits2Float(0x43441656), SkBits2Float(0x434ff492), SkBits2Float(0x433cc3d2));  // 200.633f, 196.087f, 207.955f, 188.765f
path.quadTo(SkBits2Float(0x43574716), SkBits2Float(0x4335714e), SkBits2Float(0x4361a20e), SkBits2Float(0x4335714e));  // 215.278f, 181.443f, 225.633f, 181.443f
path.quadTo(SkBits2Float(0x436bfd06), SkBits2Float(0x4335714e), SkBits2Float(0x43734f8a), SkBits2Float(0x433cc3d2));  // 235.988f, 181.443f, 243.311f, 188.765f
path.quadTo(SkBits2Float(0x437aa20e), SkBits2Float(0x43441656), SkBits2Float(0x437aa20e), SkBits2Float(0x434e714e));  // 250.633f, 196.087f, 250.633f, 206.443f
path.close();
    SkPath path154(path);
    builder.add(path154, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43ed22e3), SkBits2Float(0x439da644));  // 474.273f, 315.299f
path.quadTo(SkBits2Float(0x43ed22e3), SkBits2Float(0x43a2d3c0), SkBits2Float(0x43e979a1), SkBits2Float(0x43a67d02));  // 474.273f, 325.654f, 466.95f, 332.977f
path.quadTo(SkBits2Float(0x43e5d05f), SkBits2Float(0x43aa2644), SkBits2Float(0x43e0a2e3), SkBits2Float(0x43aa2644));  // 459.628f, 340.299f, 449.273f, 340.299f
path.quadTo(SkBits2Float(0x43db7567), SkBits2Float(0x43aa2644), SkBits2Float(0x43d7cc25), SkBits2Float(0x43a67d02));  // 438.917f, 340.299f, 431.595f, 332.977f
path.quadTo(SkBits2Float(0x43d422e3), SkBits2Float(0x43a2d3c0), SkBits2Float(0x43d422e3), SkBits2Float(0x439da644));  // 424.273f, 325.654f, 424.273f, 315.299f
path.quadTo(SkBits2Float(0x43d422e3), SkBits2Float(0x439878c8), SkBits2Float(0x43d7cc25), SkBits2Float(0x4394cf86));  // 424.273f, 304.944f, 431.595f, 297.621f
path.quadTo(SkBits2Float(0x43db7567), SkBits2Float(0x43912644), SkBits2Float(0x43e0a2e3), SkBits2Float(0x43912644));  // 438.917f, 290.299f, 449.273f, 290.299f
path.quadTo(SkBits2Float(0x43e5d05f), SkBits2Float(0x43912644), SkBits2Float(0x43e979a1), SkBits2Float(0x4394cf86));  // 459.628f, 290.299f, 466.95f, 297.621f
path.quadTo(SkBits2Float(0x43ed22e3), SkBits2Float(0x439878c8), SkBits2Float(0x43ed22e3), SkBits2Float(0x439da644));  // 474.273f, 304.944f, 474.273f, 315.299f
path.close();
    SkPath path155(path);
    builder.add(path155, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43cc4428), SkBits2Float(0x43797560));  // 408.532f, 249.458f
path.quadTo(SkBits2Float(0x43cc4428), SkBits2Float(0x4381e82c), SkBits2Float(0x43c89ae6), SkBits2Float(0x4385916e));  // 408.532f, 259.814f, 401.21f, 267.136f
path.quadTo(SkBits2Float(0x43c4f1a4), SkBits2Float(0x43893ab0), SkBits2Float(0x43bfc428), SkBits2Float(0x43893ab0));  // 393.888f, 274.458f, 383.532f, 274.458f
path.quadTo(SkBits2Float(0x43ba96ac), SkBits2Float(0x43893ab0), SkBits2Float(0x43b6ed6a), SkBits2Float(0x4385916e));  // 373.177f, 274.458f, 365.855f, 267.136f
path.quadTo(SkBits2Float(0x43b34428), SkBits2Float(0x4381e82c), SkBits2Float(0x43b34428), SkBits2Float(0x43797560));  // 358.532f, 259.814f, 358.532f, 249.458f
path.quadTo(SkBits2Float(0x43b34428), SkBits2Float(0x436f1a69), SkBits2Float(0x43b6ed6a), SkBits2Float(0x4367c7e5));  // 358.532f, 239.103f, 365.855f, 231.781f
path.quadTo(SkBits2Float(0x43ba96ac), SkBits2Float(0x43607561), SkBits2Float(0x43bfc428), SkBits2Float(0x43607561));  // 373.177f, 224.459f, 383.532f, 224.459f
path.quadTo(SkBits2Float(0x43c4f1a4), SkBits2Float(0x43607561), SkBits2Float(0x43c89ae6), SkBits2Float(0x4367c7e5));  // 393.888f, 224.459f, 401.21f, 231.781f
path.quadTo(SkBits2Float(0x43cc4428), SkBits2Float(0x436f1a69), SkBits2Float(0x43cc4428), SkBits2Float(0x43797560));  // 408.532f, 239.103f, 408.532f, 249.458f
path.close();
    SkPath path156(path);
    builder.add(path156, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42e47b81), SkBits2Float(0x43b97683));  // 114.241f, 370.926f
path.quadTo(SkBits2Float(0x42e47b81), SkBits2Float(0x43bea3ff), SkBits2Float(0x42d5d678), SkBits2Float(0x43c24d41));  // 114.241f, 381.281f, 106.919f, 388.604f
path.quadTo(SkBits2Float(0x42c73170), SkBits2Float(0x43c5f683), SkBits2Float(0x42b27b81), SkBits2Float(0x43c5f683));  // 99.5966f, 395.926f, 89.2412f, 395.926f
path.quadTo(SkBits2Float(0x429dc592), SkBits2Float(0x43c5f683), SkBits2Float(0x428f208a), SkBits2Float(0x43c24d41));  // 78.8859f, 395.926f, 71.5636f, 388.604f
path.quadTo(SkBits2Float(0x42807b81), SkBits2Float(0x43bea3ff), SkBits2Float(0x42807b81), SkBits2Float(0x43b97683));  // 64.2412f, 381.281f, 64.2412f, 370.926f
path.quadTo(SkBits2Float(0x42807b81), SkBits2Float(0x43b44907), SkBits2Float(0x428f208a), SkBits2Float(0x43b09fc5));  // 64.2412f, 360.571f, 71.5636f, 353.248f
path.quadTo(SkBits2Float(0x429dc592), SkBits2Float(0x43acf683), SkBits2Float(0x42b27b81), SkBits2Float(0x43acf683));  // 78.8859f, 345.926f, 89.2412f, 345.926f
path.quadTo(SkBits2Float(0x42c73170), SkBits2Float(0x43acf683), SkBits2Float(0x42d5d678), SkBits2Float(0x43b09fc5));  // 99.5966f, 345.926f, 106.919f, 353.248f
path.quadTo(SkBits2Float(0x42e47b81), SkBits2Float(0x43b44907), SkBits2Float(0x42e47b81), SkBits2Float(0x43b97683));  // 114.241f, 360.571f, 114.241f, 370.926f
path.close();
    SkPath path157(path);
    builder.add(path157, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4384239e), SkBits2Float(0x42a36f30));  // 264.278f, 81.7172f
path.quadTo(SkBits2Float(0x4384239e), SkBits2Float(0x42b8251f), SkBits2Float(0x43807a5c), SkBits2Float(0x42c6ca28));  // 264.278f, 92.0725f, 256.956f, 99.3948f
path.quadTo(SkBits2Float(0x4379a234), SkBits2Float(0x42d56f30), SkBits2Float(0x436f473c), SkBits2Float(0x42d56f30));  // 249.634f, 106.717f, 239.278f, 106.717f
path.quadTo(SkBits2Float(0x4364ec44), SkBits2Float(0x42d56f30), SkBits2Float(0x435d99c0), SkBits2Float(0x42c6ca28));  // 228.923f, 106.717f, 221.601f, 99.3948f
path.quadTo(SkBits2Float(0x4356473c), SkBits2Float(0x42b8251f), SkBits2Float(0x4356473c), SkBits2Float(0x42a36f30));  // 214.278f, 92.0725f, 214.278f, 81.7172f
path.quadTo(SkBits2Float(0x4356473c), SkBits2Float(0x428eb941), SkBits2Float(0x435d99c0), SkBits2Float(0x42801438));  // 214.278f, 71.3618f, 221.601f, 64.0395f
path.quadTo(SkBits2Float(0x4364ec44), SkBits2Float(0x4262de60), SkBits2Float(0x436f473c), SkBits2Float(0x4262de60));  // 228.923f, 56.7172f, 239.278f, 56.7172f
path.quadTo(SkBits2Float(0x4379a234), SkBits2Float(0x4262de60), SkBits2Float(0x43807a5c), SkBits2Float(0x42801438));  // 249.634f, 56.7172f, 256.956f, 64.0395f
path.quadTo(SkBits2Float(0x4384239e), SkBits2Float(0x428eb941), SkBits2Float(0x4384239e), SkBits2Float(0x42a36f30));  // 264.278f, 71.3618f, 264.278f, 81.7172f
path.close();
    SkPath path158(path);
    builder.add(path158, (SkPathOp) 2);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x435be541), SkBits2Float(0x43baaaf6));  // 219.896f, 373.336f
path.quadTo(SkBits2Float(0x435be541), SkBits2Float(0x43bfd872), SkBits2Float(0x435492bd), SkBits2Float(0x43c381b4));  // 219.896f, 383.691f, 212.573f, 391.013f
path.quadTo(SkBits2Float(0x434d4038), SkBits2Float(0x43c72af6), SkBits2Float(0x4342e541), SkBits2Float(0x43c72af6));  // 205.251f, 398.336f, 194.896f, 398.336f
path.quadTo(SkBits2Float(0x43388a4a), SkBits2Float(0x43c72af6), SkBits2Float(0x433137c5), SkBits2Float(0x43c381b4));  // 184.54f, 398.336f, 177.218f, 391.013f
path.quadTo(SkBits2Float(0x4329e541), SkBits2Float(0x43bfd872), SkBits2Float(0x4329e541), SkBits2Float(0x43baaaf6));  // 169.896f, 383.691f, 169.896f, 373.336f
path.quadTo(SkBits2Float(0x4329e541), SkBits2Float(0x43b57d7a), SkBits2Float(0x433137c5), SkBits2Float(0x43b1d438));  // 169.896f, 362.98f, 177.218f, 355.658f
path.quadTo(SkBits2Float(0x43388a4a), SkBits2Float(0x43ae2af6), SkBits2Float(0x4342e541), SkBits2Float(0x43ae2af6));  // 184.54f, 348.336f, 194.896f, 348.336f
path.quadTo(SkBits2Float(0x434d4038), SkBits2Float(0x43ae2af6), SkBits2Float(0x435492bd), SkBits2Float(0x43b1d438));  // 205.251f, 348.336f, 212.573f, 355.658f
path.quadTo(SkBits2Float(0x435be541), SkBits2Float(0x43b57d7a), SkBits2Float(0x435be541), SkBits2Float(0x43baaaf6));  // 219.896f, 362.98f, 219.896f, 373.336f
path.close();
    SkPath path159(path);
    builder.add(path159, (SkPathOp) 2);

    builder.resolve(&path);
}

static void (*firstTest)(skiatest::Reporter* , const char* filename) = nullptr;
static void (*stopTest)(skiatest::Reporter* , const char* filename) = nullptr;

static struct TestDesc tests[] = {
    TEST(build1_1),
};


static const size_t testCount = SK_ARRAY_COUNT(tests);

static bool runReverse = false;

DEF_TEST(PathOpsBuildUse, reporter) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    RunTestSet(reporter, tests, testCount, firstTest, nullptr, stopTest, runReverse);
}
