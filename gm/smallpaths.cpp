/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"

typedef SkScalar (*MakePathProc)(SkPath*);

static SkScalar make_triangle(SkPath* path) {
    constexpr int gCoord[] = {
        10, 20, 15, 5, 30, 30
    };
    path->moveTo(SkIntToScalar(gCoord[0]), SkIntToScalar(gCoord[1]));
    path->lineTo(SkIntToScalar(gCoord[2]), SkIntToScalar(gCoord[3]));
    path->lineTo(SkIntToScalar(gCoord[4]), SkIntToScalar(gCoord[5]));
    path->close();
    path->offset(SkIntToScalar(10), SkIntToScalar(0));
    return SkIntToScalar(30);
}

static SkScalar make_rect(SkPath* path) {
    SkRect r = { SkIntToScalar(10), SkIntToScalar(10),
                 SkIntToScalar(30), SkIntToScalar(30) };
    path->addRect(r);
    path->offset(SkIntToScalar(10), SkIntToScalar(0));
    return SkIntToScalar(30);
}

static SkScalar make_oval(SkPath* path) {
    SkRect r = { SkIntToScalar(10), SkIntToScalar(10),
                 SkIntToScalar(30), SkIntToScalar(30) };
    path->addOval(r);
    path->offset(SkIntToScalar(10), SkIntToScalar(0));
    return SkIntToScalar(30);
}

static SkScalar make_star(SkPath* path, int n) {
    const SkScalar c = SkIntToScalar(45);
    const SkScalar r = SkIntToScalar(20);

    SkScalar rad = -SK_ScalarPI / 2;
    const SkScalar drad = (n >> 1) * SK_ScalarPI * 2 / n;

    path->moveTo(c, c - r);
    for (int i = 1; i < n; i++) {
        rad += drad;
        SkScalar cosV, sinV = SkScalarSinCos(rad, &cosV);
        path->lineTo(c + cosV * r, c + sinV * r);
    }
    path->close();
    return r * 2 * 6 / 5;
}

static SkScalar make_star_5(SkPath* path) { return make_star(path, 5); }
static SkScalar make_star_13(SkPath* path) { return make_star(path, 13); }

static SkScalar make_three_line(SkPath* path) {
    static SkScalar xOffset = 34.f;
    static SkScalar yOffset = 50.f;
    path->moveTo(-32.5f + xOffset, 0.0f + yOffset);
    path->lineTo(32.5f + xOffset, 0.0f + yOffset);

    path->moveTo(-32.5f + xOffset, 19 + yOffset);
    path->lineTo(32.5f + xOffset, 19 + yOffset);

    path->moveTo(-32.5f + xOffset, -19 + yOffset);
    path->lineTo(32.5f + xOffset, -19 + yOffset);
    path->lineTo(-32.5f + xOffset, -19 + yOffset);

    path->close();

    return SkIntToScalar(70);
}

static SkScalar make_arrow(SkPath* path) {
    static SkScalar xOffset = 34.f;
    static SkScalar yOffset = 40.f;
    path->moveTo(-26.f + xOffset, 0.0f + yOffset);
    path->lineTo(26.f + xOffset, 0.0f + yOffset);

    path->moveTo(-28.f + xOffset, -2.4748745f + yOffset);
    path->lineTo(0 + xOffset, 25.525126f + yOffset);

    path->moveTo(-28.f + xOffset, 2.4748745f + yOffset);
    path->lineTo(0 + xOffset, -25.525126f + yOffset);
    path->lineTo(-28.f + xOffset, 2.4748745f + yOffset);

    path->close();

    return SkIntToScalar(70);
}

static SkScalar make_curve(SkPath* path) {
    static SkScalar xOffset = -382.f;
    static SkScalar yOffset = -50.f;
    path->moveTo(491 + xOffset, 56 + yOffset);
    path->conicTo(435.93292f + xOffset, 56.000031f + yOffset,
                  382.61078f + xOffset, 69.752716f + yOffset,
                  0.9920463f);

    return SkIntToScalar(40);
}

static SkScalar make_battery(SkPath* path) {
    static SkScalar xOffset = 5.f;

    path->moveTo(24.67f + xOffset, 0.33000004f);
    path->lineTo(8.3299999f + xOffset, 0.33000004f);
    path->lineTo(8.3299999f + xOffset, 5.3299999f);
    path->lineTo(0.33000004f + xOffset, 5.3299999f);
    path->lineTo(0.33000004f + xOffset, 50.669998f);
    path->lineTo(32.669998f + xOffset, 50.669998f);
    path->lineTo(32.669998f + xOffset, 5.3299999f);
    path->lineTo(24.67f + xOffset, 5.3299999f);
    path->lineTo(24.67f + xOffset, 0.33000004f);
    path->close();

    path->moveTo(25.727224f + xOffset, 12.886665f);
    path->lineTo(10.907918f + xOffset, 12.886665f);
    path->lineTo(7.5166659f + xOffset, 28.683645f);
    path->lineTo(14.810181f + xOffset, 28.683645f);
    path->lineTo(7.7024879f + xOffset, 46.135998f);
    path->lineTo(28.049999f + xOffset, 25.136419f);
    path->lineTo(16.854223f + xOffset, 25.136419f);
    path->lineTo(25.727224f + xOffset, 12.886665f);
    path->close();
    return SkIntToScalar(50);
}

static SkScalar make_battery2(SkPath* path) {
    static SkScalar xOffset = 5.f;

    path->moveTo(32.669998f + xOffset, 9.8640003f);
    path->lineTo(0.33000004f + xOffset, 9.8640003f);
    path->lineTo(0.33000004f + xOffset, 50.669998f);
    path->lineTo(32.669998f + xOffset, 50.669998f);
    path->lineTo(32.669998f + xOffset, 9.8640003f);
    path->close();

    path->moveTo(10.907918f + xOffset, 12.886665f);
    path->lineTo(25.727224f + xOffset, 12.886665f);
    path->lineTo(16.854223f + xOffset, 25.136419f);
    path->lineTo(28.049999f + xOffset, 25.136419f);
    path->lineTo(7.7024879f + xOffset, 46.135998f);
    path->lineTo(14.810181f + xOffset, 28.683645f);
    path->lineTo(7.5166659f + xOffset, 28.683645f);
    path->lineTo(10.907918f + xOffset, 12.886665f);
    path->close();

    return SkIntToScalar(70);
}

constexpr MakePathProc gProcs[] = {
    make_triangle,
    make_rect,
    make_oval,
    make_star_5,
    make_star_13,
    make_three_line,
    make_arrow,
    make_curve,
    make_battery,
    make_battery2
};

constexpr SkScalar gWidths[] = {
    2.0f,
    3.0f,
    4.0f,
    5.0f,
    6.0f,
    7.0f,
    7.0f,
    14.0f,
    0.0f,
    0.0f,
};

constexpr SkScalar gMiters[] = {
    2.0f,
    3.0f,
    3.0f,
    3.0f,
    4.0f,
    4.0f,
    4.0f,
    4.0f,
    4.0f,
    4.0f,
};

#define N   17 //SK_ARRAY_COUNT(gProcs)

// This GM tests out drawing small paths (i.e., for Ganesh, using the Distance
// Field path renderer) which are filled, stroked and filledAndStroked. In
// particular this ensures that any cache keys in use include the stroking
// parameters.
class SmallPathsGM : public skiagm::GM {
    SkPath  fPath[N];
    SkScalar fDY[N];
protected:
    void onOnceBeforeDraw() override {

        SkPath* skPath = NULL;
        SkPath* skPaths = fPath;
        skPath = skPaths + 0;
        skPath->moveTo(1132.12f, 498.50f);
        skPath->cubicTo(1130.20f, 498.00f, 1126.95f, 500.77f, 1129.25f, 503.50f);
        skPath->cubicTo(1131.51f, 506.25f, 1135.24f, 510.75f, 1135.24f, 510.75f);
        skPath->cubicTo(1135.24f, 510.75f, 1139.71f, 506.75f, 1141.12f, 504.63f);
        skPath->cubicTo(1142.49f, 502.47f, 1141.50f, 500.88f, 1139.12f, 500.62f);
        skPath->cubicTo(1136.69f, 500.41f, 1135.37f, 502.37f, 1135.37f, 502.37f);
        skPath->cubicTo(1135.37f, 502.37f, 1133.99f, 499.00f, 1132.12f, 498.50f);

        skPath = skPaths + 1;
        skPath->moveTo(991.00f, 499.13f);
        skPath->cubicTo(987.67f, 498.48f, 987.11f, 501.62f, 987.49f, 502.62f);
        skPath->cubicTo(987.87f, 503.63f, 993.37f, 512.12f, 993.37f, 512.12f);
        skPath->cubicTo(993.37f, 512.12f, 996.44f, 508.75f, 999.13f, 506.25f);
        skPath->cubicTo(1001.88f, 503.73f, 1000.13f, 501.38f, 998.37f, 501.13f);
        skPath->cubicTo(996.58f, 500.91f, 993.50f, 502.25f, 993.50f, 502.25f);
        skPath->cubicTo(993.50f, 502.25f, 994.38f, 499.75f, 991.00f, 499.13f);

        skPath = skPaths + 2;
        skPath->moveTo(827.25f, 495.00f);
        skPath->cubicTo(824.56f, 494.86f, 822.22f, 497.12f, 823.63f, 500.50f);
        skPath->cubicTo(825.00f, 503.88f, 829.00f, 510.00f, 829.00f, 510.00f);
        skPath->cubicTo(829.00f, 510.00f, 836.22f, 504.00f, 837.50f, 502.50f);
        skPath->cubicTo(838.76f, 500.99f, 840.26f, 495.87f, 835.25f, 495.50f);
        skPath->cubicTo(830.26f, 495.14f, 829.75f, 499.62f, 829.75f, 499.62f);
        skPath->cubicTo(829.75f, 499.62f, 829.88f, 495.12f, 827.25f, 495.00f);

        skPath = skPaths + 3;
        skPath->moveTo(691.25f, 493.25f);
        skPath->cubicTo(688.44f, 492.89f, 686.07f, 497.62f, 688.63f, 500.50f);
        skPath->cubicTo(691.12f, 503.37f, 697.25f, 509.75f, 697.25f, 509.75f);
        skPath->cubicTo(697.25f, 509.75f, 704.49f, 503.12f, 705.13f, 499.62f);
        skPath->cubicTo(705.75f, 496.09f, 703.87f, 495.25f, 701.25f, 495.00f);
        skPath->cubicTo(698.56f, 494.78f, 696.63f, 498.50f, 696.63f, 498.50f);
        skPath->cubicTo(696.63f, 498.50f, 694.13f, 493.62f, 691.25f, 493.25f);

        skPath = skPaths + 4;
        skPath->moveTo(556.62f, 496.12f);
        skPath->cubicTo(553.93f, 495.26f, 551.99f, 497.28f, 552.50f, 499.87f);
        skPath->cubicTo(553.00f, 502.50f, 558.37f, 508.13f, 558.37f, 508.13f);
        skPath->cubicTo(558.37f, 508.13f, 563.84f, 504.37f, 565.25f, 502.50f);
        skPath->cubicTo(566.62f, 500.63f, 566.37f, 498.25f, 563.88f, 497.25f);
        skPath->cubicTo(561.32f, 496.25f, 558.25f, 500.00f, 558.25f, 500.00f);
        skPath->cubicTo(558.25f, 500.00f, 559.37f, 497.00f, 556.62f, 496.12f);

        skPath = skPaths + 5;
        skPath->moveTo(415.74f, 496.12f);
        skPath->cubicTo(413.18f, 494.76f, 409.86f, 497.00f, 410.50f, 499.38f);
        skPath->cubicTo(411.12f, 501.75f, 416.00f, 509.13f, 416.00f, 509.13f);
        skPath->cubicTo(416.00f, 509.13f, 422.09f, 505.00f, 423.37f, 503.50f);
        skPath->cubicTo(424.63f, 501.99f, 423.74f, 499.63f, 421.12f, 498.75f);
        skPath->cubicTo(418.43f, 497.89f, 416.00f, 501.13f, 416.00f, 501.13f);
        skPath->cubicTo(416.00f, 501.13f, 418.24f, 497.50f, 415.74f, 496.12f);

        skPath = skPaths + 6;
        skPath->moveTo(252.12f, 492.37f);
        skPath->cubicTo(249.69f, 491.00f, 245.84f, 493.62f, 247.24f, 496.50f);
        skPath->cubicTo(248.61f, 499.37f, 253.88f, 507.38f, 253.88f, 507.38f);
        skPath->cubicTo(253.88f, 507.38f, 259.46f, 502.25f, 261.25f, 500.00f);
        skPath->cubicTo(263.00f, 497.76f, 262.00f, 495.13f, 259.38f, 494.25f);
        skPath->cubicTo(256.69f, 493.39f, 253.38f, 496.50f, 253.38f, 496.50f);
        skPath->cubicTo(253.38f, 496.50f, 254.50f, 493.75f, 252.12f, 492.37f);

        skPath = skPaths + 7;
        skPath->moveTo(116.38f, 491.62f);
        skPath->cubicTo(115.10f, 491.26f, 110.96f, 493.88f, 112.76f, 498.12f);
        skPath->cubicTo(114.51f, 502.37f, 121.37f, 505.63f, 121.37f, 505.63f);
        skPath->cubicTo(121.37f, 505.63f, 126.17f, 501.12f, 127.71f, 498.24f);
        skPath->cubicTo(128.44f, 496.87f, 128.76f, 495.75f, 128.26f, 493.62f);
        skPath->cubicTo(127.74f, 491.46f, 125.86f, 491.47f, 123.99f, 492.62f);
        skPath->cubicTo(122.07f, 493.75f, 120.50f, 496.38f, 120.50f, 496.38f);
        skPath->cubicTo(120.50f, 496.38f, 118.40f, 492.16f, 116.38f, 491.62f);

        skPath = skPaths + 8;
        skPath->moveTo(1152.87f, 504.50f);
        skPath->cubicTo(1152.87f, 504.50f, 1152.41f, 508.88f, 1156.25f, 508.00f);
        skPath->cubicTo(1160.13f, 507.14f, 1162.62f, 502.25f, 1161.13f, 502.13f);
        skPath->cubicTo(1159.59f, 501.98f, 1160.17f, 507.50f, 1163.62f, 507.00f);
        skPath->cubicTo(1167.12f, 506.50f, 1169.24f, 503.50f, 1167.87f, 503.00f);
        skPath->cubicTo(1166.46f, 502.50f, 1167.07f, 508.26f, 1169.75f, 508.00f);
        skPath->cubicTo(1172.38f, 507.79f, 1177.37f, 502.37f, 1175.87f, 502.25f);
        skPath->cubicTo(1174.34f, 502.11f, 1179.66f, 507.12f, 1183.63f, 506.63f);
        skPath->cubicTo(1187.64f, 506.12f, 1191.76f, 503.24f, 1189.38f, 503.12f);
        skPath->cubicTo(1186.94f, 502.98f, 1192.67f, 507.87f, 1196.51f, 507.38f);
        skPath->cubicTo(1200.38f, 506.87f, 1206.75f, 504.25f, 1204.62f, 503.63f);
        skPath->cubicTo(1202.44f, 502.98f, 1206.92f, 509.13f, 1209.87f, 508.75f);
        skPath->cubicTo(1212.86f, 508.39f, 1216.63f, 504.39f, 1214.63f, 504.75f);
        skPath->cubicTo(1212.58f, 505.12f, 1217.16f, 507.63f, 1220.88f, 507.00f);
        skPath->cubicTo(1224.63f, 506.35f, 1226.88f, 502.23f, 1225.13f, 502.37f);
        skPath->cubicTo(1223.33f, 502.50f, 1227.38f, 506.75f, 1232.50f, 506.50f);
        skPath->cubicTo(1237.62f, 506.28f, 1241.50f, 504.12f, 1240.00f, 503.00f);
        skPath->cubicTo(1238.46f, 501.85f, 1238.50f, 509.38f, 1243.49f, 508.50f);
        skPath->cubicTo(1248.50f, 507.64f, 1249.87f, 504.12f, 1248.50f, 503.37f);
        skPath->cubicTo(1247.09f, 502.65f, 1249.55f, 508.00f, 1252.75f, 507.00f);
        skPath->cubicTo(1256.00f, 505.99f, 1257.50f, 503.50f, 1256.37f, 502.62f);
        skPath->cubicTo(1255.22f, 501.76f, 1256.44f, 507.84f, 1259.12f, 509.00f);
        skPath->cubicTo(1261.88f, 510.12f, 1263.24f, 507.87f, 1263.24f, 507.87f);

        skPath = skPaths + 9;
        skPath->moveTo(1012.88f, 504.88f);
        skPath->cubicTo(1012.88f, 504.88f, 1016.29f, 509.63f, 1019.62f, 508.38f);
        skPath->cubicTo(1023.00f, 507.15f, 1025.24f, 504.38f, 1024.37f, 503.50f);
        skPath->cubicTo(1023.48f, 502.64f, 1023.53f, 507.50f, 1027.75f, 507.38f);
        skPath->cubicTo(1032.00f, 507.23f, 1033.25f, 502.75f, 1032.50f, 502.75f);
        skPath->cubicTo(1031.73f, 502.75f, 1032.55f, 507.25f, 1035.75f, 507.00f);
        skPath->cubicTo(1039.00f, 506.79f, 1041.13f, 503.62f, 1040.13f, 503.12f);
        skPath->cubicTo(1039.10f, 502.62f, 1040.93f, 506.63f, 1044.01f, 506.63f);
        skPath->cubicTo(1047.13f, 506.63f, 1053.75f, 503.50f, 1051.75f, 503.00f);
        skPath->cubicTo(1049.70f, 502.50f, 1051.93f, 507.87f, 1054.87f, 507.62f);
        skPath->cubicTo(1057.87f, 507.41f, 1061.88f, 504.50f, 1060.62f, 504.25f);
        skPath->cubicTo(1059.34f, 504.04f, 1057.40f, 507.62f, 1061.62f, 508.13f);
        skPath->cubicTo(1065.87f, 508.62f, 1067.62f, 503.75f, 1067.62f, 503.75f);
        skPath->cubicTo(1067.62f, 503.75f, 1067.03f, 509.75f, 1070.87f, 509.50f);
        skPath->cubicTo(1074.75f, 509.28f, 1077.88f, 504.12f, 1076.51f, 504.12f);
        skPath->cubicTo(1075.10f, 504.12f, 1076.06f, 507.66f, 1078.87f, 507.87f);
        skPath->cubicTo(1081.75f, 508.13f, 1085.75f, 503.63f, 1083.88f, 503.25f);
        skPath->cubicTo(1081.96f, 502.89f, 1084.43f, 507.38f, 1087.24f, 507.00f);
        skPath->cubicTo(1090.12f, 506.64f, 1094.00f, 502.75f, 1091.62f, 502.50f);
        skPath->cubicTo(1089.19f, 502.29f, 1094.71f, 507.12f, 1096.50f, 507.12f);
        skPath->cubicTo(1098.25f, 507.12f, 1102.87f, 503.75f, 1101.25f, 503.75f);
        skPath->cubicTo(1099.58f, 503.75f, 1104.06f, 507.86f, 1106.50f, 508.00f);
        skPath->cubicTo(1108.88f, 508.13f, 1112.50f, 503.75f, 1110.25f, 503.63f);
        skPath->cubicTo(1107.94f, 503.48f, 1111.68f, 507.62f, 1114.88f, 507.62f);
        skPath->cubicTo(1118.13f, 507.62f, 1118.00f, 503.25f, 1118.00f, 503.25f);

        skPath = skPaths + 10;
        skPath->moveTo(845.88f, 502.37f);
        skPath->cubicTo(845.88f, 502.37f, 847.80f, 509.13f, 851.38f, 508.25f);
        skPath->cubicTo(855.00f, 507.38f, 857.13f, 501.85f, 855.63f, 502.00f);
        skPath->cubicTo(854.09f, 502.12f, 856.67f, 507.50f, 860.38f, 507.00f);
        skPath->cubicTo(864.13f, 506.50f, 869.75f, 502.62f, 868.12f, 502.37f);
        skPath->cubicTo(866.46f, 502.16f, 866.80f, 506.50f, 870.13f, 506.50f);
        skPath->cubicTo(873.51f, 506.50f, 879.74f, 503.25f, 877.88f, 502.50f);
        skPath->cubicTo(875.96f, 501.78f, 877.80f, 506.04f, 881.00f, 506.25f);
        skPath->cubicTo(884.25f, 506.51f, 889.25f, 502.00f, 887.13f, 501.88f);
        skPath->cubicTo(884.95f, 501.73f, 888.28f, 507.87f, 892.25f, 507.25f);
        skPath->cubicTo(896.26f, 506.60f, 896.00f, 502.62f, 895.50f, 502.37f);
        skPath->cubicTo(894.99f, 502.16f, 895.26f, 508.13f, 900.25f, 507.00f);
        skPath->cubicTo(905.25f, 505.85f, 906.38f, 502.75f, 905.75f, 502.37f);
        skPath->cubicTo(905.11f, 502.01f, 904.13f, 508.63f, 909.12f, 508.38f);
        skPath->cubicTo(914.12f, 508.16f, 915.88f, 503.63f, 914.75f, 503.25f);
        skPath->cubicTo(913.60f, 502.89f, 915.14f, 508.50f, 919.74f, 508.13f);
        skPath->cubicTo(924.36f, 507.77f, 927.39f, 502.25f, 926.13f, 501.75f);
        skPath->cubicTo(924.85f, 501.25f, 926.62f, 507.00f, 932.25f, 506.75f);
        skPath->cubicTo(937.87f, 506.53f, 939.88f, 502.50f, 938.12f, 501.88f);
        skPath->cubicTo(936.33f, 501.23f, 936.55f, 508.62f, 940.01f, 508.25f);
        skPath->cubicTo(943.50f, 507.89f, 948.51f, 503.37f, 947.38f, 503.00f);
        skPath->cubicTo(946.23f, 502.64f, 946.34f, 508.41f, 952.87f, 508.62f);
        skPath->cubicTo(959.37f, 508.87f, 961.13f, 503.12f, 960.50f, 502.75f);
        skPath->cubicTo(959.86f, 502.39f, 959.09f, 509.38f, 966.13f, 509.38f);
        skPath->cubicTo(973.13f, 509.38f, 973.75f, 504.50f, 973.25f, 504.12f);
        skPath->cubicTo(972.74f, 503.76f, 973.53f, 508.87f, 977.37f, 508.38f);
        skPath->cubicTo(981.25f, 507.87f, 982.26f, 505.50f, 982.13f, 505.00f);

        skPath = skPaths + 11;
        skPath->moveTo(719.24f, 502.25f);
        skPath->cubicTo(719.24f, 502.25f, 722.51f, 505.35f, 727.50f, 505.50f);
        skPath->cubicTo(732.51f, 505.62f, 737.13f, 502.62f, 735.87f, 502.37f);
        skPath->cubicTo(734.59f, 502.16f, 735.17f, 505.13f, 738.88f, 504.88f);
        skPath->cubicTo(742.63f, 504.66f, 746.00f, 500.50f, 744.87f, 499.87f);
        skPath->cubicTo(743.72f, 499.23f, 744.37f, 506.50f, 749.88f, 505.75f);
        skPath->cubicTo(755.38f, 505.03f, 754.12f, 500.00f, 753.75f, 499.75f);
        skPath->cubicTo(753.37f, 499.54f, 753.38f, 506.12f, 758.50f, 505.50f);
        skPath->cubicTo(763.62f, 504.85f, 764.38f, 500.00f, 763.25f, 500.00f);
        skPath->cubicTo(762.10f, 500.00f, 762.85f, 505.50f, 768.87f, 504.88f);
        skPath->cubicTo(774.87f, 504.23f, 776.13f, 499.74f, 774.63f, 499.62f);
        skPath->cubicTo(773.09f, 499.48f, 775.39f, 504.50f, 780.38f, 504.50f);
        skPath->cubicTo(785.38f, 504.50f, 788.62f, 500.13f, 786.75f, 499.38f);
        skPath->cubicTo(784.83f, 498.66f, 787.55f, 505.12f, 790.87f, 505.00f);
        skPath->cubicTo(794.25f, 504.86f, 800.88f, 500.88f, 798.63f, 499.62f);
        skPath->cubicTo(796.33f, 498.40f, 797.73f, 504.03f, 804.01f, 504.75f);
        skPath->cubicTo(810.25f, 505.50f, 811.75f, 503.25f, 810.75f, 502.50f);
        skPath->cubicTo(809.73f, 501.78f, 809.68f, 505.35f, 812.88f, 507.00f);
        skPath->cubicTo(816.13f, 508.63f, 816.26f, 507.12f, 816.26f, 507.12f);

        skPath = skPaths + 12;
        skPath->moveTo(576.26f, 501.13f);
        skPath->cubicTo(576.26f, 501.13f, 576.91f, 506.38f, 580.88f, 505.12f);
        skPath->cubicTo(584.88f, 503.90f, 586.00f, 499.87f, 584.37f, 499.87f);
        skPath->cubicTo(582.71f, 499.87f, 583.16f, 505.00f, 587.38f, 504.63f);
        skPath->cubicTo(591.63f, 504.27f, 593.25f, 499.87f, 592.13f, 499.75f);
        skPath->cubicTo(590.98f, 499.61f, 591.28f, 504.88f, 595.38f, 504.00f);
        skPath->cubicTo(599.50f, 503.14f, 601.88f, 499.87f, 600.13f, 499.38f);
        skPath->cubicTo(598.34f, 498.87f, 600.01f, 503.09f, 604.75f, 503.88f);
        skPath->cubicTo(609.13f, 504.63f, 613.26f, 502.12f, 612.01f, 501.00f);
        skPath->cubicTo(610.73f, 499.85f, 611.03f, 504.37f, 615.13f, 504.25f);
        skPath->cubicTo(619.25f, 504.11f, 621.88f, 500.37f, 620.62f, 500.25f);
        skPath->cubicTo(619.34f, 500.10f, 620.03f, 504.37f, 623.87f, 504.12f);
        skPath->cubicTo(627.75f, 503.91f, 629.49f, 500.62f, 628.62f, 500.13f);
        skPath->cubicTo(627.72f, 499.62f, 626.98f, 505.16f, 633.25f, 505.38f);
        skPath->cubicTo(639.50f, 505.63f, 640.26f, 500.37f, 639.13f, 499.87f);
        skPath->cubicTo(637.98f, 499.37f, 639.51f, 505.63f, 644.38f, 504.63f);
        skPath->cubicTo(649.25f, 503.62f, 652.12f, 499.50f, 650.62f, 498.87f);
        skPath->cubicTo(649.09f, 498.23f, 650.25f, 504.37f, 655.63f, 504.12f);
        skPath->cubicTo(661.00f, 503.91f, 663.39f, 499.63f, 662.63f, 499.38f);
        skPath->cubicTo(661.86f, 499.16f, 661.75f, 506.25f, 667.12f, 505.38f);
        skPath->cubicTo(672.50f, 504.51f, 671.88f, 500.25f, 671.13f, 500.25f);
        skPath->cubicTo(670.36f, 500.25f, 670.64f, 504.88f, 675.25f, 504.88f);
        skPath->cubicTo(679.87f, 504.88f, 681.13f, 501.75f, 680.37f, 501.62f);
        skPath->cubicTo(679.60f, 501.48f, 678.90f, 506.63f, 683.25f, 506.63f);
        skPath->cubicTo(687.63f, 506.63f, 687.74f, 504.75f, 687.74f, 504.75f);

        skPath = skPaths + 13;
        skPath->moveTo(436.75f, 502.13f);
        skPath->cubicTo(436.75f, 502.13f, 439.80f, 506.12f, 443.00f, 505.50f);
        skPath->cubicTo(446.25f, 504.85f, 448.87f, 500.88f, 447.87f, 500.38f);
        skPath->cubicTo(446.85f, 499.87f, 447.81f, 506.00f, 450.88f, 505.25f);
        skPath->cubicTo(454.00f, 504.53f, 458.00f, 500.25f, 456.50f, 499.75f);
        skPath->cubicTo(454.96f, 499.25f, 457.05f, 505.13f, 460.12f, 504.88f);
        skPath->cubicTo(463.24f, 504.66f, 464.63f, 500.00f, 464.00f, 499.75f);
        skPath->cubicTo(463.36f, 499.54f, 463.51f, 505.63f, 467.99f, 505.38f);
        skPath->cubicTo(472.50f, 505.16f, 475.51f, 501.75f, 474.75f, 501.25f);
        skPath->cubicTo(473.98f, 500.75f, 473.36f, 505.25f, 479.50f, 505.25f);
        skPath->cubicTo(485.63f, 505.25f, 483.37f, 501.87f, 483.62f, 501.37f);
        skPath->cubicTo(483.88f, 500.87f, 481.48f, 505.50f, 487.24f, 505.00f);
        skPath->cubicTo(492.99f, 504.50f, 491.62f, 500.88f, 491.12f, 500.50f);
        skPath->cubicTo(490.61f, 500.14f, 492.31f, 506.00f, 495.13f, 506.00f);
        skPath->cubicTo(498.01f, 506.00f, 500.13f, 503.00f, 499.38f, 502.75f);
        skPath->cubicTo(498.61f, 502.53f, 499.90f, 506.00f, 504.13f, 506.00f);
        skPath->cubicTo(508.38f, 506.00f, 508.75f, 503.37f, 508.12f, 503.00f);
        skPath->cubicTo(507.48f, 502.64f, 507.11f, 506.12f, 513.13f, 504.37f);
        skPath->cubicTo(519.13f, 502.65f, 515.62f, 499.50f, 515.62f, 499.50f);
        skPath->cubicTo(515.62f, 499.50f, 515.39f, 505.25f, 519.87f, 504.63f);
        skPath->cubicTo(524.38f, 503.98f, 526.13f, 501.25f, 525.38f, 501.13f);
        skPath->cubicTo(524.61f, 500.98f, 524.53f, 504.63f, 528.76f, 504.63f);
        skPath->cubicTo(533.00f, 504.63f, 535.37f, 502.88f, 534.25f, 502.50f);
        skPath->cubicTo(533.09f, 502.14f, 533.26f, 505.25f, 538.25f, 504.12f);
        skPath->cubicTo(543.26f, 502.97f, 542.13f, 500.62f, 542.13f, 500.62f);

        skPath = skPaths + 14;
        skPath->moveTo(270.25f, 502.88f);
        skPath->cubicTo(270.25f, 502.88f, 275.07f, 506.25f, 277.38f, 504.25f);
        skPath->cubicTo(279.63f, 502.24f, 280.24f, 499.00f, 279.37f, 498.75f);
        skPath->cubicTo(278.48f, 498.54f, 277.00f, 503.50f, 282.38f, 504.00f);
        skPath->cubicTo(287.76f, 504.50f, 292.25f, 500.37f, 291.00f, 499.87f);
        skPath->cubicTo(289.72f, 499.37f, 290.53f, 504.75f, 294.37f, 504.63f);
        skPath->cubicTo(298.25f, 504.48f, 303.00f, 500.25f, 302.25f, 499.50f);
        skPath->cubicTo(301.48f, 498.78f, 299.39f, 503.50f, 304.13f, 503.50f);
        skPath->cubicTo(308.88f, 503.50f, 311.24f, 498.00f, 309.75f, 497.50f);
        skPath->cubicTo(308.21f, 496.99f, 309.38f, 505.62f, 314.75f, 504.50f);
        skPath->cubicTo(320.13f, 503.34f, 320.61f, 499.62f, 319.24f, 499.13f);
        skPath->cubicTo(317.84f, 498.62f, 319.04f, 505.12f, 322.88f, 505.00f);
        skPath->cubicTo(326.76f, 504.86f, 331.87f, 500.13f, 329.87f, 498.38f);
        skPath->cubicTo(327.82f, 496.65f, 328.14f, 505.87f, 332.88f, 505.38f);
        skPath->cubicTo(337.63f, 504.87f, 339.25f, 500.63f, 338.25f, 500.38f);
        skPath->cubicTo(337.23f, 500.16f, 336.18f, 505.73f, 344.76f, 505.87f);
        skPath->cubicTo(353.38f, 505.99f, 351.13f, 499.00f, 350.37f, 499.00f);
        skPath->cubicTo(349.61f, 499.00f, 350.37f, 504.62f, 355.62f, 504.12f);
        skPath->cubicTo(360.87f, 503.62f, 363.12f, 499.25f, 361.37f, 498.50f);
        skPath->cubicTo(359.58f, 497.78f, 358.77f, 504.86f, 363.00f, 505.00f);
        skPath->cubicTo(367.24f, 505.12f, 372.62f, 498.63f, 371.25f, 498.38f);
        skPath->cubicTo(369.84f, 498.16f, 370.11f, 505.99f, 376.13f, 505.87f);
        skPath->cubicTo(382.13f, 505.73f, 386.25f, 502.25f, 385.00f, 500.88f);
        skPath->cubicTo(383.72f, 499.51f, 383.12f, 506.66f, 388.62f, 507.38f);
        skPath->cubicTo(394.12f, 508.13f, 396.99f, 502.75f, 396.12f, 502.37f);
        skPath->cubicTo(395.23f, 502.01f, 394.34f, 505.38f, 401.00f, 505.38f);
        skPath->cubicTo(404.75f, 505.38f, 405.12f, 503.37f, 405.12f, 503.37f);

        skPath = skPaths + 15;
        skPath->moveTo(136.63f, 502.13f);
        skPath->cubicTo(136.63f, 502.13f, 138.93f, 504.12f, 141.62f, 502.62f);
        skPath->cubicTo(144.37f, 501.11f, 143.76f, 499.13f, 142.63f, 499.13f);
        skPath->cubicTo(141.48f, 499.13f, 142.78f, 503.25f, 146.62f, 503.00f);
        skPath->cubicTo(150.50f, 502.78f, 149.62f, 497.25f, 148.62f, 497.25f);
        skPath->cubicTo(147.60f, 497.25f, 148.40f, 503.50f, 152.76f, 503.12f);
        skPath->cubicTo(157.13f, 502.76f, 161.00f, 498.37f, 159.24f, 497.87f);
        skPath->cubicTo(157.45f, 497.37f, 158.55f, 503.63f, 161.37f, 503.63f);
        skPath->cubicTo(164.25f, 503.63f, 169.63f, 497.38f, 168.63f, 497.12f);
        skPath->cubicTo(167.60f, 496.91f, 166.73f, 503.76f, 172.62f, 503.50f);
        skPath->cubicTo(178.50f, 503.29f, 178.00f, 498.12f, 177.50f, 497.63f);
        skPath->cubicTo(176.99f, 497.12f, 177.91f, 505.12f, 181.62f, 503.75f);
        skPath->cubicTo(185.37f, 502.38f, 189.36f, 497.87f, 187.99f, 497.50f);
        skPath->cubicTo(186.59f, 497.14f, 186.11f, 502.98f, 192.00f, 503.12f);
        skPath->cubicTo(197.88f, 503.24f, 200.74f, 497.87f, 199.24f, 497.87f);
        skPath->cubicTo(197.71f, 497.87f, 199.86f, 502.88f, 205.62f, 502.00f);
        skPath->cubicTo(211.37f, 501.13f, 211.25f, 496.89f, 210.12f, 497.25f);
        skPath->cubicTo(208.97f, 497.63f, 209.64f, 503.12f, 214.37f, 502.75f);
        skPath->cubicTo(219.12f, 502.39f, 223.24f, 498.75f, 222.37f, 497.25f);
        skPath->cubicTo(221.48f, 495.74f, 217.97f, 503.50f, 224.63f, 503.00f);
        skPath->cubicTo(231.26f, 502.50f, 237.00f, 499.38f, 235.25f, 498.50f);
        skPath->cubicTo(233.46f, 497.64f, 232.04f, 503.34f, 235.75f, 504.50f);
        skPath->cubicTo(239.50f, 505.62f, 240.50f, 504.00f, 240.50f, 504.00f);

        skPath = skPaths + 16;
        skPath->moveTo(11.49f, 503.50f);
        skPath->cubicTo(11.49f, 503.50f, 18.75f, 502.62f, 17.25f, 501.00f);
        skPath->cubicTo(15.72f, 499.34f, 16.44f, 504.25f, 19.25f, 504.19f);
        skPath->cubicTo(22.07f, 504.12f, 25.75f, 501.37f, 24.19f, 500.44f);
        skPath->cubicTo(22.66f, 499.50f, 23.30f, 504.50f, 26.88f, 504.19f);
        skPath->cubicTo(30.50f, 503.90f, 33.75f, 500.62f, 32.00f, 499.69f);
        skPath->cubicTo(30.21f, 498.75f, 29.89f, 504.31f, 34.62f, 504.12f);
        skPath->cubicTo(39.31f, 503.91f, 41.43f, 499.13f, 40.37f, 498.94f);
        skPath->cubicTo(39.35f, 498.72f, 39.35f, 507.12f, 42.93f, 505.56f);
        skPath->cubicTo(46.55f, 503.98f, 49.82f, 498.74f, 48.26f, 498.62f);
        skPath->cubicTo(46.72f, 498.48f, 45.31f, 505.75f, 50.69f, 504.68f);
        skPath->cubicTo(56.06f, 503.60f, 58.50f, 499.40f, 57.00f, 499.69f);
        skPath->cubicTo(55.46f, 500.00f, 57.10f, 506.56f, 60.81f, 505.81f);
        skPath->cubicTo(64.56f, 505.09f, 67.81f, 499.50f, 66.12f, 499.44f);
        skPath->cubicTo(64.46f, 499.36f, 66.05f, 505.63f, 69.12f, 505.00f);
        skPath->cubicTo(72.18f, 504.35f, 77.57f, 499.19f, 75.25f, 499.00f);
        skPath->cubicTo(72.95f, 498.79f, 72.46f, 505.81f, 76.94f, 505.63f);
        skPath->cubicTo(81.45f, 505.41f, 87.39f, 500.25f, 85.57f, 498.75f);
        skPath->cubicTo(83.78f, 497.24f, 81.64f, 504.56f, 91.62f, 503.88f);
        skPath->cubicTo(101.62f, 503.16f, 101.49f, 498.31f, 100.12f, 497.75f);
        skPath->cubicTo(98.71f, 497.17f, 100.47f, 504.50f, 104.44f, 503.19f);
        skPath->cubicTo(108.38f, 501.89f, 108.25f, 497.51f, 107.38f, 497.87f);


        for (size_t i = 0; i < N; i++) {
            fDY[i] = 0.0f;
        }
    }

    SkString onShortName() override {
        return SkString("smallpaths");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(2.0f);
        paint.setStyle(SkPaint::Style::kStroke_Style);

        // first column: filled paths
        canvas->save();
        canvas->scale(3, 3);
        for (size_t i = 0; i < N; i++) {
            canvas->drawPath(fPath[i], paint);
            canvas->translate(SkIntToScalar(0), fDY[i]);
        }
        canvas->restore();
        return;

        canvas->translate(SkIntToScalar(120), SkIntToScalar(0));

        // second column: stroked paths
        canvas->save();
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeCap(SkPaint::kButt_Cap);
        for (size_t i = 0; i < N; i++) {
            paint.setStrokeWidth(gWidths[i]);
            paint.setStrokeMiter(gMiters[i]);
            canvas->drawPath(fPath[i], paint);
            canvas->translate(SkIntToScalar(0), fDY[i]);
        }
        canvas->restore();
        canvas->translate(SkIntToScalar(120), SkIntToScalar(0));

        // third column: stroked paths with different widths
        canvas->save();
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeCap(SkPaint::kButt_Cap);
        for (size_t i = 0; i < N; i++) {
            paint.setStrokeWidth(gWidths[i] + 2.0f);
            paint.setStrokeMiter(gMiters[i]);
            canvas->drawPath(fPath[i], paint);
            canvas->translate(SkIntToScalar(0), fDY[i]);
        }
        canvas->restore();
        canvas->translate(SkIntToScalar(120), SkIntToScalar(0));

        // fourth column: stroked and filled paths
        paint.setStyle(SkPaint::kStrokeAndFill_Style);
        paint.setStrokeCap(SkPaint::kButt_Cap);
        for (size_t i = 0; i < N; i++) {
            paint.setStrokeWidth(gWidths[i]);
            paint.setStrokeMiter(gMiters[i]);
            canvas->drawPath(fPath[i], paint);
            canvas->translate(SkIntToScalar(0), fDY[i]);
        }

    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new SmallPathsGM;)
