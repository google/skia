/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/PathOpsDebug.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsThreadedCommon.h"

#include <atomic>

#define TEST(name) { name, #name }

static void tiger8(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(SkBits2Float(0x43f639c5), SkBits2Float(0x4361375a));  // 492.451f, 225.216f
path.quadTo(SkBits2Float(0x43f58ce4), SkBits2Float(0x435d2a04), SkBits2Float(0x43f71bd9), SkBits2Float(0x435ac7d8));  // 491.101f, 221.164f, 494.218f, 218.781f
path.quadTo(SkBits2Float(0x43f7d69d), SkBits2Float(0x4359aa35), SkBits2Float(0x43f8b3b3), SkBits2Float(0x435951c5));  // 495.677f, 217.665f, 497.404f, 217.319f
path.conicTo(SkBits2Float(0x43f8ba67), SkBits2Float(0x43594f16), SkBits2Float(0x43f8c136), SkBits2Float(0x43594dd9), SkBits2Float(0x3f7fa2b1));  // 497.456f, 217.309f, 497.509f, 217.304f, 0.998576f
path.quadTo(SkBits2Float(0x43fcc3a8), SkBits2Float(0x43589340), SkBits2Float(0x43ff01dc), SkBits2Float(0x4352e191));  // 505.529f, 216.575f, 510.015f, 210.881f
path.conicTo(SkBits2Float(0x43ff5113), SkBits2Float(0x4352187b), SkBits2Float(0x43ffb59e), SkBits2Float(0x4352b6e9), SkBits2Float(0x3f3504f3));  // 510.633f, 210.096f, 511.419f, 210.714f, 0.707107f
path.conicTo(SkBits2Float(0x43ffdc85), SkBits2Float(0x4352f435), SkBits2Float(0x43ffe4a9), SkBits2Float(0x435355e9), SkBits2Float(0x3f6ec0ae));  // 511.723f, 210.954f, 511.786f, 211.336f, 0.932628f
path.quadTo(SkBits2Float(0x4400461c), SkBits2Float(0x435b3080), SkBits2Float(0x4400b692), SkBits2Float(0x4360b229));  // 513.095f, 219.189f, 514.853f, 224.696f
path.conicTo(SkBits2Float(0x4400c662), SkBits2Float(0x43617856), SkBits2Float(0x44009920), SkBits2Float(0x4361decb), SkBits2Float(0x3f46ad5b));  // 515.1f, 225.47f, 514.393f, 225.87f, 0.776083f
path.quadTo(SkBits2Float(0x43fb4920), SkBits2Float(0x43688f50), SkBits2Float(0x43f8340f), SkBits2Float(0x4365b887));  // 502.571f, 232.56f, 496.407f, 229.721f
path.quadTo(SkBits2Float(0x43f72cd2), SkBits2Float(0x4364c612), SkBits2Float(0x43f69888), SkBits2Float(0x4362e330));  // 494.35f, 228.774f, 493.192f, 226.887f
path.quadTo(SkBits2Float(0x43f66a00), SkBits2Float(0x43624bae), SkBits2Float(0x43f64c73), SkBits2Float(0x4361ad04));  // 492.828f, 226.296f, 492.597f, 225.676f
path.quadTo(SkBits2Float(0x43f642ea), SkBits2Float(0x436179d2), SkBits2Float(0x43f63c1c), SkBits2Float(0x43614abe));  // 492.523f, 225.476f, 492.47f, 225.292f
path.quadTo(SkBits2Float(0x43f639c9), SkBits2Float(0x43613aa5), SkBits2Float(0x43f63809), SkBits2Float(0x43612cda));  // 492.451f, 225.229f, 492.438f, 225.175f
path.quadTo(SkBits2Float(0x43f63777), SkBits2Float(0x43612855), SkBits2Float(0x43f636df), SkBits2Float(0x43612357));  // 492.433f, 225.158f, 492.429f, 225.138f
path.quadTo(SkBits2Float(0x43f6368f), SkBits2Float(0x436120b2), SkBits2Float(0x43f6367b), SkBits2Float(0x43612005));  // 492.426f, 225.128f, 492.426f, 225.125f
path.lineTo(SkBits2Float(0x43f63656), SkBits2Float(0x43611ebc));  // 492.424f, 225.12f
path.lineTo(SkBits2Float(0x43f63647), SkBits2Float(0x43611e34));  // 492.424f, 225.118f
path.lineTo(SkBits2Float(0x43f6363f), SkBits2Float(0x43611df3));  // 492.424f, 225.117f
path.lineTo(SkBits2Float(0x43f6363e), SkBits2Float(0x43611de5));  // 492.424f, 225.117f
path.lineTo(SkBits2Float(0x43f6363f), SkBits2Float(0x43611deb));  // 492.424f, 225.117f
path.lineTo(SkBits2Float(0x43f63647), SkBits2Float(0x43611e37));  // 492.424f, 225.118f
path.lineTo(SkBits2Float(0x43f63644), SkBits2Float(0x43611e19));  // 492.424f, 225.118f
path.quadTo(SkBits2Float(0x43f6365c), SkBits2Float(0x43611ee7), SkBits2Float(0x43f6365d), SkBits2Float(0x43611ef9));  // 492.425f, 225.121f, 492.425f, 225.121f
path.quadTo(SkBits2Float(0x43f63666), SkBits2Float(0x43611f4b), SkBits2Float(0x43f63672), SkBits2Float(0x43611fb1));  // 492.425f, 225.122f, 492.425f, 225.124f
path.quadTo(SkBits2Float(0x43f636ab), SkBits2Float(0x436121a4), SkBits2Float(0x43f636e3), SkBits2Float(0x4361236a));  // 492.427f, 225.131f, 492.429f, 225.138f
path.quadTo(SkBits2Float(0x43f636fd), SkBits2Float(0x43612443), SkBits2Float(0x43f63705), SkBits2Float(0x4361247e));  // 492.43f, 225.142f, 492.43f, 225.143f
path.quadTo(SkBits2Float(0x43f637d7), SkBits2Float(0x43612b15), SkBits2Float(0x43f638dc), SkBits2Float(0x436131b0));  // 492.436f, 225.168f, 492.444f, 225.194f
path.quadTo(SkBits2Float(0x43f63b88), SkBits2Float(0x43614303), SkBits2Float(0x43f63f62), SkBits2Float(0x43615368));  // 492.465f, 225.262f, 492.495f, 225.326f
path.quadTo(SkBits2Float(0x43f6436f), SkBits2Float(0x4361649f), SkBits2Float(0x43f648b2), SkBits2Float(0x43617468));  // 492.527f, 225.393f, 492.568f, 225.455f
path.quadTo(SkBits2Float(0x43f68760), SkBits2Float(0x43623072), SkBits2Float(0x43f6ec71), SkBits2Float(0x4361cb60));  // 493.058f, 226.189f, 493.847f, 225.794f
path.quadTo(SkBits2Float(0x43f722ef), SkBits2Float(0x436194e0), SkBits2Float(0x43f73027), SkBits2Float(0x43611df0));  // 494.273f, 225.582f, 494.376f, 225.117f
path.quadTo(SkBits2Float(0x43f73334), SkBits2Float(0x43610284), SkBits2Float(0x43f73333), SkBits2Float(0x4360e667));  // 494.4f, 225.01f, 494.4f, 224.9f
path.lineTo(SkBits2Float(0x43f63638), SkBits2Float(0x43611daf));  // 492.424f, 225.116f
path.lineTo(SkBits2Float(0x43f6b333), SkBits2Float(0x4360e666));  // 493.4f, 224.9f
path.lineTo(SkBits2Float(0x43f639c5), SkBits2Float(0x4361375a));  // 492.451f, 225.216f
path.close();
path.moveTo(SkBits2Float(0x43f72ca1), SkBits2Float(0x43609572));  // 494.349f, 224.584f
path.conicTo(SkBits2Float(0x43f72ebd), SkBits2Float(0x4360a219), SkBits2Float(0x43f7302e), SkBits2Float(0x4360af1f), SkBits2Float(0x3f7fa741));  // 494.365f, 224.633f, 494.376f, 224.684f, 0.998646f
path.lineTo(SkBits2Float(0x43f63333), SkBits2Float(0x4360e667));  // 492.4f, 224.9f
path.quadTo(SkBits2Float(0x43f63333), SkBits2Float(0x4360ca4b), SkBits2Float(0x43f6363f), SkBits2Float(0x4360aede));  // 492.4f, 224.79f, 492.424f, 224.683f
path.quadTo(SkBits2Float(0x43f64377), SkBits2Float(0x436037ee), SkBits2Float(0x43f679f5), SkBits2Float(0x4360016e));  // 492.527f, 224.218f, 492.953f, 224.006f
path.quadTo(SkBits2Float(0x43f6df06), SkBits2Float(0x435f9c5c), SkBits2Float(0x43f71db4), SkBits2Float(0x43605866));  // 493.742f, 223.611f, 494.232f, 224.345f
path.quadTo(SkBits2Float(0x43f722f8), SkBits2Float(0x43606830), SkBits2Float(0x43f72704), SkBits2Float(0x43607966));  // 494.273f, 224.407f, 494.305f, 224.474f
path.quadTo(SkBits2Float(0x43f72ae0), SkBits2Float(0x436089cd), SkBits2Float(0x43f72d8a), SkBits2Float(0x43609b1e));  // 494.335f, 224.538f, 494.356f, 224.606f
path.quadTo(SkBits2Float(0x43f72e8e), SkBits2Float(0x4360a1b8), SkBits2Float(0x43f72f61), SkBits2Float(0x4360a850));  // 494.364f, 224.632f, 494.37f, 224.657f
path.quadTo(SkBits2Float(0x43f72f68), SkBits2Float(0x4360a88a), SkBits2Float(0x43f72f83), SkBits2Float(0x4360a964));  // 494.37f, 224.658f, 494.371f, 224.662f
path.quadTo(SkBits2Float(0x43f72fbb), SkBits2Float(0x4360ab2a), SkBits2Float(0x43f72ff4), SkBits2Float(0x4360ad1d));  // 494.373f, 224.669f, 494.375f, 224.676f
path.quadTo(SkBits2Float(0x43f73000), SkBits2Float(0x4360ad83), SkBits2Float(0x43f73009), SkBits2Float(0x4360add5));  // 494.375f, 224.678f, 494.375f, 224.679f
path.quadTo(SkBits2Float(0x43f7300b), SkBits2Float(0x4360ade9), SkBits2Float(0x43f73022), SkBits2Float(0x4360aeb5));  // 494.375f, 224.679f, 494.376f, 224.682f
path.lineTo(SkBits2Float(0x43f7301f), SkBits2Float(0x4360ae97));  // 494.376f, 224.682f
path.lineTo(SkBits2Float(0x43f73027), SkBits2Float(0x4360aee3));  // 494.376f, 224.683f
path.lineTo(SkBits2Float(0x43f73028), SkBits2Float(0x4360aeeb));  // 494.376f, 224.683f
path.lineTo(SkBits2Float(0x43f73027), SkBits2Float(0x4360aedf));  // 494.376f, 224.683f
path.lineTo(SkBits2Float(0x43f73021), SkBits2Float(0x4360aeaa));  // 494.376f, 224.682f
path.lineTo(SkBits2Float(0x43f73016), SkBits2Float(0x4360ae50));  // 494.376f, 224.681f
path.lineTo(SkBits2Float(0x43f73007), SkBits2Float(0x4360adc1));  // 494.375f, 224.679f
path.lineTo(SkBits2Float(0x43f72ff9), SkBits2Float(0x4360ad4d));  // 494.375f, 224.677f
path.quadTo(SkBits2Float(0x43f7300d), SkBits2Float(0x4360adf7), SkBits2Float(0x43f73031), SkBits2Float(0x4360af12));  // 494.375f, 224.68f, 494.376f, 224.684f
path.quadTo(SkBits2Float(0x43f730f0), SkBits2Float(0x4360b4f1), SkBits2Float(0x43f7320a), SkBits2Float(0x4360bc94));  // 494.382f, 224.707f, 494.391f, 224.737f
path.quadTo(SkBits2Float(0x43f73625), SkBits2Float(0x4360d8fe), SkBits2Float(0x43f73c59), SkBits2Float(0x4360fa4a));  // 494.423f, 224.848f, 494.471f, 224.978f
path.quadTo(SkBits2Float(0x43f75132), SkBits2Float(0x43616a36), SkBits2Float(0x43f772ac), SkBits2Float(0x4361d738));  // 494.634f, 225.415f, 494.896f, 225.841f
path.quadTo(SkBits2Float(0x43f7de60), SkBits2Float(0x436335ea), SkBits2Float(0x43f89f25), SkBits2Float(0x4363e779));  // 495.737f, 227.211f, 497.243f, 227.904f
path.quadTo(SkBits2Float(0x43fb3d30), SkBits2Float(0x436650a0), SkBits2Float(0x44005a14), SkBits2Float(0x43602133));  // 502.478f, 230.315f, 513.407f, 224.13f
path.lineTo(SkBits2Float(0x4400799a), SkBits2Float(0x4360ffff));  // 513.9f, 225
path.lineTo(SkBits2Float(0x44003ca2), SkBits2Float(0x43614dd5));  // 512.947f, 225.304f
path.quadTo(SkBits2Float(0x43ff92b8), SkBits2Float(0x435ba8f8), SkBits2Float(0x43fee825), SkBits2Float(0x4353aa15));  // 511.146f, 219.66f, 509.814f, 211.664f
path.lineTo(SkBits2Float(0x43ff6667), SkBits2Float(0x43537fff));  // 510.8f, 211.5f
path.lineTo(SkBits2Float(0x43ffcaf2), SkBits2Float(0x43541e6d));  // 511.586f, 212.119f
path.quadTo(SkBits2Float(0x43fd4888), SkBits2Float(0x435a7d38), SkBits2Float(0x43f8d864), SkBits2Float(0x435b4bbf));  // 506.567f, 218.489f, 497.691f, 219.296f
path.lineTo(SkBits2Float(0x43f8cccd), SkBits2Float(0x435a4ccc));  // 497.6f, 218.3f
path.lineTo(SkBits2Float(0x43f8e5e7), SkBits2Float(0x435b47d3));  // 497.796f, 219.281f
path.quadTo(SkBits2Float(0x43f84300), SkBits2Float(0x435b88fd), SkBits2Float(0x43f7b75b), SkBits2Float(0x435c5e8e));  // 496.523f, 219.535f, 495.432f, 220.369f
path.quadTo(SkBits2Float(0x43f6b984), SkBits2Float(0x435de2c4), SkBits2Float(0x43f72ca1), SkBits2Float(0x43609572));  // 493.449f, 221.886f, 494.349f, 224.584f
path.close();
testSimplify(reporter, path, filename);
}

// fails to include a line of edges, probably mis-sorting
static void tiger8a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(SkBits2Float(0x43f639c5), SkBits2Float(0x4361375a));  // 492.451f, 225.216f
path.quadTo(SkBits2Float(0x43f58ce4), SkBits2Float(0x435d2a04), SkBits2Float(0x43f71bd9), SkBits2Float(0x435ac7d8));  // 491.101f, 221.164f, 494.218f, 218.781f
path.quadTo(SkBits2Float(0x43f7d69d), SkBits2Float(0x4359aa35), SkBits2Float(0x43f8b3b3), SkBits2Float(0x435951c5));  // 495.677f, 217.665f, 497.404f, 217.319f
path.conicTo(SkBits2Float(0x43f8ba67), SkBits2Float(0x43594f16), SkBits2Float(0x43f8c136), SkBits2Float(0x43594dd9), SkBits2Float(0x3f7fa2b1));  // 497.456f, 217.309f, 497.509f, 217.304f, 0.998576f
path.quadTo(SkBits2Float(0x43fcc3a8), SkBits2Float(0x43589340), SkBits2Float(0x43ff01dc), SkBits2Float(0x4352e191));  // 505.529f, 216.575f, 510.015f, 210.881f
path.conicTo(SkBits2Float(0x43ff5113), SkBits2Float(0x4352187b), SkBits2Float(0x43ffb59e), SkBits2Float(0x4352b6e9), SkBits2Float(0x3f3504f3));  // 510.633f, 210.096f, 511.419f, 210.714f, 0.707107f
path.conicTo(SkBits2Float(0x43ffdc85), SkBits2Float(0x4352f435), SkBits2Float(0x43ffe4a9), SkBits2Float(0x435355e9), SkBits2Float(0x3f6ec0ae));  // 511.723f, 210.954f, 511.786f, 211.336f, 0.932628f
path.quadTo(SkBits2Float(0x4400461c), SkBits2Float(0x435b3080), SkBits2Float(0x4400b692), SkBits2Float(0x4360b229));  // 513.095f, 219.189f, 514.853f, 224.696f
path.conicTo(SkBits2Float(0x4400c662), SkBits2Float(0x43617856), SkBits2Float(0x44009920), SkBits2Float(0x4361decb), SkBits2Float(0x3f46ad5b));  // 515.1f, 225.47f, 514.393f, 225.87f, 0.776083f
path.quadTo(SkBits2Float(0x43fb4920), SkBits2Float(0x43688f50), SkBits2Float(0x43f8340f), SkBits2Float(0x4365b887));  // 502.571f, 232.56f, 496.407f, 229.721f
path.quadTo(SkBits2Float(0x43f72cd2), SkBits2Float(0x4364c612), SkBits2Float(0x43f69888), SkBits2Float(0x4362e330));  // 494.35f, 228.774f, 493.192f, 226.887f
path.quadTo(SkBits2Float(0x43f66a00), SkBits2Float(0x43624bae), SkBits2Float(0x43f64c73), SkBits2Float(0x4361ad04));  // 492.828f, 226.296f, 492.597f, 225.676f
path.quadTo(SkBits2Float(0x43f642ea), SkBits2Float(0x436179d2), SkBits2Float(0x43f63c1c), SkBits2Float(0x43614abe));  // 492.523f, 225.476f, 492.47f, 225.292f
path.quadTo(SkBits2Float(0x43f639c9), SkBits2Float(0x43613aa5), SkBits2Float(0x43f63809), SkBits2Float(0x43612cda));  // 492.451f, 225.229f, 492.438f, 225.175f
path.quadTo(SkBits2Float(0x43f63777), SkBits2Float(0x43612855), SkBits2Float(0x43f636df), SkBits2Float(0x43612357));  // 492.433f, 225.158f, 492.429f, 225.138f
path.quadTo(SkBits2Float(0x43f6368f), SkBits2Float(0x436120b2), SkBits2Float(0x43f6367b), SkBits2Float(0x43612005));  // 492.426f, 225.128f, 492.426f, 225.125f
path.lineTo(SkBits2Float(0x43f63656), SkBits2Float(0x43611ebc));  // 492.424f, 225.12f
path.lineTo(SkBits2Float(0x43f63647), SkBits2Float(0x43611e34));  // 492.424f, 225.118f
path.lineTo(SkBits2Float(0x43f6363f), SkBits2Float(0x43611df3));  // 492.424f, 225.117f
path.lineTo(SkBits2Float(0x43f6363e), SkBits2Float(0x43611de5));  // 492.424f, 225.117f
path.lineTo(SkBits2Float(0x43f6363f), SkBits2Float(0x43611deb));  // 492.424f, 225.117f
path.lineTo(SkBits2Float(0x43f63647), SkBits2Float(0x43611e37));  // 492.424f, 225.118f
path.lineTo(SkBits2Float(0x43f63644), SkBits2Float(0x43611e19));  // 492.424f, 225.118f
path.quadTo(SkBits2Float(0x43f6365c), SkBits2Float(0x43611ee7), SkBits2Float(0x43f6365d), SkBits2Float(0x43611ef9));  // 492.425f, 225.121f, 492.425f, 225.121f
path.quadTo(SkBits2Float(0x43f63666), SkBits2Float(0x43611f4b), SkBits2Float(0x43f63672), SkBits2Float(0x43611fb1));  // 492.425f, 225.122f, 492.425f, 225.124f
path.quadTo(SkBits2Float(0x43f636ab), SkBits2Float(0x436121a4), SkBits2Float(0x43f636e3), SkBits2Float(0x4361236a));  // 492.427f, 225.131f, 492.429f, 225.138f
path.quadTo(SkBits2Float(0x43f636fd), SkBits2Float(0x43612443), SkBits2Float(0x43f63705), SkBits2Float(0x4361247e));  // 492.43f, 225.142f, 492.43f, 225.143f
path.quadTo(SkBits2Float(0x43f637d7), SkBits2Float(0x43612b15), SkBits2Float(0x43f638dc), SkBits2Float(0x436131b0));  // 492.436f, 225.168f, 492.444f, 225.194f
path.quadTo(SkBits2Float(0x43f63b88), SkBits2Float(0x43614303), SkBits2Float(0x43f63f62), SkBits2Float(0x43615368));  // 492.465f, 225.262f, 492.495f, 225.326f
path.quadTo(SkBits2Float(0x43f6436f), SkBits2Float(0x4361649f), SkBits2Float(0x43f648b2), SkBits2Float(0x43617468));  // 492.527f, 225.393f, 492.568f, 225.455f
path.quadTo(SkBits2Float(0x43f68760), SkBits2Float(0x43623072), SkBits2Float(0x43f6ec71), SkBits2Float(0x4361cb60));  // 493.058f, 226.189f, 493.847f, 225.794f
path.quadTo(SkBits2Float(0x43f722ef), SkBits2Float(0x436194e0), SkBits2Float(0x43f73027), SkBits2Float(0x43611df0));  // 494.273f, 225.582f, 494.376f, 225.117f
path.quadTo(SkBits2Float(0x43f73334), SkBits2Float(0x43610284), SkBits2Float(0x43f73333), SkBits2Float(0x4360e667));  // 494.4f, 225.01f, 494.4f, 224.9f
path.lineTo(SkBits2Float(0x43f63638), SkBits2Float(0x43611daf));  // 492.424f, 225.116f
path.lineTo(SkBits2Float(0x43f6b333), SkBits2Float(0x4360e666));  // 493.4f, 224.9f
path.lineTo(SkBits2Float(0x43f639c5), SkBits2Float(0x4361375a));  // 492.451f, 225.216f
path.close();
testSimplify(reporter, path, filename);
}

static std::atomic<int> gTigerTests{0};

static void tiger8a_x(skiatest::Reporter* reporter, uint64_t testlines) {
    SkPath path;
uint64_t i = 0;
if (testlines & (1LL << i++)) path.moveTo(SkBits2Float(0x43f639c5), SkBits2Float(0x4361375a));  // 492.451f, 225.216f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f58ce4), SkBits2Float(0x435d2a04), SkBits2Float(0x43f71bd9), SkBits2Float(0x435ac7d8));  // 491.101f, 221.164f, 494.218f, 218.781f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f7d69d), SkBits2Float(0x4359aa35), SkBits2Float(0x43f8b3b3), SkBits2Float(0x435951c5));  // 495.677f, 217.665f, 497.404f, 217.319f
if (testlines & (1LL << i++)) path.conicTo(SkBits2Float(0x43f8ba67), SkBits2Float(0x43594f16), SkBits2Float(0x43f8c136), SkBits2Float(0x43594dd9), SkBits2Float(0x3f7fa2b1));  // 497.456f, 217.309f, 497.509f, 217.304f, 0.998576f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43fcc3a8), SkBits2Float(0x43589340), SkBits2Float(0x43ff01dc), SkBits2Float(0x4352e191));  // 505.529f, 216.575f, 510.015f, 210.881f
if (testlines & (1LL << i++)) path.conicTo(SkBits2Float(0x43ff5113), SkBits2Float(0x4352187b), SkBits2Float(0x43ffb59e), SkBits2Float(0x4352b6e9), SkBits2Float(0x3f3504f3));  // 510.633f, 210.096f, 511.419f, 210.714f, 0.707107f
if (testlines & (1LL << i++)) path.conicTo(SkBits2Float(0x43ffdc85), SkBits2Float(0x4352f435), SkBits2Float(0x43ffe4a9), SkBits2Float(0x435355e9), SkBits2Float(0x3f6ec0ae));  // 511.723f, 210.954f, 511.786f, 211.336f, 0.932628f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x4400461c), SkBits2Float(0x435b3080), SkBits2Float(0x4400b692), SkBits2Float(0x4360b229));  // 513.095f, 219.189f, 514.853f, 224.696f
if (testlines & (1LL << i++)) path.conicTo(SkBits2Float(0x4400c662), SkBits2Float(0x43617856), SkBits2Float(0x44009920), SkBits2Float(0x4361decb), SkBits2Float(0x3f46ad5b));  // 515.1f, 225.47f, 514.393f, 225.87f, 0.776083f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43fb4920), SkBits2Float(0x43688f50), SkBits2Float(0x43f8340f), SkBits2Float(0x4365b887));  // 502.571f, 232.56f, 496.407f, 229.721f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f72cd2), SkBits2Float(0x4364c612), SkBits2Float(0x43f69888), SkBits2Float(0x4362e330));  // 494.35f, 228.774f, 493.192f, 226.887f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f66a00), SkBits2Float(0x43624bae), SkBits2Float(0x43f64c73), SkBits2Float(0x4361ad04));  // 492.828f, 226.296f, 492.597f, 225.676f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f642ea), SkBits2Float(0x436179d2), SkBits2Float(0x43f63c1c), SkBits2Float(0x43614abe));  // 492.523f, 225.476f, 492.47f, 225.292f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f639c9), SkBits2Float(0x43613aa5), SkBits2Float(0x43f63809), SkBits2Float(0x43612cda));  // 492.451f, 225.229f, 492.438f, 225.175f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f63777), SkBits2Float(0x43612855), SkBits2Float(0x43f636df), SkBits2Float(0x43612357));  // 492.433f, 225.158f, 492.429f, 225.138f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f6368f), SkBits2Float(0x436120b2), SkBits2Float(0x43f6367b), SkBits2Float(0x43612005));  // 492.426f, 225.128f, 492.426f, 225.125f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f63656), SkBits2Float(0x43611ebc));  // 492.424f, 225.12f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f63647), SkBits2Float(0x43611e34));  // 492.424f, 225.118f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f6363f), SkBits2Float(0x43611df3));  // 492.424f, 225.117f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f6363e), SkBits2Float(0x43611de5));  // 492.424f, 225.117f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f6363f), SkBits2Float(0x43611deb));  // 492.424f, 225.117f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f63647), SkBits2Float(0x43611e37));  // 492.424f, 225.118f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f63644), SkBits2Float(0x43611e19));  // 492.424f, 225.118f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f6365c), SkBits2Float(0x43611ee7), SkBits2Float(0x43f6365d), SkBits2Float(0x43611ef9));  // 492.425f, 225.121f, 492.425f, 225.121f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f63666), SkBits2Float(0x43611f4b), SkBits2Float(0x43f63672), SkBits2Float(0x43611fb1));  // 492.425f, 225.122f, 492.425f, 225.124f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f636ab), SkBits2Float(0x436121a4), SkBits2Float(0x43f636e3), SkBits2Float(0x4361236a));  // 492.427f, 225.131f, 492.429f, 225.138f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f636fd), SkBits2Float(0x43612443), SkBits2Float(0x43f63705), SkBits2Float(0x4361247e));  // 492.43f, 225.142f, 492.43f, 225.143f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f637d7), SkBits2Float(0x43612b15), SkBits2Float(0x43f638dc), SkBits2Float(0x436131b0));  // 492.436f, 225.168f, 492.444f, 225.194f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f63b88), SkBits2Float(0x43614303), SkBits2Float(0x43f63f62), SkBits2Float(0x43615368));  // 492.465f, 225.262f, 492.495f, 225.326f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f6436f), SkBits2Float(0x4361649f), SkBits2Float(0x43f648b2), SkBits2Float(0x43617468));  // 492.527f, 225.393f, 492.568f, 225.455f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f68760), SkBits2Float(0x43623072), SkBits2Float(0x43f6ec71), SkBits2Float(0x4361cb60));  // 493.058f, 226.189f, 493.847f, 225.794f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f722ef), SkBits2Float(0x436194e0), SkBits2Float(0x43f73027), SkBits2Float(0x43611df0));  // 494.273f, 225.582f, 494.376f, 225.117f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f73334), SkBits2Float(0x43610284), SkBits2Float(0x43f73333), SkBits2Float(0x4360e667));  // 494.4f, 225.01f, 494.4f, 224.9f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f63638), SkBits2Float(0x43611daf));  // 492.424f, 225.116f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f6b333), SkBits2Float(0x4360e666));  // 493.4f, 224.9f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f639c5), SkBits2Float(0x4361375a));  // 492.451f, 225.216f
if (testlines & (1LL << i++)) path.close();
SkString testName;
testName.printf("tiger8a_x%d", ++gTigerTests);
testSimplify(reporter, path, testName.c_str());
}

#include "include/utils/SkRandom.h"

static void tiger8a_h_1(skiatest::Reporter* reporter, const char* ) {
    uint64_t testlines = 0x0000000000002008;  // best so far: 0x0000001d14c14bb1;
    tiger8a_x(reporter, testlines);
}

static void tiger8b_x(skiatest::Reporter* reporter, uint64_t testlines) {
    SkPath path;
uint64_t i = 0;
if (testlines & (1LL << i++)) path.moveTo(SkBits2Float(0x43f72ca1), SkBits2Float(0x43609572));  // 494.349f, 224.584f
if (testlines & (1LL << i++)) path.conicTo(SkBits2Float(0x43f72ebd), SkBits2Float(0x4360a219), SkBits2Float(0x43f7302e), SkBits2Float(0x4360af1f), SkBits2Float(0x3f7fa741));  // 494.365f, 224.633f, 494.376f, 224.684f, 0.998646f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f63333), SkBits2Float(0x4360e667));  // 492.4f, 224.9f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f63333), SkBits2Float(0x4360ca4b), SkBits2Float(0x43f6363f), SkBits2Float(0x4360aede));  // 492.4f, 224.79f, 492.424f, 224.683f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f64377), SkBits2Float(0x436037ee), SkBits2Float(0x43f679f5), SkBits2Float(0x4360016e));  // 492.527f, 224.218f, 492.953f, 224.006f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f6df06), SkBits2Float(0x435f9c5c), SkBits2Float(0x43f71db4), SkBits2Float(0x43605866));  // 493.742f, 223.611f, 494.232f, 224.345f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f722f8), SkBits2Float(0x43606830), SkBits2Float(0x43f72704), SkBits2Float(0x43607966));  // 494.273f, 224.407f, 494.305f, 224.474f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f72ae0), SkBits2Float(0x436089cd), SkBits2Float(0x43f72d8a), SkBits2Float(0x43609b1e));  // 494.335f, 224.538f, 494.356f, 224.606f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f72e8e), SkBits2Float(0x4360a1b8), SkBits2Float(0x43f72f61), SkBits2Float(0x4360a850));  // 494.364f, 224.632f, 494.37f, 224.657f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f72f68), SkBits2Float(0x4360a88a), SkBits2Float(0x43f72f83), SkBits2Float(0x4360a964));  // 494.37f, 224.658f, 494.371f, 224.662f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f72fbb), SkBits2Float(0x4360ab2a), SkBits2Float(0x43f72ff4), SkBits2Float(0x4360ad1d));  // 494.373f, 224.669f, 494.375f, 224.676f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f73000), SkBits2Float(0x4360ad83), SkBits2Float(0x43f73009), SkBits2Float(0x4360add5));  // 494.375f, 224.678f, 494.375f, 224.679f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f7300b), SkBits2Float(0x4360ade9), SkBits2Float(0x43f73022), SkBits2Float(0x4360aeb5));  // 494.375f, 224.679f, 494.376f, 224.682f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f7301f), SkBits2Float(0x4360ae97));  // 494.376f, 224.682f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f73027), SkBits2Float(0x4360aee3));  // 494.376f, 224.683f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f73028), SkBits2Float(0x4360aeeb));  // 494.376f, 224.683f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f73027), SkBits2Float(0x4360aedf));  // 494.376f, 224.683f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f73021), SkBits2Float(0x4360aeaa));  // 494.376f, 224.682f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f73016), SkBits2Float(0x4360ae50));  // 494.376f, 224.681f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f73007), SkBits2Float(0x4360adc1));  // 494.375f, 224.679f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f72ff9), SkBits2Float(0x4360ad4d));  // 494.375f, 224.677f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f7300d), SkBits2Float(0x4360adf7), SkBits2Float(0x43f73031), SkBits2Float(0x4360af12));  // 494.375f, 224.68f, 494.376f, 224.684f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f730f0), SkBits2Float(0x4360b4f1), SkBits2Float(0x43f7320a), SkBits2Float(0x4360bc94));  // 494.382f, 224.707f, 494.391f, 224.737f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f73625), SkBits2Float(0x4360d8fe), SkBits2Float(0x43f73c59), SkBits2Float(0x4360fa4a));  // 494.423f, 224.848f, 494.471f, 224.978f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f75132), SkBits2Float(0x43616a36), SkBits2Float(0x43f772ac), SkBits2Float(0x4361d738));  // 494.634f, 225.415f, 494.896f, 225.841f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f7de60), SkBits2Float(0x436335ea), SkBits2Float(0x43f89f25), SkBits2Float(0x4363e779));  // 495.737f, 227.211f, 497.243f, 227.904f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43fb3d30), SkBits2Float(0x436650a0), SkBits2Float(0x44005a14), SkBits2Float(0x43602133));  // 502.478f, 230.315f, 513.407f, 224.13f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x4400799a), SkBits2Float(0x4360ffff));  // 513.9f, 225
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x44003ca2), SkBits2Float(0x43614dd5));  // 512.947f, 225.304f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43ff92b8), SkBits2Float(0x435ba8f8), SkBits2Float(0x43fee825), SkBits2Float(0x4353aa15));  // 511.146f, 219.66f, 509.814f, 211.664f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43ff6667), SkBits2Float(0x43537fff));  // 510.8f, 211.5f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43ffcaf2), SkBits2Float(0x43541e6d));  // 511.586f, 212.119f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43fd4888), SkBits2Float(0x435a7d38), SkBits2Float(0x43f8d864), SkBits2Float(0x435b4bbf));  // 506.567f, 218.489f, 497.691f, 219.296f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f8cccd), SkBits2Float(0x435a4ccc));  // 497.6f, 218.3f
if (testlines & (1LL << i++)) path.lineTo(SkBits2Float(0x43f8e5e7), SkBits2Float(0x435b47d3));  // 497.796f, 219.281f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f84300), SkBits2Float(0x435b88fd), SkBits2Float(0x43f7b75b), SkBits2Float(0x435c5e8e));  // 496.523f, 219.535f, 495.432f, 220.369f
if (testlines & (1LL << i++)) path.quadTo(SkBits2Float(0x43f6b984), SkBits2Float(0x435de2c4), SkBits2Float(0x43f72ca1), SkBits2Float(0x43609572));  // 493.449f, 221.886f, 494.349f, 224.584f
if (testlines & (1LL << i++)) path.close();
SkString testName;
testName.printf("tiger8b_x%d", ++gTigerTests);
testSimplify(reporter, path, testName.c_str());
}

static void testTiger(PathOpsThreadState* data) {
    uint64_t testlines = ((uint64_t) data->fB << 32) | (unsigned int) data->fA;
    if (data->fC) {
        tiger8b_x(data->fReporter, testlines);
    } else {
        tiger8a_x(data->fReporter, testlines);
    }
}

static void tiger_threaded(skiatest::Reporter* reporter, const char* filename) {
    initializeTests(reporter, "tigerb");
    PathOpsThreadedTestRunner testRunner(reporter);
    for (int ab = 0; ab < 2; ++ab) {
        SkRandom r;
        int testCount = reporter->allowExtendedTest() ? 10000 : 100;
        for (int samples = 2; samples < 37; ++samples) {
            for (int tests = 0; tests < testCount; ++tests) {
                uint64_t testlines = 0;
                for (int i = 0; i < samples; ++i) {
                    int bit;
                    do {
                        bit = r.nextRangeU(0, 38);
                    } while (testlines & (1LL << bit));
                    testlines |= 1LL << bit;
                }
                *testRunner.fRunnables.append() =
                        new PathOpsThreadedRunnable(&testTiger,
                                                    (int) (unsigned) (testlines & 0xFFFFFFFF),
                                                    (int) (unsigned) (testlines >> 32),
                                                    ab, 0, &testRunner);
            }
        }
    }
    testRunner.render();
}

static void tiger8b_h_1(skiatest::Reporter* reporter, const char* filename) {
    uint64_t testlines = 0x000000000f27b9e3;  // best so far: 0x000000201304b4a3
    tiger8b_x(reporter, testlines);
}

// tries to add same edge twice
static void tiger8b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.moveTo(SkBits2Float(0x43f72ca1), SkBits2Float(0x43609572));  // 494.349f, 224.584f
path.conicTo(SkBits2Float(0x43f72ebd), SkBits2Float(0x4360a219), SkBits2Float(0x43f7302e), SkBits2Float(0x4360af1f), SkBits2Float(0x3f7fa741));  // 494.365f, 224.633f, 494.376f, 224.684f, 0.998646f
path.lineTo(SkBits2Float(0x43f63333), SkBits2Float(0x4360e667));  // 492.4f, 224.9f
path.quadTo(SkBits2Float(0x43f63333), SkBits2Float(0x4360ca4b), SkBits2Float(0x43f6363f), SkBits2Float(0x4360aede));  // 492.4f, 224.79f, 492.424f, 224.683f
path.quadTo(SkBits2Float(0x43f64377), SkBits2Float(0x436037ee), SkBits2Float(0x43f679f5), SkBits2Float(0x4360016e));  // 492.527f, 224.218f, 492.953f, 224.006f
path.quadTo(SkBits2Float(0x43f6df06), SkBits2Float(0x435f9c5c), SkBits2Float(0x43f71db4), SkBits2Float(0x43605866));  // 493.742f, 223.611f, 494.232f, 224.345f
path.quadTo(SkBits2Float(0x43f722f8), SkBits2Float(0x43606830), SkBits2Float(0x43f72704), SkBits2Float(0x43607966));  // 494.273f, 224.407f, 494.305f, 224.474f
path.quadTo(SkBits2Float(0x43f72ae0), SkBits2Float(0x436089cd), SkBits2Float(0x43f72d8a), SkBits2Float(0x43609b1e));  // 494.335f, 224.538f, 494.356f, 224.606f
path.quadTo(SkBits2Float(0x43f72e8e), SkBits2Float(0x4360a1b8), SkBits2Float(0x43f72f61), SkBits2Float(0x4360a850));  // 494.364f, 224.632f, 494.37f, 224.657f
path.quadTo(SkBits2Float(0x43f72f68), SkBits2Float(0x4360a88a), SkBits2Float(0x43f72f83), SkBits2Float(0x4360a964));  // 494.37f, 224.658f, 494.371f, 224.662f
path.quadTo(SkBits2Float(0x43f72fbb), SkBits2Float(0x4360ab2a), SkBits2Float(0x43f72ff4), SkBits2Float(0x4360ad1d));  // 494.373f, 224.669f, 494.375f, 224.676f
path.quadTo(SkBits2Float(0x43f73000), SkBits2Float(0x4360ad83), SkBits2Float(0x43f73009), SkBits2Float(0x4360add5));  // 494.375f, 224.678f, 494.375f, 224.679f
path.quadTo(SkBits2Float(0x43f7300b), SkBits2Float(0x4360ade9), SkBits2Float(0x43f73022), SkBits2Float(0x4360aeb5));  // 494.375f, 224.679f, 494.376f, 224.682f
path.lineTo(SkBits2Float(0x43f7301f), SkBits2Float(0x4360ae97));  // 494.376f, 224.682f
path.lineTo(SkBits2Float(0x43f73027), SkBits2Float(0x4360aee3));  // 494.376f, 224.683f
path.lineTo(SkBits2Float(0x43f73028), SkBits2Float(0x4360aeeb));  // 494.376f, 224.683f
path.lineTo(SkBits2Float(0x43f73027), SkBits2Float(0x4360aedf));  // 494.376f, 224.683f
path.lineTo(SkBits2Float(0x43f73021), SkBits2Float(0x4360aeaa));  // 494.376f, 224.682f
path.lineTo(SkBits2Float(0x43f73016), SkBits2Float(0x4360ae50));  // 494.376f, 224.681f
path.lineTo(SkBits2Float(0x43f73007), SkBits2Float(0x4360adc1));  // 494.375f, 224.679f
path.lineTo(SkBits2Float(0x43f72ff9), SkBits2Float(0x4360ad4d));  // 494.375f, 224.677f
path.quadTo(SkBits2Float(0x43f7300d), SkBits2Float(0x4360adf7), SkBits2Float(0x43f73031), SkBits2Float(0x4360af12));  // 494.375f, 224.68f, 494.376f, 224.684f
path.quadTo(SkBits2Float(0x43f730f0), SkBits2Float(0x4360b4f1), SkBits2Float(0x43f7320a), SkBits2Float(0x4360bc94));  // 494.382f, 224.707f, 494.391f, 224.737f
path.quadTo(SkBits2Float(0x43f73625), SkBits2Float(0x4360d8fe), SkBits2Float(0x43f73c59), SkBits2Float(0x4360fa4a));  // 494.423f, 224.848f, 494.471f, 224.978f
path.quadTo(SkBits2Float(0x43f75132), SkBits2Float(0x43616a36), SkBits2Float(0x43f772ac), SkBits2Float(0x4361d738));  // 494.634f, 225.415f, 494.896f, 225.841f
path.quadTo(SkBits2Float(0x43f7de60), SkBits2Float(0x436335ea), SkBits2Float(0x43f89f25), SkBits2Float(0x4363e779));  // 495.737f, 227.211f, 497.243f, 227.904f
path.quadTo(SkBits2Float(0x43fb3d30), SkBits2Float(0x436650a0), SkBits2Float(0x44005a14), SkBits2Float(0x43602133));  // 502.478f, 230.315f, 513.407f, 224.13f
path.lineTo(SkBits2Float(0x4400799a), SkBits2Float(0x4360ffff));  // 513.9f, 225
path.lineTo(SkBits2Float(0x44003ca2), SkBits2Float(0x43614dd5));  // 512.947f, 225.304f
path.quadTo(SkBits2Float(0x43ff92b8), SkBits2Float(0x435ba8f8), SkBits2Float(0x43fee825), SkBits2Float(0x4353aa15));  // 511.146f, 219.66f, 509.814f, 211.664f
path.lineTo(SkBits2Float(0x43ff6667), SkBits2Float(0x43537fff));  // 510.8f, 211.5f
path.lineTo(SkBits2Float(0x43ffcaf2), SkBits2Float(0x43541e6d));  // 511.586f, 212.119f
path.quadTo(SkBits2Float(0x43fd4888), SkBits2Float(0x435a7d38), SkBits2Float(0x43f8d864), SkBits2Float(0x435b4bbf));  // 506.567f, 218.489f, 497.691f, 219.296f
path.lineTo(SkBits2Float(0x43f8cccd), SkBits2Float(0x435a4ccc));  // 497.6f, 218.3f
path.lineTo(SkBits2Float(0x43f8e5e7), SkBits2Float(0x435b47d3));  // 497.796f, 219.281f
path.quadTo(SkBits2Float(0x43f84300), SkBits2Float(0x435b88fd), SkBits2Float(0x43f7b75b), SkBits2Float(0x435c5e8e));  // 496.523f, 219.535f, 495.432f, 220.369f
path.quadTo(SkBits2Float(0x43f6b984), SkBits2Float(0x435de2c4), SkBits2Float(0x43f72ca1), SkBits2Float(0x43609572));  // 493.449f, 221.886f, 494.349f, 224.584f
path.close();
testSimplify(reporter, path, filename);
}



static void (*skipTest)(skiatest::Reporter* , const char* filename) = nullptr;
static void (*firstTest)(skiatest::Reporter* , const char* filename) = nullptr;
static void (*stopTest)(skiatest::Reporter* , const char* filename) = nullptr;

static TestDesc tests[] = {
    TEST(tiger8a_h_1),
    TEST(tiger8a),
    TEST(tiger8b_h_1),
    TEST(tiger8b),
    TEST(tiger8),
    TEST(tiger_threaded),
};

static const size_t testCount = SK_ARRAY_COUNT(tests);
static bool runReverse = false;

DEF_TEST(PathOpsTiger, reporter) {
    RunTestSet(reporter, tests, testCount, firstTest, skipTest, stopTest, runReverse);
}
