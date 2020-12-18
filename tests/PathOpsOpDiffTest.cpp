/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsTestCommon.h"

#define TEST(name) { name, #name }

static SkPath getPath0() {
  SkPath path;
  path.setFillType(SkPathFillType::kWinding);
  path.moveTo(SkBits2Float(0x4533f96d), SkBits2Float(0x44b574e9));  // 2879.59f, 1451.65f
  path.lineTo(SkBits2Float(0x45270bbc), SkBits2Float(0x44b574e9));  // 2672.73f, 1451.65f
  path.lineTo(SkBits2Float(0x45197516), SkBits2Float(0x44b574e9));  // 2455.32f, 1451.65f
  path.quadTo(SkBits2Float(0x451818f0), SkBits2Float(0x44b574f0), SkBits2Float(0x4516ee62), SkBits2Float(0x44b56102));  // 2433.56f, 1451.65f, 2414.9f, 1451.03f
  path.lineTo(SkBits2Float(0x4514901e), SkBits2Float(0x44b53941));  // 2377.01f, 1449.79f
  path.quadTo(SkBits2Float(0x4513f010), SkBits2Float(0x44b52ee8), SkBits2Float(0x4513d9c4), SkBits2Float(0x44b50f16));  // 2367, 1449.47f, 2365.61f, 1448.47f
  path.quadTo(SkBits2Float(0x4513cf82), SkBits2Float(0x44b5006e), SkBits2Float(0x4513cf3d), SkBits2Float(0x44b4e937));  // 2364.97f, 1448.01f, 2364.95f, 1447.29f
  path.quadTo(SkBits2Float(0x4513cefb), SkBits2Float(0x44b4d2f2), SkBits2Float(0x4513d846), SkBits2Float(0x44b4c31c));  // 2364.94f, 1446.59f, 2365.52f, 1446.1f
  path.quadTo(SkBits2Float(0x4513df85), SkBits2Float(0x44b4b6c3), SkBits2Float(0x4513f016), SkBits2Float(0x44b4ac42));  // 2365.97f, 1445.71f, 2367.01f, 1445.38f
  path.quadTo(SkBits2Float(0x45140ae9), SkBits2Float(0x44b49b40), SkBits2Float(0x451447ed), SkBits2Float(0x44b486ea));  // 2368.68f, 1444.85f, 2372.5f, 1444.22f
  path.quadTo(SkBits2Float(0x451537b0), SkBits2Float(0x44b43708), SkBits2Float(0x45174760), SkBits2Float(0x44b3fb4b));  // 2387.48f, 1441.72f, 2420.46f, 1439.85f
  path.quadTo(SkBits2Float(0x451956f0), SkBits2Float(0x44b3bfa0), SkBits2Float(0x451c8644), SkBits2Float(0x44b3bf97));  // 2453.43f, 1437.99f, 2504.39f, 1437.99f
  path.lineTo(SkBits2Float(0x452347a8), SkBits2Float(0x44b3bf97));  // 2612.48f, 1437.99f
  path.lineTo(SkBits2Float(0x452973f4), SkBits2Float(0x44b3bf97));  // 2711.25f, 1437.99f
  path.quadTo(SkBits2Float(0x452c0e40), SkBits2Float(0x44b3bf80), SkBits2Float(0x452ce96e), SkBits2Float(0x44b3d383));  // 2752.89f, 1437.98f, 2766.59f, 1438.61f
  path.quadTo(SkBits2Float(0x452d5840), SkBits2Float(0x44b3dd98), SkBits2Float(0x452d729a), SkBits2Float(0x44b3e81f));  // 2773.52f, 1438.92f, 2775.16f, 1439.25f
  path.quadTo(SkBits2Float(0x452d8043), SkBits2Float(0x44b3ed96), SkBits2Float(0x452d85c0), SkBits2Float(0x44b3fc39));  // 2776.02f, 1439.42f, 2776.36f, 1439.88f
  path.quadTo(SkBits2Float(0x452d8c17), SkBits2Float(0x44b40d22), SkBits2Float(0x452d88f5), SkBits2Float(0x44b4212d));  // 2776.76f, 1440.41f, 2776.56f, 1441.04f
  path.quadTo(SkBits2Float(0x452d86e6), SkBits2Float(0x44b42e5b), SkBits2Float(0x452d8127), SkBits2Float(0x44b436b6));  // 2776.43f, 1441.45f, 2776.07f, 1441.71f
  path.quadTo(SkBits2Float(0x452d7d72), SkBits2Float(0x44b43c1a), SkBits2Float(0x452d77e5), SkBits2Float(0x44b43f46));  // 2775.84f, 1441.88f, 2775.49f, 1441.98f
  path.quadTo(SkBits2Float(0x452d6d55), SkBits2Float(0x44b44550), SkBits2Float(0x452d4d92), SkBits2Float(0x44b44a9a));  // 2774.83f, 1442.17f, 2772.85f, 1442.33f
  path.quadTo(SkBits2Float(0x452cd60c), SkBits2Float(0x44b45e84), SkBits2Float(0x452b01f0), SkBits2Float(0x44b4727c));  // 2765.38f, 1442.95f, 2736.12f, 1443.58f
  path.quadTo(SkBits2Float(0x45292f90), SkBits2Float(0x44b48660), SkBits2Float(0x4526bc3a), SkBits2Float(0x44b48660));  // 2706.97f, 1444.2f, 2667.76f, 1444.2f
  path.lineTo(SkBits2Float(0x4520df70), SkBits2Float(0x44b48660));  // 2573.96f, 1444.2f
  path.quadTo(SkBits2Float(0x451d7780), SkBits2Float(0x44b48660), SkBits2Float(0x451a0003), SkBits2Float(0x44b4727e));  // 2519.47f, 1444.2f, 2464, 1443.58f
  path.quadTo(SkBits2Float(0x45168a80), SkBits2Float(0x44b45ec0), SkBits2Float(0x451259b3), SkBits2Float(0x44b422f8));  // 2408.66f, 1442.96f, 2341.61f, 1441.09f
  path.quadTo(SkBits2Float(0x450e2a00), SkBits2Float(0x44b3e780), SkBits2Float(0x450d7fdf), SkBits2Float(0x44b3e75a));  // 2274.62f, 1439.23f, 2263.99f, 1439.23f
  path.quadTo(SkBits2Float(0x450cd5f0), SkBits2Float(0x44b3e758), SkBits2Float(0x450c718a), SkBits2Float(0x44b3d346));  // 2253.37f, 1439.23f, 2247.1f, 1438.6f
  path.quadTo(SkBits2Float(0x450c3d0d), SkBits2Float(0x44b3c8c6), SkBits2Float(0x450c2e04), SkBits2Float(0x44b3bcbf));  // 2243.82f, 1438.27f, 2242.88f, 1437.9f
  path.quadTo(SkBits2Float(0x450c24e0), SkBits2Float(0x44b3b56e), SkBits2Float(0x450c20a8), SkBits2Float(0x44b3a7ef));  // 2242.3f, 1437.67f, 2242.04f, 1437.25f
  path.quadTo(SkBits2Float(0x450c1681), SkBits2Float(0x44b38772), SkBits2Float(0x450c25e0), SkBits2Float(0x44b36edb));  // 2241.41f, 1436.23f, 2242.37f, 1435.46f
  path.quadTo(SkBits2Float(0x450c2cc3), SkBits2Float(0x44b363d7), SkBits2Float(0x450c3ded), SkBits2Float(0x44b35cf9));  // 2242.8f, 1435.12f, 2243.87f, 1434.91f
  path.lineTo(SkBits2Float(0x450d04b4), SkBits2Float(0x44b30d76));  // 2256.29f, 1432.42f
  path.quadTo(SkBits2Float(0x450d9a04), SkBits2Float(0x44b2d1c0), SkBits2Float(0x450e73f0), SkBits2Float(0x44b26ea8));  // 2265.63f, 1430.55f, 2279.25f, 1427.46f
  path.quadTo(SkBits2Float(0x450f4f90), SkBits2Float(0x44b20ad0), SkBits2Float(0x45119b65), SkBits2Float(0x44b19337));  // 2292.97f, 1424.34f, 2329.71f, 1420.6f
  path.quadTo(SkBits2Float(0x4513e5a0), SkBits2Float(0x44b11c00), SkBits2Float(0x451750dc), SkBits2Float(0x44b090b9));  // 2366.35f, 1416.88f, 2421.05f, 1412.52f
  path.quadTo(SkBits2Float(0x451abcc0), SkBits2Float(0x44b00580), SkBits2Float(0x451e316b), SkBits2Float(0x44aff193));  // 2475.8f, 1408.17f, 2531.09f, 1407.55f
  path.quadTo(SkBits2Float(0x4521a8c0), SkBits2Float(0x44afddc0), SkBits2Float(0x45242250), SkBits2Float(0x44afddb1));  // 2586.55f, 1406.93f, 2626.14f, 1406.93f
  path.quadTo(SkBits2Float(0x45269f20), SkBits2Float(0x44afddc0), SkBits2Float(0x452734d1), SkBits2Float(0x44aff1a9));  // 2665.95f, 1406.93f, 2675.3f, 1407.55f
  path.quadTo(SkBits2Float(0x4527835a), SkBits2Float(0x44affc24), SkBits2Float(0x452795d6), SkBits2Float(0x44b00e9d));  // 2680.21f, 1407.88f, 2681.36f, 1408.46f
  path.quadTo(SkBits2Float(0x4527a48d), SkBits2Float(0x44b01d54), SkBits2Float(0x4527a48d), SkBits2Float(0x44b039f1));  // 2682.28f, 1408.92f, 2682.28f, 1409.81f
  path.quadTo(SkBits2Float(0x4527a48d), SkBits2Float(0x44b054ce), SkBits2Float(0x452796f5), SkBits2Float(0x44b064aa));  // 2682.28f, 1410.65f, 2681.43f, 1411.15f
  path.quadTo(SkBits2Float(0x45278cd2), SkBits2Float(0x44b0707d), SkBits2Float(0x45277370), SkBits2Float(0x44b07bc5));  // 2680.8f, 1411.52f, 2679.21f, 1411.87f
  path.quadTo(SkBits2Float(0x45271888), SkBits2Float(0x44b0a42c), SkBits2Float(0x452600cd), SkBits2Float(0x44b0cc20));  // 2673.53f, 1413.13f, 2656.05f, 1414.38f
  path.lineTo(SkBits2Float(0x45231749), SkBits2Float(0x44b12f87));  // 2609.46f, 1417.49f
  path.quadTo(SkBits2Float(0x45214340), SkBits2Float(0x44b16b40), SkBits2Float(0x451f163e), SkBits2Float(0x44b16b41));  // 2580.2f, 1419.35f, 2545.39f, 1419.35f
  path.lineTo(SkBits2Float(0x451872ac), SkBits2Float(0x44b16b41));  // 2439.17f, 1419.35f
  path.lineTo(SkBits2Float(0x4513ddde), SkBits2Float(0x44b16b41));  // 2365.87f, 1419.35f
  path.lineTo(SkBits2Float(0x4513d5bc), SkBits2Float(0x44b16b41));  // 2365.36f, 1419.35f
  path.lineTo(SkBits2Float(0x4513d5bc), SkBits2Float(0x44b1437f));  // 2365.36f, 1418.11f
  path.lineTo(SkBits2Float(0x4513d5bc), SkBits2Float(0x44b11bbd));  // 2365.36f, 1416.87f
  path.lineTo(SkBits2Float(0x45140f90), SkBits2Float(0x44b11bbd));  // 2368.97f, 1416.87f
  path.quadTo(SkBits2Float(0x45145e00), SkBits2Float(0x44b11bbc), SkBits2Float(0x4515a543), SkBits2Float(0x44b0e044));  // 2373.88f, 1416.87f, 2394.33f, 1415.01f
  path.quadTo(SkBits2Float(0x4516ee60), SkBits2Float(0x44b0a480), SkBits2Float(0x45191aec), SkBits2Float(0x44b07cbd));  // 2414.9f, 1413.14f, 2449.68f, 1411.9f
  path.quadTo(SkBits2Float(0x451b4740), SkBits2Float(0x44b05520), SkBits2Float(0x451e6d08), SkBits2Float(0x44b04115));  // 2484.45f, 1410.66f, 2534.81f, 1410.03f
  path.quadTo(SkBits2Float(0x45218fc0), SkBits2Float(0x44b02d40), SkBits2Float(0x4524a384), SkBits2Float(0x44b02d34));  // 2584.98f, 1409.41f, 2634.22f, 1409.41f
  path.lineTo(SkBits2Float(0x452a588e), SkBits2Float(0x44b02d34));  // 2725.53f, 1409.41f
  path.quadTo(SkBits2Float(0x452cfda0), SkBits2Float(0x44b02d40), SkBits2Float(0x452e1582), SkBits2Float(0x44b07d1e));  // 2767.85f, 1409.41f, 2785.34f, 1411.91f
  path.quadTo(SkBits2Float(0x452ea188), SkBits2Float(0x44b0a518), SkBits2Float(0x452ee09a), SkBits2Float(0x44b0c363));  // 2794.1f, 1413.16f, 2798.04f, 1414.11f
  path.quadTo(SkBits2Float(0x452f0212), SkBits2Float(0x44b0d374), SkBits2Float(0x452f1015), SkBits2Float(0x44b0e233));  // 2800.13f, 1414.61f, 2801.01f, 1415.07f
  path.quadTo(SkBits2Float(0x452f1c90), SkBits2Float(0x44b0ef56), SkBits2Float(0x452f2057), SkBits2Float(0x44b102c5));  // 2801.79f, 1415.48f, 2802.02f, 1416.09f
  path.quadTo(SkBits2Float(0x452f2736), SkBits2Float(0x44b12622), SkBits2Float(0x452f1633), SkBits2Float(0x44b13cd2));  // 2802.45f, 1417.19f, 2801.39f, 1417.9f
  path.quadTo(SkBits2Float(0x452ef48c), SkBits2Float(0x44b169b1), SkBits2Float(0x452dc5d1), SkBits2Float(0x44b1ba6a));  // 2799.28f, 1419.3f, 2780.36f, 1421.83f
  path.quadTo(SkBits2Float(0x452c9a90), SkBits2Float(0x44b20a30), SkBits2Float(0x452a0095), SkBits2Float(0x44b26d8e));  // 2761.66f, 1424.32f, 2720.04f, 1427.42f
  path.quadTo(SkBits2Float(0x452766c0), SkBits2Float(0x44b2d0e0), SkBits2Float(0x4521ec5a), SkBits2Float(0x44b3208d));  // 2678.42f, 1430.53f, 2590.77f, 1433.02f
  path.quadTo(SkBits2Float(0x451c7480), SkBits2Float(0x44b37040), SkBits2Float(0x45193974), SkBits2Float(0x44b37015));  // 2503.28f, 1435.51f, 2451.59f, 1435.5f
  path.lineTo(SkBits2Float(0x4513f1c0), SkBits2Float(0x44b37015));  // 2367.11f, 1435.5f
  path.lineTo(SkBits2Float(0x451175a6), SkBits2Float(0x44b37015));  // 2327.35f, 1435.5f
  path.quadTo(SkBits2Float(0x451150c4), SkBits2Float(0x44b37014), SkBits2Float(0x4511484f), SkBits2Float(0x44b35af3));  // 2325.05f, 1435.5f, 2324.52f, 1434.84f
  path.quadTo(SkBits2Float(0x45113ccd), SkBits2Float(0x44b33e2d), SkBits2Float(0x451149cd), SkBits2Float(0x44b32316));  // 2323.8f, 1433.94f, 2324.61f, 1433.1f
  path.quadTo(SkBits2Float(0x45114d4a), SkBits2Float(0x44b31bd2), SkBits2Float(0x4511531f), SkBits2Float(0x44b3165a));  // 2324.83f, 1432.87f, 2325.2f, 1432.7f
  path.quadTo(SkBits2Float(0x45115999), SkBits2Float(0x44b31047), SkBits2Float(0x45116664), SkBits2Float(0x44b3092c));  // 2325.6f, 1432.51f, 2326.4f, 1432.29f
  path.quadTo(SkBits2Float(0x45119422), SkBits2Float(0x44b2efc2), SkBits2Float(0x4512252c), SkBits2Float(0x44b2bdc1));  // 2329.26f, 1431.49f, 2338.32f, 1429.93f
  path.quadTo(SkBits2Float(0x45134648), SkBits2Float(0x44b25a10), SkBits2Float(0x451449ac), SkBits2Float(0x44b23226));  // 2356.39f, 1426.81f, 2372.6f, 1425.57f
  path.quadTo(SkBits2Float(0x45154cc0), SkBits2Float(0x44b20a40), SkBits2Float(0x45182c4b), SkBits2Float(0x44b1cead));  // 2388.8f, 1424.32f, 2434.77f, 1422.46f
  path.quadTo(SkBits2Float(0x451b0bc0), SkBits2Float(0x44b19320), SkBits2Float(0x451e4549), SkBits2Float(0x44b17f22));  // 2480.73f, 1420.6f, 2532.33f, 1419.97f
  path.quadTo(SkBits2Float(0x45217bc0), SkBits2Float(0x44b16b40), SkBits2Float(0x4524a385), SkBits2Float(0x44b16b40));  // 2583.73f, 1419.35f, 2634.22f, 1419.35f
  path.quadTo(SkBits2Float(0x4527ca20), SkBits2Float(0x44b16b40), SkBits2Float(0x452a1d40), SkBits2Float(0x44b17f23));  // 2684.63f, 1419.35f, 2721.83f, 1419.97f
  path.lineTo(SkBits2Float(0x452c5e5e), SkBits2Float(0x44b192ff));  // 2757.9f, 1420.59f
  path.conicTo(SkBits2Float(0x452c723e), SkBits2Float(0x44b193ae), SkBits2Float(0x452c71e6), SkBits2Float(0x44b1bb6e), SkBits2Float(0x3f3504f3));  // 2759.14f, 1420.61f, 2759.12f, 1421.86f, 0.707107f
  path.conicTo(SkBits2Float(0x452c7191), SkBits2Float(0x44b1e1e9), SkBits2Float(0x452c5e54), SkBits2Float(0x44b1e27f), SkBits2Float(0x3f37eea2));  // 2759.1f, 1423.06f, 2757.9f, 1423.08f, 0.718485f
  path.lineTo(SkBits2Float(0x452c5db0), SkBits2Float(0x44b1e284));  // 2757.86f, 1423.08f
  path.lineTo(SkBits2Float(0x4520cb8e), SkBits2Float(0x44b1e286));  // 2572.72f, 1423.08f
  path.lineTo(SkBits2Float(0x4513ddde), SkBits2Float(0x44b1e286));  // 2365.87f, 1423.08f
  path.quadTo(SkBits2Float(0x45131dc0), SkBits2Float(0x44b1e288), SkBits2Float(0x45131574), SkBits2Float(0x44b1d1e8));  // 2353.86f, 1423.08f, 2353.34f, 1422.56f
  path.quadTo(SkBits2Float(0x45130e6c), SkBits2Float(0x44b1c3d8), SkBits2Float(0x45130fc9), SkBits2Float(0x44b1b02f));  // 2352.9f, 1422.12f, 2352.99f, 1421.51f
  path.quadTo(SkBits2Float(0x451310e0), SkBits2Float(0x44b1a07c), SkBits2Float(0x45131712), SkBits2Float(0x44b19682));  // 2353.05f, 1421.02f, 2353.44f, 1420.7f
  path.quadTo(SkBits2Float(0x45131a35), SkBits2Float(0x44b19176), SkBits2Float(0x45131e3b), SkBits2Float(0x44b18f0f));  // 2353.64f, 1420.55f, 2353.89f, 1420.47f
  path.quadTo(SkBits2Float(0x4513203c), SkBits2Float(0x44b18ddc), SkBits2Float(0x45132300), SkBits2Float(0x44b18d19));  // 2354.01f, 1420.43f, 2354.19f, 1420.41f
  path.quadTo(SkBits2Float(0x45132630), SkBits2Float(0x44b18c37), SkBits2Float(0x45132c2b), SkBits2Float(0x44b18b59));  // 2354.39f, 1420.38f, 2354.76f, 1420.35f
  path.quadTo(SkBits2Float(0x45133720), SkBits2Float(0x44b189c2), SkBits2Float(0x45134d89), SkBits2Float(0x44b187dd));  // 2355.45f, 1420.3f, 2356.85f, 1420.25f
  path.quadTo(SkBits2Float(0x45137981), SkBits2Float(0x44b18426), SkBits2Float(0x4513d361), SkBits2Float(0x44b17f25));  // 2359.59f, 1420.13f, 2365.21f, 1419.97f
  path.quadTo(SkBits2Float(0x45153900), SkBits2Float(0x44b16b40), SkBits2Float(0x45176652), SkBits2Float(0x44b16b40));  // 2387.56f, 1419.35f, 2422.4f, 1419.35f
  path.lineTo(SkBits2Float(0x451ccbd7), SkBits2Float(0x44b16b40));  // 2508.74f, 1419.35f
  path.quadTo(SkBits2Float(0x452006e0), SkBits2Float(0x44b16b40), SkBits2Float(0x452542d6), SkBits2Float(0x44b19304));  // 2560.43f, 1419.35f, 2644.18f, 1420.59f
  path.lineTo(SkBits2Float(0x452c0446), SkBits2Float(0x44b1cea6));  // 2752.27f, 1422.46f
  path.quadTo(SkBits2Float(0x452d87d0), SkBits2Float(0x44b1e280), SkBits2Float(0x452e9e8e), SkBits2Float(0x44b1f66d));  // 2776.49f, 1423.08f, 2793.91f, 1423.7f
  path.quadTo(SkBits2Float(0x452fb4a0), SkBits2Float(0x44b20a50), SkBits2Float(0x45305329), SkBits2Float(0x44b20a46));  // 2811.29f, 1424.32f, 2821.2f, 1424.32f
  path.quadTo(SkBits2Float(0x4530f3d0), SkBits2Float(0x44b20a48), SkBits2Float(0x45313117), SkBits2Float(0x44b21eb2));  // 2831.24f, 1424.32f, 2835.07f, 1424.96f
  path.quadTo(SkBits2Float(0x453140fe), SkBits2Float(0x44b223ff), SkBits2Float(0x453148cb), SkBits2Float(0x44b22840));  // 2836.06f, 1425.12f, 2836.55f, 1425.26f
  path.quadTo(SkBits2Float(0x45314e86), SkBits2Float(0x44b22b60), SkBits2Float(0x45315208), SkBits2Float(0x44b22f46));  // 2836.91f, 1425.36f, 2837.13f, 1425.48f
  path.quadTo(SkBits2Float(0x453157b2), SkBits2Float(0x44b23592), SkBits2Float(0x45315aa6), SkBits2Float(0x44b24033));  // 2837.48f, 1425.67f, 2837.67f, 1426.01f
  path.quadTo(SkBits2Float(0x4531621f), SkBits2Float(0x44b25b1c), SkBits2Float(0x45315735), SkBits2Float(0x44b270ef));  // 2838.13f, 1426.85f, 2837.45f, 1427.53f
  path.quadTo(SkBits2Float(0x45314ee6), SkBits2Float(0x44b2818c), SkBits2Float(0x453123e1), SkBits2Float(0x44b2818c));  // 2836.93f, 1428.05f, 2834.24f, 1428.05f
  path.lineTo(SkBits2Float(0x45304938), SkBits2Float(0x44b2818c));  // 2820.58f, 1428.05f
  path.lineTo(SkBits2Float(0x45304938), SkBits2Float(0x44b23208));  // 2820.58f, 1425.56f
  path.lineTo(SkBits2Float(0x453123e1), SkBits2Float(0x44b23208));  // 2834.24f, 1425.56f
  path.quadTo(SkBits2Float(0x4531349f), SkBits2Float(0x44b23208), SkBits2Float(0x45313d44), SkBits2Float(0x44b230e1));  // 2835.29f, 1425.56f, 2835.83f, 1425.53f
  path.quadTo(SkBits2Float(0x453140d8), SkBits2Float(0x44b23066), SkBits2Float(0x45314270), SkBits2Float(0x44b22fd7));  // 2836.05f, 1425.51f, 2836.15f, 1425.49f
  path.quadTo(SkBits2Float(0x45313ee9), SkBits2Float(0x44b23115), SkBits2Float(0x45313b19), SkBits2Float(0x44b238b5));  // 2835.93f, 1425.53f, 2835.69f, 1425.77f
  path.quadTo(SkBits2Float(0x453130cd), SkBits2Float(0x44b24d48), SkBits2Float(0x453137e4), SkBits2Float(0x44b266cd));  // 2835.05f, 1426.42f, 2835.49f, 1427.21f
  path.quadTo(SkBits2Float(0x45313a75), SkBits2Float(0x44b27008), SkBits2Float(0x45313eb8), SkBits2Float(0x44b274c6));  // 2835.65f, 1427.5f, 2835.92f, 1427.65f
  path.lineTo(SkBits2Float(0x45313e55), SkBits2Float(0x44b274f6));  // 2835.9f, 1427.66f
  path.quadTo(SkBits2Float(0x45313879), SkBits2Float(0x44b271c4), SkBits2Float(0x45312a8d), SkBits2Float(0x44b26d20));  // 2835.53f, 1427.56f, 2834.66f, 1427.41f
  path.quadTo(SkBits2Float(0x4530f088), SkBits2Float(0x44b259c8), SkBits2Float(0x45305329), SkBits2Float(0x44b259ca));  // 2831.03f, 1426.81f, 2821.2f, 1426.81f
  path.quadTo(SkBits2Float(0x452fb400), SkBits2Float(0x44b259c0), SkBits2Float(0x452e9d22), SkBits2Float(0x44b245e3));  // 2811.25f, 1426.8f, 2793.82f, 1426.18f
  path.quadTo(SkBits2Float(0x452d8690), SkBits2Float(0x44b23200), SkBits2Float(0x452c0396), SkBits2Float(0x44b21e26));  // 2776.41f, 1425.56f, 2752.22f, 1424.94f
  path.lineTo(SkBits2Float(0x45254240), SkBits2Float(0x44b1e284));  // 2644.14f, 1423.08f
  path.quadTo(SkBits2Float(0x452006e0), SkBits2Float(0x44b1bac0), SkBits2Float(0x451ccbd7), SkBits2Float(0x44b1bac4));  // 2560.43f, 1421.84f, 2508.74f, 1421.84f
  path.lineTo(SkBits2Float(0x45176652), SkBits2Float(0x44b1bac4));  // 2422.4f, 1421.84f
  path.quadTo(SkBits2Float(0x45153980), SkBits2Float(0x44b1bac0), SkBits2Float(0x4513d47b), SkBits2Float(0x44b1cea1));  // 2387.59f, 1421.84f, 2365.28f, 1422.46f
  path.quadTo(SkBits2Float(0x45137aac), SkBits2Float(0x44b1d39f), SkBits2Float(0x45134f37), SkBits2Float(0x44b1d74d));  // 2359.67f, 1422.61f, 2356.95f, 1422.73f
  path.quadTo(SkBits2Float(0x45133979), SkBits2Float(0x44b1d924), SkBits2Float(0x45132f0b), SkBits2Float(0x44b1daa7));  // 2355.59f, 1422.79f, 2354.94f, 1422.83f
  path.quadTo(SkBits2Float(0x45132a54), SkBits2Float(0x44b1db56), SkBits2Float(0x45132874), SkBits2Float(0x44b1dbdb));  // 2354.65f, 1422.85f, 2354.53f, 1422.87f
  path.quadTo(SkBits2Float(0x451328b1), SkBits2Float(0x44b1dbca), SkBits2Float(0x45132997), SkBits2Float(0x44b1db41));  // 2354.54f, 1422.87f, 2354.6f, 1422.85f
  path.quadTo(SkBits2Float(0x45132d10), SkBits2Float(0x44b1d92f), SkBits2Float(0x45133000), SkBits2Float(0x44b1d474));  // 2354.82f, 1422.79f, 2355, 1422.64f
  path.quadTo(SkBits2Float(0x45133617), SkBits2Float(0x44b1caa7), SkBits2Float(0x4513372b), SkBits2Float(0x44b1bb19));  // 2355.38f, 1422.33f, 2355.45f, 1421.85f
  path.quadTo(SkBits2Float(0x45133885), SkBits2Float(0x44b1a797), SkBits2Float(0x45133192), SkBits2Float(0x44b199b0));  // 2355.53f, 1421.24f, 2355.1f, 1420.8f
  path.quadTo(SkBits2Float(0x45132d3c), SkBits2Float(0x44b19104), SkBits2Float(0x451327d6), SkBits2Float(0x44b18f32));  // 2354.83f, 1420.53f, 2354.49f, 1420.47f
  path.quadTo(SkBits2Float(0x451327e7), SkBits2Float(0x44b18f37), SkBits2Float(0x451328ae), SkBits2Float(0x44b18f5a));  // 2354.49f, 1420.48f, 2354.54f, 1420.48f
  path.quadTo(SkBits2Float(0x45132b59), SkBits2Float(0x44b18fd0), SkBits2Float(0x451330dd), SkBits2Float(0x44b1903e));  // 2354.71f, 1420.49f, 2355.05f, 1420.51f
  path.quadTo(SkBits2Float(0x45133c91), SkBits2Float(0x44b19127), SkBits2Float(0x4513539c), SkBits2Float(0x44b191c5));  // 2355.79f, 1420.54f, 2357.23f, 1420.56f
  path.quadTo(SkBits2Float(0x451381f0), SkBits2Float(0x44b19300), SkBits2Float(0x4513ddde), SkBits2Float(0x44b19302));  // 2360.12f, 1420.59f, 2365.87f, 1420.59f
  path.lineTo(SkBits2Float(0x4520cb8e), SkBits2Float(0x44b19302));  // 2572.72f, 1420.59f
  path.lineTo(SkBits2Float(0x452c5d14), SkBits2Float(0x44b19304));  // 2757.82f, 1420.59f
  path.lineTo(SkBits2Float(0x452c5db8), SkBits2Float(0x44b192ff));  // 2757.86f, 1420.59f
  path.lineTo(SkBits2Float(0x452c5e06), SkBits2Float(0x44b1babf));  // 2757.88f, 1421.84f
  path.lineTo(SkBits2Float(0x452c5dae), SkBits2Float(0x44b1e27f));  // 2757.85f, 1423.08f
  path.lineTo(SkBits2Float(0x452a1c96), SkBits2Float(0x44b1cea3));  // 2721.79f, 1422.46f
  path.quadTo(SkBits2Float(0x4527c9e0), SkBits2Float(0x44b1bac0), SkBits2Float(0x4524a385), SkBits2Float(0x44b1bac4));  // 2684.62f, 1421.84f, 2634.22f, 1421.84f
  path.quadTo(SkBits2Float(0x45217be0), SkBits2Float(0x44b1bac0), SkBits2Float(0x451e45c3), SkBits2Float(0x44b1cea4));  // 2583.74f, 1421.84f, 2532.36f, 1422.46f
  path.quadTo(SkBits2Float(0x451b0cc0), SkBits2Float(0x44b1e280), SkBits2Float(0x45182de7), SkBits2Float(0x44b21e1f));  // 2480.8f, 1423.08f, 2434.87f, 1424.94f
  path.quadTo(SkBits2Float(0x45154f20), SkBits2Float(0x44b259a0), SkBits2Float(0x45144cb8), SkBits2Float(0x44b2816e));  // 2388.95f, 1426.8f, 2372.79f, 1428.04f
  path.quadTo(SkBits2Float(0x45134b38), SkBits2Float(0x44b2a910), SkBits2Float(0x45122bee), SkBits2Float(0x44b30c1d));  // 2356.7f, 1429.28f, 2338.75f, 1432.38f
  path.quadTo(SkBits2Float(0x45119cd2), SkBits2Float(0x44b33d74), SkBits2Float(0x45117108), SkBits2Float(0x44b355c8));  // 2329.8f, 1433.92f, 2327.06f, 1434.68f
  path.quadTo(SkBits2Float(0x45116774), SkBits2Float(0x44b35b1a), SkBits2Float(0x451163ff), SkBits2Float(0x44b35e58));  // 2326.47f, 1434.85f, 2326.25f, 1434.95f
  path.quadTo(SkBits2Float(0x451164da), SkBits2Float(0x44b35d8a), SkBits2Float(0x4511667b), SkBits2Float(0x44b35a26));  // 2326.3f, 1434.92f, 2326.41f, 1434.82f
  path.quadTo(SkBits2Float(0x4511719e), SkBits2Float(0x44b342f1), SkBits2Float(0x4511675b), SkBits2Float(0x44b32947));  // 2327.1f, 1434.09f, 2326.46f, 1433.29f
  path.quadTo(SkBits2Float(0x451163e6), SkBits2Float(0x44b320a3), SkBits2Float(0x45115fc6), SkBits2Float(0x44b31e6d));  // 2326.24f, 1433.02f, 2325.99f, 1432.95f
  path.quadTo(SkBits2Float(0x4511606e), SkBits2Float(0x44b31ec7), SkBits2Float(0x45116296), SkBits2Float(0x44b31f3a));  // 2326.03f, 1432.96f, 2326.16f, 1432.98f
  path.quadTo(SkBits2Float(0x45116905), SkBits2Float(0x44b32091), SkBits2Float(0x451175a6), SkBits2Float(0x44b32091));  // 2326.56f, 1433.02f, 2327.35f, 1433.02f
  path.lineTo(SkBits2Float(0x4513f1c0), SkBits2Float(0x44b32091));  // 2367.11f, 1433.02f
  path.lineTo(SkBits2Float(0x45193974), SkBits2Float(0x44b32091));  // 2451.59f, 1433.02f
  path.quadTo(SkBits2Float(0x451c7400), SkBits2Float(0x44b32080), SkBits2Float(0x4521eb3a), SkBits2Float(0x44b2d113));  // 2503.25f, 1433.02f, 2590.7f, 1430.53f
  path.quadTo(SkBits2Float(0x452764a0), SkBits2Float(0x44b28180), SkBits2Float(0x4529fd9f), SkBits2Float(0x44b21e44));  // 2678.29f, 1428.05f, 2719.85f, 1424.95f
  path.quadTo(SkBits2Float(0x452c9670), SkBits2Float(0x44b1bb10), SkBits2Float(0x452dc08f), SkBits2Float(0x44b16b9a));  // 2761.4f, 1421.85f, 2780.03f, 1419.36f
  path.quadTo(SkBits2Float(0x452ee628), SkBits2Float(0x44b11d50), SkBits2Float(0x452f0025), SkBits2Float(0x44b0faaa));  // 2798.38f, 1416.92f, 2800.01f, 1415.83f
  path.quadTo(SkBits2Float(0x452ef695), SkBits2Float(0x44b1076a), SkBits2Float(0x452efb49), SkBits2Float(0x44b11f97));  // 2799.41f, 1416.23f, 2799.71f, 1416.99f
  path.quadTo(SkBits2Float(0x452efce4), SkBits2Float(0x44b127d9), SkBits2Float(0x452efd91), SkBits2Float(0x44b1288f));  // 2799.81f, 1417.25f, 2799.85f, 1417.27f
  path.quadTo(SkBits2Float(0x452ef3f8), SkBits2Float(0x44b11e76), SkBits2Float(0x452ed752), SkBits2Float(0x44b110b5));  // 2799.25f, 1416.95f, 2797.46f, 1416.52f
  path.quadTo(SkBits2Float(0x452e9a08), SkBits2Float(0x44b0f348), SkBits2Float(0x452e0fe2), SkBits2Float(0x44b0cbd4));  // 2793.63f, 1415.6f, 2784.99f, 1414.37f
  path.quadTo(SkBits2Float(0x452cfb00), SkBits2Float(0x44b07cc0), SkBits2Float(0x452a588e), SkBits2Float(0x44b07cb8));  // 2767.69f, 1411.9f, 2725.53f, 1411.9f
  path.lineTo(SkBits2Float(0x4524a384), SkBits2Float(0x44b07cb8));  // 2634.22f, 1411.9f
  path.quadTo(SkBits2Float(0x45219000), SkBits2Float(0x44b07ca0), SkBits2Float(0x451e6d86), SkBits2Float(0x44b09097));  // 2585, 1411.89f, 2534.85f, 1412.52f
  path.quadTo(SkBits2Float(0x451b4820), SkBits2Float(0x44b0a4a0), SkBits2Float(0x45191c58), SkBits2Float(0x44b0cc33));  // 2484.51f, 1413.14f, 2449.77f, 1414.38f
  path.quadTo(SkBits2Float(0x4516f0e0), SkBits2Float(0x44b0f3e0), SkBits2Float(0x4515a8dd), SkBits2Float(0x44b12f74));  // 2415.05f, 1415.62f, 2394.55f, 1417.48f
  path.quadTo(SkBits2Float(0x45145fcc), SkBits2Float(0x44b16b40), SkBits2Float(0x45140f90), SkBits2Float(0x44b16b41));  // 2373.99f, 1419.35f, 2368.97f, 1419.35f
  path.lineTo(SkBits2Float(0x4513d5bc), SkBits2Float(0x44b16b41));  // 2365.36f, 1419.35f
  path.conicTo(SkBits2Float(0x4513c1db), SkBits2Float(0x44b16b41), SkBits2Float(0x4513c1db), SkBits2Float(0x44b1437f), SkBits2Float(0x3f3504f3));  // 2364.12f, 1419.35f, 2364.12f, 1418.11f, 0.707107f
  path.conicTo(SkBits2Float(0x4513c1db), SkBits2Float(0x44b11bbd), SkBits2Float(0x4513d5bc), SkBits2Float(0x44b11bbd), SkBits2Float(0x3f3504f3));  // 2364.12f, 1416.87f, 2365.36f, 1416.87f, 0.707107f
  path.lineTo(SkBits2Float(0x4513ddde), SkBits2Float(0x44b11bbd));  // 2365.87f, 1416.87f
  path.lineTo(SkBits2Float(0x451872ac), SkBits2Float(0x44b11bbd));  // 2439.17f, 1416.87f
  path.lineTo(SkBits2Float(0x451f163e), SkBits2Float(0x44b11bbd));  // 2545.39f, 1416.87f
  path.quadTo(SkBits2Float(0x45214200), SkBits2Float(0x44b11bd0), SkBits2Float(0x452314a3), SkBits2Float(0x44b0e031));  // 2580.12f, 1416.87f, 2609.29f, 1415.01f
  path.lineTo(SkBits2Float(0x4525fdf7), SkBits2Float(0x44b07cd0));  // 2655.87f, 1411.9f
  path.quadTo(SkBits2Float(0x452712c8), SkBits2Float(0x44b05548), SkBits2Float(0x45276ad0), SkBits2Float(0x44b02e27));  // 2673.17f, 1410.67f, 2678.68f, 1409.44f
  path.quadTo(SkBits2Float(0x45277e26), SkBits2Float(0x44b0258f), SkBits2Float(0x452782ed), SkBits2Float(0x44b01ffc));  // 2679.88f, 1409.17f, 2680.18f, 1409
  path.quadTo(SkBits2Float(0x45277ccc), SkBits2Float(0x44b02722), SkBits2Float(0x45277ccb), SkBits2Float(0x44b039ef));  // 2679.8f, 1409.22f, 2679.8f, 1409.81f
  path.quadTo(SkBits2Float(0x45277ccc), SkBits2Float(0x44b04e78), SkBits2Float(0x4527840e), SkBits2Float(0x44b055bb));  // 2679.8f, 1410.45f, 2680.25f, 1410.68f
  path.quadTo(SkBits2Float(0x452778ba), SkBits2Float(0x44b04a66), SkBits2Float(0x4527322b), SkBits2Float(0x44b040ff));  // 2679.55f, 1410.32f, 2675.14f, 1410.03f
  path.quadTo(SkBits2Float(0x45269dc0), SkBits2Float(0x44b02d40), SkBits2Float(0x45242250), SkBits2Float(0x44b02d35));  // 2665.86f, 1409.41f, 2626.14f, 1409.41f
  path.quadTo(SkBits2Float(0x4521a900), SkBits2Float(0x44b02d40), SkBits2Float(0x451e31dd), SkBits2Float(0x44b04115));  // 2586.56f, 1409.41f, 2531.12f, 1410.03f
  path.quadTo(SkBits2Float(0x451abe80), SkBits2Float(0x44b054e0), SkBits2Float(0x45175404), SkBits2Float(0x44b0dffb));  // 2475.91f, 1410.65f, 2421.25f, 1415
  path.quadTo(SkBits2Float(0x4513e960), SkBits2Float(0x44b16b20), SkBits2Float(0x45119f6b), SkBits2Float(0x44b1e251));  // 2366.59f, 1419.35f, 2329.96f, 1423.07f
  path.quadTo(SkBits2Float(0x450f5600), SkBits2Float(0x44b25968), SkBits2Float(0x450e7cc0), SkBits2Float(0x44b2bc30));  // 2293.38f, 1426.79f, 2279.8f, 1429.88f
  path.quadTo(SkBits2Float(0x450da240), SkBits2Float(0x44b31f88), SkBits2Float(0x450d0c80), SkBits2Float(0x44b35b6e));  // 2266.14f, 1432.99f, 2256.78f, 1434.86f
  path.lineTo(SkBits2Float(0x450c45b9), SkBits2Float(0x44b3aaf1));  // 2244.36f, 1437.34f
  path.quadTo(SkBits2Float(0x450c4099), SkBits2Float(0x44b3acfe), SkBits2Float(0x450c3dca), SkBits2Float(0x44b3aeca));  // 2244.04f, 1437.41f, 2243.86f, 1437.46f
  path.lineTo(SkBits2Float(0x450c3eb6), SkBits2Float(0x44b3acf1));  // 2243.92f, 1437.4f
  path.quadTo(SkBits2Float(0x450c4afa), SkBits2Float(0x44b39953), SkBits2Float(0x450c4260), SkBits2Float(0x44b37dcd));  // 2244.69f, 1436.79f, 2244.15f, 1435.93f
  path.quadTo(SkBits2Float(0x450c3fb6), SkBits2Float(0x44b37545), SkBits2Float(0x450c3cc8), SkBits2Float(0x44b372ed));  // 2243.98f, 1435.66f, 2243.8f, 1435.59f
  path.quadTo(SkBits2Float(0x450c4697), SkBits2Float(0x44b37ac7), SkBits2Float(0x450c757e), SkBits2Float(0x44b38428));  // 2244.41f, 1435.84f, 2247.34f, 1436.13f
  path.quadTo(SkBits2Float(0x450cd7e0), SkBits2Float(0x44b397d8), SkBits2Float(0x450d7fdf), SkBits2Float(0x44b397d6));  // 2253.49f, 1436.75f, 2263.99f, 1436.74f
  path.quadTo(SkBits2Float(0x450e2aa0), SkBits2Float(0x44b397c0), SkBits2Float(0x45125acd), SkBits2Float(0x44b3d37c));  // 2274.66f, 1436.74f, 2341.68f, 1438.61f
  path.quadTo(SkBits2Float(0x45168b60), SkBits2Float(0x44b40f20), SkBits2Float(0x451a0075), SkBits2Float(0x44b422fc));  // 2408.71f, 1440.47f, 2464.03f, 1441.09f
  path.quadTo(SkBits2Float(0x451d7780), SkBits2Float(0x44b436e0), SkBits2Float(0x4520df70), SkBits2Float(0x44b436dc));  // 2519.47f, 1441.71f, 2573.96f, 1441.71f
  path.lineTo(SkBits2Float(0x4526bc3a), SkBits2Float(0x44b436dc));  // 2667.76f, 1441.71f
  path.quadTo(SkBits2Float(0x45292f50), SkBits2Float(0x44b436d0), SkBits2Float(0x452b0118), SkBits2Float(0x44b422fe));  // 2706.96f, 1441.71f, 2736.07f, 1441.09f
  path.quadTo(SkBits2Float(0x452cd3f4), SkBits2Float(0x44b40f10), SkBits2Float(0x452d4a44), SkBits2Float(0x44b3fb5e));  // 2765.25f, 1440.47f, 2772.64f, 1439.86f
  path.quadTo(SkBits2Float(0x452d6625), SkBits2Float(0x44b3f6b8), SkBits2Float(0x452d6cf9), SkBits2Float(0x44b3f2d2));  // 2774.38f, 1439.71f, 2774.81f, 1439.59f
  path.quadTo(SkBits2Float(0x452d6bc2), SkBits2Float(0x44b3f384), SkBits2Float(0x452d69c5), SkBits2Float(0x44b3f668));  // 2774.73f, 1439.61f, 2774.61f, 1439.7f
  path.quadTo(SkBits2Float(0x452d64e0), SkBits2Float(0x44b3fd86), SkBits2Float(0x452d6303), SkBits2Float(0x44b40975));  // 2774.3f, 1439.92f, 2774.19f, 1440.3f
  path.quadTo(SkBits2Float(0x452d6012), SkBits2Float(0x44b41c44), SkBits2Float(0x452d65f2), SkBits2Float(0x44b42bed));  // 2774, 1440.88f, 2774.37f, 1441.37f
  path.quadTo(SkBits2Float(0x452d6920), SkBits2Float(0x44b43466), SkBits2Float(0x452d6d11), SkBits2Float(0x44b43724));  // 2774.57f, 1441.64f, 2774.82f, 1441.72f
  path.quadTo(SkBits2Float(0x452d6c93), SkBits2Float(0x44b436cc), SkBits2Float(0x452d6ace), SkBits2Float(0x44b43617));  // 2774.79f, 1441.71f, 2774.68f, 1441.69f
  path.quadTo(SkBits2Float(0x452d536c), SkBits2Float(0x44b42cbc), SkBits2Float(0x452ce7a0), SkBits2Float(0x44b422f1));  // 2773.21f, 1441.4f, 2766.48f, 1441.09f
  path.quadTo(SkBits2Float(0x452c0d40), SkBits2Float(0x44b40f20), SkBits2Float(0x452973f4), SkBits2Float(0x44b40f1b));  // 2752.83f, 1440.47f, 2711.25f, 1440.47f
  path.lineTo(SkBits2Float(0x452347a8), SkBits2Float(0x44b40f1b));  // 2612.48f, 1440.47f
  path.lineTo(SkBits2Float(0x451c8644), SkBits2Float(0x44b40f1b));  // 2504.39f, 1440.47f
  path.quadTo(SkBits2Float(0x45195800), SkBits2Float(0x44b40f20), SkBits2Float(0x451749a0), SkBits2Float(0x44b44aad));  // 2453.5f, 1440.47f, 2420.6f, 1442.33f
  path.quadTo(SkBits2Float(0x45153c18), SkBits2Float(0x44b48630), SkBits2Float(0x45144e77), SkBits2Float(0x44b4d558));  // 2387.76f, 1444.19f, 2372.9f, 1446.67f
  path.quadTo(SkBits2Float(0x4514143d), SkBits2Float(0x44b4e8c0), SkBits2Float(0x4513fc1a), SkBits2Float(0x44b4f80e));  // 2369.26f, 1447.27f, 2367.76f, 1447.75f
  path.quadTo(SkBits2Float(0x4513f331), SkBits2Float(0x44b4fdb4), SkBits2Float(0x4513f20e), SkBits2Float(0x44b4ffa4));  // 2367.2f, 1447.93f, 2367.13f, 1447.99f
  path.quadTo(SkBits2Float(0x4513f728), SkBits2Float(0x44b4f6f0), SkBits2Float(0x4513f6fb), SkBits2Float(0x44b4e75d));  // 2367.45f, 1447.72f, 2367.44f, 1447.23f
  path.quadTo(SkBits2Float(0x4513f6ca), SkBits2Float(0x44b4d6d5), SkBits2Float(0x4513f0e0), SkBits2Float(0x44b4ce62));  // 2367.42f, 1446.71f, 2367.05f, 1446.45f
  path.quadTo(SkBits2Float(0x4513fd5c), SkBits2Float(0x44b4e040), SkBits2Float(0x4514916c), SkBits2Float(0x44b4e9c9));  // 2367.83f, 1447.01f, 2377.09f, 1447.31f
  path.lineTo(SkBits2Float(0x4516efb6), SkBits2Float(0x44b5118a));  // 2414.98f, 1448.55f
  path.quadTo(SkBits2Float(0x451819a0), SkBits2Float(0x44b52560), SkBits2Float(0x45197516), SkBits2Float(0x44b52565));  // 2433.6f, 1449.17f, 2455.32f, 1449.17f
  path.lineTo(SkBits2Float(0x45270bbc), SkBits2Float(0x44b52565));  // 2672.73f, 1449.17f
  path.lineTo(SkBits2Float(0x4533f96d), SkBits2Float(0x44b52565));  // 2879.59f, 1449.17f
  path.lineTo(SkBits2Float(0x4533f96d), SkBits2Float(0x44b574e9));  // 2879.59f, 1451.65f
  path.close();
  return path;
}

static SkPath getPath1() {
  SkPath path;
  path.setFillType(SkPathFillType::kWinding);
  path.moveTo(SkBits2Float(0x45304938), SkBits2Float(0x44b2818c));  // 2820.58f, 1428.05f
  path.lineTo(SkBits2Float(0x452ecf8a), SkBits2Float(0x44b2818c));  // 2796.97f, 1428.05f
  path.quadTo(SkBits2Float(0x452deb68), SkBits2Float(0x44b28188), SkBits2Float(0x452c999a), SkBits2Float(0x44b29567));  // 2782.71f, 1428.05f, 2761.6f, 1428.67f
  path.quadTo(SkBits2Float(0x452b4710), SkBits2Float(0x44b2a950), SkBits2Float(0x4529d758), SkBits2Float(0x44b2a94d));  // 2740.44f, 1429.29f, 2717.46f, 1429.29f
  path.lineTo(SkBits2Float(0x451cf398), SkBits2Float(0x44b2a94d));  // 2511.22f, 1429.29f
  path.lineTo(SkBits2Float(0x451143f4), SkBits2Float(0x44b2a94d));  // 2324.25f, 1429.29f
  path.quadTo(SkBits2Float(0x4511305a), SkBits2Float(0x44b2a94d), SkBits2Float(0x45112aea), SkBits2Float(0x44b2a3dc));  // 2323.02f, 1429.29f, 2322.68f, 1429.12f
  path.quadTo(SkBits2Float(0x45112591), SkBits2Float(0x44b29e83), SkBits2Float(0x45112290), SkBits2Float(0x44b2944a));  // 2322.35f, 1428.95f, 2322.16f, 1428.63f
  path.quadTo(SkBits2Float(0x45111c3a), SkBits2Float(0x44b27ebc), SkBits2Float(0x4511232b), SkBits2Float(0x44b269e9));  // 2321.76f, 1427.96f, 2322.2f, 1427.31f
  path.quadTo(SkBits2Float(0x451126e0), SkBits2Float(0x44b25ec9), SkBits2Float(0x45112d5a), SkBits2Float(0x44b25a29));  // 2322.43f, 1426.96f, 2322.83f, 1426.82f
  path.quadTo(SkBits2Float(0x45112f91), SkBits2Float(0x44b25894), SkBits2Float(0x451132b5), SkBits2Float(0x44b25788));  // 2322.97f, 1426.77f, 2323.17f, 1426.74f
  path.quadTo(SkBits2Float(0x45113620), SkBits2Float(0x44b25665), SkBits2Float(0x45113d02), SkBits2Float(0x44b25504));  // 2323.38f, 1426.7f, 2323.81f, 1426.66f
  path.quadTo(SkBits2Float(0x45115639), SkBits2Float(0x44b24ff9), SkBits2Float(0x4511a61b), SkBits2Float(0x44b245fc));  // 2325.39f, 1426.5f, 2330.38f, 1426.19f
  path.quadTo(SkBits2Float(0x4512449c), SkBits2Float(0x44b23228), SkBits2Float(0x4515116d), SkBits2Float(0x44b20a4a));  // 2340.29f, 1425.57f, 2385.09f, 1424.32f
  path.quadTo(SkBits2Float(0x4517dc40), SkBits2Float(0x44b1e280), SkBits2Float(0x451abd12), SkBits2Float(0x44b1e285));  // 2429.77f, 1423.08f, 2475.82f, 1423.08f
  path.lineTo(SkBits2Float(0x4520d57f), SkBits2Float(0x44b1e285));  // 2573.34f, 1423.08f
  path.quadTo(SkBits2Float(0x45240f80), SkBits2Float(0x44b1e280), SkBits2Float(0x45270252), SkBits2Float(0x44b20a4a));  // 2624.97f, 1423.08f, 2672.15f, 1424.32f
  path.quadTo(SkBits2Float(0x4529f640), SkBits2Float(0x44b23220), SkBits2Float(0x452c2d6d), SkBits2Float(0x44b29593));  // 2719.39f, 1425.57f, 2754.84f, 1428.67f
  path.quadTo(SkBits2Float(0x452e6440), SkBits2Float(0x44b2f900), SkBits2Float(0x452ee671), SkBits2Float(0x44b32108));  // 2790.27f, 1431.78f, 2798.4f, 1433.03f
  path.quadTo(SkBits2Float(0x452f6ab8), SkBits2Float(0x44b349b0), SkBits2Float(0x452f9546), SkBits2Float(0x44b37447));  // 2806.67f, 1434.3f, 2809.33f, 1435.63f
  path.quadTo(SkBits2Float(0x452fa2cd), SkBits2Float(0x44b381cf), SkBits2Float(0x452fa924), SkBits2Float(0x44b3909a));  // 2810.18f, 1436.06f, 2810.57f, 1436.52f
  path.quadTo(SkBits2Float(0x452fb6ab), SkBits2Float(0x44b3b02c), SkBits2Float(0x452fabd4), SkBits2Float(0x44b3d0af));  // 2811.42f, 1437.51f, 2810.74f, 1438.52f
  path.quadTo(SkBits2Float(0x452fa215), SkBits2Float(0x44b3edea), SkBits2Float(0x452f7bc4), SkBits2Float(0x44b3faaf));  // 2810.13f, 1439.43f, 2807.74f, 1439.83f
  path.quadTo(SkBits2Float(0x452f3eda), SkBits2Float(0x44b40efa), SkBits2Float(0x452dd79d), SkBits2Float(0x44b422f8));  // 2803.93f, 1440.47f, 2781.48f, 1441.09f
  path.quadTo(SkBits2Float(0x452c7200), SkBits2Float(0x44b436e0), SkBits2Float(0x4529af97), SkBits2Float(0x44b436dd));  // 2759.12f, 1441.71f, 2714.97f, 1441.71f
  path.lineTo(SkBits2Float(0x45212ef3), SkBits2Float(0x44b436dd));  // 2578.93f, 1441.71f
  path.lineTo(SkBits2Float(0x4519a6c8), SkBits2Float(0x44b436dd));  // 2458.42f, 1441.71f
  path.quadTo(SkBits2Float(0x4517dcb0), SkBits2Float(0x44b436d0), SkBits2Float(0x451631c0), SkBits2Float(0x44b422f9));  // 2429.79f, 1441.71f, 2403.11f, 1441.09f
  path.quadTo(SkBits2Float(0x45148750), SkBits2Float(0x44b40f20), SkBits2Float(0x45127810), SkBits2Float(0x44b40f1b));  // 2376.46f, 1440.47f, 2343.5f, 1440.47f
  path.quadTo(SkBits2Float(0x45106870), SkBits2Float(0x44b40f20), SkBits2Float(0x450ffa2b), SkBits2Float(0x44b3fb10));  // 2310.53f, 1440.47f, 2303.64f, 1439.85f
  path.quadTo(SkBits2Float(0x450fdcfc), SkBits2Float(0x44b3f5c2), SkBits2Float(0x450fce22), SkBits2Float(0x44b3ed46));  // 2301.81f, 1439.68f, 2300.88f, 1439.41f
  path.quadTo(SkBits2Float(0x450fae9a), SkBits2Float(0x44b3db41), SkBits2Float(0x450fb481), SkBits2Float(0x44b3ac04));  // 2298.91f, 1438.85f, 2299.28f, 1437.38f
  path.quadTo(SkBits2Float(0x450fb9e2), SkBits2Float(0x44b380fd), SkBits2Float(0x45100116), SkBits2Float(0x44b35d64));  // 2299.62f, 1436.03f, 2304.07f, 1434.92f
  path.quadTo(SkBits2Float(0x451079e4), SkBits2Float(0x44b32100), SkBits2Float(0x451212be), SkBits2Float(0x44b2d13f));  // 2311.62f, 1433.03f, 2337.17f, 1430.54f
  path.quadTo(SkBits2Float(0x4513aab0), SkBits2Float(0x44b281b0), SkBits2Float(0x45166d49), SkBits2Float(0x44b259ce));  // 2362.67f, 1428.05f, 2406.83f, 1426.81f
  path.quadTo(SkBits2Float(0x45192e60), SkBits2Float(0x44b23200), SkBits2Float(0x451c9a25), SkBits2Float(0x44b23208));  // 2450.9f, 1425.56f, 2505.63f, 1425.56f
  path.lineTo(SkBits2Float(0x4523972a), SkBits2Float(0x44b23208));  // 2617.45f, 1425.56f
  path.quadTo(SkBits2Float(0x45272980), SkBits2Float(0x44b23200), SkBits2Float(0x452b33fa), SkBits2Float(0x44b28193));  // 2674.59f, 1425.56f, 2739.25f, 1428.05f
  path.quadTo(SkBits2Float(0x452f4040), SkBits2Float(0x44b2d140), SkBits2Float(0x453013df), SkBits2Float(0x44b35e3f));  // 2804.02f, 1430.54f, 2817.24f, 1434.95f
  path.quadTo(SkBits2Float(0x453081d0), SkBits2Float(0x44b3a780), SkBits2Float(0x45309959), SkBits2Float(0x44b3e264));  // 2824.11f, 1437.23f, 2825.58f, 1439.07f
  path.quadTo(SkBits2Float(0x4530af13), SkBits2Float(0x44b418b5), SkBits2Float(0x453095ca), SkBits2Float(0x44b445ab));  // 2826.94f, 1440.77f, 2825.36f, 1442.18f
  path.quadTo(SkBits2Float(0x453086d2), SkBits2Float(0x44b46048), SkBits2Float(0x45306191), SkBits2Float(0x44b4717b));  // 2824.43f, 1443.01f, 2822.1f, 1443.55f
  path.quadTo(SkBits2Float(0x452fdf0c), SkBits2Float(0x44b4adb8), SkBits2Float(0x452e3c56), SkBits2Float(0x44b4fd77));  // 2813.94f, 1445.43f, 2787.77f, 1447.92f
  path.quadTo(SkBits2Float(0x452c9a00), SkBits2Float(0x44b54d20), SkBits2Float(0x452849c9), SkBits2Float(0x44b54d28));  // 2761.62f, 1450.41f, 2692.61f, 1450.41f
  path.lineTo(SkBits2Float(0x4522315c), SkBits2Float(0x44b54d28));  // 2595.08f, 1450.41f
  path.lineTo(SkBits2Float(0x451e8128), SkBits2Float(0x44b54d28));  // 2536.07f, 1450.41f
  path.quadTo(SkBits2Float(0x451c99e0), SkBits2Float(0x44b54d40), SkBits2Float(0x451ae3ed), SkBits2Float(0x44b5255b));  // 2505.62f, 1450.41f, 2478.25f, 1449.17f
  path.quadTo(SkBits2Float(0x45192ef0), SkBits2Float(0x44b4fda0), SkBits2Float(0x4517be29), SkBits2Float(0x44b4c1e0));  // 2450.93f, 1447.93f, 2427.89f, 1446.06f
  path.quadTo(SkBits2Float(0x45164e50), SkBits2Float(0x44b48640), SkBits2Float(0x451506d6), SkBits2Float(0x44b45e8b));  // 2404.89f, 1444.2f, 2384.43f, 1442.95f
  path.quadTo(SkBits2Float(0x4513bf80), SkBits2Float(0x44b436d0), SkBits2Float(0x45129538), SkBits2Float(0x44b422f6));  // 2363.97f, 1441.71f, 2345.33f, 1441.09f
  path.quadTo(SkBits2Float(0x45116b50), SkBits2Float(0x44b40f20), SkBits2Float(0x4510694c), SkBits2Float(0x44b40f1b));  // 2326.71f, 1440.47f, 2310.58f, 1440.47f
  path.quadTo(SkBits2Float(0x450fe7a4), SkBits2Float(0x44b40f1c), SkBits2Float(0x450faba1), SkBits2Float(0x44b40a1a));  // 2302.48f, 1440.47f, 2298.73f, 1440.32f
  path.quadTo(SkBits2Float(0x450f8cbf), SkBits2Float(0x44b40787), SkBits2Float(0x450f7f44), SkBits2Float(0x44b4037b));  // 2296.8f, 1440.24f, 2295.95f, 1440.11f
  path.quadTo(SkBits2Float(0x450f7548), SkBits2Float(0x44b4007c), SkBits2Float(0x450f7020), SkBits2Float(0x44b3fa78));  // 2295.33f, 1440.02f, 2295.01f, 1439.83f
  path.quadTo(SkBits2Float(0x450f6218), SkBits2Float(0x44b3ea17), SkBits2Float(0x450f6628), SkBits2Float(0x44b3cba1));  // 2294.13f, 1439.32f, 2294.38f, 1438.36f
  path.quadTo(SkBits2Float(0x450f689e), SkBits2Float(0x44b3b929), SkBits2Float(0x450f71df), SkBits2Float(0x44b3afe9));  // 2294.54f, 1437.79f, 2295.12f, 1437.5f
  path.quadTo(SkBits2Float(0x450f88ed), SkBits2Float(0x44b398dc), SkBits2Float(0x450fe5e5), SkBits2Float(0x44b38434));  // 2296.56f, 1436.78f, 2302.37f, 1436.13f
  path.quadTo(SkBits2Float(0x45103fe8), SkBits2Float(0x44b37034), SkBits2Float(0x4510fd57), SkBits2Float(0x44b35c42));  // 2307.99f, 1435.51f, 2319.83f, 1434.88f
  path.lineTo(SkBits2Float(0x451294ea), SkBits2Float(0x44b3347f));  // 2345.31f, 1433.64f
  path.quadTo(SkBits2Float(0x45137030), SkBits2Float(0x44b32098), SkBits2Float(0x4514a4a6), SkBits2Float(0x44b32091));  // 2359.01f, 1433.02f, 2378.29f, 1433.02f
  path.lineTo(SkBits2Float(0x451911b2), SkBits2Float(0x44b32091));  // 2449.11f, 1433.02f
  path.quadTo(SkBits2Float(0x451c4c60), SkBits2Float(0x44b32080), SkBits2Float(0x452022cb), SkBits2Float(0x44b33473));  // 2500.77f, 1433.02f, 2562.17f, 1433.64f
  path.quadTo(SkBits2Float(0x4523fc40), SkBits2Float(0x44b34880), SkBits2Float(0x4527488a), SkBits2Float(0x44b3abc8));  // 2623.77f, 1434.27f, 2676.53f, 1437.37f
  path.quadTo(SkBits2Float(0x452a96e0), SkBits2Float(0x44b40f60), SkBits2Float(0x452ca648), SkBits2Float(0x44b4c291));  // 2729.43f, 1440.48f, 2762.39f, 1446.08f
  path.quadTo(SkBits2Float(0x452daf20), SkBits2Float(0x44b51c80), SkBits2Float(0x452e0c68), SkBits2Float(0x44b55908));  // 2778.95f, 1448.89f, 2784.78f, 1450.78f
  path.quadTo(SkBits2Float(0x452e4488), SkBits2Float(0x44b57d70), SkBits2Float(0x452e4a80), SkBits2Float(0x44b5a863));  // 2788.28f, 1451.92f, 2788.66f, 1453.26f
  path.quadTo(SkBits2Float(0x452e5068), SkBits2Float(0x44b5d2e6), SkBits2Float(0x452e38bf), SkBits2Float(0x44b5eaf6));  // 2789.03f, 1454.59f, 2787.55f, 1455.34f
  path.quadTo(SkBits2Float(0x452e2d0a), SkBits2Float(0x44b5f6dd), SkBits2Float(0x452e165c), SkBits2Float(0x44b5ff5f));  // 2786.81f, 1455.71f, 2785.4f, 1455.98f
  path.quadTo(SkBits2Float(0x452d7698), SkBits2Float(0x44b63b50), SkBits2Float(0x452caede), SkBits2Float(0x44b6633f));  // 2775.41f, 1457.85f, 2762.93f, 1459.1f
  path.quadTo(SkBits2Float(0x452be790), SkBits2Float(0x44b68b18), SkBits2Float(0x452b2054), SkBits2Float(0x44b69f08));  // 2750.47f, 1460.35f, 2738.02f, 1460.97f
  path.quadTo(SkBits2Float(0x452a58f0), SkBits2Float(0x44b6b2f8), SkBits2Float(0x45294243), SkBits2Float(0x44b6b2f6));  // 2725.56f, 1461.59f, 2708.14f, 1461.59f
  path.lineTo(SkBits2Float(0x4524047e), SkBits2Float(0x44b6b2f6));  // 2624.28f, 1461.59f
  path.lineTo(SkBits2Float(0x451e1dc4), SkBits2Float(0x44b6b2f6));  // 2529.86f, 1461.59f
  path.quadTo(SkBits2Float(0x451c5e20), SkBits2Float(0x44b6b2f0), SkBits2Float(0x451abc20), SkBits2Float(0x44b68b28));  // 2501.88f, 1461.59f, 2475.76f, 1460.35f
  path.quadTo(SkBits2Float(0x45191b20), SkBits2Float(0x44b66370), SkBits2Float(0x4513f126), SkBits2Float(0x44b613ea));  // 2449.7f, 1459.11f, 2367.07f, 1456.62f
  path.quadTo(SkBits2Float(0x450ec6c0), SkBits2Float(0x44b5c440), SkBits2Float(0x450e0016), SkBits2Float(0x44b5b07f));  // 2284.42f, 1454.13f, 2272.01f, 1453.52f
  path.quadTo(SkBits2Float(0x450d9a88), SkBits2Float(0x44b5a658), SkBits2Float(0x450d7d20), SkBits2Float(0x44b5964c));  // 2265.66f, 1453.2f, 2263.82f, 1452.7f
  path.quadTo(SkBits2Float(0x450d71ec), SkBits2Float(0x44b59030), SkBits2Float(0x450d6c2c), SkBits2Float(0x44b58638));  // 2263.12f, 1452.51f, 2262.76f, 1452.19f
  path.quadTo(SkBits2Float(0x450d647c), SkBits2Float(0x44b578e4), SkBits2Float(0x450d6420), SkBits2Float(0x44b56588));  // 2262.28f, 1451.78f, 2262.26f, 1451.17f
  path.quadTo(SkBits2Float(0x450d63c4), SkBits2Float(0x44b5521c), SkBits2Float(0x450d6afc), SkBits2Float(0x44b543ad));  // 2262.24f, 1450.57f, 2262.69f, 1450.11f
  path.quadTo(SkBits2Float(0x450d736d), SkBits2Float(0x44b532cc), SkBits2Float(0x450d8f70), SkBits2Float(0x44b52659));  // 2263.21f, 1449.59f, 2264.96f, 1449.2f
  path.quadTo(SkBits2Float(0x450de9ec), SkBits2Float(0x44b4fe20), SkBits2Float(0x450f5acf), SkBits2Float(0x44b4ae5c));  // 2270.62f, 1447.94f, 2293.68f, 1445.45f
  path.quadTo(SkBits2Float(0x4510cb50), SkBits2Float(0x44b45eb0), SkBits2Float(0x4512db14), SkBits2Float(0x44b44abf));  // 2316.71f, 1442.96f, 2349.69f, 1442.34f
  path.quadTo(SkBits2Float(0x4514ea10), SkBits2Float(0x44b436e0), SkBits2Float(0x45184aea), SkBits2Float(0x44b436dc));  // 2382.63f, 1441.71f, 2436.68f, 1441.71f
  path.lineTo(SkBits2Float(0x451f8392), SkBits2Float(0x44b436dc));  // 2552.22f, 1441.71f
  path.quadTo(SkBits2Float(0x45235ae0), SkBits2Float(0x44b436c0), SkBits2Float(0x45269ee1), SkBits2Float(0x44b45ea1));  // 2613.68f, 1441.71f, 2665.93f, 1442.96f
  path.quadTo(SkBits2Float(0x4529e340), SkBits2Float(0x44b48660), SkBits2Float(0x452b2cd4), SkBits2Float(0x44b4fe49));  // 2718.2f, 1444.2f, 2738.8f, 1447.95f
  path.quadTo(SkBits2Float(0x452bd3a0), SkBits2Float(0x44b53af0), SkBits2Float(0x452bfdea), SkBits2Float(0x44b5653b));  // 2749.23f, 1449.84f, 2751.87f, 1451.16f
  path.quadTo(SkBits2Float(0x452c0dc7), SkBits2Float(0x44b57518), SkBits2Float(0x452c12ea), SkBits2Float(0x44b589a4));  // 2752.86f, 1451.66f, 2753.18f, 1452.3f
  path.quadTo(SkBits2Float(0x452c1cf0), SkBits2Float(0x44b5b1ba), SkBits2Float(0x452c08bf), SkBits2Float(0x44b5cca7));  // 2753.81f, 1453.55f, 2752.55f, 1454.4f
  path.quadTo(SkBits2Float(0x452bf999), SkBits2Float(0x44b5e0db), SkBits2Float(0x452bca9f), SkBits2Float(0x44b5ebe8));  // 2751.6f, 1455.03f, 2748.66f, 1455.37f
  path.quadTo(SkBits2Float(0x452b20b8), SkBits2Float(0x44b613e0), SkBits2Float(0x452a6493), SkBits2Float(0x44b63b78));  // 2738.04f, 1456.62f, 2726.29f, 1457.86f
  path.lineTo(SkBits2Float(0x452a6069), SkBits2Float(0x44b5ec64));  // 2726.03f, 1455.39f
  path.quadTo(SkBits2Float(0x452b1c60), SkBits2Float(0x44b5c4d8), SkBits2Float(0x452bc5f9), SkBits2Float(0x44b59cf0));  // 2737.77f, 1454.15f, 2748.37f, 1452.9f
  path.quadTo(SkBits2Float(0x452beb7b), SkBits2Float(0x44b5941e), SkBits2Float(0x452bf2b1), SkBits2Float(0x44b58a7f));  // 2750.72f, 1452.63f, 2751.17f, 1452.33f
  path.quadTo(SkBits2Float(0x452be9ae), SkBits2Float(0x44b59684), SkBits2Float(0x452bef5a), SkBits2Float(0x44b5ad34));  // 2750.6f, 1452.7f, 2750.96f, 1453.41f
  path.lineTo(SkBits2Float(0x452bec22), SkBits2Float(0x44b5ac59));  // 2750.76f, 1453.39f
  path.quadTo(SkBits2Float(0x452bc6e8), SkBits2Float(0x44b58728), SkBits2Float(0x452b25b8), SkBits2Float(0x44b54c83));  // 2748.43f, 1452.22f, 2738.36f, 1450.39f
  path.quadTo(SkBits2Float(0x4529df20), SkBits2Float(0x44b4d5c0), SkBits2Float(0x45269def), SkBits2Float(0x44b4ae1f));  // 2717.95f, 1446.68f, 2665.87f, 1445.44f
  path.quadTo(SkBits2Float(0x45235a60), SkBits2Float(0x44b48680), SkBits2Float(0x451f8392), SkBits2Float(0x44b48660));  // 2613.65f, 1444.2f, 2552.22f, 1444.2f
  path.lineTo(SkBits2Float(0x45184aea), SkBits2Float(0x44b48660));  // 2436.68f, 1444.2f
  path.quadTo(SkBits2Float(0x4514ea60), SkBits2Float(0x44b48660), SkBits2Float(0x4512dbd4), SkBits2Float(0x44b49a3f));  // 2382.65f, 1444.2f, 2349.74f, 1444.82f
  path.quadTo(SkBits2Float(0x4510cde0), SkBits2Float(0x44b4ae20), SkBits2Float(0x450f5f15), SkBits2Float(0x44b4fd6a));  // 2316.87f, 1445.44f, 2293.94f, 1447.92f
  path.quadTo(SkBits2Float(0x450df062), SkBits2Float(0x44b54cb4), SkBits2Float(0x450d9810), SkBits2Float(0x44b573f7));  // 2271.02f, 1450.4f, 2265.5f, 1451.62f
  path.quadTo(SkBits2Float(0x450d8759), SkBits2Float(0x44b57b64), SkBits2Float(0x450d8718), SkBits2Float(0x44b57be7));  // 2264.46f, 1451.86f, 2264.44f, 1451.87f
  path.quadTo(SkBits2Float(0x450d8c22), SkBits2Float(0x44b571d4), SkBits2Float(0x450d8bda), SkBits2Float(0x44b56298));  // 2264.76f, 1451.56f, 2264.74f, 1451.08f
  path.quadTo(SkBits2Float(0x450d8b93), SkBits2Float(0x44b5536c), SkBits2Float(0x450d8636), SkBits2Float(0x44b54a22));  // 2264.72f, 1450.61f, 2264.39f, 1450.32f
  path.lineTo(SkBits2Float(0x450d8796), SkBits2Float(0x44b54996));  // 2264.47f, 1450.3f
  path.quadTo(SkBits2Float(0x450da0d4), SkBits2Float(0x44b5575c), SkBits2Float(0x450e0212), SkBits2Float(0x44b56115));  // 2266.05f, 1450.73f, 2272.13f, 1451.03f
  path.quadTo(SkBits2Float(0x450ec840), SkBits2Float(0x44b57500), SkBits2Float(0x4513f258), SkBits2Float(0x44b5c470));  // 2284.52f, 1451.66f, 2367.15f, 1454.14f
  path.quadTo(SkBits2Float(0x45191cb0), SkBits2Float(0x44b61400), SkBits2Float(0x451abe04), SkBits2Float(0x44b63bbc));  // 2449.79f, 1456.62f, 2475.88f, 1457.87f
  path.quadTo(SkBits2Float(0x451c5f10), SkBits2Float(0x44b66370), SkBits2Float(0x451e1dc4), SkBits2Float(0x44b66372));  // 2501.94f, 1459.11f, 2529.86f, 1459.11f
  path.lineTo(SkBits2Float(0x4524047e), SkBits2Float(0x44b66372));  // 2624.28f, 1459.11f
  path.lineTo(SkBits2Float(0x45294243), SkBits2Float(0x44b66372));  // 2708.14f, 1459.11f
  path.quadTo(SkBits2Float(0x452a57f8), SkBits2Float(0x44b66378), SkBits2Float(0x452b1e58), SkBits2Float(0x44b64f9e));  // 2725.5f, 1459.11f, 2737.9f, 1458.49f
  path.quadTo(SkBits2Float(0x452be498), SkBits2Float(0x44b63bd0), SkBits2Float(0x452caaea), SkBits2Float(0x44b61421));  // 2750.29f, 1457.87f, 2762.68f, 1456.63f
  path.quadTo(SkBits2Float(0x452d70f0), SkBits2Float(0x44b5ec80), SkBits2Float(0x452e0f08), SkBits2Float(0x44b5b139));  // 2775.06f, 1455.39f, 2784.94f, 1453.54f
  path.quadTo(SkBits2Float(0x452e201d), SkBits2Float(0x44b5aad1), SkBits2Float(0x452e26bb), SkBits2Float(0x44b5a416));  // 2786.01f, 1453.34f, 2786.42f, 1453.13f
  path.quadTo(SkBits2Float(0x452e2166), SkBits2Float(0x44b5a984), SkBits2Float(0x452e2432), SkBits2Float(0x44b5bdab));  // 2786.09f, 1453.3f, 2786.26f, 1453.93f
  path.quadTo(SkBits2Float(0x452e23f2), SkBits2Float(0x44b5bbe4), SkBits2Float(0x452e0024), SkBits2Float(0x44b5a4aa));  // 2786.25f, 1453.87f, 2784.01f, 1453.15f
  path.quadTo(SkBits2Float(0x452da5a0), SkBits2Float(0x44b569f0), SkBits2Float(0x452c9fa0), SkBits2Float(0x44b510f5));  // 2778.35f, 1451.31f, 2761.98f, 1448.53f
  path.quadTo(SkBits2Float(0x452a9240), SkBits2Float(0x44b45e80), SkBits2Float(0x45274634), SkBits2Float(0x44b3fb28));  // 2729.14f, 1442.95f, 2676.39f, 1439.85f
  path.quadTo(SkBits2Float(0x4523fae0), SkBits2Float(0x44b39800), SkBits2Float(0x45202265), SkBits2Float(0x44b383f5));  // 2623.68f, 1436.75f, 2562.15f, 1436.12f
  path.quadTo(SkBits2Float(0x451c4c60), SkBits2Float(0x44b37020), SkBits2Float(0x451911b2), SkBits2Float(0x44b37015));  // 2500.77f, 1435.5f, 2449.11f, 1435.5f
  path.lineTo(SkBits2Float(0x4514a4a6), SkBits2Float(0x44b37015));  // 2378.29f, 1435.5f
  path.quadTo(SkBits2Float(0x45137110), SkBits2Float(0x44b37010), SkBits2Float(0x451296da), SkBits2Float(0x44b383e9));  // 2359.07f, 1435.5f, 2345.43f, 1436.12f
  path.lineTo(SkBits2Float(0x4510ff6d), SkBits2Float(0x44b3aba8));  // 2319.96f, 1437.36f
  path.quadTo(SkBits2Float(0x4510432a), SkBits2Float(0x44b3bf78), SkBits2Float(0x450fea49), SkBits2Float(0x44b3d33a));  // 2308.2f, 1437.98f, 2302.64f, 1438.6f
  path.quadTo(SkBits2Float(0x450f945c), SkBits2Float(0x44b3e652), SkBits2Float(0x450f83a7), SkBits2Float(0x44b3f707));  // 2297.27f, 1439.2f, 2296.23f, 1439.72f
  path.quadTo(SkBits2Float(0x450f8a6a), SkBits2Float(0x44b3f044), SkBits2Float(0x450f8c92), SkBits2Float(0x44b3e01f));  // 2296.65f, 1439.51f, 2296.79f, 1439
  path.quadTo(SkBits2Float(0x450f9052), SkBits2Float(0x44b3c3fd), SkBits2Float(0x450f8428), SkBits2Float(0x44b3b5ca));  // 2297.02f, 1438.12f, 2296.26f, 1437.68f
  path.lineTo(SkBits2Float(0x450f852a), SkBits2Float(0x44b3b4d9));  // 2296.32f, 1437.65f
  path.quadTo(SkBits2Float(0x450f9088), SkBits2Float(0x44b3b842), SkBits2Float(0x450fad49), SkBits2Float(0x44b3baa8));  // 2297.03f, 1437.76f, 2298.83f, 1437.83f
  path.quadTo(SkBits2Float(0x450fe874), SkBits2Float(0x44b3bf94), SkBits2Float(0x4510694c), SkBits2Float(0x44b3bf97));  // 2302.53f, 1437.99f, 2310.58f, 1437.99f
  path.quadTo(SkBits2Float(0x45116c00), SkBits2Float(0x44b3bfa0), SkBits2Float(0x4512968c), SkBits2Float(0x44b3d37e));  // 2326.75f, 1437.99f, 2345.41f, 1438.61f
  path.quadTo(SkBits2Float(0x4513c160), SkBits2Float(0x44b3e760), SkBits2Float(0x4515093e), SkBits2Float(0x44b40f2d));  // 2364.09f, 1439.23f, 2384.58f, 1440.47f
  path.quadTo(SkBits2Float(0x45165110), SkBits2Float(0x44b436f0), SkBits2Float(0x4517c15f), SkBits2Float(0x44b472a0));  // 2405.07f, 1441.72f, 2428.09f, 1443.58f
  path.quadTo(SkBits2Float(0x45193170), SkBits2Float(0x44b4ae50), SkBits2Float(0x451ae5bb), SkBits2Float(0x44b4d5ed));  // 2451.09f, 1445.45f, 2478.36f, 1446.69f
  path.quadTo(SkBits2Float(0x451c9ac0), SkBits2Float(0x44b4fda0), SkBits2Float(0x451e8128), SkBits2Float(0x44b4fda4));  // 2505.67f, 1447.93f, 2536.07f, 1447.93f
  path.lineTo(SkBits2Float(0x4522315c), SkBits2Float(0x44b4fda4));  // 2595.08f, 1447.93f
  path.lineTo(SkBits2Float(0x452849c9), SkBits2Float(0x44b4fda4));  // 2692.61f, 1447.93f
  path.quadTo(SkBits2Float(0x452c9820), SkBits2Float(0x44b4fda0), SkBits2Float(0x452e3892), SkBits2Float(0x44b4ae4f));  // 2761.51f, 1447.93f, 2787.54f, 1445.45f
  path.quadTo(SkBits2Float(0x452fd8b0), SkBits2Float(0x44b45f0c), SkBits2Float(0x453058a1), SkBits2Float(0x44b42401));  // 2813.54f, 1442.97f, 2821.54f, 1441.13f
  path.quadTo(SkBits2Float(0x453073fd), SkBits2Float(0x44b41760), SkBits2Float(0x45307b60), SkBits2Float(0x44b40a3d));  // 2823.25f, 1440.73f, 2823.71f, 1440.32f
  path.quadTo(SkBits2Float(0x45307873), SkBits2Float(0x44b40f70), SkBits2Float(0x45307a4d), SkBits2Float(0x44b41410));  // 2823.53f, 1440.48f, 2823.64f, 1440.63f
  path.quadTo(SkBits2Float(0x45306a14), SkBits2Float(0x44b3eb84), SkBits2Float(0x4530074d), SkBits2Float(0x44b3a9ad));  // 2822.63f, 1439.36f, 2816.46f, 1437.3f
  path.quadTo(SkBits2Float(0x452f3940), SkBits2Float(0x44b32040), SkBits2Float(0x452b3272), SkBits2Float(0x44b2d107));  // 2803.58f, 1433.01f, 2739.15f, 1430.53f
  path.quadTo(SkBits2Float(0x452728c0), SkBits2Float(0x44b28180), SkBits2Float(0x4523972a), SkBits2Float(0x44b2818c));  // 2674.55f, 1428.05f, 2617.45f, 1428.05f
  path.lineTo(SkBits2Float(0x451c9a25), SkBits2Float(0x44b2818c));  // 2505.63f, 1428.05f
  path.quadTo(SkBits2Float(0x45192ee0), SkBits2Float(0x44b28180), SkBits2Float(0x45166e67), SkBits2Float(0x44b2a94a));  // 2450.93f, 1428.05f, 2406.9f, 1429.29f
  path.quadTo(SkBits2Float(0x4513ad30), SkBits2Float(0x44b2d120), SkBits2Float(0x4512169a), SkBits2Float(0x44b32061));  // 2362.82f, 1430.54f, 2337.41f, 1433.01f
  path.quadTo(SkBits2Float(0x451080b4), SkBits2Float(0x44b36f90), SkBits2Float(0x45100aba), SkBits2Float(0x44b3aa88));  // 2312.04f, 1435.49f, 2304.67f, 1437.33f
  path.quadTo(SkBits2Float(0x450fee3d), SkBits2Float(0x44b3b8c6), SkBits2Float(0x450fdff2), SkBits2Float(0x44b3c435));  // 2302.89f, 1437.77f, 2302, 1438.13f
  path.quadTo(SkBits2Float(0x450fda17), SkBits2Float(0x44b3c8e4), SkBits2Float(0x450fd7a1), SkBits2Float(0x44b3cc2c));  // 2301.63f, 1438.28f, 2301.48f, 1438.38f
  path.quadTo(SkBits2Float(0x450fd9d5), SkBits2Float(0x44b3c93c), SkBits2Float(0x450fdb13), SkBits2Float(0x44b3bf4c));  // 2301.61f, 1438.29f, 2301.69f, 1437.98f
  path.quadTo(SkBits2Float(0x450fddae), SkBits2Float(0x44b3aa6d), SkBits2Float(0x450fd5e8), SkBits2Float(0x44b39ef7));  // 2301.85f, 1437.33f, 2301.37f, 1436.97f
  path.quadTo(SkBits2Float(0x450fd5f8), SkBits2Float(0x44b39f0f), SkBits2Float(0x450fd90e), SkBits2Float(0x44b3a0d2));  // 2301.37f, 1436.97f, 2301.57f, 1437.03f
  path.quadTo(SkBits2Float(0x450fe44d), SkBits2Float(0x44b3a73f), SkBits2Float(0x450ffdc5), SkBits2Float(0x44b3abe0));  // 2302.27f, 1437.23f, 2303.86f, 1437.37f
  path.quadTo(SkBits2Float(0x45106a40), SkBits2Float(0x44b3bfa0), SkBits2Float(0x45127810), SkBits2Float(0x44b3bf97));  // 2310.64f, 1437.99f, 2343.5f, 1437.99f
  path.quadTo(SkBits2Float(0x451487b0), SkBits2Float(0x44b3bf90), SkBits2Float(0x451632ac), SkBits2Float(0x44b3d37b));  // 2376.48f, 1437.99f, 2403.17f, 1438.61f
  path.quadTo(SkBits2Float(0x4517dd10), SkBits2Float(0x44b3e760), SkBits2Float(0x4519a6c8), SkBits2Float(0x44b3e759));  // 2429.82f, 1439.23f, 2458.42f, 1439.23f
  path.lineTo(SkBits2Float(0x45212ef3), SkBits2Float(0x44b3e759));  // 2578.93f, 1439.23f
  path.lineTo(SkBits2Float(0x4529af97), SkBits2Float(0x44b3e759));  // 2714.97f, 1439.23f
  path.quadTo(SkBits2Float(0x452c7180), SkBits2Float(0x44b3e750), SkBits2Float(0x452dd683), SkBits2Float(0x44b3d37c));  // 2759.09f, 1439.23f, 2781.41f, 1438.61f
  path.quadTo(SkBits2Float(0x452f3b06), SkBits2Float(0x44b3bfa6), SkBits2Float(0x452f753c), SkBits2Float(0x44b3ac41));  // 2803.69f, 1437.99f, 2807.33f, 1437.38f
  path.quadTo(SkBits2Float(0x452f8a8e), SkBits2Float(0x44b3a526), SkBits2Float(0x452f8ac0), SkBits2Float(0x44b3a491));  // 2808.66f, 1437.16f, 2808.67f, 1437.14f
  path.quadTo(SkBits2Float(0x452f84e2), SkBits2Float(0x44b3b62c), SkBits2Float(0x452f8af4), SkBits2Float(0x44b3c458));  // 2808.31f, 1437.69f, 2808.68f, 1438.14f
  path.quadTo(SkBits2Float(0x452f89d7), SkBits2Float(0x44b3c1be), SkBits2Float(0x452f837e), SkBits2Float(0x44b3bb65));  // 2808.61f, 1438.05f, 2808.22f, 1437.86f
  path.quadTo(SkBits2Float(0x452f5e80), SkBits2Float(0x44b39664), SkBits2Float(0x452ee065), SkBits2Float(0x44b36f9e));  // 2805.91f, 1436.7f, 2798.02f, 1435.49f
  path.quadTo(SkBits2Float(0x452e5f80), SkBits2Float(0x44b34800), SkBits2Float(0x452c29f3), SkBits2Float(0x44b2e4c9));  // 2789.97f, 1434.25f, 2754.62f, 1431.15f
  path.quadTo(SkBits2Float(0x4529f400), SkBits2Float(0x44b28180), SkBits2Float(0x45270146), SkBits2Float(0x44b259c6));  // 2719.25f, 1428.05f, 2672.08f, 1426.81f
  path.quadTo(SkBits2Float(0x45240f00), SkBits2Float(0x44b23200), SkBits2Float(0x4520d57f), SkBits2Float(0x44b23209));  // 2624.94f, 1425.56f, 2573.34f, 1425.56f
  path.lineTo(SkBits2Float(0x451abd12), SkBits2Float(0x44b23209));  // 2475.82f, 1425.56f
  path.quadTo(SkBits2Float(0x4517dce0), SkBits2Float(0x44b23200), SkBits2Float(0x45151287), SkBits2Float(0x44b259c6));  // 2429.8f, 1425.56f, 2385.16f, 1426.81f
  path.quadTo(SkBits2Float(0x45124660), SkBits2Float(0x44b281a0), SkBits2Float(0x4511a895), SkBits2Float(0x44b29558));  // 2340.4f, 1428.05f, 2330.54f, 1428.67f
  path.quadTo(SkBits2Float(0x45115968), SkBits2Float(0x44b29f3e), SkBits2Float(0x451140f6), SkBits2Float(0x44b2a422));  // 2325.59f, 1428.98f, 2324.06f, 1429.13f
  path.quadTo(SkBits2Float(0x45113b62), SkBits2Float(0x44b2a540), SkBits2Float(0x4511393f), SkBits2Float(0x44b2a5f6));  // 2323.71f, 1429.16f, 2323.58f, 1429.19f
  path.quadTo(SkBits2Float(0x45113998), SkBits2Float(0x44b2a5d7), SkBits2Float(0x45113ab8), SkBits2Float(0x44b2a50b));  // 2323.6f, 1429.18f, 2323.67f, 1429.16f
  path.quadTo(SkBits2Float(0x451140a8), SkBits2Float(0x44b2a0ce), SkBits2Float(0x4511443f), SkBits2Float(0x44b29607));  // 2324.04f, 1429.03f, 2324.27f, 1428.69f
  path.quadTo(SkBits2Float(0x45114b21), SkBits2Float(0x44b28164), SkBits2Float(0x451144d6), SkBits2Float(0x44b26bfe));  // 2324.7f, 1428.04f, 2324.3f, 1427.37f
  path.quadTo(SkBits2Float(0x451141e0), SkBits2Float(0x44b261ed), SkBits2Float(0x45113cb2), SkBits2Float(0x44b25cbe));  // 2324.12f, 1427.06f, 2323.79f, 1426.9f
  path.quadTo(SkBits2Float(0x4511397a), SkBits2Float(0x44b25986), SkBits2Float(0x4511393a), SkBits2Float(0x44b25981));  // 2323.59f, 1426.8f, 2323.58f, 1426.8f
  path.quadTo(SkBits2Float(0x45113ce2), SkBits2Float(0x44b259c9), SkBits2Float(0x451143f4), SkBits2Float(0x44b259c9));  // 2323.81f, 1426.81f, 2324.25f, 1426.81f
  path.lineTo(SkBits2Float(0x451cf398), SkBits2Float(0x44b259c9));  // 2511.22f, 1426.81f
  path.lineTo(SkBits2Float(0x4529d758), SkBits2Float(0x44b259c9));  // 2717.46f, 1426.81f
  path.quadTo(SkBits2Float(0x452b4690), SkBits2Float(0x44b259c0), SkBits2Float(0x452c986e), SkBits2Float(0x44b245ed));  // 2740.41f, 1426.8f, 2761.53f, 1426.19f
  path.quadTo(SkBits2Float(0x452deae8), SkBits2Float(0x44b23210), SkBits2Float(0x452ecf8a), SkBits2Float(0x44b23208));  // 2782.68f, 1425.56f, 2796.97f, 1425.56f
  path.lineTo(SkBits2Float(0x45304938), SkBits2Float(0x44b23208));  // 2820.58f, 1425.56f
  path.lineTo(SkBits2Float(0x45304938), SkBits2Float(0x44b2818c));  // 2820.58f, 1428.05f
  path.close();
  return path;
}

static SkPath getPath2() {
  SkPath path;
  path.setFillType(SkPathFillType::kWinding);
  path.moveTo(SkBits2Float(0x452a6493), SkBits2Float(0x44b63b78));  // 2726.29f, 1457.86f
  path.quadTo(SkBits2Float(0x4529a6f0), SkBits2Float(0x44b66360), SkBits2Float(0x4528c1eb), SkBits2Float(0x44b67749));  // 2714.43f, 1459.11f, 2700.12f, 1459.73f
  path.quadTo(SkBits2Float(0x4527dcd0), SkBits2Float(0x44b68b30), SkBits2Float(0x45256a4c), SkBits2Float(0x44b68b34));  // 2685.8f, 1460.35f, 2646.64f, 1460.35f
  path.lineTo(SkBits2Float(0x451c18f0), SkBits2Float(0x44b68b34));  // 2497.56f, 1460.35f
  path.lineTo(SkBits2Float(0x4514c277), SkBits2Float(0x44b68b34));  // 2380.15f, 1460.35f
  path.quadTo(SkBits2Float(0x45148300), SkBits2Float(0x44b68b34), SkBits2Float(0x45147306), SkBits2Float(0x44b67e6c));  // 2376.19f, 1460.35f, 2375.19f, 1459.95f
  path.quadTo(SkBits2Float(0x451464b8), SkBits2Float(0x44b672fa), SkBits2Float(0x4514635d), SkBits2Float(0x44b657df));  // 2374.29f, 1459.59f, 2374.21f, 1458.75f
  path.quadTo(SkBits2Float(0x45146205), SkBits2Float(0x44b63cfd), SkBits2Float(0x45146ec2), SkBits2Float(0x44b62c01));  // 2374.13f, 1457.91f, 2374.92f, 1457.38f
  path.quadTo(SkBits2Float(0x451476ee), SkBits2Float(0x44b6211b), SkBits2Float(0x45148b4f), SkBits2Float(0x44b61576));  // 2375.43f, 1457.03f, 2376.71f, 1456.67f
  path.quadTo(SkBits2Float(0x4514d27a), SkBits2Float(0x44b5ecca), SkBits2Float(0x451639f4), SkBits2Float(0x44b59ce9));  // 2381.15f, 1455.4f, 2403.62f, 1452.9f
  path.quadTo(SkBits2Float(0x4517a0c0), SkBits2Float(0x44b54d20), SkBits2Float(0x451a4538), SkBits2Float(0x44b5256b));  // 2426.05f, 1450.41f, 2468.33f, 1449.17f
  path.quadTo(SkBits2Float(0x451cea00), SkBits2Float(0x44b4fda0), SkBits2Float(0x45229eb1), SkBits2Float(0x44b4fda4));  // 2510.62f, 1447.93f, 2601.92f, 1447.93f
  path.lineTo(SkBits2Float(0x452bdc2d), SkBits2Float(0x44b4fda4));  // 2749.76f, 1447.93f
  path.lineTo(SkBits2Float(0x4531fe8a), SkBits2Float(0x44b4fda4));  // 2847.91f, 1447.93f
  path.quadTo(SkBits2Float(0x45349b40), SkBits2Float(0x44b4fdc0), SkBits2Float(0x45354706), SkBits2Float(0x44b562a6));  // 2889.7f, 1447.93f, 2900.44f, 1451.08f
  path.quadTo(SkBits2Float(0x4535a1d8), SkBits2Float(0x44b59814), SkBits2Float(0x4535b4b5), SkBits2Float(0x44b5ca63));  // 2906.12f, 1452.75f, 2907.29f, 1454.32f
  path.quadTo(SkBits2Float(0x4535c738), SkBits2Float(0x44b5fbc1), SkBits2Float(0x4535af68), SkBits2Float(0x44b6256d));  // 2908.45f, 1455.87f, 2906.96f, 1457.17f
  path.quadTo(SkBits2Float(0x4535a204), SkBits2Float(0x44b63cda), SkBits2Float(0x45358247), SkBits2Float(0x44b64e2b));  // 2906.13f, 1457.9f, 2904.14f, 1458.44f
  path.quadTo(SkBits2Float(0x45351300), SkBits2Float(0x44b68ae0), SkBits2Float(0x45326d53), SkBits2Float(0x44b6ee7c));  // 2897.19f, 1460.34f, 2854.83f, 1463.45f
  path.quadTo(SkBits2Float(0x452fc860), SkBits2Float(0x44b75200), SkBits2Float(0x452e3a74), SkBits2Float(0x44b751fc));  // 2812.52f, 1466.56f, 2787.65f, 1466.56f
  path.lineTo(SkBits2Float(0x452aeda3), SkBits2Float(0x44b751fc));  // 2734.85f, 1466.56f
  path.lineTo(SkBits2Float(0x45275150), SkBits2Float(0x44b751fc));  // 2677.08f, 1466.56f
  path.quadTo(SkBits2Float(0x452573a0), SkBits2Float(0x44b75200), SkBits2Float(0x45219b79), SkBits2Float(0x44b70270));  // 2647.23f, 1466.56f, 2585.72f, 1464.08f
  path.quadTo(SkBits2Float(0x451dc2a0), SkBits2Float(0x44b6b2c0), SkBits2Float(0x451922ec), SkBits2Float(0x44b5748f));  // 2524.16f, 1461.59f, 2450.18f, 1451.64f
  path.quadTo(SkBits2Float(0x451483c0), SkBits2Float(0x44b43680), SkBits2Float(0x451358c4), SkBits2Float(0x44b3bed2));  // 2376.23f, 1441.7f, 2357.55f, 1437.96f
  path.quadTo(SkBits2Float(0x4512c2e4), SkBits2Float(0x44b382e0), SkBits2Float(0x45127c8b), SkBits2Float(0x44b35aac));  // 2348.18f, 1436.09f, 2343.78f, 1434.83f
  path.quadTo(SkBits2Float(0x451257fc), SkBits2Float(0x44b345c9), SkBits2Float(0x4512476b), SkBits2Float(0x44b33538));  // 2341.5f, 1434.18f, 2340.46f, 1433.66f
  path.quadTo(SkBits2Float(0x45123c10), SkBits2Float(0x44b329de), SkBits2Float(0x451236d9), SkBits2Float(0x44b31cd5));  // 2339.75f, 1433.31f, 2339.43f, 1432.9f
  path.quadTo(SkBits2Float(0x4512293f), SkBits2Float(0x44b2fad5), SkBits2Float(0x4512384f), SkBits2Float(0x44b2dcb4));  // 2338.58f, 1431.84f, 2339.52f, 1430.9f
  path.quadTo(SkBits2Float(0x451250c8), SkBits2Float(0x44b2abc1), SkBits2Float(0x4513953a), SkBits2Float(0x44b24662));  // 2341.05f, 1429.37f, 2361.33f, 1426.2f
  path.quadTo(SkBits2Float(0x4514d4e0), SkBits2Float(0x44b1e280), SkBits2Float(0x45175270), SkBits2Float(0x44b1e285));  // 2381.3f, 1423.08f, 2421.15f, 1423.08f
  path.lineTo(SkBits2Float(0x451d431c), SkBits2Float(0x44b1e285));  // 2516.19f, 1423.08f
  path.quadTo(SkBits2Float(0x4520b980), SkBits2Float(0x44b1e280), SkBits2Float(0x45248fd7), SkBits2Float(0x44b1f667));  // 2571.59f, 1423.08f, 2632.99f, 1423.7f
  path.quadTo(SkBits2Float(0x452867a0), SkBits2Float(0x44b20a40), SkBits2Float(0x452ca3dd), SkBits2Float(0x44b26db5));  // 2694.48f, 1424.32f, 2762.24f, 1427.43f
  path.quadTo(SkBits2Float(0x4530e000), SkBits2Float(0x44b2d100), SkBits2Float(0x45319e40), SkBits2Float(0x44b30d2d));  // 2830, 1430.53f, 2841.89f, 1432.41f
  path.quadTo(SkBits2Float(0x4531ffb0), SkBits2Float(0x44b32bf0), SkBits2Float(0x45321b4e), SkBits2Float(0x44b3478d));  // 2847.98f, 1433.37f, 2849.71f, 1434.24f
  path.quadTo(SkBits2Float(0x4532267a), SkBits2Float(0x44b352b8), SkBits2Float(0x45322b4c), SkBits2Float(0x44b361fe));  // 2850.4f, 1434.58f, 2850.71f, 1435.06f
  path.quadTo(SkBits2Float(0x45323526), SkBits2Float(0x44b38131), SkBits2Float(0x453228bb), SkBits2Float(0x44b39d20));  // 2851.32f, 1436.04f, 2850.55f, 1436.91f
  path.quadTo(SkBits2Float(0x45321f7d), SkBits2Float(0x44b3b1ea), SkBits2Float(0x453202da), SkBits2Float(0x44b3bea5));  // 2849.97f, 1437.56f, 2848.18f, 1437.96f
  path.quadTo(SkBits2Float(0x4531a7e0), SkBits2Float(0x44b3e714), SkBits2Float(0x4530fd4b), SkBits2Float(0x44b3fb28));  // 2842.49f, 1439.22f, 2831.83f, 1439.85f
  path.quadTo(SkBits2Float(0x45305460), SkBits2Float(0x44b40f08), SkBits2Float(0x452f6f6d), SkBits2Float(0x44b422f2));  // 2821.27f, 1440.47f, 2806.96f, 1441.09f
  path.quadTo(SkBits2Float(0x452e8a50), SkBits2Float(0x44b436d8), SkBits2Float(0x452d69bc), SkBits2Float(0x44b436dd));  // 2792.64f, 1441.71f, 2774.61f, 1441.71f
  path.lineTo(SkBits2Float(0x45217486), SkBits2Float(0x44b436dd));  // 2583.28f, 1441.71f
  path.lineTo(SkBits2Float(0x45140f90), SkBits2Float(0x44b436dd));  // 2368.97f, 1441.71f
  path.quadTo(SkBits2Float(0x4512c750), SkBits2Float(0x44b436e0), SkBits2Float(0x45122faf), SkBits2Float(0x44b431e3));  // 2348.46f, 1441.71f, 2338.98f, 1441.56f
  path.quadTo(SkBits2Float(0x4511e370), SkBits2Float(0x44b42f60), SkBits2Float(0x4511c360), SkBits2Float(0x44b42b9c));  // 2334.21f, 1441.48f, 2332.21f, 1441.36f
  path.quadTo(SkBits2Float(0x4511b1e5), SkBits2Float(0x44b4298d), SkBits2Float(0x4511ab82), SkBits2Float(0x44b426ab));  // 2331.12f, 1441.3f, 2330.72f, 1441.21f
  path.quadTo(SkBits2Float(0x4511a797), SkBits2Float(0x44b424e6), SkBits2Float(0x4511a4a0), SkBits2Float(0x44b421a7));  // 2330.47f, 1441.15f, 2330.29f, 1441.05f
  path.quadTo(SkBits2Float(0x45119f98), SkBits2Float(0x44b41c24), SkBits2Float(0x45119cbb), SkBits2Float(0x44b4127e));  // 2329.97f, 1440.88f, 2329.8f, 1440.58f
  path.quadTo(SkBits2Float(0x45119698), SkBits2Float(0x44b3fdce), SkBits2Float(0x45119cc8), SkBits2Float(0x44b3e92e));  // 2329.41f, 1439.93f, 2329.8f, 1439.29f
  path.quadTo(SkBits2Float(0x4511a1a8), SkBits2Float(0x44b3d8f0), SkBits2Float(0x4511ad63), SkBits2Float(0x44b3d43e));  // 2330.1f, 1438.78f, 2330.84f, 1438.63f
  path.quadTo(SkBits2Float(0x4511df00), SkBits2Float(0x44b3c066), SkBits2Float(0x45124cc1), SkBits2Float(0x44b3987b));  // 2333.94f, 1438.01f, 2340.8f, 1436.77f
  path.quadTo(SkBits2Float(0x4512bb6c), SkBits2Float(0x44b3703c), SkBits2Float(0x4513a15f), SkBits2Float(0x44b35c3d));  // 2347.71f, 1435.51f, 2362.09f, 1434.88f
  path.lineTo(SkBits2Float(0x45154cb0), SkBits2Float(0x44b3347d));  // 2388.79f, 1433.64f
  path.quadTo(SkBits2Float(0x45161400), SkBits2Float(0x44b32090), SkBits2Float(0x451734a0), SkBits2Float(0x44b32091));  // 2401.25f, 1433.02f, 2419.29f, 1433.02f
  path.quadTo(SkBits2Float(0x451854a0), SkBits2Float(0x44b320a0), SkBits2Float(0x4519c410), SkBits2Float(0x44b30cb4));  // 2437.29f, 1433.02f, 2460.25f, 1432.4f
  path.quadTo(SkBits2Float(0x451b3400), SkBits2Float(0x44b2f8d0), SkBits2Float(0x451e7738), SkBits2Float(0x44b2f8d0));  // 2483.25f, 1431.78f, 2535.45f, 1431.78f
  path.quadTo(SkBits2Float(0x4521b840), SkBits2Float(0x44b2f8c0), SkBits2Float(0x45248fea), SkBits2Float(0x44b30cb1));  // 2587.52f, 1431.77f, 2632.99f, 1432.4f
  path.quadTo(SkBits2Float(0x45276660), SkBits2Float(0x44b320a0), SkBits2Float(0x4528ff18), SkBits2Float(0x44b38440));  // 2678.4f, 1433.02f, 2703.94f, 1436.13f
  path.quadTo(SkBits2Float(0x452a9b50), SkBits2Float(0x44b3e8c0), SkBits2Float(0x452abd08), SkBits2Float(0x44b415c8));  // 2729.71f, 1439.27f, 2731.81f, 1440.68f
  path.quadTo(SkBits2Float(0x452ac5cc), SkBits2Float(0x44b42178), SkBits2Float(0x452ac7a7), SkBits2Float(0x44b4348e));  // 2732.36f, 1441.05f, 2732.48f, 1441.64f
  path.quadTo(SkBits2Float(0x452ac9c0), SkBits2Float(0x44b44a1c), SkBits2Float(0x452ac23c), SkBits2Float(0x44b45bca));  // 2732.61f, 1442.32f, 2732.14f, 1442.87f
  path.quadTo(SkBits2Float(0x452abc7b), SkBits2Float(0x44b46952), SkBits2Float(0x452aaf0c), SkBits2Float(0x44b47471));  // 2731.78f, 1443.29f, 2730.94f, 1443.64f
  path.quadTo(SkBits2Float(0x452a9b7d), SkBits2Float(0x44b484a0), SkBits2Float(0x452a6e60), SkBits2Float(0x44b49940));  // 2729.72f, 1444.14f, 2726.9f, 1444.79f
  path.quadTo(SkBits2Float(0x452a16a0), SkBits2Float(0x44b4c15c), SkBits2Float(0x4529593e), SkBits2Float(0x44b4fd27));  // 2721.41f, 1446.04f, 2709.58f, 1447.91f
  path.quadTo(SkBits2Float(0x4527de80), SkBits2Float(0x44b574c0), SkBits2Float(0x45260036), SkBits2Float(0x44b59ca1));  // 2685.91f, 1451.65f, 2656.01f, 1452.89f
  path.quadTo(SkBits2Float(0x45242300), SkBits2Float(0x44b5c480), SkBits2Float(0x452192a3), SkBits2Float(0x44b5d84b));  // 2626.19f, 1454.14f, 2585.16f, 1454.76f
  path.quadTo(SkBits2Float(0x451f01a0), SkBits2Float(0x44b5ec20), SkBits2Float(0x451c9034), SkBits2Float(0x44b5ec2e));  // 2544.1f, 1455.38f, 2505.01f, 1455.38f
  path.lineTo(SkBits2Float(0x4518907d), SkBits2Float(0x44b5ec2e));  // 2441.03f, 1455.38f
  path.lineTo(SkBits2Float(0x4516c74c), SkBits2Float(0x44b5ec2e));  // 2412.46f, 1455.38f
  path.quadTo(SkBits2Float(0x4516be92), SkBits2Float(0x44b5ec2e), SkBits2Float(0x4516b9b3), SkBits2Float(0x44b5eae9));  // 2411.91f, 1455.38f, 2411.61f, 1455.34f
  path.quadTo(SkBits2Float(0x4516b52f), SkBits2Float(0x44b5e9bc), SkBits2Float(0x4516b1d7), SkBits2Float(0x44b5e6de));  // 2411.32f, 1455.3f, 2411.11f, 1455.21f
  path.quadTo(SkBits2Float(0x4516a037), SkBits2Float(0x44b5d7c4), SkBits2Float(0x4516a626), SkBits2Float(0x44b5b422));  // 2410.01f, 1454.74f, 2410.38f, 1453.63f
  path.quadTo(SkBits2Float(0x4516aa4a), SkBits2Float(0x44b59b41), SkBits2Float(0x4516c550), SkBits2Float(0x44b58ec7));  // 2410.64f, 1452.85f, 2412.33f, 1452.46f
  path.quadTo(SkBits2Float(0x4516e658), SkBits2Float(0x44b57f87), SkBits2Float(0x45174566), SkBits2Float(0x44b56185));  // 2414.4f, 1451.99f, 2420.34f, 1451.05f
  path.quadTo(SkBits2Float(0x451802e8), SkBits2Float(0x44b525a8), SkBits2Float(0x451a0845), SkBits2Float(0x44b4c231));  // 2432.18f, 1449.18f, 2464.52f, 1446.07f
  path.quadTo(SkBits2Float(0x451c0e50), SkBits2Float(0x44b45e80), SkBits2Float(0x451fdc6a), SkBits2Float(0x44b42301));  // 2496.89f, 1442.95f, 2557.78f, 1441.09f
  path.quadTo(SkBits2Float(0x4523ac00), SkBits2Float(0x44b3e740), SkBits2Float(0x4527f056), SkBits2Float(0x44b3e759));  // 2618.75f, 1439.23f, 2687.02f, 1439.23f
  path.quadTo(SkBits2Float(0x452c3700), SkBits2Float(0x44b3e740), SkBits2Float(0x453022a5), SkBits2Float(0x44b45eb0));  // 2755.44f, 1439.23f, 2818.17f, 1442.96f
  path.quadTo(SkBits2Float(0x45341080), SkBits2Float(0x44b4d640), SkBits2Float(0x453461a3), SkBits2Float(0x44b4fed4));  // 2881.03f, 1446.7f, 2886.1f, 1447.96f
  path.quadTo(SkBits2Float(0x453493fc), SkBits2Float(0x44b517fe), SkBits2Float(0x453499ea), SkBits2Float(0x44b53b9d));  // 2889.25f, 1448.75f, 2889.62f, 1449.86f
  path.quadTo(SkBits2Float(0x4534a129), SkBits2Float(0x44b5671c), SkBits2Float(0x4534875c), SkBits2Float(0x44b57af5));  // 2890.07f, 1451.22f, 2888.46f, 1451.84f
  path.quadTo(SkBits2Float(0x45347d2e), SkBits2Float(0x44b582c9), SkBits2Float(0x45346991), SkBits2Float(0x44b58863));  // 2887.82f, 1452.09f, 2886.6f, 1452.26f
  path.quadTo(SkBits2Float(0x4534235e), SkBits2Float(0x44b59c70), SkBits2Float(0x45320994), SkBits2Float(0x44b5d83d));  // 2882.21f, 1452.89f, 2848.6f, 1454.76f
  path.quadTo(SkBits2Float(0x452ff0c0), SkBits2Float(0x44b61400), SkBits2Float(0x452ea7c8), SkBits2Float(0x44b613ef));  // 2815.05f, 1456.62f, 2794.49f, 1456.62f
  path.lineTo(SkBits2Float(0x4527c894), SkBits2Float(0x44b613ef));  // 2684.54f, 1456.62f
  path.lineTo(SkBits2Float(0x45202298), SkBits2Float(0x44b613ef));  // 2562.16f, 1456.62f
  path.quadTo(SkBits2Float(0x451e1320), SkBits2Float(0x44b613e0), SkBits2Float(0x451c2215), SkBits2Float(0x44b5ec25));  // 2529.2f, 1456.62f, 2498.13f, 1455.38f
  path.quadTo(SkBits2Float(0x451a31c0), SkBits2Float(0x44b5c480), SkBits2Float(0x4518cc1f), SkBits2Float(0x44b5c46c));  // 2467.11f, 1454.14f, 2444.76f, 1454.14f
  path.quadTo(SkBits2Float(0x451765f0), SkBits2Float(0x44b5c470), SkBits2Float(0x451680db), SkBits2Float(0x44b5b082));  // 2422.37f, 1454.14f, 2408.05f, 1453.52f
  path.quadTo(SkBits2Float(0x45159ca8), SkBits2Float(0x44b59ca8), SkBits2Float(0x45152fcc), SkBits2Float(0x44b59cab));  // 2393.79f, 1452.9f, 2386.99f, 1452.9f
  path.lineTo(SkBits2Float(0x4514d349), SkBits2Float(0x44b59cab));  // 2381.21f, 1452.9f
  path.lineTo(SkBits2Float(0x4514d349), SkBits2Float(0x44b574e9));  // 2381.21f, 1451.65f
  path.lineTo(SkBits2Float(0x4514d349), SkBits2Float(0x44b54d27));  // 2381.21f, 1450.41f
  path.lineTo(SkBits2Float(0x4514d658), SkBits2Float(0x44b54d27));  // 2381.4f, 1450.41f
  path.quadTo(SkBits2Float(0x4514e6a0), SkBits2Float(0x44b54d27), SkBits2Float(0x451532c1), SkBits2Float(0x44b5140d));  // 2382.41f, 1450.41f, 2387.17f, 1448.63f
  path.lineTo(SkBits2Float(0x451540b7), SkBits2Float(0x44b55e7f));  // 2388.04f, 1450.95f
  path.quadTo(SkBits2Float(0x4514edd3), SkBits2Float(0x44b59cac), SkBits2Float(0x4514d658), SkBits2Float(0x44b59cab));  // 2382.86f, 1452.9f, 2381.4f, 1452.9f
  path.lineTo(SkBits2Float(0x4514d349), SkBits2Float(0x44b59cab));  // 2381.21f, 1452.9f
  path.conicTo(SkBits2Float(0x4514bf68), SkBits2Float(0x44b59cab), SkBits2Float(0x4514bf68), SkBits2Float(0x44b574e9), SkBits2Float(0x3f3504f3));  // 2379.96f, 1452.9f, 2379.96f, 1451.65f, 0.707107f
  path.conicTo(SkBits2Float(0x4514bf68), SkBits2Float(0x44b54d27), SkBits2Float(0x4514d349), SkBits2Float(0x44b54d27), SkBits2Float(0x3f3504f3));  // 2379.96f, 1450.41f, 2381.21f, 1450.41f, 0.707107f
  path.lineTo(SkBits2Float(0x45152fcc), SkBits2Float(0x44b54d27));  // 2386.99f, 1450.41f
  path.quadTo(SkBits2Float(0x45159d78), SkBits2Float(0x44b54d28), SkBits2Float(0x45168295), SkBits2Float(0x44b56112));  // 2393.84f, 1450.41f, 2408.16f, 1451.03f
  path.quadTo(SkBits2Float(0x451766c0), SkBits2Float(0x44b574e0), SkBits2Float(0x4518cc1f), SkBits2Float(0x44b574e8));  // 2422.42f, 1451.65f, 2444.76f, 1451.65f
  path.quadTo(SkBits2Float(0x451a3280), SkBits2Float(0x44b57500), SkBits2Float(0x451c23ab), SkBits2Float(0x44b59cb3));  // 2467.16f, 1451.66f, 2498.23f, 1452.9f
  path.quadTo(SkBits2Float(0x451e13e0), SkBits2Float(0x44b5c480), SkBits2Float(0x45202298), SkBits2Float(0x44b5c46b));  // 2529.24f, 1454.14f, 2562.16f, 1454.14f
  path.lineTo(SkBits2Float(0x4527c894), SkBits2Float(0x44b5c46b));  // 2684.54f, 1454.14f
  path.lineTo(SkBits2Float(0x452ea7c8), SkBits2Float(0x44b5c46b));  // 2794.49f, 1454.14f
  path.quadTo(SkBits2Float(0x452fefc0), SkBits2Float(0x44b5c460), SkBits2Float(0x45320760), SkBits2Float(0x44b588d9));  // 2814.98f, 1454.14f, 2848.46f, 1452.28f
  path.quadTo(SkBits2Float(0x45341f78), SkBits2Float(0x44b54d3c), SkBits2Float(0x453463f1), SkBits2Float(0x44b539ad));  // 2881.97f, 1450.41f, 2886.25f, 1449.8f
  path.quadTo(SkBits2Float(0x4534731b), SkBits2Float(0x44b53558), SkBits2Float(0x45347916), SkBits2Float(0x44b530bf));  // 2887.19f, 1449.67f, 2887.57f, 1449.52f
  path.quadTo(SkBits2Float(0x45346f6f), SkBits2Float(0x44b5382b), SkBits2Float(0x45347432), SkBits2Float(0x44b554c1));  // 2886.96f, 1449.76f, 2887.26f, 1450.65f
  path.quadTo(SkBits2Float(0x453475b0), SkBits2Float(0x44b55db7), SkBits2Float(0x45347842), SkBits2Float(0x44b5613b));  // 2887.36f, 1450.93f, 2887.52f, 1451.04f
  path.quadTo(SkBits2Float(0x45347708), SkBits2Float(0x44b55f8c), SkBits2Float(0x453473af), SkBits2Float(0x44b55ccb));  // 2887.44f, 1450.99f, 2887.23f, 1450.9f
  path.quadTo(SkBits2Float(0x45346a78), SkBits2Float(0x44b55534), SkBits2Float(0x453457ff), SkBits2Float(0x44b54bf8));  // 2886.65f, 1450.66f, 2885.5f, 1450.37f
  path.quadTo(SkBits2Float(0x45340a80), SkBits2Float(0x44b52540), SkBits2Float(0x45302049), SkBits2Float(0x44b4ae10));  // 2880.66f, 1449.16f, 2818.02f, 1445.44f
  path.quadTo(SkBits2Float(0x452c35c0), SkBits2Float(0x44b43700), SkBits2Float(0x4527f056), SkBits2Float(0x44b436dd));  // 2755.36f, 1441.72f, 2687.02f, 1441.71f
  path.quadTo(SkBits2Float(0x4523aca0), SkBits2Float(0x44b436c0), SkBits2Float(0x451fdda2), SkBits2Float(0x44b4727b));  // 2618.79f, 1441.71f, 2557.85f, 1443.58f
  path.quadTo(SkBits2Float(0x451c10e0), SkBits2Float(0x44b4ae00), SkBits2Float(0x451a0c13), SkBits2Float(0x44b51157));  // 2497.05f, 1445.44f, 2464.75f, 1448.54f
  path.quadTo(SkBits2Float(0x451807e8), SkBits2Float(0x44b57498), SkBits2Float(0x45174b9a), SkBits2Float(0x44b5b00f));  // 2432.49f, 1451.64f, 2420.73f, 1453.5f
  path.quadTo(SkBits2Float(0x4516edea), SkBits2Float(0x44b5cda2), SkBits2Float(0x4516ce40), SkBits2Float(0x44b5dc41));  // 2414.87f, 1454.43f, 2412.89f, 1454.88f
  path.quadTo(SkBits2Float(0x4516c710), SkBits2Float(0x44b5df92), SkBits2Float(0x4516c3cc), SkBits2Float(0x44b5e1c0));  // 2412.44f, 1454.99f, 2412.24f, 1455.05f
  path.lineTo(SkBits2Float(0x4516c36c), SkBits2Float(0x44b5e1e9));  // 2412.21f, 1455.06f
  path.quadTo(SkBits2Float(0x4516c978), SkBits2Float(0x44b5dba8), SkBits2Float(0x4516cbde), SkBits2Float(0x44b5cd42));  // 2412.59f, 1454.86f, 2412.74f, 1454.41f
  path.quadTo(SkBits2Float(0x4516d17f), SkBits2Float(0x44b5ab7e), SkBits2Float(0x4516c17f), SkBits2Float(0x44b59dc8));  // 2413.09f, 1453.36f, 2412.09f, 1452.93f
  path.quadTo(SkBits2Float(0x4516bfc8), SkBits2Float(0x44b59c4f), SkBits2Float(0x4516bed7), SkBits2Float(0x44b59c11));  // 2411.99f, 1452.88f, 2411.93f, 1452.88f
  path.quadTo(SkBits2Float(0x4516c122), SkBits2Float(0x44b59caa), SkBits2Float(0x4516c74c), SkBits2Float(0x44b59caa));  // 2412.07f, 1452.9f, 2412.46f, 1452.9f
  path.lineTo(SkBits2Float(0x4518907d), SkBits2Float(0x44b59caa));  // 2441.03f, 1452.9f
  path.lineTo(SkBits2Float(0x451c9034), SkBits2Float(0x44b59caa));  // 2505.01f, 1452.9f
  path.quadTo(SkBits2Float(0x451f01a0), SkBits2Float(0x44b59ca0), SkBits2Float(0x45219209), SkBits2Float(0x44b588cb));  // 2544.1f, 1452.89f, 2585.13f, 1452.27f
  path.quadTo(SkBits2Float(0x452421d0), SkBits2Float(0x44b574e0), SkBits2Float(0x4525fe8e), SkBits2Float(0x44b54d2f));  // 2626.11f, 1451.65f, 2655.91f, 1450.41f
  path.quadTo(SkBits2Float(0x4527daa0), SkBits2Float(0x44b52580), SkBits2Float(0x4529530a), SkBits2Float(0x44b4ae9d));  // 2685.66f, 1449.17f, 2709.19f, 1445.46f
  path.quadTo(SkBits2Float(0x452a0f18), SkBits2Float(0x44b47340), SkBits2Float(0x452a6584), SkBits2Float(0x44b44bbc));  // 2720.94f, 1443.6f, 2726.34f, 1442.37f
  path.quadTo(SkBits2Float(0x452a8f62), SkBits2Float(0x44b43898), SkBits2Float(0x452a9fd8), SkBits2Float(0x44b42af9));  // 2728.96f, 1441.77f, 2729.99f, 1441.34f
  path.quadTo(SkBits2Float(0x452aa313), SkBits2Float(0x44b4284d), SkBits2Float(0x452aa4cd), SkBits2Float(0x44b42646));  // 2730.19f, 1441.26f, 2730.3f, 1441.2f
  path.lineTo(SkBits2Float(0x452aa3f2), SkBits2Float(0x44b4284c));  // 2730.25f, 1441.26f
  path.quadTo(SkBits2Float(0x452a9f13), SkBits2Float(0x44b433c1), SkBits2Float(0x452aa0a1), SkBits2Float(0x44b443ba));  // 2729.94f, 1441.62f, 2730.04f, 1442.12f
  path.quadTo(SkBits2Float(0x452aa1f1), SkBits2Float(0x44b45138), SkBits2Float(0x452aa6fa), SkBits2Float(0x44b457f0));  // 2730.12f, 1442.54f, 2730.44f, 1442.75f
  path.quadTo(SkBits2Float(0x452a8d20), SkBits2Float(0x44b43570), SkBits2Float(0x4528fa48), SkBits2Float(0x44b3d32e));  // 2728.82f, 1441.67f, 2703.64f, 1438.6f
  path.quadTo(SkBits2Float(0x452763a0), SkBits2Float(0x44b37000), SkBits2Float(0x45248f5e), SkBits2Float(0x44b35c33));  // 2678.23f, 1435.5f, 2632.96f, 1434.88f
  path.quadTo(SkBits2Float(0x4521b800), SkBits2Float(0x44b34860), SkBits2Float(0x451e7738), SkBits2Float(0x44b34854));  // 2587.5f, 1434.26f, 2535.45f, 1434.26f
  path.quadTo(SkBits2Float(0x451b3480), SkBits2Float(0x44b34850), SkBits2Float(0x4519c522), SkBits2Float(0x44b35c30));  // 2483.28f, 1434.26f, 2460.32f, 1434.88f
  path.quadTo(SkBits2Float(0x45185520), SkBits2Float(0x44b37010), SkBits2Float(0x451734a0), SkBits2Float(0x44b37015));  // 2437.32f, 1435.5f, 2419.29f, 1435.5f
  path.quadTo(SkBits2Float(0x451614f8), SkBits2Float(0x44b37018), SkBits2Float(0x45154e88), SkBits2Float(0x44b383eb));  // 2401.31f, 1435.5f, 2388.91f, 1436.12f
  path.lineTo(SkBits2Float(0x4513a319), SkBits2Float(0x44b3abad));  // 2362.19f, 1437.36f
  path.quadTo(SkBits2Float(0x4512bfdc), SkBits2Float(0x44b3bf70), SkBits2Float(0x451253dd), SkBits2Float(0x44b3e6b5));  // 2347.99f, 1437.98f, 2341.24f, 1439.21f
  path.quadTo(SkBits2Float(0x4511e695), SkBits2Float(0x44b40e74), SkBits2Float(0x4511b52f), SkBits2Float(0x44b42236));  // 2334.41f, 1440.45f, 2331.32f, 1441.07f
  path.quadTo(SkBits2Float(0x4511bab4), SkBits2Float(0x44b42002), SkBits2Float(0x4511bee0), SkBits2Float(0x44b41216));  // 2331.67f, 1441, 2331.93f, 1440.57f
  path.quadTo(SkBits2Float(0x4511c4e4), SkBits2Float(0x44b3fe0a), SkBits2Float(0x4511beed), SkBits2Float(0x44b3e9ee));  // 2332.31f, 1439.94f, 2331.93f, 1439.31f
  path.quadTo(SkBits2Float(0x4511bc3c), SkBits2Float(0x44b3e0da), SkBits2Float(0x4511b7b6), SkBits2Float(0x44b3dbe7));  // 2331.76f, 1439.03f, 2331.48f, 1438.87f
  path.quadTo(SkBits2Float(0x4511b5c8), SkBits2Float(0x44b3d9ca), SkBits2Float(0x4511b444), SkBits2Float(0x44b3d91b));  // 2331.36f, 1438.81f, 2331.27f, 1438.78f
  path.quadTo(SkBits2Float(0x4511b77c), SkBits2Float(0x44b3da8f), SkBits2Float(0x4511c5b6), SkBits2Float(0x44b3dc3c));  // 2331.47f, 1438.83f, 2332.36f, 1438.88f
  path.quadTo(SkBits2Float(0x4511e4f6), SkBits2Float(0x44b3dfea), SkBits2Float(0x45123055), SkBits2Float(0x44b3e263));  // 2334.31f, 1439, 2339.02f, 1439.07f
  path.quadTo(SkBits2Float(0x4512c770), SkBits2Float(0x44b3e760), SkBits2Float(0x45140f90), SkBits2Float(0x44b3e759));  // 2348.46f, 1439.23f, 2368.97f, 1439.23f
  path.lineTo(SkBits2Float(0x45217486), SkBits2Float(0x44b3e759));  // 2583.28f, 1439.23f
  path.lineTo(SkBits2Float(0x452d69bc), SkBits2Float(0x44b3e759));  // 2774.61f, 1439.23f
  path.quadTo(SkBits2Float(0x452e8980), SkBits2Float(0x44b3e750), SkBits2Float(0x452f6db3), SkBits2Float(0x44b3d382));  // 2792.59f, 1439.23f, 2806.86f, 1438.61f
  path.quadTo(SkBits2Float(0x45305258), SkBits2Float(0x44b3bfa8), SkBits2Float(0x4530faf5), SkBits2Float(0x44b3abc8));  // 2821.15f, 1437.99f, 2831.68f, 1437.37f
  path.quadTo(SkBits2Float(0x4531a260), SkBits2Float(0x44b39814), SkBits2Float(0x4531fa3a), SkBits2Float(0x44b37107));  // 2842.15f, 1436.75f, 2847.64f, 1435.53f
  path.quadTo(SkBits2Float(0x45320a50), SkBits2Float(0x44b369e1), SkBits2Float(0x45320b03), SkBits2Float(0x44b3684e));  // 2848.64f, 1435.31f, 2848.69f, 1435.26f
  path.quadTo(SkBits2Float(0x4532038f), SkBits2Float(0x44b3790f), SkBits2Float(0x453209b0), SkBits2Float(0x44b38c74));  // 2848.22f, 1435.78f, 2848.61f, 1436.39f
  path.lineTo(SkBits2Float(0x45320986), SkBits2Float(0x44b38eab));  // 2848.6f, 1436.46f
  path.quadTo(SkBits2Float(0x4531f368), SkBits2Float(0x44b37890), SkBits2Float(0x4531980c), SkBits2Float(0x44b35bb7));  // 2847.21f, 1435.77f, 2841.5f, 1434.87f
  path.quadTo(SkBits2Float(0x4530dc00), SkBits2Float(0x44b32040), SkBits2Float(0x452ca20b), SkBits2Float(0x44b2bd23));  // 2829.75f, 1433.01f, 2762.13f, 1429.91f
  path.quadTo(SkBits2Float(0x452866a0), SkBits2Float(0x44b259c0), SkBits2Float(0x45248f71), SkBits2Float(0x44b245e9));  // 2694.41f, 1426.8f, 2632.97f, 1426.18f
  path.quadTo(SkBits2Float(0x4520b980), SkBits2Float(0x44b23200), SkBits2Float(0x451d431c), SkBits2Float(0x44b23209));  // 2571.59f, 1425.56f, 2516.19f, 1425.56f
  path.lineTo(SkBits2Float(0x45175270), SkBits2Float(0x44b23209));  // 2421.15f, 1425.56f
  path.quadTo(SkBits2Float(0x4514d7f0), SkBits2Float(0x44b23200), SkBits2Float(0x45139b5e), SkBits2Float(0x44b294f2));  // 2381.5f, 1425.56f, 2361.71f, 1428.65f
  path.quadTo(SkBits2Float(0x451263b9), SkBits2Float(0x44b2f652), SkBits2Float(0x4512546d), SkBits2Float(0x44b314ec));  // 2342.23f, 1431.7f, 2341.28f, 1432.65f
  path.quadTo(SkBits2Float(0x45125e84), SkBits2Float(0x44b300bc), SkBits2Float(0x451255e3), SkBits2Float(0x44b2eb29));  // 2341.91f, 1432.02f, 2341.37f, 1431.35f
  path.lineTo(SkBits2Float(0x45125933), SkBits2Float(0x44b2ee1a));  // 2341.57f, 1431.44f
  path.quadTo(SkBits2Float(0x45126676), SkBits2Float(0x44b2fb5c), SkBits2Float(0x45128777), SkBits2Float(0x44b30e38));  // 2342.4f, 1431.85f, 2344.47f, 1432.44f
  path.quadTo(SkBits2Float(0x4512cc40), SkBits2Float(0x44b33588), SkBits2Float(0x45136090), SkBits2Float(0x44b370da));  // 2348.77f, 1433.67f, 2358.04f, 1435.53f
  path.quadTo(SkBits2Float(0x45148a40), SkBits2Float(0x44b3e7c0), SkBits2Float(0x45192838), SkBits2Float(0x44b525c1));  // 2376.64f, 1439.24f, 2450.51f, 1449.18f
  path.quadTo(SkBits2Float(0x451dc620), SkBits2Float(0x44b66380), SkBits2Float(0x45219d13), SkBits2Float(0x44b6b2fe));  // 2524.38f, 1459.11f, 2585.82f, 1461.59f
  path.quadTo(SkBits2Float(0x45257460), SkBits2Float(0x44b70280), SkBits2Float(0x45275150), SkBits2Float(0x44b70278));  // 2647.27f, 1464.08f, 2677.08f, 1464.08f
  path.lineTo(SkBits2Float(0x452aeda3), SkBits2Float(0x44b70278));  // 2734.85f, 1464.08f
  path.lineTo(SkBits2Float(0x452e3a74), SkBits2Float(0x44b70278));  // 2787.65f, 1464.08f
  path.quadTo(SkBits2Float(0x452fc700), SkBits2Float(0x44b70280), SkBits2Float(0x45326a69), SkBits2Float(0x44b69f30));  // 2812.44f, 1464.08f, 2854.65f, 1460.97f
  path.quadTo(SkBits2Float(0x45350c40), SkBits2Float(0x44b63c24), SkBits2Float(0x453577d1), SkBits2Float(0x44b60175));  // 2896.77f, 1457.88f, 2903.49f, 1456.05f
  path.quadTo(SkBits2Float(0x45358ebb), SkBits2Float(0x44b5f4f5), SkBits2Float(0x4535953a), SkBits2Float(0x44b5e997));  // 2904.92f, 1455.65f, 2905.33f, 1455.3f
  path.quadTo(SkBits2Float(0x4535914b), SkBits2Float(0x44b5f078), SkBits2Float(0x453594e7), SkBits2Float(0x44b5fa17));  // 2905.08f, 1455.51f, 2905.31f, 1455.82f
  path.quadTo(SkBits2Float(0x453589f8), SkBits2Float(0x44b5dce8), SkBits2Float(0x45353bce), SkBits2Float(0x44b5aeee));  // 2904.62f, 1454.9f, 2899.74f, 1453.47f
  path.quadTo(SkBits2Float(0x453495a0), SkBits2Float(0x44b54d40), SkBits2Float(0x4531fe8a), SkBits2Float(0x44b54d28));  // 2889.35f, 1450.41f, 2847.91f, 1450.41f
  path.lineTo(SkBits2Float(0x452bdc2d), SkBits2Float(0x44b54d28));  // 2749.76f, 1450.41f
  path.lineTo(SkBits2Float(0x45229eb1), SkBits2Float(0x44b54d28));  // 2601.92f, 1450.41f
  path.quadTo(SkBits2Float(0x451cea80), SkBits2Float(0x44b54d20), SkBits2Float(0x451a4664), SkBits2Float(0x44b574e5));  // 2510.66f, 1450.41f, 2468.4f, 1451.65f
  path.quadTo(SkBits2Float(0x4517a380), SkBits2Float(0x44b59c90), SkBits2Float(0x45163e58), SkBits2Float(0x44b5ebef));  // 2426.22f, 1452.89f, 2403.9f, 1455.37f
  path.quadTo(SkBits2Float(0x4514da30), SkBits2Float(0x44b63b16), SkBits2Float(0x4514963b), SkBits2Float(0x44b661ea));  // 2381.64f, 1457.85f, 2377.39f, 1459.06f
  path.quadTo(SkBits2Float(0x451487d3), SkBits2Float(0x44b66a26), SkBits2Float(0x451484d0), SkBits2Float(0x44b66e29));  // 2376.49f, 1459.32f, 2376.3f, 1459.44f
  path.quadTo(SkBits2Float(0x45148bf6), SkBits2Float(0x44b664a3), SkBits2Float(0x45148aed), SkBits2Float(0x44b64ff5));  // 2376.75f, 1459.14f, 2376.68f, 1458.5f
  path.quadTo(SkBits2Float(0x451489e2), SkBits2Float(0x44b63b10), SkBits2Float(0x451481ca), SkBits2Float(0x44b63498));  // 2376.62f, 1457.85f, 2376.11f, 1457.64f
  path.quadTo(SkBits2Float(0x45148aa8), SkBits2Float(0x44b63bb0), SkBits2Float(0x4514c277), SkBits2Float(0x44b63bb0));  // 2376.67f, 1457.87f, 2380.15f, 1457.87f
  path.lineTo(SkBits2Float(0x451c18f0), SkBits2Float(0x44b63bb0));  // 2497.56f, 1457.87f
  path.lineTo(SkBits2Float(0x45256a4c), SkBits2Float(0x44b63bb0));  // 2646.64f, 1457.87f
  path.quadTo(SkBits2Float(0x4527dc00), SkBits2Float(0x44b63bb8), SkBits2Float(0x4528c031), SkBits2Float(0x44b627d9));  // 2685.75f, 1457.87f, 2700.01f, 1457.25f
  path.quadTo(SkBits2Float(0x4529a3f8), SkBits2Float(0x44b61408), SkBits2Float(0x452a6069), SkBits2Float(0x44b5ec64));  // 2714.25f, 1456.63f, 2726.03f, 1455.39f
  path.lineTo(SkBits2Float(0x452a6493), SkBits2Float(0x44b63b78));  // 2726.29f, 1457.86f
  path.close();
  return path;
}

static SkPath getPath3() {
  SkPath path;
  path.setFillType(SkPathFillType::kWinding);
  path.moveTo(SkBits2Float(0x4527c847), SkBits2Float(0x44b6dab5));  // 2684.52f, 1462.83f
  path.lineTo(SkBits2Float(0x451d7481), SkBits2Float(0x44b68b32));  // 2519.28f, 1460.35f
  path.quadTo(SkBits2Float(0x45197ce0), SkBits2Float(0x44b66340), SkBits2Float(0x45168a9e), SkBits2Float(0x44b613e0));  // 2455.8f, 1459.1f, 2408.66f, 1456.62f
  path.quadTo(SkBits2Float(0x45139760), SkBits2Float(0x44b5c460), SkBits2Float(0x45124fb5), SkBits2Float(0x44b5b087));  // 2361.46f, 1454.14f, 2340.98f, 1453.52f
  path.quadTo(SkBits2Float(0x4511ab88), SkBits2Float(0x44b5a698), SkBits2Float(0x451162e4), SkBits2Float(0x44b59c92));  // 2330.72f, 1453.21f, 2326.18f, 1452.89f
  path.quadTo(SkBits2Float(0x45113db0), SkBits2Float(0x44b59771), SkBits2Float(0x45112f77), SkBits2Float(0x44b59206));  // 2323.86f, 1452.73f, 2322.97f, 1452.56f
  path.quadTo(SkBits2Float(0x45112068), SkBits2Float(0x44b58c49), SkBits2Float(0x45111b06), SkBits2Float(0x44b57b11));  // 2322.03f, 1452.38f, 2321.69f, 1451.85f
  path.quadTo(SkBits2Float(0x451114c0), SkBits2Float(0x44b566fe), SkBits2Float(0x45111a4d), SkBits2Float(0x44b5521a));  // 2321.3f, 1451.22f, 2321.64f, 1450.57f
  path.quadTo(SkBits2Float(0x45111d15), SkBits2Float(0x44b547a1), SkBits2Float(0x4511226a), SkBits2Float(0x44b54150));  // 2321.82f, 1450.24f, 2322.15f, 1450.04f
  path.quadTo(SkBits2Float(0x451125f6), SkBits2Float(0x44b53d1c), SkBits2Float(0x45112b42), SkBits2Float(0x44b53a76));  // 2322.37f, 1449.91f, 2322.7f, 1449.83f
  path.quadTo(SkBits2Float(0x45115462), SkBits2Float(0x44b525e4), SkBits2Float(0x4513eff9), SkBits2Float(0x44b4ae49));  // 2325.27f, 1449.18f, 2367, 1445.45f
  path.quadTo(SkBits2Float(0x45168a80), SkBits2Float(0x44b436e0), SkBits2Float(0x45182328), SkBits2Float(0x44b436dc));  // 2408.66f, 1441.71f, 2434.2f, 1441.71f
  path.lineTo(SkBits2Float(0x451dc450), SkBits2Float(0x44b436dc));  // 2524.27f, 1441.71f
  path.lineTo(SkBits2Float(0x45280436), SkBits2Float(0x44b436dc));  // 2688.26f, 1441.71f
  path.quadTo(SkBits2Float(0x452e3940), SkBits2Float(0x44b43700), SkBits2Float(0x453187bd), SkBits2Float(0x44b45ea1));  // 2787.58f, 1441.72f, 2840.48f, 1442.96f
  path.quadTo(SkBits2Float(0x4534d500), SkBits2Float(0x44b48640), SkBits2Float(0x4536814d), SkBits2Float(0x44b4d60e));  // 2893.31f, 1444.2f, 2920.08f, 1446.69f
  path.quadTo(SkBits2Float(0x45382e40), SkBits2Float(0x44b525e0), SkBits2Float(0x4538576a), SkBits2Float(0x44b53a76));  // 2946.89f, 1449.18f, 2949.46f, 1449.83f
  path.quadTo(SkBits2Float(0x4538684c), SkBits2Float(0x44b542e6), SkBits2Float(0x45386b47), SkBits2Float(0x44b55daf));  // 2950.52f, 1450.09f, 2950.7f, 1450.93f
  path.quadTo(SkBits2Float(0x45386f66), SkBits2Float(0x44b582be), SkBits2Float(0x45385a86), SkBits2Float(0x44b591a7));  // 2950.96f, 1452.09f, 2949.66f, 1452.55f
  path.quadTo(SkBits2Float(0x45384fe0), SkBits2Float(0x44b59942), SkBits2Float(0x4538327f), SkBits2Float(0x44b5a145));  // 2948.99f, 1452.79f, 2947.16f, 1453.04f
  path.quadTo(SkBits2Float(0x4537fb52), SkBits2Float(0x44b5b050), SkBits2Float(0x45377976), SkBits2Float(0x44b5c44e));  // 2943.71f, 1453.51f, 2935.59f, 1454.13f
  path.quadTo(SkBits2Float(0x45367660), SkBits2Float(0x44b5ec30), SkBits2Float(0x4533c7bb), SkBits2Float(0x44b5ec2e));  // 2919.4f, 1455.38f, 2876.48f, 1455.38f
  path.lineTo(SkBits2Float(0x452d55db), SkBits2Float(0x44b5ec2e));  // 2773.37f, 1455.38f
  path.quadTo(SkBits2Float(0x45299180), SkBits2Float(0x44b5ec00), SkBits2Float(0x452586f9), SkBits2Float(0x44b574d8));  // 2713.09f, 1455.38f, 2648.44f, 1451.65f
  path.quadTo(SkBits2Float(0x45217d00), SkBits2Float(0x44b4fd80), SkBits2Float(0x451e9330), SkBits2Float(0x44b47253));  // 2583.81f, 1447.92f, 2537.2f, 1443.57f
  path.lineTo(SkBits2Float(0x451a8979), SkBits2Float(0x44b3ab89));  // 2472.59f, 1437.36f
  path.quadTo(SkBits2Float(0x4519f0f0), SkBits2Float(0x44b38bf8), SkBits2Float(0x4519e1cc), SkBits2Float(0x44b36622));  // 2463.06f, 1436.37f, 2462.11f, 1435.19f
  path.quadTo(SkBits2Float(0x4519d82a), SkBits2Float(0x44b34e0e), SkBits2Float(0x4519df9f), SkBits2Float(0x44b333a0));  // 2461.51f, 1434.44f, 2461.98f, 1433.61f
  path.quadTo(SkBits2Float(0x4519e376), SkBits2Float(0x44b32602), SkBits2Float(0x4519ebd9), SkBits2Float(0x44b31da0));  // 2462.22f, 1433.19f, 2462.74f, 1432.93f
  path.quadTo(SkBits2Float(0x4519f42c), SkBits2Float(0x44b3154e), SkBits2Float(0x451a051c), SkBits2Float(0x44b30ead));  // 2463.26f, 1432.67f, 2464.32f, 1432.46f
  path.quadTo(SkBits2Float(0x451a22c8), SkBits2Float(0x44b30310), SkBits2Float(0x451a6209), SkBits2Float(0x44b2f8f1));  // 2466.17f, 1432.1f, 2470.13f, 1431.78f
  path.quadTo(SkBits2Float(0x451b5b40), SkBits2Float(0x44b2d110), SkBits2Float(0x451d6ade), SkBits2Float(0x44b2d10e));  // 2485.7f, 1430.53f, 2518.68f, 1430.53f
  path.lineTo(SkBits2Float(0x4522ee34), SkBits2Float(0x44b2d10e));  // 2606.89f, 1430.53f
  path.lineTo(SkBits2Float(0x452a6c6e), SkBits2Float(0x44b2d10e));  // 2726.78f, 1430.53f
  path.quadTo(SkBits2Float(0x452e7680), SkBits2Float(0x44b2d100), SkBits2Float(0x4531a60e), SkBits2Float(0x44b3209e));  // 2791.41f, 1430.53f, 2842.38f, 1433.02f
  path.quadTo(SkBits2Float(0x4534d540), SkBits2Float(0x44b37020), SkBits2Float(0x4536097a), SkBits2Float(0x44b397ec));  // 2893.33f, 1435.5f, 2912.59f, 1436.75f
  path.quadTo(SkBits2Float(0x4536a380), SkBits2Float(0x44b3abc8), SkBits2Float(0x4536df89), SkBits2Float(0x44b3b5cb));  // 2922.22f, 1437.37f, 2925.97f, 1437.68f
  path.quadTo(SkBits2Float(0x4536fe80), SkBits2Float(0x44b3baf4), SkBits2Float(0x453705d0), SkBits2Float(0x44b3bde1));  // 2927.91f, 1437.84f, 2928.36f, 1437.93f
  path.quadTo(SkBits2Float(0x453712ae), SkBits2Float(0x44b3c307), SkBits2Float(0x45371667), SkBits2Float(0x44b3d963));  // 2929.17f, 1438.09f, 2929.4f, 1438.79f
  path.quadTo(SkBits2Float(0x45371b00), SkBits2Float(0x44b3f4fe), SkBits2Float(0x45370f3e), SkBits2Float(0x44b4063c));  // 2929.69f, 1439.66f, 2928.95f, 1440.19f
  path.quadTo(SkBits2Float(0x45370c5c), SkBits2Float(0x44b40a76), SkBits2Float(0x453708dc), SkBits2Float(0x44b40c85));  // 2928.77f, 1440.33f, 2928.55f, 1440.39f
  path.quadTo(SkBits2Float(0x453706dc), SkBits2Float(0x44b40db2), SkBits2Float(0x4537046b), SkBits2Float(0x44b40e3d));  // 2928.43f, 1440.43f, 2928.28f, 1440.44f
  path.quadTo(SkBits2Float(0x45370084), SkBits2Float(0x44b40f1b), SkBits2Float(0x4536f6bb), SkBits2Float(0x44b40f1b));  // 2928.03f, 1440.47f, 2927.42f, 1440.47f
  path.lineTo(SkBits2Float(0x45354b5a), SkBits2Float(0x44b40f1b));  // 2900.71f, 1440.47f
  path.lineTo(SkBits2Float(0x453119f0), SkBits2Float(0x44b40f1b));  // 2833.62f, 1440.47f
  path.quadTo(SkBits2Float(0x452e4ea0), SkBits2Float(0x44b40f20), SkBits2Float(0x452a309c), SkBits2Float(0x44b3fb39));  // 2788.91f, 1440.47f, 2723.04f, 1439.85f
  path.lineTo(SkBits2Float(0x452110f7), SkBits2Float(0x44b3d378));  // 2577.06f, 1438.61f
  path.quadTo(SkBits2Float(0x451c0d40), SkBits2Float(0x44b3bf80), SkBits2Float(0x4517f0b7), SkBits2Float(0x44b3700d));  // 2496.83f, 1437.98f, 2431.04f, 1435.5f
  path.quadTo(SkBits2Float(0x4513d440), SkBits2Float(0x44b32080), SkBits2Float(0x4511bb39), SkBits2Float(0x44b32092));  // 2365.27f, 1433.02f, 2331.7f, 1433.02f
  path.lineTo(SkBits2Float(0x450f7ac2), SkBits2Float(0x44b32092));  // 2295.67f, 1433.02f
  path.quadTo(SkBits2Float(0x450f73d4), SkBits2Float(0x44b32092), SkBits2Float(0x450f711d), SkBits2Float(0x44b31f7c));  // 2295.24f, 1433.02f, 2295.07f, 1432.98f
  path.quadTo(SkBits2Float(0x450f6cfa), SkBits2Float(0x44b31dd4), SkBits2Float(0x450f698f), SkBits2Float(0x44b31918));  // 2294.81f, 1432.93f, 2294.6f, 1432.78f
  path.quadTo(SkBits2Float(0x450f606c), SkBits2Float(0x44b30c6e), SkBits2Float(0x450f6105), SkBits2Float(0x44b2f63c));  // 2294.03f, 1432.39f, 2294.06f, 1431.69f
  path.quadTo(SkBits2Float(0x450f618a), SkBits2Float(0x44b2e31c), SkBits2Float(0x450f6957), SkBits2Float(0x44b2d7f8));  // 2294.1f, 1431.1f, 2294.58f, 1430.75f
  path.quadTo(SkBits2Float(0x450f6d13), SkBits2Float(0x44b2d2a4), SkBits2Float(0x450f71d5), SkBits2Float(0x44b2d0f6));  // 2294.82f, 1430.58f, 2295.11f, 1430.53f
  path.quadTo(SkBits2Float(0x450f7351), SkBits2Float(0x44b2d070), SkBits2Float(0x450f75da), SkBits2Float(0x44b2cffa));  // 2295.21f, 1430.51f, 2295.37f, 1430.5f
  path.quadTo(SkBits2Float(0x450f7d04), SkBits2Float(0x44b2ceac), SkBits2Float(0x450f950d), SkBits2Float(0x44b2cc25));  // 2295.81f, 1430.46f, 2297.32f, 1430.38f
  path.quadTo(SkBits2Float(0x450fc4a6), SkBits2Float(0x44b2c722), SkBits2Float(0x451036bd), SkBits2Float(0x44b2bd37));  // 2300.29f, 1430.22f, 2307.42f, 1429.91f
  path.quadTo(SkBits2Float(0x45111bd8), SkBits2Float(0x44b2a948), SkBits2Float(0x4514ea38), SkBits2Float(0x44b2a94c));  // 2321.74f, 1429.29f, 2382.64f, 1429.29f
  path.quadTo(SkBits2Float(0x4518b9e0), SkBits2Float(0x44b2a940), SkBits2Float(0x451c86ac), SkBits2Float(0x44b2d111));  // 2443.62f, 1429.29f, 2504.42f, 1430.53f
  path.quadTo(SkBits2Float(0x45204f80), SkBits2Float(0x44b2f8c0), SkBits2Float(0x45243663), SkBits2Float(0x44b30cb1));  // 2564.97f, 1431.77f, 2627.4f, 1432.4f
  path.quadTo(SkBits2Float(0x452818e0), SkBits2Float(0x44b320c0), SkBits2Float(0x452b205a), SkBits2Float(0x44b37022));  // 2689.55f, 1433.02f, 2738.02f, 1435.5f
  path.quadTo(SkBits2Float(0x452e2820), SkBits2Float(0x44b3bfc0), SkBits2Float(0x452f216a), SkBits2Float(0x44b3fb83));  // 2786.51f, 1437.99f, 2802.09f, 1439.86f
  path.quadTo(SkBits2Float(0x452f9e78), SkBits2Float(0x44b41984), SkBits2Float(0x452fc474), SkBits2Float(0x44b428ba));  // 2809.9f, 1440.8f, 2812.28f, 1441.27f
  path.quadTo(SkBits2Float(0x452fcf3c), SkBits2Float(0x44b42d0a), SkBits2Float(0x452fd47e), SkBits2Float(0x44b430d3));  // 2812.95f, 1441.41f, 2813.28f, 1441.53f
  path.quadTo(SkBits2Float(0x452fda9c), SkBits2Float(0x44b4353a), SkBits2Float(0x452fde56), SkBits2Float(0x44b43cae));  // 2813.66f, 1441.66f, 2813.9f, 1441.9f
  path.quadTo(SkBits2Float(0x452fe606), SkBits2Float(0x44b44c0e), SkBits2Float(0x452fe412), SkBits2Float(0x44b46123));  // 2814.38f, 1442.38f, 2814.25f, 1443.04f
  path.quadTo(SkBits2Float(0x452fe264), SkBits2Float(0x44b4734a), SkBits2Float(0x452fda50), SkBits2Float(0x44b47cfb));  // 2814.15f, 1443.6f, 2813.64f, 1443.91f
  path.quadTo(SkBits2Float(0x452fd27c), SkBits2Float(0x44b48660), SkBits2Float(0x452fb422), SkBits2Float(0x44b48660));  // 2813.16f, 1444.2f, 2811.26f, 1444.2f
  path.lineTo(SkBits2Float(0x452b1565), SkBits2Float(0x44b48660));  // 2737.34f, 1444.2f
  path.lineTo(SkBits2Float(0x45233db7), SkBits2Float(0x44b48660));  // 2611.86f, 1444.2f
  path.quadTo(SkBits2Float(0x451fa1c0), SkBits2Float(0x44b48680), SkBits2Float(0x451deb9e), SkBits2Float(0x44b4727c));  // 2554.11f, 1444.2f, 2526.73f, 1443.58f
  path.quadTo(SkBits2Float(0x451c3610), SkBits2Float(0x44b45ea0), SkBits2Float(0x45194364), SkBits2Float(0x44b45e9e));  // 2499.38f, 1442.96f, 2452.21f, 1442.96f
  path.lineTo(SkBits2Float(0x4514fe19), SkBits2Float(0x44b45e9e));  // 2383.88f, 1442.96f
  path.lineTo(SkBits2Float(0x4513cfbf), SkBits2Float(0x44b45e9e));  // 2364.98f, 1442.96f
  path.lineTo(SkBits2Float(0x4513cfbf), SkBits2Float(0x44b436dc));  // 2364.98f, 1441.71f
  path.lineTo(SkBits2Float(0x4513cfbf), SkBits2Float(0x44b40f1a));  // 2364.98f, 1440.47f
  path.lineTo(SkBits2Float(0x4513d3ee), SkBits2Float(0x44b40f1a));  // 2365.25f, 1440.47f
  path.quadTo(SkBits2Float(0x4513fbf5), SkBits2Float(0x44b40f19), SkBits2Float(0x451560f0), SkBits2Float(0x44b3fb3e));  // 2367.75f, 1440.47f, 2390.06f, 1439.85f
  path.lineTo(SkBits2Float(0x451a139e), SkBits2Float(0x44b3bf9c));  // 2465.23f, 1437.99f
  path.quadTo(SkBits2Float(0x451d5f20), SkBits2Float(0x44b39800), SkBits2Float(0x4521d7bd), SkBits2Float(0x44b383f6));  // 2517.95f, 1436.75f, 2589.48f, 1436.12f
  path.lineTo(SkBits2Float(0x452b1f40), SkBits2Float(0x44b37015));  // 2737.95f, 1435.5f
  path.lineTo(SkBits2Float(0x453470b2), SkBits2Float(0x44b37014));  // 2887.04f, 1435.5f
  path.quadTo(SkBits2Float(0x4538f0c0), SkBits2Float(0x44b37040), SkBits2Float(0x453c34bb), SkBits2Float(0x44b383f6));  // 2959.05f, 1435.51f, 3011.3f, 1436.12f
  path.quadTo(SkBits2Float(0x453f77c0), SkBits2Float(0x44b397c0), SkBits2Float(0x45410fd7), SkBits2Float(0x44b3bfa5));  // 3063.48f, 1436.74f, 3088.99f, 1437.99f
  path.quadTo(SkBits2Float(0x4541dbf0), SkBits2Float(0x44b3d390), SkBits2Float(0x4542219d), SkBits2Float(0x44b3dd83));  // 3101.75f, 1438.61f, 3106.1f, 1438.92f
  path.quadTo(SkBits2Float(0x45424bcb), SkBits2Float(0x44b3e38a), SkBits2Float(0x45425247), SkBits2Float(0x44b3f082));  // 3108.74f, 1439.11f, 3109.14f, 1439.52f
  path.quadTo(SkBits2Float(0x45425e6a), SkBits2Float(0x44b408c8), SkBits2Float(0x45425447), SkBits2Float(0x44b4246f));  // 3109.9f, 1440.27f, 3109.27f, 1441.14f
  path.quadTo(SkBits2Float(0x454250af), SkBits2Float(0x44b42e3c), SkBits2Float(0x45424ae2), SkBits2Float(0x44b43282));  // 3109.04f, 1441.44f, 3108.68f, 1441.58f
  path.quadTo(SkBits2Float(0x4542486c), SkBits2Float(0x44b43451), SkBits2Float(0x45424552), SkBits2Float(0x44b43521));  // 3108.53f, 1441.63f, 3108.33f, 1441.66f
  path.quadTo(SkBits2Float(0x454242a8), SkBits2Float(0x44b435d3), SkBits2Float(0x45423ddf), SkBits2Float(0x44b43635));  // 3108.17f, 1441.68f, 3107.87f, 1441.69f
  path.quadTo(SkBits2Float(0x454235ac), SkBits2Float(0x44b436dd), SkBits2Float(0x4542252a), SkBits2Float(0x44b436dd));  // 3107.35f, 1441.71f, 3106.32f, 1441.71f
  path.lineTo(SkBits2Float(0x45314ba2), SkBits2Float(0x44b436dd));  // 2836.73f, 1441.71f
  path.lineTo(SkBits2Float(0x451f2a1e), SkBits2Float(0x44b436dd));  // 2546.63f, 1441.71f
  path.quadTo(SkBits2Float(0x451d5fc0), SkBits2Float(0x44b436e0), SkBits2Float(0x451b8d5e), SkBits2Float(0x44b422f9));  // 2517.98f, 1441.71f, 2488.84f, 1441.09f
  path.quadTo(SkBits2Float(0x451aa2b0), SkBits2Float(0x44b41900), SkBits2Float(0x451a3e5e), SkBits2Float(0x44b3ffe3));  // 2474.17f, 1440.78f, 2467.9f, 1440
  path.quadTo(SkBits2Float(0x451a094c), SkBits2Float(0x44b3f29e), SkBits2Float(0x4519f619), SkBits2Float(0x44b3df6c));  // 2464.58f, 1439.58f, 2463.38f, 1438.98f
  path.quadTo(SkBits2Float(0x4519e22b), SkBits2Float(0x44b3cb7e), SkBits2Float(0x4519e364), SkBits2Float(0x44b3a80b));  // 2462.14f, 1438.36f, 2462.21f, 1437.25f
  path.quadTo(SkBits2Float(0x4519e46e), SkBits2Float(0x44b389d0), SkBits2Float(0x4519f65e), SkBits2Float(0x44b37551));  // 2462.28f, 1436.31f, 2463.4f, 1435.67f
  path.quadTo(SkBits2Float(0x451a3fbe), SkBits2Float(0x44b32174), SkBits2Float(0x451c1740), SkBits2Float(0x44b2d133));  // 2467.98f, 1433.05f, 2497.45f, 1430.54f
  path.quadTo(SkBits2Float(0x451dea50), SkBits2Float(0x44b281c0), SkBits2Float(0x4521c3a1), SkBits2Float(0x44b259cd));  // 2526.64f, 1428.05f, 2588.23f, 1426.81f
  path.quadTo(SkBits2Float(0x452599e0), SkBits2Float(0x44b23200), SkBits2Float(0x452a3abc), SkBits2Float(0x44b23208));  // 2649.62f, 1425.56f, 2723.67f, 1425.56f
  path.quadTo(SkBits2Float(0x452edac0), SkBits2Float(0x44b23200), SkBits2Float(0x4533b42b), SkBits2Float(0x44b259cc));  // 2797.67f, 1425.56f, 2875.26f, 1426.81f
  path.quadTo(SkBits2Float(0x45389000), SkBits2Float(0x44b28180), SkBits2Float(0x453c545a), SkBits2Float(0x44b34888));  // 2953, 1428.05f, 3013.27f, 1434.27f
  path.quadTo(SkBits2Float(0x45401980), SkBits2Float(0x44b40fc0), SkBits2Float(0x4541a7db), SkBits2Float(0x44b4aee6));  // 3073.59f, 1440.49f, 3098.49f, 1445.47f
  path.quadTo(SkBits2Float(0x45427110), SkBits2Float(0x44b4ff60), SkBits2Float(0x4542b175), SkBits2Float(0x44b53d31));  // 3111.07f, 1447.98f, 3115.09f, 1449.91f
  path.quadTo(SkBits2Float(0x4542df22), SkBits2Float(0x44b56908), SkBits2Float(0x4542d818), SkBits2Float(0x44b5a157));  // 3117.95f, 1451.28f, 3117.51f, 1453.04f
  path.quadTo(SkBits2Float(0x4542d226), SkBits2Float(0x44b5d0ec), SkBits2Float(0x4542a18d), SkBits2Float(0x44b5ead6));  // 3117.13f, 1454.53f, 3114.1f, 1455.34f
  path.quadTo(SkBits2Float(0x45420a20), SkBits2Float(0x44b63b98), SkBits2Float(0x45404888), SkBits2Float(0x44b64f8f));  // 3104.63f, 1457.86f, 3076.53f, 1458.49f
  path.quadTo(SkBits2Float(0x453e88c0), SkBits2Float(0x44b66380), SkBits2Float(0x453d2cf8), SkBits2Float(0x44b66372));  // 3048.55f, 1459.11f, 3026.81f, 1459.11f
  path.lineTo(SkBits2Float(0x453d2cf8), SkBits2Float(0x44b613ee));  // 3026.81f, 1456.62f
  path.quadTo(SkBits2Float(0x453e8860), SkBits2Float(0x44b613f0), SkBits2Float(0x454047a6), SkBits2Float(0x44b60011));  // 3048.52f, 1456.62f, 3076.48f, 1456
  path.quadTo(SkBits2Float(0x45420478), SkBits2Float(0x44b5ec50), SkBits2Float(0x4542974f), SkBits2Float(0x44b59e02));  // 3104.28f, 1455.38f, 3113.46f, 1452.94f
  path.quadTo(SkBits2Float(0x4542a794), SkBits2Float(0x44b59555), SkBits2Float(0x4542af88), SkBits2Float(0x44b58c86));  // 3114.47f, 1452.67f, 3114.97f, 1452.39f
  path.lineTo(SkBits2Float(0x4542b186), SkBits2Float(0x44b58e0d));  // 3115.1f, 1452.44f
  path.quadTo(SkBits2Float(0x4542b050), SkBits2Float(0x44b597c2), SkBits2Float(0x4542b179), SkBits2Float(0x44b59a08));  // 3115.02f, 1452.74f, 3115.09f, 1452.81f
  path.quadTo(SkBits2Float(0x4542acd3), SkBits2Float(0x44b590ef), SkBits2Float(0x4542a041), SkBits2Float(0x44b584df));  // 3114.8f, 1452.53f, 3114.02f, 1452.15f
  path.quadTo(SkBits2Float(0x45426460), SkBits2Float(0x44b54b68), SkBits2Float(0x4541a00f), SkBits2Float(0x44b4fcde));  // 3110.27f, 1450.36f, 3098, 1447.9f
  path.quadTo(SkBits2Float(0x454013c0), SkBits2Float(0x44b45e40), SkBits2Float(0x453c5046), SkBits2Float(0x44b397a0));  // 3073.23f, 1442.95f, 3013.02f, 1436.74f
  path.quadTo(SkBits2Float(0x45388dc0), SkBits2Float(0x44b2d100), SkBits2Float(0x4533b389), SkBits2Float(0x44b2a94c));  // 2952.86f, 1430.53f, 2875.22f, 1429.29f
  path.quadTo(SkBits2Float(0x452edac0), SkBits2Float(0x44b28180), SkBits2Float(0x452a3abc), SkBits2Float(0x44b2818c));  // 2797.67f, 1428.05f, 2723.67f, 1428.05f
  path.quadTo(SkBits2Float(0x45259a20), SkBits2Float(0x44b28180), SkBits2Float(0x4521c46f), SkBits2Float(0x44b2a94b));  // 2649.63f, 1428.05f, 2588.28f, 1429.29f
  path.quadTo(SkBits2Float(0x451dec70), SkBits2Float(0x44b2d110), SkBits2Float(0x451c1aa0), SkBits2Float(0x44b3206d));  // 2526.78f, 1430.53f, 2497.66f, 1433.01f
  path.quadTo(SkBits2Float(0x451a4bde), SkBits2Float(0x44b36f30), SkBits2Float(0x451a0a18), SkBits2Float(0x44b3ba5b));  // 2468.74f, 1435.47f, 2464.63f, 1437.82f
  path.quadTo(SkBits2Float(0x451a0aa2), SkBits2Float(0x44b3b9bd), SkBits2Float(0x451a0b0e), SkBits2Float(0x44b3ad85));  // 2464.66f, 1437.8f, 2464.69f, 1437.42f
  path.quadTo(SkBits2Float(0x451a0b7c), SkBits2Float(0x44b3a11b), SkBits2Float(0x451a07b7), SkBits2Float(0x44b398d0));  // 2464.72f, 1437.03f, 2464.48f, 1436.78f
  path.lineTo(SkBits2Float(0x451a07e1), SkBits2Float(0x44b3984e));  // 2464.49f, 1436.76f
  path.quadTo(SkBits2Float(0x451a14fd), SkBits2Float(0x44b3a56a), SkBits2Float(0x451a434c), SkBits2Float(0x44b3b0fd));  // 2465.31f, 1437.17f, 2468.21f, 1437.53f
  path.quadTo(SkBits2Float(0x451aa598), SkBits2Float(0x44b3c998), SkBits2Float(0x451b8e36), SkBits2Float(0x44b3d37b));  // 2474.35f, 1438.3f, 2488.89f, 1438.61f
  path.quadTo(SkBits2Float(0x451d6010), SkBits2Float(0x44b3e760), SkBits2Float(0x451f2a1e), SkBits2Float(0x44b3e759));  // 2518, 1439.23f, 2546.63f, 1439.23f
  path.lineTo(SkBits2Float(0x45314ba2), SkBits2Float(0x44b3e759));  // 2836.73f, 1439.23f
  path.lineTo(SkBits2Float(0x4542252a), SkBits2Float(0x44b3e759));  // 3106.32f, 1439.23f
  path.quadTo(SkBits2Float(0x454234f7), SkBits2Float(0x44b3e759), SkBits2Float(0x45423c49), SkBits2Float(0x44b3e6c3));  // 3107.31f, 1439.23f, 3107.77f, 1439.21f
  path.quadTo(SkBits2Float(0x45423f45), SkBits2Float(0x44b3e686), SkBits2Float(0x4542402e), SkBits2Float(0x44b3e649));  // 3107.95f, 1439.2f, 3108.01f, 1439.2f
  path.quadTo(SkBits2Float(0x45423edd), SkBits2Float(0x44b3e6a1), SkBits2Float(0x45423d24), SkBits2Float(0x44b3e7e6));  // 3107.93f, 1439.21f, 3107.82f, 1439.25f
  path.quadTo(SkBits2Float(0x454237b3), SkBits2Float(0x44b3ebe9), SkBits2Float(0x45423437), SkBits2Float(0x44b3f569));  // 3107.48f, 1439.37f, 3107.26f, 1439.67f
  path.quadTo(SkBits2Float(0x45422a2e), SkBits2Float(0x44b410c5), SkBits2Float(0x4542362b), SkBits2Float(0x44b428bc));  // 3106.64f, 1440.52f, 3107.39f, 1441.27f
  path.quadTo(SkBits2Float(0x454239dc), SkBits2Float(0x44b4301c), SkBits2Float(0x45423e70), SkBits2Float(0x44b43277));  // 3107.62f, 1441.5f, 3107.9f, 1441.58f
  path.lineTo(SkBits2Float(0x45423e57), SkBits2Float(0x44b43279));  // 3107.9f, 1441.58f
  path.quadTo(SkBits2Float(0x45423c81), SkBits2Float(0x44b431e2), SkBits2Float(0x4542388c), SkBits2Float(0x44b43110));  // 3107.78f, 1441.56f, 3107.53f, 1441.53f
  path.quadTo(SkBits2Float(0x45422fd6), SkBits2Float(0x44b42f43), SkBits2Float(0x45421ec7), SkBits2Float(0x44b42cd3));  // 3106.99f, 1441.48f, 3105.92f, 1441.4f
  path.quadTo(SkBits2Float(0x4541d970), SkBits2Float(0x44b422e8), SkBits2Float(0x45410de7), SkBits2Float(0x44b40f0f));  // 3101.59f, 1441.09f, 3088.87f, 1440.47f
  path.quadTo(SkBits2Float(0x453f7680), SkBits2Float(0x44b3e740), SkBits2Float(0x453c3441), SkBits2Float(0x44b3d378));  // 3063.41f, 1439.23f, 3011.27f, 1438.61f
  path.quadTo(SkBits2Float(0x4538f080), SkBits2Float(0x44b3bf80), SkBits2Float(0x453470b2), SkBits2Float(0x44b3bf98));  // 2959.03f, 1437.98f, 2887.04f, 1437.99f
  path.lineTo(SkBits2Float(0x452b1f6a), SkBits2Float(0x44b3bf97));  // 2737.96f, 1437.99f
  path.lineTo(SkBits2Float(0x4521d815), SkBits2Float(0x44b3d378));  // 2589.51f, 1438.61f
  path.quadTo(SkBits2Float(0x451d5fe0), SkBits2Float(0x44b3e780), SkBits2Float(0x451a149a), SkBits2Float(0x44b40f18));  // 2517.99f, 1439.23f, 2465.29f, 1440.47f
  path.lineTo(SkBits2Float(0x4515620a), SkBits2Float(0x44b44aba));  // 2390.13f, 1442.34f
  path.quadTo(SkBits2Float(0x4513fc7f), SkBits2Float(0x44b45e9d), SkBits2Float(0x4513d3ee), SkBits2Float(0x44b45e9e));  // 2367.78f, 1442.96f, 2365.25f, 1442.96f
  path.lineTo(SkBits2Float(0x4513cfbf), SkBits2Float(0x44b45e9e));  // 2364.98f, 1442.96f
  path.conicTo(SkBits2Float(0x4513bbde), SkBits2Float(0x44b45e9e), SkBits2Float(0x4513bbde), SkBits2Float(0x44b436dc), SkBits2Float(0x3f3504f3));  // 2363.74f, 1442.96f, 2363.74f, 1441.71f, 0.707107f
  path.conicTo(SkBits2Float(0x4513bbde), SkBits2Float(0x44b40f1a), SkBits2Float(0x4513cfbf), SkBits2Float(0x44b40f1a), SkBits2Float(0x3f3504f3));  // 2363.74f, 1440.47f, 2364.98f, 1440.47f, 0.707107f
  path.lineTo(SkBits2Float(0x4514fe19), SkBits2Float(0x44b40f1a));  // 2383.88f, 1440.47f
  path.lineTo(SkBits2Float(0x45194364), SkBits2Float(0x44b40f1a));  // 2452.21f, 1440.47f
  path.quadTo(SkBits2Float(0x451c3680), SkBits2Float(0x44b40f10), SkBits2Float(0x451dec86), SkBits2Float(0x44b422fe));  // 2499.41f, 1440.47f, 2526.78f, 1441.09f
  path.quadTo(SkBits2Float(0x451fa200), SkBits2Float(0x44b436c0), SkBits2Float(0x45233db7), SkBits2Float(0x44b436dc));  // 2554.12f, 1441.71f, 2611.86f, 1441.71f
  path.lineTo(SkBits2Float(0x452b1565), SkBits2Float(0x44b436dc));  // 2737.34f, 1441.71f
  path.lineTo(SkBits2Float(0x452fb422), SkBits2Float(0x44b436dc));  // 2811.26f, 1441.71f
  path.quadTo(SkBits2Float(0x452fbfe6), SkBits2Float(0x44b436dc), SkBits2Float(0x452fc63a), SkBits2Float(0x44b43604));  // 2811.99f, 1441.71f, 2812.39f, 1441.69f
  path.lineTo(SkBits2Float(0x452fc5dc), SkBits2Float(0x44b438cd));  // 2812.37f, 1441.78f
  path.quadTo(SkBits2Float(0x452fbe90), SkBits2Float(0x44b4418e), SkBits2Float(0x452fbcfa), SkBits2Float(0x44b452a9));  // 2811.91f, 1442.05f, 2811.81f, 1442.58f
  path.quadTo(SkBits2Float(0x452fbb1e), SkBits2Float(0x44b466b0), SkBits2Float(0x452fc23a), SkBits2Float(0x44b474e8));  // 2811.69f, 1443.21f, 2812.14f, 1443.65f
  path.quadTo(SkBits2Float(0x452fc4c9), SkBits2Float(0x44b47a07), SkBits2Float(0x452fc706), SkBits2Float(0x44b47ba3));  // 2812.3f, 1443.81f, 2812.44f, 1443.86f
  path.quadTo(SkBits2Float(0x452fc48a), SkBits2Float(0x44b479da), SkBits2Float(0x452fbca8), SkBits2Float(0x44b476b2));  // 2812.28f, 1443.81f, 2811.79f, 1443.71f
  path.quadTo(SkBits2Float(0x452f9830), SkBits2Float(0x44b4681c), SkBits2Float(0x452f1cae), SkBits2Float(0x44b44a75));  // 2809.51f, 1443.25f, 2801.79f, 1442.33f
  path.quadTo(SkBits2Float(0x452e24c0), SkBits2Float(0x44b40ee0), SkBits2Float(0x452b1e50), SkBits2Float(0x44b3bf8a));  // 2786.3f, 1440.46f, 2737.89f, 1437.99f
  path.quadTo(SkBits2Float(0x452817a0), SkBits2Float(0x44b37040), SkBits2Float(0x452435fd), SkBits2Float(0x44b35c33));  // 2689.48f, 1435.51f, 2627.37f, 1434.88f
  path.quadTo(SkBits2Float(0x45204f00), SkBits2Float(0x44b34840), SkBits2Float(0x451c85dc), SkBits2Float(0x44b3208f));  // 2564.94f, 1434.26f, 2504.37f, 1433.02f
  path.quadTo(SkBits2Float(0x4518b9a0), SkBits2Float(0x44b2f8c0), SkBits2Float(0x4514ea38), SkBits2Float(0x44b2f8d0));  // 2443.6f, 1431.77f, 2382.64f, 1431.78f
  path.quadTo(SkBits2Float(0x45111ca8), SkBits2Float(0x44b2f8d0), SkBits2Float(0x45103877), SkBits2Float(0x44b30ca7));  // 2321.79f, 1431.78f, 2307.53f, 1432.4f
  path.quadTo(SkBits2Float(0x450fc64f), SkBits2Float(0x44b31693), SkBits2Float(0x450f9723), SkBits2Float(0x44b31b8b));  // 2300.39f, 1432.71f, 2297.45f, 1432.86f
  path.quadTo(SkBits2Float(0x450f7ff2), SkBits2Float(0x44b31dfc), SkBits2Float(0x450f7974), SkBits2Float(0x44b31f2a));  // 2296, 1432.94f, 2295.59f, 1432.97f
  path.lineTo(SkBits2Float(0x450f78bd), SkBits2Float(0x44b31f44));  // 2295.55f, 1432.98f
  path.quadTo(SkBits2Float(0x450f7cd6), SkBits2Float(0x44b31dd2), SkBits2Float(0x450f806f), SkBits2Float(0x44b318b0));  // 2295.8f, 1432.93f, 2296.03f, 1432.77f
  path.quadTo(SkBits2Float(0x450f8834), SkBits2Float(0x44b30d9a), SkBits2Float(0x450f88b7), SkBits2Float(0x44b2fa84));  // 2296.51f, 1432.43f, 2296.54f, 1431.83f
  path.quadTo(SkBits2Float(0x450f8950), SkBits2Float(0x44b2e45c), SkBits2Float(0x450f8035), SkBits2Float(0x44b2d7be));  // 2296.58f, 1431.14f, 2296.01f, 1430.74f
  path.quadTo(SkBits2Float(0x450f7cda), SkBits2Float(0x44b2d318), SkBits2Float(0x450f78e9), SkBits2Float(0x44b2d184));  // 2295.8f, 1430.6f, 2295.56f, 1430.55f
  path.lineTo(SkBits2Float(0x450f7ac2), SkBits2Float(0x44b2d10e));  // 2295.67f, 1430.53f
  path.lineTo(SkBits2Float(0x4511bb39), SkBits2Float(0x44b2d10e));  // 2331.7f, 1430.53f
  path.quadTo(SkBits2Float(0x4513d4e0), SkBits2Float(0x44b2d140), SkBits2Float(0x4517f237), SkBits2Float(0x44b32099));  // 2365.3f, 1430.54f, 2431.14f, 1433.02f
  path.quadTo(SkBits2Float(0x451c0e40), SkBits2Float(0x44b37000), SkBits2Float(0x4521114d), SkBits2Float(0x44b383f6));  // 2496.89f, 1435.5f, 2577.08f, 1436.12f
  path.lineTo(SkBits2Float(0x452a30fc), SkBits2Float(0x44b3abb7));  // 2723.06f, 1437.37f
  path.quadTo(SkBits2Float(0x452e4ea0), SkBits2Float(0x44b3bfa0), SkBits2Float(0x453119f0), SkBits2Float(0x44b3bf97));  // 2788.91f, 1437.99f, 2833.62f, 1437.99f
  path.lineTo(SkBits2Float(0x45354b5a), SkBits2Float(0x44b3bf97));  // 2900.71f, 1437.99f
  path.lineTo(SkBits2Float(0x4536f6bb), SkBits2Float(0x44b3bf97));  // 2927.42f, 1437.99f
  path.quadTo(SkBits2Float(0x4536fe56), SkBits2Float(0x44b3bf97), SkBits2Float(0x45370007), SkBits2Float(0x44b3bf37));  // 2927.9f, 1437.99f, 2928, 1437.98f
  path.quadTo(SkBits2Float(0x4536fefc), SkBits2Float(0x44b3bf71), SkBits2Float(0x4536fda4), SkBits2Float(0x44b3c03d));  // 2927.94f, 1437.98f, 2927.85f, 1438.01f
  path.quadTo(SkBits2Float(0x4536fa76), SkBits2Float(0x44b3c21b), SkBits2Float(0x4536f7bc), SkBits2Float(0x44b3c61c));  // 2927.65f, 1438.07f, 2927.48f, 1438.19f
  path.quadTo(SkBits2Float(0x4536ec20), SkBits2Float(0x44b3d723), SkBits2Float(0x4536f0af), SkBits2Float(0x44b3f283));  // 2926.76f, 1438.72f, 2927.04f, 1439.58f
  path.quadTo(SkBits2Float(0x4536f440), SkBits2Float(0x44b407f1), SkBits2Float(0x4536fe04), SkBits2Float(0x44b40bd9));  // 2927.27f, 1440.25f, 2927.88f, 1440.37f
  path.quadTo(SkBits2Float(0x4536f8ea), SkBits2Float(0x44b409ce), SkBits2Float(0x4536dc3b), SkBits2Float(0x44b40507));  // 2927.56f, 1440.31f, 2925.76f, 1440.16f
  path.quadTo(SkBits2Float(0x4536a090), SkBits2Float(0x44b3fb18), SkBits2Float(0x453606ea), SkBits2Float(0x44b3e744));  // 2922.04f, 1439.85f, 2912.43f, 1439.23f
  path.quadTo(SkBits2Float(0x4534d300), SkBits2Float(0x44b3bfa0), SkBits2Float(0x4531a41e), SkBits2Float(0x44b37008));  // 2893.19f, 1437.99f, 2842.26f, 1435.5f
  path.quadTo(SkBits2Float(0x452e7600), SkBits2Float(0x44b32080), SkBits2Float(0x452a6c6e), SkBits2Float(0x44b32092));  // 2791.38f, 1433.02f, 2726.78f, 1433.02f
  path.lineTo(SkBits2Float(0x4522ee34), SkBits2Float(0x44b32092));  // 2606.89f, 1433.02f
  path.lineTo(SkBits2Float(0x451d6ade), SkBits2Float(0x44b32092));  // 2518.68f, 1433.02f
  path.quadTo(SkBits2Float(0x451b5cc8), SkBits2Float(0x44b32090), SkBits2Float(0x451a6535), SkBits2Float(0x44b34833));  // 2485.8f, 1433.02f, 2470.33f, 1434.26f
  path.quadTo(SkBits2Float(0x451a2832), SkBits2Float(0x44b351f8), SkBits2Float(0x451a0cbe), SkBits2Float(0x44b35cb5));  // 2466.51f, 1434.56f, 2464.8f, 1434.9f
  path.quadTo(SkBits2Float(0x451a011e), SkBits2Float(0x44b36142), SkBits2Float(0x4519fda1), SkBits2Float(0x44b364be));  // 2464.07f, 1435.04f, 2463.85f, 1435.15f
  path.quadTo(SkBits2Float(0x451a001d), SkBits2Float(0x44b36242), SkBits2Float(0x451a023f), SkBits2Float(0x44b35ab2));  // 2464.01f, 1435.07f, 2464.14f, 1434.83f
  path.quadTo(SkBits2Float(0x451a07fe), SkBits2Float(0x44b34654), SkBits2Float(0x451a00d8), SkBits2Float(0x44b33476));  // 2464.5f, 1434.2f, 2464.05f, 1433.64f
  path.quadTo(SkBits2Float(0x451a0598), SkBits2Float(0x44b34050), SkBits2Float(0x451a8d47), SkBits2Float(0x44b35c63));  // 2464.35f, 1434.01f, 2472.83f, 1434.89f
  path.lineTo(SkBits2Float(0x451e96e2), SkBits2Float(0x44b42327));  // 2537.43f, 1441.1f
  path.quadTo(SkBits2Float(0x45218040), SkBits2Float(0x44b4ae40), SkBits2Float(0x45258943), SkBits2Float(0x44b52576));  // 2584.02f, 1445.45f, 2648.58f, 1449.17f
  path.quadTo(SkBits2Float(0x45299280), SkBits2Float(0x44b59cc0), SkBits2Float(0x452d55db), SkBits2Float(0x44b59caa));  // 2713.16f, 1452.9f, 2773.37f, 1452.9f
  path.lineTo(SkBits2Float(0x4533c7bb), SkBits2Float(0x44b59caa));  // 2876.48f, 1452.9f
  path.quadTo(SkBits2Float(0x453674e0), SkBits2Float(0x44b59cb0), SkBits2Float(0x4537766a), SkBits2Float(0x44b57506));  // 2919.3f, 1452.9f, 2935.4f, 1451.66f
  path.quadTo(SkBits2Float(0x4537f724), SkBits2Float(0x44b56136), SkBits2Float(0x45382d1f), SkBits2Float(0x44b5527d));  // 2943.45f, 1451.04f, 2946.82f, 1450.58f
  path.quadTo(SkBits2Float(0x45384664), SkBits2Float(0x44b54b99), SkBits2Float(0x45384d26), SkBits2Float(0x44b546c5));  // 2948.4f, 1450.36f, 2948.82f, 1450.21f
  path.quadTo(SkBits2Float(0x453840f9), SkBits2Float(0x44b54f79), SkBits2Float(0x45384479), SkBits2Float(0x44b56ef1));  // 2948.06f, 1450.48f, 2948.28f, 1451.47f
  path.quadTo(SkBits2Float(0x453846d5), SkBits2Float(0x44b58422), SkBits2Float(0x45384dc6), SkBits2Float(0x44b5879a));  // 2948.43f, 1452.13f, 2948.86f, 1452.24f
  path.quadTo(SkBits2Float(0x45382790), SkBits2Float(0x44b57480), SkBits2Float(0x45367d9f), SkBits2Float(0x44b5253a));  // 2946.47f, 1451.64f, 2919.85f, 1449.16f
  path.quadTo(SkBits2Float(0x4534d2a0), SkBits2Float(0x44b4d5c0), SkBits2Float(0x453186cd), SkBits2Float(0x44b4ae1f));  // 2893.16f, 1446.68f, 2840.43f, 1445.44f
  path.quadTo(SkBits2Float(0x452e38c0), SkBits2Float(0x44b48640), SkBits2Float(0x45280436), SkBits2Float(0x44b48660));  // 2787.55f, 1444.2f, 2688.26f, 1444.2f
  path.lineTo(SkBits2Float(0x451dc450), SkBits2Float(0x44b48660));  // 2524.27f, 1444.2f
  path.lineTo(SkBits2Float(0x45182328), SkBits2Float(0x44b48660));  // 2434.2f, 1444.2f
  path.quadTo(SkBits2Float(0x45168c60), SkBits2Float(0x44b48660), SkBits2Float(0x4513f385), SkBits2Float(0x44b4fd7b));  // 2408.77f, 1444.2f, 2367.22f, 1447.92f
  path.quadTo(SkBits2Float(0x45115b06), SkBits2Float(0x44b57488), SkBits2Float(0x451134e6), SkBits2Float(0x44b5879a));  // 2325.69f, 1451.64f, 2323.31f, 1452.24f
  path.quadTo(SkBits2Float(0x4511353a), SkBits2Float(0x44b5876f), SkBits2Float(0x451136ae), SkBits2Float(0x44b585b8));  // 2323.33f, 1452.23f, 2323.42f, 1452.18f
  path.quadTo(SkBits2Float(0x45113af4), SkBits2Float(0x44b580a7), SkBits2Float(0x45113d69), SkBits2Float(0x44b5776a));  // 2323.68f, 1452.02f, 2323.84f, 1451.73f
  path.quadTo(SkBits2Float(0x451142a2), SkBits2Float(0x44b563c6), SkBits2Float(0x45113cbe), SkBits2Float(0x44b550ef));  // 2324.16f, 1451.12f, 2323.8f, 1450.53f
  path.quadTo(SkBits2Float(0x45113961), SkBits2Float(0x44b5462c), SkBits2Float(0x45113481), SkBits2Float(0x44b542da));  // 2323.59f, 1450.19f, 2323.28f, 1450.09f
  path.quadTo(SkBits2Float(0x45113500), SkBits2Float(0x44b54331), SkBits2Float(0x451136e7), SkBits2Float(0x44b543ea));  // 2323.31f, 1450.1f, 2323.43f, 1450.12f
  path.quadTo(SkBits2Float(0x451142d0), SkBits2Float(0x44b54872), SkBits2Float(0x451165a0), SkBits2Float(0x44b54d40));  // 2324.18f, 1450.26f, 2326.35f, 1450.41f
  path.quadTo(SkBits2Float(0x4511ad70), SkBits2Float(0x44b55728), SkBits2Float(0x451250e9), SkBits2Float(0x44b5610d));  // 2330.84f, 1450.72f, 2341.06f, 1451.03f
  path.quadTo(SkBits2Float(0x451398e0), SkBits2Float(0x44b574e0), SkBits2Float(0x45168cb4), SkBits2Float(0x44b5c47a));  // 2361.55f, 1451.65f, 2408.79f, 1454.14f
  path.quadTo(SkBits2Float(0x45197e40), SkBits2Float(0x44b613c0), SkBits2Float(0x451d751b), SkBits2Float(0x44b63bb2));  // 2455.89f, 1456.62f, 2519.32f, 1457.87f
  path.lineTo(SkBits2Float(0x4527c8e1), SkBits2Float(0x44b68b35));  // 2684.55f, 1460.35f
  path.lineTo(SkBits2Float(0x4527c847), SkBits2Float(0x44b6dab5));  // 2684.52f, 1462.83f
  path.close();
  return path;
}

static SkPath getPath4() {
  SkPath path;
  path.setFillType(SkPathFillType::kWinding);
  path.moveTo(SkBits2Float(0x453d2cf8), SkBits2Float(0x44b66372));  // 3026.81f, 1459.11f
  path.lineTo(SkBits2Float(0x453a2faa), SkBits2Float(0x44b66372));  // 2978.98f, 1459.11f
  path.lineTo(SkBits2Float(0x4536c508), SkBits2Float(0x44b66372));  // 2924.31f, 1459.11f
  path.quadTo(SkBits2Float(0x4534fad0), SkBits2Float(0x44b66370), SkBits2Float(0x4532f6a1), SkBits2Float(0x44b64f90));  // 2895.68f, 1459.11f, 2863.41f, 1458.49f
  path.lineTo(SkBits2Float(0x452d4b81), SkBits2Float(0x44b613ed));  // 2772.72f, 1456.62f
  path.quadTo(SkBits2Float(0x4529a380), SkBits2Float(0x44b5ec00), SkBits2Float(0x4526894b), SkBits2Float(0x44b588b6));  // 2714.22f, 1455.38f, 2664.58f, 1452.27f
  path.quadTo(SkBits2Float(0x45236e80), SkBits2Float(0x44b52560), SkBits2Float(0x4521b9a4), SkBits2Float(0x44b51182));  // 2614.91f, 1449.17f, 2587.6f, 1448.55f
  path.quadTo(SkBits2Float(0x45200420), SkBits2Float(0x44b4fda0), SkBits2Float(0x451fbf34), SkBits2Float(0x44b4fda4));  // 2560.26f, 1447.93f, 2555.95f, 1447.93f
  path.quadTo(SkBits2Float(0x451faae5), SkBits2Float(0x44b4fda4), SkBits2Float(0x451fa1e3), SkBits2Float(0x44b4f83c));  // 2554.68f, 1447.93f, 2554.12f, 1447.76f
  path.quadTo(SkBits2Float(0x451f939d), SkBits2Float(0x44b4efab), SkBits2Float(0x451f90c7), SkBits2Float(0x44b4d61e));  // 2553.23f, 1447.49f, 2553.05f, 1446.69f
  path.quadTo(SkBits2Float(0x451f8db0), SkBits2Float(0x44b4ba4a), SkBits2Float(0x451f9a6f), SkBits2Float(0x44b4a72c));  // 2552.86f, 1445.82f, 2553.65f, 1445.22f
  path.quadTo(SkBits2Float(0x451fa769), SkBits2Float(0x44b493b6), SkBits2Float(0x451fe1bb), SkBits2Float(0x44b473e5));  // 2554.46f, 1444.62f, 2558.11f, 1443.62f
  path.quadTo(SkBits2Float(0x45205130), SkBits2Float(0x44b43718), SkBits2Float(0x452332b3), SkBits2Float(0x44b3e768));  // 2565.07f, 1441.72f, 2611.17f, 1439.23f
  path.quadTo(SkBits2Float(0x452613e0), SkBits2Float(0x44b397c0), SkBits2Float(0x452955ab), SkBits2Float(0x44b37017));  // 2657.24f, 1436.74f, 2709.35f, 1435.5f
  path.quadTo(SkBits2Float(0x452c99a0), SkBits2Float(0x44b34840), SkBits2Float(0x453070fa), SkBits2Float(0x44b34852));  // 2761.6f, 1434.26f, 2823.06f, 1434.26f
  path.quadTo(SkBits2Float(0x45344a80), SkBits2Float(0x44b34840), SkBits2Float(0x4538175e), SkBits2Float(0x44b37017));  // 2884.66f, 1434.26f, 2945.46f, 1435.5f
  path.quadTo(SkBits2Float(0x453be640), SkBits2Float(0x44b397c0), SkBits2Float(0x453e277e), SkBits2Float(0x44b40f50));  // 3006.39f, 1436.74f, 3042.47f, 1440.48f
  path.quadTo(SkBits2Float(0x45406880), SkBits2Float(0x44b486a0), SkBits2Float(0x4540d6ca), SkBits2Float(0x44b4aec5));  // 3078.53f, 1444.21f, 3085.42f, 1445.46f
  path.quadTo(SkBits2Float(0x4540f446), SkBits2Float(0x44b4b97e), SkBits2Float(0x45410216), SkBits2Float(0x44b4c51f));  // 3087.27f, 1445.8f, 3088.13f, 1446.16f
  path.quadTo(SkBits2Float(0x45410c90), SkBits2Float(0x44b4cdf2), SkBits2Float(0x454111b8), SkBits2Float(0x44b4daa2));  // 3088.79f, 1446.44f, 3089.11f, 1446.83f
  path.quadTo(SkBits2Float(0x45411d86), SkBits2Float(0x44b4f7b0), SkBits2Float(0x45411259), SkBits2Float(0x44b5157e));  // 3089.85f, 1447.74f, 3089.15f, 1448.67f
  path.quadTo(SkBits2Float(0x454105f4), SkBits2Float(0x44b5368e), SkBits2Float(0x4540b83c), SkBits2Float(0x44b54cc1));  // 3088.37f, 1449.7f, 3083.51f, 1450.4f
  path.quadTo(SkBits2Float(0x45402c18), SkBits2Float(0x44b574d0), SkBits2Float(0x453e89c8), SkBits2Float(0x44b59c9e));  // 3074.76f, 1451.65f, 3048.61f, 1452.89f
  path.quadTo(SkBits2Float(0x453ce7c0), SkBits2Float(0x44b5c470), SkBits2Float(0x453a753e), SkBits2Float(0x44b5c46c));  // 3022.48f, 1454.14f, 2983.33f, 1454.14f
  path.quadTo(SkBits2Float(0x453802e0), SkBits2Float(0x44b5c460), SkBits2Float(0x4532ec9d), SkBits2Float(0x44b588c7));  // 2944.18f, 1454.14f, 2862.79f, 1452.27f
  path.lineTo(SkBits2Float(0x452b0b10), SkBits2Float(0x44b53945));  // 2736.69f, 1449.79f
  path.quadTo(SkBits2Float(0x45283de0), SkBits2Float(0x44b52560), SkBits2Float(0x4527475f), SkBits2Float(0x44b52566));  // 2691.87f, 1449.17f, 2676.46f, 1449.17f
  path.quadTo(SkBits2Float(0x45264e18), SkBits2Float(0x44b52570), SkBits2Float(0x4525d5f9), SkBits2Float(0x44b51161));  // 2660.88f, 1449.17f, 2653.37f, 1448.54f
  path.quadTo(SkBits2Float(0x45255c5c), SkBits2Float(0x44b4fd1c), SkBits2Float(0x45253c42), SkBits2Float(0x44b4e7b9));  // 2645.77f, 1447.91f, 2643.77f, 1447.24f
  path.quadTo(SkBits2Float(0x45252c50), SkBits2Float(0x44b4dd18), SkBits2Float(0x45252ae1), SkBits2Float(0x44b4c05f));  // 2642.77f, 1446.91f, 2642.68f, 1446.01f
  path.quadTo(SkBits2Float(0x4525296a), SkBits2Float(0x44b4a309), SkBits2Float(0x45253867), SkBits2Float(0x44b492ae));  // 2642.59f, 1445.09f, 2643.53f, 1444.58f
  path.quadTo(SkBits2Float(0x45254196), SkBits2Float(0x44b488a8), SkBits2Float(0x45255952), SkBits2Float(0x44b47d7d));  // 2644.1f, 1444.27f, 2645.58f, 1443.92f
  path.quadTo(SkBits2Float(0x4525844c), SkBits2Float(0x44b46944), SkBits2Float(0x4525e88f), SkBits2Float(0x44b44b2d));  // 2648.27f, 1443.29f, 2654.53f, 1442.35f
  path.quadTo(SkBits2Float(0x4526afb0), SkBits2Float(0x44b40f70), SkBits2Float(0x4528a1a6), SkBits2Float(0x44b3bfb8));  // 2666.98f, 1440.48f, 2698.1f, 1437.99f
  path.quadTo(SkBits2Float(0x452a9340), SkBits2Float(0x44b37020), SkBits2Float(0x452d9b6e), SkBits2Float(0x44b37014));  // 2729.2f, 1435.5f, 2777.71f, 1435.5f
  path.quadTo(SkBits2Float(0x4530a540), SkBits2Float(0x44b37020), SkBits2Float(0x45340d14), SkBits2Float(0x44b35c34));  // 2826.33f, 1435.5f, 2880.82f, 1434.88f
  path.quadTo(SkBits2Float(0x45377540), SkBits2Float(0x44b34860), SkBits2Float(0x453a438c), SkBits2Float(0x44b34852));  // 2935.33f, 1434.26f, 2980.22f, 1434.26f
  path.quadTo(SkBits2Float(0x453d0f80), SkBits2Float(0x44b34860), SkBits2Float(0x453eb18a), SkBits2Float(0x44b37020));  // 3024.97f, 1434.26f, 3051.1f, 1435.5f
  path.quadTo(SkBits2Float(0x453f8270), SkBits2Float(0x44b38400), SkBits2Float(0x453fc5de), SkBits2Float(0x44b38e02));  // 3064.15f, 1436.12f, 3068.37f, 1436.44f
  path.quadTo(SkBits2Float(0x453fd716), SkBits2Float(0x44b3908f), SkBits2Float(0x453fdf6b), SkBits2Float(0x44b39285));  // 3069.44f, 1436.52f, 3069.96f, 1436.58f
  path.quadTo(SkBits2Float(0x453fe461), SkBits2Float(0x44b393b0), SkBits2Float(0x453fe71d), SkBits2Float(0x44b394e7));  // 3070.27f, 1436.62f, 3070.44f, 1436.65f
  path.quadTo(SkBits2Float(0x453feaba), SkBits2Float(0x44b39682), SkBits2Float(0x453fed83), SkBits2Float(0x44b3998c));  // 3070.67f, 1436.7f, 3070.84f, 1436.8f
  path.quadTo(SkBits2Float(0x453ff52f), SkBits2Float(0x44b3a1ec), SkBits2Float(0x453ff765), SkBits2Float(0x44b3b29f));  // 3071.32f, 1437.06f, 3071.46f, 1437.58f
  path.quadTo(SkBits2Float(0x453ffb0d), SkBits2Float(0x44b3ce44), SkBits2Float(0x453fef1c), SkBits2Float(0x44b3de30));  // 3071.69f, 1438.45f, 3070.94f, 1438.94f
  path.quadTo(SkBits2Float(0x453fe83c), SkBits2Float(0x44b3e75a), SkBits2Float(0x453fbcf2), SkBits2Float(0x44b3e75a));  // 3070.51f, 1439.23f, 3067.81f, 1439.23f
  path.lineTo(SkBits2Float(0x453586fc), SkBits2Float(0x44b3e75a));  // 2904.44f, 1439.23f
  path.lineTo(SkBits2Float(0x4527d284), SkBits2Float(0x44b3e75a));  // 2685.16f, 1439.23f
  path.quadTo(SkBits2Float(0x4523be80), SkBits2Float(0x44b3e780), SkBits2Float(0x45213846), SkBits2Float(0x44b3bf93));  // 2619.91f, 1439.23f, 2579.52f, 1437.99f
  path.quadTo(SkBits2Float(0x451eb160), SkBits2Float(0x44b397e0), SkBits2Float(0x451dfdc1), SkBits2Float(0x44b36fd6));  // 2539.09f, 1436.75f, 2527.86f, 1435.49f
  path.quadTo(SkBits2Float(0x451da2a4), SkBits2Float(0x44b35b94), SkBits2Float(0x451d8fa5), SkBits2Float(0x44b350bc));  // 2522.17f, 1434.86f, 2520.98f, 1434.52f
  path.quadTo(SkBits2Float(0x451d7a3b), SkBits2Float(0x44b34480), SkBits2Float(0x451d7d4d), SkBits2Float(0x44b31fa1));  // 2519.64f, 1434.14f, 2519.83f, 1432.99f
  path.quadTo(SkBits2Float(0x451d7fdf), SkBits2Float(0x44b300c6), SkBits2Float(0x451d9319), SkBits2Float(0x44b2fba6));  // 2519.99f, 1432.02f, 2521.19f, 1431.86f
  path.quadTo(SkBits2Float(0x451d9dbc), SkBits2Float(0x44b2f8d0), SkBits2Float(0x451dba60), SkBits2Float(0x44b2f8d0));  // 2521.86f, 1431.78f, 2523.65f, 1431.78f
  path.quadTo(SkBits2Float(0x451e2710), SkBits2Float(0x44b2f8d0), SkBits2Float(0x451ebb77), SkBits2Float(0x44b2e506));  // 2530.44f, 1431.78f, 2539.72f, 1431.16f
  path.quadTo(SkBits2Float(0x451f5138), SkBits2Float(0x44b2d110), SkBits2Float(0x45203679), SkBits2Float(0x44b2d10e));  // 2549.08f, 1430.53f, 2563.4f, 1430.53f
  path.lineTo(SkBits2Float(0x4522630e), SkBits2Float(0x44b2d10e));  // 2598.19f, 1430.53f
  path.lineTo(SkBits2Float(0x452715ad), SkBits2Float(0x44b2d10e));  // 2673.35f, 1430.53f
  path.quadTo(SkBits2Float(0x452a8180), SkBits2Float(0x44b2d120), SkBits2Float(0x452c53dd), SkBits2Float(0x44b2e4f2));  // 2728.09f, 1430.54f, 2757.24f, 1431.15f
  path.quadTo(SkBits2Float(0x452e27d0), SkBits2Float(0x44b2f8e0), SkBits2Float(0x452fb59f), SkBits2Float(0x44b3348f));  // 2786.49f, 1431.78f, 2811.35f, 1433.64f
  path.quadTo(SkBits2Float(0x453143e0), SkBits2Float(0x44b37040), SkBits2Float(0x45335310), SkBits2Float(0x44b3fb92));  // 2836.24f, 1435.51f, 2869.19f, 1439.86f
  path.quadTo(SkBits2Float(0x45356c40), SkBits2Float(0x44b48980), SkBits2Float(0x45357af3), SkBits2Float(0x44b4c419));  // 2902.77f, 1444.3f, 2903.68f, 1446.13f
  path.quadTo(SkBits2Float(0x453583ba), SkBits2Float(0x44b4e739), SkBits2Float(0x45357245), SkBits2Float(0x44b4fc9a));  // 2904.23f, 1447.23f, 2903.14f, 1447.89f
  path.quadTo(SkBits2Float(0x45356dbb), SkBits2Float(0x44b50229), SkBits2Float(0x4535664e), SkBits2Float(0x44b505cf));  // 2902.86f, 1448.07f, 2902.39f, 1448.18f
  path.quadTo(SkBits2Float(0x45355c10), SkBits2Float(0x44b50ad7), SkBits2Float(0x453547b5), SkBits2Float(0x44b50ed8));  // 2901.75f, 1448.34f, 2900.48f, 1448.46f
  path.quadTo(SkBits2Float(0x4535212a), SkBits2Float(0x44b5166e), SkBits2Float(0x4534d23b), SkBits2Float(0x44b51b70));  // 2898.07f, 1448.7f, 2893.14f, 1448.86f
  path.quadTo(SkBits2Float(0x45343560), SkBits2Float(0x44b52568), SkBits2Float(0x4532f703), SkBits2Float(0x44b52566));  // 2883.34f, 1449.17f, 2863.44f, 1449.17f
  path.lineTo(SkBits2Float(0x452f151c), SkBits2Float(0x44b52566));  // 2801.32f, 1449.17f
  path.lineTo(SkBits2Float(0x4527f055), SkBits2Float(0x44b52566));  // 2687.02f, 1449.17f
  path.quadTo(SkBits2Float(0x45222ec0), SkBits2Float(0x44b52580), SkBits2Float(0x451ef82f), SkBits2Float(0x44b51184));  // 2594.92f, 1449.17f, 2543.51f, 1448.55f
  path.quadTo(SkBits2Float(0x451bbde0), SkBits2Float(0x44b4fd80), SkBits2Float(0x4519ff59), SkBits2Float(0x44b4d5d9));  // 2491.87f, 1447.92f, 2463.96f, 1446.68f
  path.quadTo(SkBits2Float(0x45183f40), SkBits2Float(0x44b4ae10), SkBits2Float(0x4517ef00), SkBits2Float(0x44b499f2));  // 2435.95f, 1445.44f, 2430.94f, 1444.81f
  path.quadTo(SkBits2Float(0x4517c54d), SkBits2Float(0x44b48f86), SkBits2Float(0x4517bbf8), SkBits2Float(0x44b4894d));  // 2428.33f, 1444.49f, 2427.75f, 1444.29f
  path.quadTo(SkBits2Float(0x4517b8b7), SkBits2Float(0x44b48722), SkBits2Float(0x4517b639), SkBits2Float(0x44b483f6));  // 2427.54f, 1444.22f, 2427.39f, 1444.12f
  path.quadTo(SkBits2Float(0x4517a8b4), SkBits2Float(0x44b472c1), SkBits2Float(0x4517adb0), SkBits2Float(0x44b454d7));  // 2426.54f, 1443.59f, 2426.86f, 1442.65f
  path.quadTo(SkBits2Float(0x4517b1ba), SkBits2Float(0x44b43c99), SkBits2Float(0x4517c010), SkBits2Float(0x44b43881));  // 2427.11f, 1441.89f, 2428, 1441.77f
  path.quadTo(SkBits2Float(0x4517c5d2), SkBits2Float(0x44b436dc), SkBits2Float(0x4517d3a6), SkBits2Float(0x44b436dc));  // 2428.36f, 1441.71f, 2429.23f, 1441.71f
  path.lineTo(SkBits2Float(0x451f2a1e), SkBits2Float(0x44b436dc));  // 2546.63f, 1441.71f
  path.lineTo(SkBits2Float(0x45281817), SkBits2Float(0x44b436dc));  // 2689.51f, 1441.71f
  path.quadTo(SkBits2Float(0x4529e100), SkBits2Float(0x44b436f0), SkBits2Float(0x452d2f09), SkBits2Float(0x44b4866b));  // 2718.06f, 1441.72f, 2770.94f, 1444.2f
  path.quadTo(SkBits2Float(0x45307d20), SkBits2Float(0x44b4d600), SkBits2Float(0x45323a9d), SkBits2Float(0x44b4e9c6));  // 2823.82f, 1446.69f, 2851.66f, 1447.31f
  path.quadTo(SkBits2Float(0x4533fba0), SkBits2Float(0x44b4fdc0), SkBits2Float(0x45342f05), SkBits2Float(0x44b5124a));  // 2879.73f, 1447.93f, 2882.94f, 1448.57f
  path.quadTo(SkBits2Float(0x45343ca9), SkBits2Float(0x44b517bf), SkBits2Float(0x4534436c), SkBits2Float(0x44b51c41));  // 2883.79f, 1448.74f, 2884.21f, 1448.88f
  path.quadTo(SkBits2Float(0x4534490e), SkBits2Float(0x44b52002), SkBits2Float(0x45344c93), SkBits2Float(0x44b5250a));  // 2884.57f, 1449, 2884.79f, 1449.16f
  path.quadTo(SkBits2Float(0x45345260), SkBits2Float(0x44b52d54), SkBits2Float(0x45345489), SkBits2Float(0x44b53a4c));  // 2885.15f, 1449.42f, 2885.28f, 1449.82f
  path.quadTo(SkBits2Float(0x45345889), SkBits2Float(0x44b55250), SkBits2Float(0x45344f8a), SkBits2Float(0x44b5644c));  // 2885.53f, 1450.57f, 2884.97f, 1451.13f
  path.quadTo(SkBits2Float(0x4534473b), SkBits2Float(0x44b574e9), SkBits2Float(0x4534212e), SkBits2Float(0x44b574e9));  // 2884.45f, 1451.65f, 2882.07f, 1451.65f
  path.lineTo(SkBits2Float(0x45288f5c), SkBits2Float(0x44b574e9));  // 2696.96f, 1451.65f
  path.lineTo(SkBits2Float(0x451d254a), SkBits2Float(0x44b574e9));  // 2514.33f, 1451.65f
  path.quadTo(SkBits2Float(0x451d15b3), SkBits2Float(0x44b574e9), SkBits2Float(0x451d1124), SkBits2Float(0x44b55970));  // 2513.36f, 1451.65f, 2513.07f, 1450.79f
  path.quadTo(SkBits2Float(0x451d0bf1), SkBits2Float(0x44b53a1a), SkBits2Float(0x451d1a6a), SkBits2Float(0x44b52a08));  // 2512.75f, 1449.82f, 2513.65f, 1449.31f
  path.quadTo(SkBits2Float(0x451d1c47), SkBits2Float(0x44b527f8), SkBits2Float(0x451d1e61), SkBits2Float(0x44b526a9));  // 2513.77f, 1449.25f, 2513.9f, 1449.21f
  path.quadTo(SkBits2Float(0x451d1faa), SkBits2Float(0x44b525dd), SkBits2Float(0x451d213c), SkBits2Float(0x44b5253c));  // 2513.98f, 1449.18f, 2514.08f, 1449.16f
  path.quadTo(SkBits2Float(0x451d2433), SkBits2Float(0x44b5240c), SkBits2Float(0x451d2c0a), SkBits2Float(0x44b52201));  // 2514.26f, 1449.13f, 2514.75f, 1449.06f
  path.quadTo(SkBits2Float(0x451d3a93), SkBits2Float(0x44b51e37), SkBits2Float(0x451d5c3f), SkBits2Float(0x44b516bb));  // 2515.66f, 1448.94f, 2517.77f, 1448.71f
  path.quadTo(SkBits2Float(0x451da046), SkBits2Float(0x44b5079c), SkBits2Float(0x451e2f99), SkBits2Float(0x44b4e9f8));  // 2522.02f, 1448.24f, 2530.97f, 1447.31f
  path.quadTo(SkBits2Float(0x451f5060), SkBits2Float(0x44b4ae40), SkBits2Float(0x452169d9), SkBits2Float(0x44b48666));  // 2549.02f, 1445.45f, 2582.62f, 1444.2f
  path.quadTo(SkBits2Float(0x45238280), SkBits2Float(0x44b45ea0), SkBits2Float(0x45264ee6), SkBits2Float(0x44b45e9e));  // 2616.16f, 1442.96f, 2660.93f, 1442.96f
  path.lineTo(SkBits2Float(0x452c35a0), SkBits2Float(0x44b45e9e));  // 2755.35f, 1442.96f
  path.quadTo(SkBits2Float(0x452f51a0), SkBits2Float(0x44b45ea0), SkBits2Float(0x4532445f), SkBits2Float(0x44b4727f));  // 2805.1f, 1442.96f, 2852.27f, 1443.58f
  path.quadTo(SkBits2Float(0x45353b60), SkBits2Float(0x44b48680), SkBits2Float(0x4535708e), SkBits2Float(0x44b4b0f8));  // 2899.71f, 1444.2f, 2903.03f, 1445.53f
  path.quadTo(SkBits2Float(0x45358b50), SkBits2Float(0x44b4c660), SkBits2Float(0x4535947f), SkBits2Float(0x44b4d2a0));  // 2904.71f, 1446.2f, 2905.28f, 1446.58f
  path.quadTo(SkBits2Float(0x4535a6d4), SkBits2Float(0x44b4eb12), SkBits2Float(0x45359e5a), SkBits2Float(0x44b50cf2));  // 2906.43f, 1447.35f, 2905.9f, 1448.4f
  path.quadTo(SkBits2Float(0x4535983c), SkBits2Float(0x44b52566), SkBits2Float(0x453586fc), SkBits2Float(0x44b52566));  // 2905.51f, 1449.17f, 2904.44f, 1449.17f
  path.quadTo(SkBits2Float(0x45357489), SkBits2Float(0x44b52566), SkBits2Float(0x45353059), SkBits2Float(0x44b538e0));  // 2903.28f, 1449.17f, 2899.02f, 1449.78f
  path.lineTo(SkBits2Float(0x45352ab9), SkBits2Float(0x44b4ea2a));  // 2898.67f, 1447.32f
  path.quadTo(SkBits2Float(0x453571ba), SkBits2Float(0x44b4d5e2), SkBits2Float(0x453586fc), SkBits2Float(0x44b4d5e2));  // 2903.11f, 1446.68f, 2904.44f, 1446.68f
  path.quadTo(SkBits2Float(0x45357fac), SkBits2Float(0x44b4d5e3), SkBits2Float(0x45357acc), SkBits2Float(0x44b4e95e));  // 2903.98f, 1446.68f, 2903.67f, 1447.29f
  path.quadTo(SkBits2Float(0x45357390), SkBits2Float(0x44b50646), SkBits2Float(0x45357e71), SkBits2Float(0x44b514c8));  // 2903.22f, 1448.2f, 2903.9f, 1448.65f
  path.quadTo(SkBits2Float(0x453578b2), SkBits2Float(0x44b50d1e), SkBits2Float(0x453561ca), SkBits2Float(0x44b4facc));  // 2903.54f, 1448.41f, 2902.11f, 1447.84f
  path.quadTo(SkBits2Float(0x45353380), SkBits2Float(0x44b4d5e0), SkBits2Float(0x453243d9), SkBits2Float(0x44b4c201));  // 2899.22f, 1446.68f, 2852.24f, 1446.06f
  path.quadTo(SkBits2Float(0x452f5140), SkBits2Float(0x44b4ae20), SkBits2Float(0x452c35a0), SkBits2Float(0x44b4ae22));  // 2805.08f, 1445.44f, 2755.35f, 1445.44f
  path.lineTo(SkBits2Float(0x45264ee6), SkBits2Float(0x44b4ae22));  // 2660.93f, 1445.44f
  path.quadTo(SkBits2Float(0x45238340), SkBits2Float(0x44b4ae20), SkBits2Float(0x45216b51), SkBits2Float(0x44b4d5dc));  // 2616.2f, 1445.44f, 2582.71f, 1446.68f
  path.quadTo(SkBits2Float(0x451f5320), SkBits2Float(0x44b4fd90), SkBits2Float(0x451e33b1), SkBits2Float(0x44b53910));  // 2549.2f, 1447.92f, 2531.23f, 1449.78f
  path.quadTo(SkBits2Float(0x451da416), SkBits2Float(0x44b556c4), SkBits2Float(0x451d60a3), SkBits2Float(0x44b565c1));  // 2522.26f, 1450.71f, 2518.04f, 1451.18f
  path.quadTo(SkBits2Float(0x451d3f60), SkBits2Float(0x44b56d25), SkBits2Float(0x451d312e), SkBits2Float(0x44b570d9));  // 2515.96f, 1451.41f, 2515.07f, 1451.53f
  path.quadTo(SkBits2Float(0x451d2ab0), SkBits2Float(0x44b5728a), SkBits2Float(0x451d2908), SkBits2Float(0x44b57334));  // 2514.67f, 1451.58f, 2514.56f, 1451.6f
  path.quadTo(SkBits2Float(0x451d2977), SkBits2Float(0x44b57308), SkBits2Float(0x451d2a29), SkBits2Float(0x44b57299));  // 2514.59f, 1451.59f, 2514.64f, 1451.58f
  path.quadTo(SkBits2Float(0x451d2bfb), SkBits2Float(0x44b57178), SkBits2Float(0x451d2db6), SkBits2Float(0x44b56f8c));  // 2514.75f, 1451.55f, 2514.86f, 1451.49f
  path.quadTo(SkBits2Float(0x451d3c0e), SkBits2Float(0x44b55f9f), SkBits2Float(0x451d36e0), SkBits2Float(0x44b54066));  // 2515.75f, 1450.99f, 2515.43f, 1450.01f
  path.quadTo(SkBits2Float(0x451d3266), SkBits2Float(0x44b52565), SkBits2Float(0x451d254a), SkBits2Float(0x44b52565));  // 2515.15f, 1449.17f, 2514.33f, 1449.17f
  path.lineTo(SkBits2Float(0x45288f5c), SkBits2Float(0x44b52565));  // 2696.96f, 1449.17f
  path.lineTo(SkBits2Float(0x4534212e), SkBits2Float(0x44b52565));  // 2882.07f, 1449.17f
  path.quadTo(SkBits2Float(0x45342f56), SkBits2Float(0x44b52565), SkBits2Float(0x453436a4), SkBits2Float(0x44b52445));  // 2882.96f, 1449.17f, 2883.42f, 1449.13f
  path.quadTo(SkBits2Float(0x4534397b), SkBits2Float(0x44b523d6), SkBits2Float(0x45343aa2), SkBits2Float(0x44b5235f));  // 2883.59f, 1449.12f, 2883.66f, 1449.11f
  path.quadTo(SkBits2Float(0x45343711), SkBits2Float(0x44b524cc), SkBits2Float(0x4534336e), SkBits2Float(0x44b52c12));  // 2883.44f, 1449.15f, 2883.21f, 1449.38f
  path.quadTo(SkBits2Float(0x45342b0e), SkBits2Float(0x44b53cd3), SkBits2Float(0x45342ed1), SkBits2Float(0x44b5536e));  // 2882.69f, 1449.9f, 2882.93f, 1450.61f
  path.quadTo(SkBits2Float(0x453430bd), SkBits2Float(0x44b55efc), SkBits2Float(0x45343577), SkBits2Float(0x44b565bc));  // 2883.05f, 1450.97f, 2883.34f, 1451.18f
  path.quadTo(SkBits2Float(0x453436cf), SkBits2Float(0x44b567a8), SkBits2Float(0x453436da), SkBits2Float(0x44b567af));  // 2883.43f, 1451.24f, 2883.43f, 1451.24f
  path.quadTo(SkBits2Float(0x45343273), SkBits2Float(0x44b564c0), SkBits2Float(0x45342739), SkBits2Float(0x44b56042));  // 2883.15f, 1451.15f, 2882.45f, 1451.01f
  path.quadTo(SkBits2Float(0x4533f730), SkBits2Float(0x44b54d10), SkBits2Float(0x453239bb), SkBits2Float(0x44b53944));  // 2879.45f, 1450.41f, 2851.61f, 1449.79f
  path.quadTo(SkBits2Float(0x45307bc0), SkBits2Float(0x44b52580), SkBits2Float(0x452d2d2b), SkBits2Float(0x44b4d5d7));  // 2823.73f, 1449.17f, 2770.82f, 1446.68f
  path.quadTo(SkBits2Float(0x4529e020), SkBits2Float(0x44b48660), SkBits2Float(0x45281817), SkBits2Float(0x44b48660));  // 2718.01f, 1444.2f, 2689.51f, 1444.2f
  path.lineTo(SkBits2Float(0x451f2a1e), SkBits2Float(0x44b48660));  // 2546.63f, 1444.2f
  path.lineTo(SkBits2Float(0x4517d3a6), SkBits2Float(0x44b48660));  // 2429.23f, 1444.2f
  path.quadTo(SkBits2Float(0x4517c8a1), SkBits2Float(0x44b48660), SkBits2Float(0x4517c5b0), SkBits2Float(0x44b48737));  // 2428.54f, 1444.2f, 2428.36f, 1444.23f
  path.quadTo(SkBits2Float(0x4517cfae), SkBits2Float(0x44b4845d), SkBits2Float(0x4517d368), SkBits2Float(0x44b46dfd));  // 2428.98f, 1444.14f, 2429.21f, 1443.44f
  path.quadTo(SkBits2Float(0x4517d989), SkBits2Float(0x44b44934), SkBits2Float(0x4517c88a), SkBits2Float(0x44b43ddf));  // 2429.6f, 1442.29f, 2428.53f, 1441.93f
  path.quadTo(SkBits2Float(0x4517ce22), SkBits2Float(0x44b44198), SkBits2Float(0x4517f3ee), SkBits2Float(0x44b44b0c));  // 2428.88f, 1442.05f, 2431.25f, 1442.35f
  path.quadTo(SkBits2Float(0x451842a0), SkBits2Float(0x44b45eb0), SkBits2Float(0x451a011d), SkBits2Float(0x44b48669));  // 2436.16f, 1442.96f, 2464.07f, 1444.2f
  path.quadTo(SkBits2Float(0x451bbee0), SkBits2Float(0x44b4ae00), SkBits2Float(0x451ef8a9), SkBits2Float(0x44b4c202));  // 2491.93f, 1445.44f, 2543.54f, 1446.06f
  path.quadTo(SkBits2Float(0x45222ec0), SkBits2Float(0x44b4d5c0), SkBits2Float(0x4527f055), SkBits2Float(0x44b4d5e2));  // 2594.92f, 1446.68f, 2687.02f, 1446.68f
  path.lineTo(SkBits2Float(0x452f151c), SkBits2Float(0x44b4d5e2));  // 2801.32f, 1446.68f
  path.lineTo(SkBits2Float(0x4532f703), SkBits2Float(0x44b4d5e2));  // 2863.44f, 1446.68f
  path.quadTo(SkBits2Float(0x453434e0), SkBits2Float(0x44b4d5e0), SkBits2Float(0x4534d0f9), SkBits2Float(0x44b4cbf8));  // 2883.3f, 1446.68f, 2893.06f, 1446.37f
  path.quadTo(SkBits2Float(0x45351e89), SkBits2Float(0x44b4c70b), SkBits2Float(0x453543d1), SkBits2Float(0x44b4bfb6));  // 2897.91f, 1446.22f, 2900.24f, 1445.99f
  path.quadTo(SkBits2Float(0x45355557), SkBits2Float(0x44b4bc43), SkBits2Float(0x45355cd2), SkBits2Float(0x44b4b897));  // 2901.33f, 1445.88f, 2901.8f, 1445.77f
  path.lineTo(SkBits2Float(0x45355d83), SkBits2Float(0x44b4b8ca));  // 2901.84f, 1445.77f
  path.quadTo(SkBits2Float(0x45354fdc), SkBits2Float(0x44b4c983), SkBits2Float(0x45355765), SkBits2Float(0x44b4e7ab));  // 2900.99f, 1446.3f, 2901.46f, 1447.24f
  path.quadTo(SkBits2Float(0x45355220), SkBits2Float(0x44b4d2e0), SkBits2Float(0x45334ddc), SkBits2Float(0x44b44a66));  // 2901.13f, 1446.59f, 2868.87f, 1442.32f
  path.quadTo(SkBits2Float(0x45313fc0), SkBits2Float(0x44b3bf60), SkBits2Float(0x452fb2a5), SkBits2Float(0x44b383d9));  // 2835.98f, 1437.98f, 2811.17f, 1436.12f
  path.quadTo(SkBits2Float(0x452e25d0), SkBits2Float(0x44b34860), SkBits2Float(0x452c5305), SkBits2Float(0x44b33470));  // 2786.36f, 1434.26f, 2757.19f, 1433.64f
  path.quadTo(SkBits2Float(0x452a8140), SkBits2Float(0x44b320a0), SkBits2Float(0x452715ad), SkBits2Float(0x44b32092));  // 2728.08f, 1433.02f, 2673.35f, 1433.02f
  path.lineTo(SkBits2Float(0x4522630e), SkBits2Float(0x44b32092));  // 2598.19f, 1433.02f
  path.lineTo(SkBits2Float(0x45203679), SkBits2Float(0x44b32092));  // 2563.4f, 1433.02f
  path.quadTo(SkBits2Float(0x451f5280), SkBits2Float(0x44b32098), SkBits2Float(0x451ebe1d), SkBits2Float(0x44b3345c));  // 2549.16f, 1433.02f, 2539.88f, 1433.64f
  path.quadTo(SkBits2Float(0x451e285c), SkBits2Float(0x44b34854), SkBits2Float(0x451dba60), SkBits2Float(0x44b34854));  // 2530.52f, 1434.26f, 2523.65f, 1434.26f
  path.quadTo(SkBits2Float(0x451da05b), SkBits2Float(0x44b34854), SkBits2Float(0x451d985b), SkBits2Float(0x44b34a76));  // 2522.02f, 1434.26f, 2521.52f, 1434.33f
  path.lineTo(SkBits2Float(0x451d984b), SkBits2Float(0x44b34a4a));  // 2521.52f, 1434.32f
  path.quadTo(SkBits2Float(0x451da29c), SkBits2Float(0x44b343a2), SkBits2Float(0x451da485), SkBits2Float(0x44b32cb1));  // 2522.16f, 1434.11f, 2522.28f, 1433.4f
  path.quadTo(SkBits2Float(0x451da748), SkBits2Float(0x44b30b8c), SkBits2Float(0x451d9a91), SkBits2Float(0x44b30448));  // 2522.46f, 1432.36f, 2521.66f, 1432.13f
  path.quadTo(SkBits2Float(0x451daa5c), SkBits2Float(0x44b30d4c), SkBits2Float(0x451e0225), SkBits2Float(0x44b320d0));  // 2522.65f, 1432.42f, 2528.13f, 1433.03f
  path.quadTo(SkBits2Float(0x451eb440), SkBits2Float(0x44b34860), SkBits2Float(0x45213980), SkBits2Float(0x44b37019));  // 2539.27f, 1434.26f, 2579.59f, 1435.5f
  path.quadTo(SkBits2Float(0x4523bf40), SkBits2Float(0x44b397c0), SkBits2Float(0x4527d284), SkBits2Float(0x44b397d6));  // 2619.95f, 1436.74f, 2685.16f, 1436.74f
  path.lineTo(SkBits2Float(0x453586fc), SkBits2Float(0x44b397d6));  // 2904.44f, 1436.74f
  path.lineTo(SkBits2Float(0x453fbcf2), SkBits2Float(0x44b397d6));  // 3067.81f, 1436.74f
  path.quadTo(SkBits2Float(0x453fcf53), SkBits2Float(0x44b397d6), SkBits2Float(0x453fd8bc), SkBits2Float(0x44b3973d));  // 3068.96f, 1436.74f, 3069.55f, 1436.73f
  path.lineTo(SkBits2Float(0x453fd90e), SkBits2Float(0x44b39c08));  // 3069.57f, 1436.88f
  path.quadTo(SkBits2Float(0x453fcd59), SkBits2Float(0x44b3aba4), SkBits2Float(0x453fd0f5), SkBits2Float(0x44b3c6f5));  // 3068.83f, 1437.36f, 3069.06f, 1438.22f
  path.quadTo(SkBits2Float(0x453fd320), SkBits2Float(0x44b3d756), SkBits2Float(0x453fda79), SkBits2Float(0x44b3df5a));  // 3069.2f, 1438.73f, 3069.65f, 1438.98f
  path.quadTo(SkBits2Float(0x453fdc9c), SkBits2Float(0x44b3e1af), SkBits2Float(0x453fde7d), SkBits2Float(0x44b3e285));  // 3069.79f, 1439.05f, 3069.91f, 1439.08f
  path.quadTo(SkBits2Float(0x453fddbe), SkBits2Float(0x44b3e22f), SkBits2Float(0x453fdac5), SkBits2Float(0x44b3e17d));  // 3069.86f, 1439.07f, 3069.67f, 1439.05f
  path.quadTo(SkBits2Float(0x453fd34b), SkBits2Float(0x44b3dfba), SkBits2Float(0x453fc2ee), SkBits2Float(0x44b3dd4e));  // 3069.21f, 1438.99f, 3068.18f, 1438.92f
  path.quadTo(SkBits2Float(0x453f7ff8), SkBits2Float(0x44b3d360), SkBits2Float(0x453eafa6), SkBits2Float(0x44b3bf8c));  // 3064, 1438.61f, 3050.98f, 1437.99f
  path.quadTo(SkBits2Float(0x453d0ea0), SkBits2Float(0x44b397e0), SkBits2Float(0x453a438c), SkBits2Float(0x44b397d6));  // 3024.91f, 1436.75f, 2980.22f, 1436.74f
  path.quadTo(SkBits2Float(0x45377540), SkBits2Float(0x44b397e0), SkBits2Float(0x45340d88), SkBits2Float(0x44b3abb6));  // 2935.33f, 1436.75f, 2880.85f, 1437.37f
  path.quadTo(SkBits2Float(0x4530a580), SkBits2Float(0x44b3bfa0), SkBits2Float(0x452d9b6e), SkBits2Float(0x44b3bf98));  // 2826.34f, 1437.99f, 2777.71f, 1437.99f
  path.quadTo(SkBits2Float(0x452a94b0), SkBits2Float(0x44b3bf80), SkBits2Float(0x4528a4d2), SkBits2Float(0x44b40efa));  // 2729.29f, 1437.98f, 2698.3f, 1440.47f
  path.quadTo(SkBits2Float(0x4526b438), SkBits2Float(0x44b45e78), SkBits2Float(0x4525ee75), SkBits2Float(0x44b499cf));  // 2667.26f, 1442.95f, 2654.9f, 1444.81f
  path.quadTo(SkBits2Float(0x45258bd2), SkBits2Float(0x44b4b767), SkBits2Float(0x4525626e), SkBits2Float(0x44b4cae3));  // 2648.74f, 1445.73f, 2646.15f, 1446.34f
  path.quadTo(SkBits2Float(0x45254fec), SkBits2Float(0x44b4d399), SkBits2Float(0x45254b71), SkBits2Float(0x44b4d87c));  // 2645, 1446.61f, 2644.72f, 1446.77f
  path.quadTo(SkBits2Float(0x45255398), SkBits2Float(0x44b4cf96), SkBits2Float(0x45255271), SkBits2Float(0x44b4b877));  // 2645.22f, 1446.49f, 2645.15f, 1445.76f
  path.quadTo(SkBits2Float(0x45255150), SkBits2Float(0x44b4a1f4), SkBits2Float(0x452548d4), SkBits2Float(0x44b49c4b));  // 2645.08f, 1445.06f, 2644.55f, 1444.88f
  path.quadTo(SkBits2Float(0x45256468), SkBits2Float(0x44b4aea8), SkBits2Float(0x4525d947), SkBits2Float(0x44b4c225));  // 2646.28f, 1445.46f, 2653.58f, 1446.07f
  path.quadTo(SkBits2Float(0x45264fc0), SkBits2Float(0x44b4d5e0), SkBits2Float(0x4527475f), SkBits2Float(0x44b4d5e2));  // 2660.98f, 1446.68f, 2676.46f, 1446.68f
  path.quadTo(SkBits2Float(0x45283e40), SkBits2Float(0x44b4d5e0), SkBits2Float(0x452b0bd8), SkBits2Float(0x44b4e9c5));  // 2691.89f, 1446.68f, 2736.74f, 1447.31f
  path.lineTo(SkBits2Float(0x4532ed87), SkBits2Float(0x44b53949));  // 2862.85f, 1449.79f
  path.quadTo(SkBits2Float(0x45380340), SkBits2Float(0x44b574e0), SkBits2Float(0x453a753e), SkBits2Float(0x44b574e8));  // 2944.2f, 1451.65f, 2983.33f, 1451.65f
  path.quadTo(SkBits2Float(0x453ce6d0), SkBits2Float(0x44b574e0), SkBits2Float(0x453e87e4), SkBits2Float(0x44b54d32));  // 3022.43f, 1451.65f, 3048.49f, 1450.41f
  path.quadTo(SkBits2Float(0x45402850), SkBits2Float(0x44b52588), SkBits2Float(0x4540b29c), SkBits2Float(0x44b4fe0b));  // 3074.52f, 1449.17f, 3083.16f, 1447.94f
  path.quadTo(SkBits2Float(0x4540f007), SkBits2Float(0x44b4ec80), SkBits2Float(0x4540f28b), SkBits2Float(0x44b4e5ca));  // 3087, 1447.39f, 3087.16f, 1447.18f
  path.quadTo(SkBits2Float(0x4540eb18), SkBits2Float(0x44b4f9a6), SkBits2Float(0x4540f2de), SkBits2Float(0x44b50cc6));  // 3086.69f, 1447.8f, 3087.18f, 1448.4f
  path.lineTo(SkBits2Float(0x4540f2a8), SkBits2Float(0x44b50e67));  // 3087.17f, 1448.45f
  path.quadTo(SkBits2Float(0x4540e8dc), SkBits2Float(0x44b50627), SkBits2Float(0x4540cfae), SkBits2Float(0x44b4fcff));  // 3086.55f, 1448.19f, 3084.98f, 1447.91f
  path.quadTo(SkBits2Float(0x454062e0), SkBits2Float(0x44b4d580), SkBits2Float(0x453e2366), SkBits2Float(0x44b45e68));  // 3078.18f, 1446.67f, 3042.21f, 1442.95f
  path.quadTo(SkBits2Float(0x453be400), SkBits2Float(0x44b3e780), SkBits2Float(0x4538168e), SkBits2Float(0x44b3bf95));  // 3006.25f, 1439.23f, 2945.41f, 1437.99f
  path.quadTo(SkBits2Float(0x45344a40), SkBits2Float(0x44b39800), SkBits2Float(0x453070fa), SkBits2Float(0x44b397d6));  // 2884.64f, 1436.75f, 2823.06f, 1436.74f
  path.quadTo(SkBits2Float(0x452c9a20), SkBits2Float(0x44b397e0), SkBits2Float(0x4529569d), SkBits2Float(0x44b3bf95));  // 2761.63f, 1436.75f, 2709.41f, 1437.99f
  path.quadTo(SkBits2Float(0x45261560), SkBits2Float(0x44b3e740), SkBits2Float(0x452334d9), SkBits2Float(0x44b436ce));  // 2657.34f, 1439.23f, 2611.3f, 1441.71f
  path.quadTo(SkBits2Float(0x4520578c), SkBits2Float(0x44b48608), SkBits2Float(0x451fec31), SkBits2Float(0x44b4c09b));  // 2565.47f, 1444.19f, 2558.76f, 1446.02f
  path.quadTo(SkBits2Float(0x451fb930), SkBits2Float(0x44b4dc6e), SkBits2Float(0x451fb249), SkBits2Float(0x44b4e6c8));  // 2555.57f, 1446.89f, 2555.14f, 1447.21f
  path.quadTo(SkBits2Float(0x451fba10), SkBits2Float(0x44b4db1f), SkBits2Float(0x451fb797), SkBits2Float(0x44b4c4e0));  // 2555.63f, 1446.85f, 2555.47f, 1446.15f
  path.quadTo(SkBits2Float(0x451fb560), SkBits2Float(0x44b4b0ec), SkBits2Float(0x451fad4f), SkBits2Float(0x44b4ac14));  // 2555.34f, 1445.53f, 2554.83f, 1445.38f
  path.quadTo(SkBits2Float(0x451fb0b8), SkBits2Float(0x44b4ae20), SkBits2Float(0x451fbf34), SkBits2Float(0x44b4ae20));  // 2555.04f, 1445.44f, 2555.95f, 1445.44f
  path.quadTo(SkBits2Float(0x45200490), SkBits2Float(0x44b4ae20), SkBits2Float(0x4521ba8c), SkBits2Float(0x44b4c204));  // 2560.29f, 1445.44f, 2587.66f, 1446.06f
  path.quadTo(SkBits2Float(0x45237040), SkBits2Float(0x44b4d5e0), SkBits2Float(0x45268bc5), SkBits2Float(0x44b5395a));  // 2615.02f, 1446.68f, 2664.74f, 1449.79f
  path.quadTo(SkBits2Float(0x4529a540), SkBits2Float(0x44b59c80), SkBits2Float(0x452d4c53), SkBits2Float(0x44b5c46f));  // 2714.33f, 1452.89f, 2772.77f, 1454.14f
  path.lineTo(SkBits2Float(0x4532f765), SkBits2Float(0x44b60010));  // 2863.46f, 1456
  path.quadTo(SkBits2Float(0x4534fb30), SkBits2Float(0x44b613e0), SkBits2Float(0x4536c508), SkBits2Float(0x44b613ee));  // 2895.7f, 1456.62f, 2924.31f, 1456.62f
  path.lineTo(SkBits2Float(0x453a2faa), SkBits2Float(0x44b613ee));  // 2978.98f, 1456.62f
  path.lineTo(SkBits2Float(0x453d2cf8), SkBits2Float(0x44b613ee));  // 3026.81f, 1456.62f
  path.lineTo(SkBits2Float(0x453d2cf8), SkBits2Float(0x44b66372));  // 3026.81f, 1459.11f
  path.close();

  return path;
}

static SkPath getPathD() {
  SkPath path;
  path.setFillType(SkPathFillType::kEvenOdd);
  path.moveTo(SkBits2Float(0x452798c8), SkBits2Float(0x44c88f9b));  // 2681.55f, 1604.49f
  path.conicTo(SkBits2Float(0x4526da4c), SkBits2Float(0x44c9b212), SkBits2Float(0x4525f70e), SkBits2Float(0x44ca4991), SkBits2Float(0x3f7c87b2));  // 2669.64f, 1613.56f, 2655.44f, 1618.3f, 0.986446f
  path.lineTo(SkBits2Float(0x4524d70e), SkBits2Float(0x44cb0991));  // 2637.44f, 1624.3f
  path.conicTo(SkBits2Float(0x45224b35), SkBits2Float(0x44ccbc21), SkBits2Float(0x4520083c), SkBits2Float(0x44c9d81b), SkBits2Float(0x3f670ba2));  // 2596.7f, 1637.88f, 2560.51f, 1614.75f, 0.902521f
  path.conicTo(SkBits2Float(0x451fb2e2), SkBits2Float(0x44ca52e8), SkBits2Float(0x451f514d), SkBits2Float(0x44caa12e), SkBits2Float(0x3f7e2012));  // 2555.18f, 1618.59f, 2549.08f, 1621.04f, 0.992677f
  path.conicTo(SkBits2Float(0x451ec1a1), SkBits2Float(0x44cc7845), SkBits2Float(0x451dc409), SkBits2Float(0x44cd51a3), SkBits2Float(0x3f73de27));  // 2540.1f, 1635.76f, 2524.25f, 1642.55f, 0.952609f
  path.conicTo(SkBits2Float(0x451d15ec), SkBits2Float(0x44cde6e2), SkBits2Float(0x451c588f), SkBits2Float(0x44cddcbd), SkBits2Float(0x3f7a0f70));  // 2513.37f, 1647.22f, 2501.53f, 1646.9f, 0.976798f
  path.conicTo(SkBits2Float(0x451c16d7), SkBits2Float(0x44cf3ad2), SkBits2Float(0x451b92a1), SkBits2Float(0x44d0433d), SkBits2Float(0x3f7a355d));  // 2497.43f, 1657.84f, 2489.16f, 1666.1f, 0.977377f
  path.conicTo(SkBits2Float(0x45192fb0), SkBits2Float(0x44d5091e), SkBits2Float(0x4516ccbf), SkBits2Float(0x44d0433d), SkBits2Float(0x3f3504f3));  // 2450.98f, 1704.28f, 2412.8f, 1666.1f, 0.707107f
  path.lineTo(SkBits2Float(0x4516bcbf), SkBits2Float(0x44d0233d));  // 2411.8f, 1665.1f
  path.conicTo(SkBits2Float(0x4515bfb0), SkBits2Float(0x44ce291f), SkBits2Float(0x4515bfb0), SkBits2Float(0x44cb5d5c), SkBits2Float(0x3f6c835f));  // 2395.98f, 1649.29f, 2395.98f, 1626.92f, 0.92388f
  path.lineTo(SkBits2Float(0x4515bfb0), SkBits2Float(0x44ca1d5c));  // 2395.98f, 1616.92f
  path.conicTo(SkBits2Float(0x4515bfb0), SkBits2Float(0x44c9bda7), SkBits2Float(0x4515c4f9), SkBits2Float(0x44c95e89), SkBits2Float(0x3f7f9bb7));  // 2395.98f, 1613.93f, 2396.31f, 1610.95f, 0.99847f
  path.lineTo(SkBits2Float(0x4515cfea), SkBits2Float(0x44c8999a));  // 2396.99f, 1604.8f
  path.conicTo(SkBits2Float(0x4515b3bd), SkBits2Float(0x44c833db), SkBits2Float(0x45159cd5), SkBits2Float(0x44c7c8f4), SkBits2Float(0x3f7faca8));  // 2395.23f, 1601.62f, 2393.8f, 1598.28f, 0.998728f
  path.conicTo(SkBits2Float(0x451563c0), SkBits2Float(0x44c6be91), SkBits2Float(0x45154d6f), SkBits2Float(0x44c5a035), SkBits2Float(0x3f7dff95));  // 2390.23f, 1589.96f, 2388.84f, 1581.01f, 0.992181f
  path.conicTo(SkBits2Float(0x4514e77b), SkBits2Float(0x44c58e1b), SkBits2Float(0x451482f9), SkBits2Float(0x44c56773), SkBits2Float(0x3f7fabb4));  // 2382.47f, 1580.44f, 2376.19f, 1579.23f, 0.998714f
  path.lineTo(SkBits2Float(0x4512e2f9), SkBits2Float(0x44c4c773));  // 2350.19f, 1574.23f
  path.conicTo(SkBits2Float(0x451292ca), SkBits2Float(0x44c4a89d), SkBits2Float(0x4512441e), SkBits2Float(0x44c47ce8), SkBits2Float(0x3f7fca50));  // 2345.17f, 1573.27f, 2340.26f, 1571.9f, 0.999181f
  path.lineTo(SkBits2Float(0x4511241e), SkBits2Float(0x44c3dce8));  // 2322.26f, 1566.9f
  path.conicTo(SkBits2Float(0x4510f2c4), SkBits2Float(0x44c3c17c), SkBits2Float(0x4510c22c), SkBits2Float(0x44c3a117), SkBits2Float(0x3f7feadc));  // 2319.17f, 1566.05f, 2316.14f, 1565.03f, 0.999677f
  path.lineTo(SkBits2Float(0x4510022c), SkBits2Float(0x44c32117));  // 2304.14f, 1561.03f
  path.conicTo(SkBits2Float(0x450f8b32), SkBits2Float(0x44c2d1c6), SkBits2Float(0x450f1a0f), SkBits2Float(0x44c26595), SkBits2Float(0x3f7f8190));  // 2296.7f, 1558.56f, 2289.63f, 1555.17f, 0.998071f
  path.conicTo(SkBits2Float(0x450edcfc), SkBits2Float(0x44c27000), SkBits2Float(0x450e9fb0), SkBits2Float(0x44c27000), SkBits2Float(0x3f7fc4b5));  // 2285.81f, 1555.5f, 2281.98f, 1555.5f, 0.999095f
  path.lineTo(SkBits2Float(0x450a6fb0), SkBits2Float(0x44c27000));  // 2214.98f, 1555.5f
  path.conicTo(SkBits2Float(0x4507b624), SkBits2Float(0x44c27000), SkBits2Float(0x450605bd), SkBits2Float(0x44be294a), SkBits2Float(0x3f666472));  // 2171.38f, 1555.5f, 2144.36f, 1521.29f, 0.89997f
  path.conicTo(SkBits2Float(0x4505d47b), SkBits2Float(0x44be1ea1), SkBits2Float(0x4505a39b), SkBits2Float(0x44be0e56), SkBits2Float(0x3f7fe514));  // 2141.28f, 1520.96f, 2138.23f, 1520.45f, 0.999589f
  path.lineTo(SkBits2Float(0x4505439b), SkBits2Float(0x44bdee56));  // 2132.23f, 1519.45f
  path.conicTo(SkBits2Float(0x45047a3b), SkBits2Float(0x44bdab36), SkBits2Float(0x4503be39), SkBits2Float(0x44bd0c15), SkBits2Float(0x3f7e3b5c));  // 2119.64f, 1517.35f, 2107.89f, 1512.38f, 0.993093f
  path.lineTo(SkBits2Float(0x4503be12), SkBits2Float(0x44bd0bf3));  // 2107.88f, 1512.37f
  path.conicTo(SkBits2Float(0x4502f2c4), SkBits2Float(0x44bc649e), SkBits2Float(0x450242e3), SkBits2Float(0x44bb5ccd), SkBits2Float(0x3f7df3f3));  // 2095.17f, 1507.14f, 2084.18f, 1498.9f, 0.992004f
  path.lineTo(SkBits2Float(0x450202e3), SkBits2Float(0x44bafccd));  // 2080.18f, 1495.9f
  path.conicTo(SkBits2Float(0x44ffe571), SkBits2Float(0x44b7e48d), SkBits2Float(0x44fee27d), SkBits2Float(0x44b2d5c7), SkBits2Float(0x3f6f23e9));  // 2047.17f, 1471.14f, 2039.08f, 1430.68f, 0.934142f
  path.conicTo(SkBits2Float(0x44fc3cb6), SkBits2Float(0x44a598e4), SkBits2Float(0x4504bccc), SkBits2Float(0x44a2f31d), SkBits2Float(0x3f3504f3));  // 2017.9f, 1324.78f, 2123.8f, 1303.6f, 0.707107f
  path.lineTo(SkBits2Float(0x45050ccc), SkBits2Float(0x44a2d31d));  // 2128.8f, 1302.6f
  path.conicTo(SkBits2Float(0x4505b495), SkBits2Float(0x44a29000), SkBits2Float(0x45065fb0), SkBits2Float(0x44a29000), SkBits2Float(0x3f7ec10f));  // 2139.29f, 1300.5f, 2149.98f, 1300.5f, 0.995133f
  path.lineTo(SkBits2Float(0x450a0e2e), SkBits2Float(0x44a29000));  // 2208.89f, 1300.5f
  path.conicTo(SkBits2Float(0x450a2b41), SkBits2Float(0x44a275b1), SkBits2Float(0x450a48e3), SkBits2Float(0x44a25dfd), SkBits2Float(0x3f7fefe9));  // 2210.7f, 1299.68f, 2212.56f, 1298.94f, 0.999754f
  path.lineTo(SkBits2Float(0x450a6f73), SkBits2Float(0x44a23f23));  // 2214.97f, 1297.97f
  path.conicTo(SkBits2Float(0x450c496a), SkBits2Float(0x44a01000), SkBits2Float(0x450e6fb0), SkBits2Float(0x44a01000), SkBits2Float(0x3f76f722));  // 2244.59f, 1280.5f, 2278.98f, 1280.5f, 0.964708f
  path.lineTo(SkBits2Float(0x45120fb0), SkBits2Float(0x44a01000));  // 2336.98f, 1280.5f
  path.conicTo(SkBits2Float(0x45122e3a), SkBits2Float(0x44a01000), SkBits2Float(0x45124cc0), SkBits2Float(0x44a011da), SkBits2Float(0x3f7ff87b));  // 2338.89f, 1280.5f, 2340.8f, 1280.56f, 0.999885f
  path.lineTo(SkBits2Float(0x45145cc0), SkBits2Float(0x44a031da));  // 2373.8f, 1281.56f
  path.conicTo(SkBits2Float(0x4514a53d), SkBits2Float(0x44a0363e), SkBits2Float(0x4514ed61), SkBits2Float(0x44a0450b), SkBits2Float(0x3f7fd5a3));  // 2378.33f, 1281.7f, 2382.84f, 1282.16f, 0.999354f
  path.lineTo(SkBits2Float(0x45170e28), SkBits2Float(0x44a0b4cb));  // 2416.88f, 1285.65f
  path.lineTo(SkBits2Float(0x45196e8f), SkBits2Float(0x44a0d117));  // 2454.91f, 1286.53f
  path.conicTo(SkBits2Float(0x451a5959), SkBits2Float(0x44a0dc03), SkBits2Float(0x451b3c98), SkBits2Float(0x44a15294), SkBits2Float(0x3f7e47c9));  // 2469.58f, 1286.88f, 2483.79f, 1290.58f, 0.993283f
  path.lineTo(SkBits2Float(0x451e0dae), SkBits2Float(0x44a170a0));  // 2528.85f, 1291.52f
  path.conicTo(SkBits2Float(0x451eaeb1), SkBits2Float(0x44a17755), SkBits2Float(0x451f4afc), SkBits2Float(0x44a1c504), SkBits2Float(0x3f7e69f4));  // 2538.92f, 1291.73f, 2548.69f, 1294.16f, 0.993804f
  path.lineTo(SkBits2Float(0x451fede8), SkBits2Float(0x44a02db6));  // 2558.87f, 1281.43f
  path.conicTo(SkBits2Float(0x451ff696), SkBits2Float(0x44a01802), SkBits2Float(0x451fffc8), SkBits2Float(0x44a0032c), SkBits2Float(0x3f7fecf0));  // 2559.41f, 1280.75f, 2559.99f, 1280.1f, 0.999709f
  path.lineTo(SkBits2Float(0x4520ea79), SkBits2Float(0x449def35));  // 2574.65f, 1263.48f
  path.lineTo(SkBits2Float(0x45217de8), SkBits2Float(0x449c8db6));  // 2583.87f, 1252.43f
  path.lineTo(SkBits2Float(0x4521fde8), SkBits2Float(0x449b4db6));  // 2591.87f, 1242.43f
  path.conicTo(SkBits2Float(0x4522aad9), SkBits2Float(0x44999d5c), SkBits2Float(0x4523bfb0), SkBits2Float(0x44999d5c), SkBits2Float(0x3f66bbcf));  // 2602.68f, 1228.92f, 2619.98f, 1228.92f, 0.901303f
  path.conicTo(SkBits2Float(0x4525ffb0), SkBits2Float(0x44999d5c), SkBits2Float(0x4525ffb0), SkBits2Float(0x449e1d5c), SkBits2Float(0x3f3504f3));  // 2655.98f, 1228.92f, 2655.98f, 1264.92f, 0.707107f
  path.lineTo(SkBits2Float(0x4525ffb0), SkBits2Float(0x449e5d5c));  // 2655.98f, 1266.92f
  path.conicTo(SkBits2Float(0x4525ffb0), SkBits2Float(0x449f184e), SkBits2Float(0x4525e221), SkBits2Float(0x449fc9a7), SkBits2Float(0x3f7cb1c3));  // 2655.98f, 1272.76f, 2654.13f, 1278.3f, 0.987087f
  path.lineTo(SkBits2Float(0x4525c221), SkBits2Float(0x44a089a7));  // 2652.13f, 1284.3f
  path.conicTo(SkBits2Float(0x4525bfab), SkBits2Float(0x44a0986b), SkBits2Float(0x4525bd02), SkBits2Float(0x44a0a70c), SkBits2Float(0x3f7ffa05));  // 2651.98f, 1284.76f, 2651.81f, 1285.22f, 0.999909f
  path.lineTo(SkBits2Float(0x4525bbb1), SkBits2Float(0x44a0ae48));  // 2651.73f, 1285.45f
  path.conicTo(SkBits2Float(0x4525f2e8), SkBits2Float(0x44a10fd3), SkBits2Float(0x4526210f), SkBits2Float(0x44a182a7), SkBits2Float(0x3f7f1313));  // 2655.18f, 1288.49f, 2658.07f, 1292.08f, 0.996385f
  path.conicTo(SkBits2Float(0x45262896), SkBits2Float(0x44a18694), SkBits2Float(0x4526300e), SkBits2Float(0x44a18ae8), SkBits2Float(0x3f7ffa07));  // 2658.54f, 1292.21f, 2659, 1292.34f, 0.999909f
  path.lineTo(SkBits2Float(0x45263f36), SkBits2Float(0x44a1493a));  // 2659.95f, 1290.29f
  path.conicTo(SkBits2Float(0x452644de), SkBits2Float(0x44a130b8), SkBits2Float(0x45264ae7), SkBits2Float(0x44a11893), SkBits2Float(0x3f7ff801));  // 2660.3f, 1289.52f, 2660.68f, 1288.77f, 0.999878f
  path.lineTo(SkBits2Float(0x45265ae4), SkBits2Float(0x44a0d89e));  // 2661.68f, 1286.77f
  path.lineTo(SkBits2Float(0x45266ae7), SkBits2Float(0x44a09893));  // 2662.68f, 1284.77f
  path.conicTo(SkBits2Float(0x4527ed4c), SkBits2Float(0x449a8f01), SkBits2Float(0x452af214), SkBits2Float(0x449d93ca), SkBits2Float(0x3f3504f3));  // 2686.83f, 1236.47f, 2735.13f, 1260.62f, 0.707107f
  path.conicTo(SkBits2Float(0x452d17f2), SkBits2Float(0x449fb9a8), SkBits2Float(0x452cc86e), SkBits2Float(0x44a47cde), SkBits2Float(0x3f509625));  // 2769.5f, 1277.8f, 2764.53f, 1315.9f, 0.814791f
  path.lineTo(SkBits2Float(0x452cfa82), SkBits2Float(0x44a48223));  // 2767.66f, 1316.07f
  path.conicTo(SkBits2Float(0x452d2617), SkBits2Float(0x44a486b9), SkBits2Float(0x452d517f), SkBits2Float(0x44a48fb4), SkBits2Float(0x3f7feb1c));  // 2770.38f, 1316.21f, 2773.09f, 1316.49f, 0.999681f
  path.lineTo(SkBits2Float(0x452e176d), SkBits2Float(0x44a4b8a7));  // 2785.46f, 1317.77f
  path.conicTo(SkBits2Float(0x452ec6e1), SkBits2Float(0x44a4825a), SkBits2Float(0x452f7830), SkBits2Float(0x44a49489), SkBits2Float(0x3f7ea8cd));  // 2796.43f, 1316.07f, 2807.51f, 1316.64f, 0.994763f
  path.lineTo(SkBits2Float(0x45342bf8), SkBits2Float(0x44a51000));  // 2882.75f, 1320.5f
  path.lineTo(SkBits2Float(0x4537cfb0), SkBits2Float(0x44a51000));  // 2940.98f, 1320.5f
  path.lineTo(SkBits2Float(0x453bac7c), SkBits2Float(0x44a5307b));  // 3002.78f, 1321.52f
  path.lineTo(SkBits2Float(0x453ef0ea), SkBits2Float(0x44a550a4));  // 3055.06f, 1322.52f
  path.lineTo(SkBits2Float(0x4541a7dd), SkBits2Float(0x44a570ef));  // 3098.49f, 1323.53f
  path.lineTo(SkBits2Float(0x4543f66c), SkBits2Float(0x44a59000));  // 3135.4f, 1324.5f
  path.lineTo(SkBits2Float(0x45453fb0), SkBits2Float(0x44a59000));  // 3155.98f, 1324.5f
  path.conicTo(SkBits2Float(0x454bffb0), SkBits2Float(0x44a59000), SkBits2Float(0x454bffb0), SkBits2Float(0x44b31000), SkBits2Float(0x3f3504f3));  // 3263.98f, 1324.5f, 3263.98f, 1432.5f, 0.707107f
  path.conicTo(SkBits2Float(0x454bffb0), SkBits2Float(0x44c09000), SkBits2Float(0x45453fb0), SkBits2Float(0x44c09000), SkBits2Float(0x3f3504f3));  // 3263.98f, 1540.5f, 3155.98f, 1540.5f, 0.707107f
  path.lineTo(SkBits2Float(0x453fffb0), SkBits2Float(0x44c09000));  // 3071.98f, 1540.5f
  path.conicTo(SkBits2Float(0x453fec7d), SkBits2Float(0x44c09000), SkBits2Float(0x453fd94c), SkBits2Float(0x44c08f26), SkBits2Float(0x3f7ffbf5));  // 3070.78f, 1540.5f, 3069.58f, 1540.47f, 0.999938f
  path.lineTo(SkBits2Float(0x453fa612), SkBits2Float(0x44c08cdf));  // 3066.38f, 1540.4f
  path.conicTo(SkBits2Float(0x453dc047), SkBits2Float(0x44c3d000), SkBits2Float(0x453b3fb0), SkBits2Float(0x44c3d000), SkBits2Float(0x3f700977));  // 3036.02f, 1566.5f, 2995.98f, 1566.5f, 0.937644f
  path.lineTo(SkBits2Float(0x453a5fb0), SkBits2Float(0x44c3d000));  // 2981.98f, 1566.5f
  path.conicTo(SkBits2Float(0x453a3691), SkBits2Float(0x44c3d000), SkBits2Float(0x453a0d7f), SkBits2Float(0x44c3cc17), SkBits2Float(0x3f7fed74));  // 2979.41f, 1566.5f, 2976.84f, 1566.38f, 0.999717f
  path.lineTo(SkBits2Float(0x4538ad1b), SkBits2Float(0x44c3aa5f));  // 2954.82f, 1565.32f
  path.lineTo(SkBits2Float(0x45367d1b), SkBits2Float(0x44c36a5f));  // 2919.82f, 1563.32f
  path.conicTo(SkBits2Float(0x45365e54), SkBits2Float(0x44c366db), SkBits2Float(0x45363fa2), SkBits2Float(0x44c36125), SkBits2Float(0x3f7ff593));  // 2917.9f, 1563.21f, 2915.98f, 1563.04f, 0.999841f
  path.lineTo(SkBits2Float(0x45338fa2), SkBits2Float(0x44c2e125));  // 2872.98f, 1559.04f
  path.conicTo(SkBits2Float(0x453378b0), SkBits2Float(0x44c2dce0), SkBits2Float(0x453361ce), SkBits2Float(0x44c2d762), SkBits2Float(0x3f7ffa2c));  // 2871.54f, 1558.9f, 2870.11f, 1558.73f, 0.999911f
  path.lineTo(SkBits2Float(0x4531a118), SkBits2Float(0x44c26bb1));  // 2842.07f, 1555.37f
  path.conicTo(SkBits2Float(0x4530cd29), SkBits2Float(0x44c32539), SkBits2Float(0x452fe6d5), SkBits2Float(0x44c350a6), SkBits2Float(0x3f7cc23f));  // 2828.82f, 1561.16f, 2814.43f, 1562.52f, 0.987339f
  path.conicTo(SkBits2Float(0x452f1514), SkBits2Float(0x44c5d388), SkBits2Float(0x452dcef7), SkBits2Float(0x44c7685c), SkBits2Float(0x3f79e8c6));  // 2801.32f, 1582.61f, 2780.94f, 1595.26f, 0.976208f
  path.conicTo(SkBits2Float(0x452b6ed4), SkBits2Float(0x44ca5b4b), SkBits2Float(0x4528b5e7), SkBits2Float(0x44c91511), SkBits2Float(0x3f6c8361));  // 2742.93f, 1618.85f, 2699.37f, 1608.66f, 0.92388f
  path.lineTo(SkBits2Float(0x452798c8), SkBits2Float(0x44c88f9b));  // 2681.55f, 1604.49f
  path.close();

  return path;
}

static void diffPathIssue(skiatest::Reporter* reporter, const char* filename) {
  SkPath path;
  path.setFillType(SkPathFillType::kWinding);
  path.addPath(getPath0());
  path.addPath(getPath1());
  path.addPath(getPath2());
  path.addPath(getPath3());
  path.addPath(getPath4());

  SkPath path_diff = getPathD();
  testPathOp(reporter, path, path_diff, SkPathOp::kDifference_SkPathOp, filename);
}

static void (*skipTest)(skiatest::Reporter* , const char* filename) = nullptr;
static void (*firstTest)(skiatest::Reporter* , const char* filename) = diffPathIssue;
static void (*stopTest)(skiatest::Reporter* , const char* filename) = nullptr;

static struct TestDesc tests[] = {
    TEST(diffPathIssue),
};

static const size_t testCount = SK_ARRAY_COUNT(tests);

static bool runReverse = false;

DEF_TEST(PathOpsDiffTest, reporter) {
    RunTestSet(reporter, tests, testCount, firstTest, skipTest, stopTest, runReverse);
}
