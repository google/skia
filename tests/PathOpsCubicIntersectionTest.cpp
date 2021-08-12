/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkGeometry.h"
#include "src/pathops/SkIntersections.h"
#include "src/pathops/SkPathOpsRect.h"
#include "src/pathops/SkReduceOrder.h"
#include "tests/PathOpsCubicIntersectionTestData.h"
#include "tests/PathOpsTestCommon.h"
#include "tests/Test.h"

#include <stdlib.h>

using namespace PathOpsCubicIntersectionTestData;

static constexpr int kFirstCubicIntersectionTest = 9;

static void standardTestCases(skiatest::Reporter* reporter) {
    for (size_t index = kFirstCubicIntersectionTest; index < tests_count; ++index) {
        int iIndex = static_cast<int>(index);
        const CubicPts& cubic1 = tests[index][0];
        const CubicPts& cubic2 = tests[index][1];
        SkDCubic c1, c2;
        c1.debugSet(cubic1.fPts);
        c2.debugSet(cubic2.fPts);
        SkReduceOrder reduce1, reduce2;
        int order1 = reduce1.reduce(c1, SkReduceOrder::kNo_Quadratics);
        int order2 = reduce2.reduce(c2, SkReduceOrder::kNo_Quadratics);
        const bool showSkipped = false;
        if (order1 < 4) {
            if (showSkipped) {
                SkDebugf("%s [%d] cubic1 order=%d\n", __FUNCTION__, iIndex, order1);
            }
            continue;
        }
        if (order2 < 4) {
            if (showSkipped) {
                SkDebugf("%s [%d] cubic2 order=%d\n", __FUNCTION__, iIndex, order2);
            }
            continue;
        }
        SkIntersections tIntersections;
        tIntersections.intersect(c1, c2);
        if (!tIntersections.used()) {
            if (showSkipped) {
                SkDebugf("%s [%d] no intersection\n", __FUNCTION__, iIndex);
            }
            continue;
        }
        if (tIntersections.isCoincident(0)) {
            if (showSkipped) {
                SkDebugf("%s [%d] coincident\n", __FUNCTION__, iIndex);
            }
            continue;
        }
        for (int pt = 0; pt < tIntersections.used(); ++pt) {
            double tt1 = tIntersections[0][pt];
            SkDPoint xy1 = c1.ptAtT(tt1);
            double tt2 = tIntersections[1][pt];
            SkDPoint xy2 = c2.ptAtT(tt2);
            if (!xy1.approximatelyEqual(xy2)) {
                SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                    __FUNCTION__, (int)index, pt, tt1, xy1.fX, xy1.fY, tt2, xy2.fX, xy2.fY);
            }
            REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
        }
        reporter->bumpTestCount();
    }
}

static const CubicPts testSet[] = {
// FIXME: uncommenting these two will cause this to fail
// this results in two curves very nearly but not exactly coincident
#if 0
{{{67.426548091427676, 37.993772624988935}, {23.483695892376684, 90.476863174921306},
      {35.597065061143162, 79.872482633158796}, {75.38634169631932, 18.244890038969412}}},
{{{67.4265481, 37.9937726}, {23.4836959, 90.4768632}, {35.5970651, 79.8724826},
      {75.3863417, 18.24489}}},
#endif

{{{0, 0}, {0, 1}, {1, 1}, {1, 0}}},
{{{1, 0}, {0, 0}, {0, 1}, {1, 1}}},

{{{0, 1}, {4, 5}, {1, 0}, {5, 3}}},
{{{0, 1}, {3, 5}, {1, 0}, {5, 4}}},

{{{0, 1}, {1, 6}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {1, 0}, {6, 1}}},

{{{0, 1}, {3, 4}, {1, 0}, {5, 1}}},
{{{0, 1}, {1, 5}, {1, 0}, {4, 3}}},

{{{0, 1}, {1, 2}, {1, 0}, {6, 1}}},
{{{0, 1}, {1, 6}, {1, 0}, {2, 1}}},

{{{0, 1}, {0, 5}, {1, 0}, {4, 0}}},
{{{0, 1}, {0, 4}, {1, 0}, {5, 0}}},

{{{0, 1}, {3, 4}, {1, 0}, {3, 0}}},
{{{0, 1}, {0, 3}, {1, 0}, {4, 3}}},

{{{0, 0}, {1, 2}, {3, 4}, {4, 4}}},
{{{0, 0}, {1, 2}, {3, 4}, {4, 4}}},
{{{4, 4}, {3, 4}, {1, 2}, {0, 0}}},

{{{0, 1}, {2, 3}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {1, 0}, {3, 2}}},

{{{0, 2}, {0, 1}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {2, 0}, {1, 0}}},

{{{0, 1}, {0, 2}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {1, 0}, {2, 0}}},

{{{0, 1}, {1, 6}, {1, 0}, {2, 0}}},
{{{0, 1}, {0, 2}, {1, 0}, {6, 1}}},

{{{0, 1}, {5, 6}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {1, 0}, {6, 5}}},

{{{95.837747722788592, 45.025976907939643}, {16.564570095652982, 0.72959763963222402},
        {63.209855865319199, 68.047528419665767}, {57.640240647662544, 59.524565264361243}}},
{{{51.593891741518817, 38.53849970667553}, {62.34752929878772, 74.924924725166022},
        {74.810149322641152, 34.17966562983564}, {29.368398119401373, 94.66719277886078}}},

{{{39.765160968417838, 33.060396198677083}, {5.1922921581157908, 66.854301452103215},
        {31.619281802149157, 25.269248720849514}, {81.541621071073038, 70.025341524754353}}},
{{{46.078911165743556, 48.259962651999651}, {20.24450549867214, 49.403916182650214},
        {0.26325131778756683, 24.46489805563581}, {15.915006546264051, 83.515023059917155}}},

{{{65.454505973241524, 93.881892270353575}, {45.867360264932437, 92.723972719499827},
        {2.1464054482739447, 74.636369140183717}, {33.774068594804994, 40.770872887582925}}},
{{{72.963387832494163, 95.659300729473728}, {11.809496633619768, 82.209921247423594},
        {13.456139067865974, 57.329313623406605}, {36.060621606214262, 70.867335643091849}}},

{{{32.484981432782945, 75.082940782924624}, {42.467313093350882, 48.131159948246157},
        {3.5963115764764657, 43.208665839959245}, {79.442476890721579, 89.709102357602262}}},
{{{18.98573861410177, 93.308887208490106}, {40.405250173250792, 91.039661826118675},
        {8.0467721950480584, 42.100282172719147}, {40.883324221187891, 26.030185504830527}}},

{{{7.5374809128872498, 82.441702896003477}, {22.444346930107265, 22.138854312775123},
        {66.76091829629658, 50.753805856571446}, {78.193478508942519, 97.7932997968948}}},
{{{97.700573130371311, 53.53260215070685}, {87.72443481149358, 84.575876772671876},
        {19.215031396232092, 47.032676472809484}, {11.989686410869325, 10.659507480757082}}},

{{{26.192053931854691, 9.8504326817814416}, {10.174241480498686, 98.476562741434464},
        {21.177712558385782, 33.814968789841501}, {75.329030899018534, 55.02231980442177}}},
{{{56.222082700683771, 24.54395039218662}, {95.589995289030483, 81.050822735322086},
        {28.180450866082897, 28.837706255185282}, {60.128952916771617, 87.311672180570511}}},

{{{42.449716172390481, 52.379709366885805}, {27.896043159019225, 48.797373636065686},
        {92.770268299044233, 89.899302036454571}, {12.102066544863426, 99.43241951960718}}},
{{{45.77532924980639, 45.958701495993274}, {37.458701356062065, 68.393691335056758},
        {37.569326692060258, 27.673713456687381}, {60.674866037757539, 62.47349659096146}}},

{{{67.426548091427676, 37.993772624988935}, {23.483695892376684, 90.476863174921306},
        {35.597065061143162, 79.872482633158796}, {75.38634169631932, 18.244890038969412}}},
{{{61.336508189019057, 82.693132843213675}, {44.639380902349664, 54.074825790745592},
        {16.815615499771951, 20.049704667203923}, {41.866884958868326, 56.735503699973002}}},

{{{18.1312339, 31.6473732}, {95.5711034, 63.5350219}, {92.3283165, 62.0158945},
        {18.5656052, 32.1268808}}},
{{{97.402018, 35.7169972}, {33.1127443, 25.8935163}, {1.13970027, 54.9424981},
        {56.4860195, 60.529264}}},
};

const int testSetCount = (int) SK_ARRAY_COUNT(testSet);

static const CubicPts newTestSet[] = {

{ { { 130.0427549999999997, 11417.41309999999976 },{ 130.2331240000000037, 11418.3192999999992 },{ 131.0370790000000056, 11419 },{ 132, 11419 } } },
{ { { 132, 11419 },{ 130.8954319999999996, 11419 },{ 130, 11418.10449999999946 },{ 130, 11417 } } },

{{{1,3}, {-1.0564518,1.79032254}, {1.45265341,0.229448318}, {1.45381773,0.22913377}}},
{{{1.45381773,0.22913377}, {1.45425761,0.229014933}, {1.0967741,0.451612949}, {0,1}}},

{{{1.64551306f, 3.57876182f}, {0.298127174f, 3.70454836f}, {-0.809808373f, 6.39524937f}, {-3.66666651f, 13.333334f}}},
{{{1, 2}, {1, 2}, {-3.66666651f, 13.333334f}, {5, 6}}},

{{{0.0660428554,1.65340209}, {-0.251940489,1.43560803}, {-0.782382965,-0.196299091}, {3.33333325,-0.666666627}}},
{{{1,3}, {-1.22353387,1.09411383}, {0.319867611,0.12996155}, {0.886705518,0.107543148}}},

{{{-0.13654758,2.10514426}, {-0.585797966,1.89349782}, {-0.807703257,-0.192306399}, {6,-1}}},
{{{1,4}, {-2.25000453,1.42241001}, {1.1314013,0.0505309105}, {1.87140274,0.0363764353}}},

{{{1.3127951622009277, 2.0637707710266113}, {1.8210518360137939, 1.9148571491241455}, {1.6106204986572266, -0.68700540065765381}, {8.5, -2.5}}},
{{{3, 4}, {0.33333325386047363, 1.3333332538604736}, {3.6666667461395264, -0.66666674613952637}, {3.6666665077209473, -0.66666656732559204}}},

{{{980.026001,1481.276}, {980.026001,1481.276}, {980.02594,1481.27576}, {980.025879,1481.27527}}},
{{{980.025879,1481.27527}, {980.025452,1481.27222}, {980.023743,1481.26038}, {980.02179,1481.24072}}},

{{{1.80943513,3.07782435}, {1.66686702,2.16806936}, {1.68301272,0}, {3,0}}},
{{{0,1}, {0,3}, {3,2}, {5,2}}},

{{{3.4386673,2.66977954}, {4.06668949,2.17046738}, {4.78887367,1.59629118}, {6,2}}},
{{{1.71985495,3.49467373}, {2.11620402,2.7201426}, {2.91897964,1.15138781}, {6,3}}},

{{{0,1}, {0.392703831,1.78540766}, {0.219947904,2.05676103}, {0.218561709,2.05630541}}},
{{{0.218561709,2.05630541}, {0.216418028,2.05560064}, {0.624105453,1.40486407}, {4.16666651,1.00000012}}},

{{{0, 1}, {3, 5}, {2, 1}, {3, 1}}},
{{{1.01366711f, 2.21379328f}, {1.09074128f, 2.23241305f}, {1.60246587f, 0.451849401f}, {5, 3}}},

{{{0, 1}, {0.541499972f, 3.16599989f}, {1.08299994f, 2.69299984f}, {2.10083938f, 1.80391729f}}},
{{{0.806384504f, 2.85426903f}, {1.52740121f, 1.99355423f}, {2.81689167f, 0.454222918f}, {5, 1}}},

{{{0, 1}, {1.90192389f, 2.90192389f}, {2.59807634f, 2.79422879f}, {3.1076951f, 2.71539044f}}},
{{{2, 3}, {2.36602545f, 3.36602545f}, {2.330127f, 3.06217766f}, {2.28460979f, 2.67691422f}}},

{{{0, 1}, {1.90192389f, 2.90192389f}, {2.59807634f, 2.79422879f}, {3.1076951f, 2.71539044f}}},
{{{2.28460979f, 2.67691422f}, {2.20577145f, 2.00961876f}, {2.09807634f, 1.09807622f}, {4, 3}}},

{{{0, 1}, {0.8211091160774231, 2.0948121547698975}, {0.91805583238601685, 2.515404224395752}, {0.91621249914169312, 2.5146586894989014}}},
{{{0.91621249914169312, 2.5146586894989014}, {0.91132104396820068, 2.5126807689666748}, {0.21079301834106445, -0.45617169141769409}, {10.5, -1.6666665077209473}}},

{{{42.6237564,68.9841232}, {32.449646,81.963089}, {14.7713947,103.565269}, {12.6310005,105.247002}}},
{{{37.2640038,95.3540039}, {37.2640038,95.3540039}, {11.3710003,83.7339935}, {-25.0779991,124.912003}}},

{{{0,1}, {4,5}, {6,0}, {1,0}}},
{{{0,6}, {0,1}, {1,0}, {5,4}}},

{{{0,1}, {4,6}, {5,1}, {6,2}}},
{{{1,5}, {2,6}, {1,0}, {6,4}}},

{{{322, 896.04803466796875}, {314.09201049804687, 833.4376220703125}, {260.24713134765625, 785}, {195, 785}}},
{{{195, 785}, {265.14016723632812, 785}, {322, 842.30755615234375}, {322, 913}}},

{{{1, 4}, {4, 5}, {3, 2}, {6, 3}}},
{{{2, 3}, {3, 6}, {4, 1}, {5, 4}}},

{{{67, 913}, {67, 917.388916015625}, {67.224380493164063, 921.72576904296875}, {67.662384033203125, 926}}},
{{{194, 1041}, {123.85984039306641, 1041}, {67, 983.69244384765625}, {67, 913}}},

{{{1,4}, {1,5}, {6,0}, {5,1}}},
{{{0,6}, {1,5}, {4,1}, {5,1}}},

{{{0,1}, {4,5}, {6,0}, {1,0}}},
{{{0,6}, {0,1}, {1,0}, {5,4}}},

{{{0,1}, {4,6}, {2,0}, {2,0}}},
{{{0,2}, {0,2}, {1,0}, {6,4}}},

{{{980.9000244140625, 1474.3280029296875}, {980.9000244140625, 1474.3280029296875}, {978.89300537109375, 1471.95703125}, {981.791015625, 1469.487060546875}}},
{{{981.791015625, 1469.487060546875}, {981.791015625, 1469.4859619140625}, {983.3580322265625, 1472.72900390625}, {980.9000244140625, 1474.3280029296875}}},

{{{275,532}, {277.209137,532}, {279,530.209106}, {279,528}}},
{{{278,529}, {278,530.65686}, {276.65686,532}, {275,532}}},

#if 0  // FIXME: asserts coincidence, not working yet
{{{195, 785}, {124.30755615234375, 785}, {67, 841.85986328125}, {67, 912}}},
{{{67, 913}, {67, 842.30755615234375}, {123.85984039306641, 785}, {194, 785}}},
#endif

{{{149,710.001465}, {149.000809,712.209961}, {150.791367,714}, {153,714}}},
{{{154,715}, {151.238571,715}, {149,712.761414}, {149,710}}},

{{{1,2}, {1,2}, {2,0}, {6,0}}},
{{{0,2}, {0,6}, {2,1}, {2,1}}},

{{{0,1}, {2,3}, {5,1}, {4,3}}},
{{{1,5}, {3,4}, {1,0}, {3,2}}},

{{{399,657}, {399,661.970581}, {403.029449,666}, {408,666}}},
{{{406,666}, {402.686279,666}, {400,663.313721}, {400,660}}},

{{{0,5}, {3,5}, {3,0}, {3,2}}},
{{{0,3}, {2,3}, {5,0}, {5,3}}},

{{{132, 11419}, {130.89543151855469, 11419}, {130, 11418.1044921875}, {130, 11417}}},

{{{3, 4}, {1, 5}, {4, 3}, {6, 4}}},
{{{3, 4}, {4, 6}, {4, 3}, {5, 1}}},

{{{130.04275512695312, 11417.413085937500 },
    {130.23312377929687, 11418.319335937500 },
    {131.03707885742187, 11419.000000000000 },
    {132.00000000000000, 11419.000000000000 }}},

{{{132.00000000000000, 11419.000000000000 },
    {130.89543151855469, 11419.000000000000 },
    {130.00000000000000, 11418.104492187500 },
    {130.00000000000000, 11417.000000000000 }}},

{{{1.0516976506771041, 2.9684399028541346 },
    {1.0604363140895228, 2.9633503074444141 },
    {1.0692548215065762, 2.9580354426587459 },
    {1.0781560339512140, 2.9525043684031349 }}},

{{{1.0523038101345104, 2.9523755204833737 },
    {1.0607035288264237, 2.9580853881628375 },
    {1.0690530472271964, 2.9633896794787749 },
    {1.0773566568712512, 2.9682969775000219 }}},

{{{1.0386522625066592, 2.9759024812329078 },
    {1.0559713690392631, 2.9661782500838885 },
    {1.0736041309019990, 2.9555348259177858 },
    {1.0915734362784633, 2.9440446879826569 }}},

{{{1.0396670794879301, 2.9435062123457261 },
    {1.0565690546812769, 2.9557413250983462 },
    {1.0732616463413533, 2.9663369676594282 },
    {1.0897791867435489, 2.9753618045797472 }}},

{{{0.8685656183311091, 3.0409266475785208 },
    {0.99189542936395292, 3.0212163698184424 },
    {1.1302108367493320, 2.9265646471747306 },
    {1.2952305904872474, 2.7940808546473788 }}},

{{{0.85437872843682727, 2.7536036928549055 },
    {1.0045584590592620, 2.9493041024831705 },
    {1.1336998329885613, 3.0248027987251747 },
    {1.2593809752247314, 3.0152560315809107 }}},

{{{0, 1}, {1, 6}, {1, 0}, {6, 2}}},
{{{0, 1}, {2, 6}, {1, 0}, {6, 1}}},

{{{134,11414}, {131.990234375,11414}, {130.32666015625,11415.482421875}, {130.04275512695312,11417.4130859375}}},
{{{132,11419}, {130.89543151855469,11419}, {130,11418.1044921875}, {130,11417}}},

{{{132,11419}, {130.89543151855469,11419}, {130,11418.1044921875}, {130,11417}}},
{{{130.04275512695312,11417.4130859375}, {130.23312377929687,11418.3193359375}, {131.03707885742187,11419}, {132,11419}}},

{{{0, 1}, {2, 3}, {5, 1}, {4, 3}}},
{{{1, 5}, {3, 4}, {1, 0}, {3, 2}}},

{{{3, 5}, {1, 6}, {5, 0}, {3, 1}}},
{{{0, 5}, {1, 3}, {5, 3}, {6, 1}}},

{{{0, 1}, {1, 5}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {1, 0}, {5, 1}}},

{{{1, 3}, {5, 6}, {5, 3}, {5, 4}}},
{{{3, 5}, {4, 5}, {3, 1}, {6, 5}}},

{{{0, 5}, {0, 5}, {5, 4}, {6, 4}}},
{{{4, 5}, {4, 6}, {5, 0}, {5, 0}}},

{{{0, 4}, {1, 3}, {5, 4}, {4, 2}}},
{{{4, 5}, {2, 4}, {4, 0}, {3, 1}}},

{{{0, 2}, {1, 5}, {3, 2}, {4, 1}}},
{{{2, 3}, {1, 4}, {2, 0}, {5, 1}}},

{{{0, 2}, {2, 3}, {5, 1}, {3, 2}}},
{{{1, 5}, {2, 3}, {2, 0}, {3, 2}}},

{{{2, 6}, {4, 5}, {1, 0}, {6, 1}}},
{{{0, 1}, {1, 6}, {6, 2}, {5, 4}}},

{{{0, 1}, {1, 2}, {6, 5}, {5, 4}}},
{{{5, 6}, {4, 5}, {1, 0}, {2, 1}}},

{{{2.5119999999999996, 1.5710000000000002}, {2.6399999999999983, 1.6599999999999997},
        {2.8000000000000007, 1.8000000000000003}, {3, 2}}},
{{{2.4181876227114887, 1.9849772580462195}, {2.8269904869227211, 2.009330650246834},
        {3.2004679292461624, 1.9942047174679169}, {3.4986199496818058, 2.0035994597094731}}},

{{{2, 3}, {1, 4}, {1, 0}, {6, 0}}},
{{{0, 1}, {0, 6}, {3, 2}, {4, 1}}},

{{{0, 2}, {1, 5}, {1, 0}, {6, 1}}},
{{{0, 1}, {1, 6}, {2, 0}, {5, 1}}},

{{{0, 1}, {1, 5}, {2, 1}, {4, 0}}},
{{{1, 2}, {0, 4}, {1, 0}, {5, 1}}},

{{{0, 1}, {3, 5}, {2, 1}, {3, 1}}},
{{{1, 2}, {1, 3}, {1, 0}, {5, 3}}},

{{{0, 1}, {2, 5}, {6, 0}, {5, 3}}},
{{{0, 6}, {3, 5}, {1, 0}, {5, 2}}},

{{{0, 1}, {3, 6}, {1, 0}, {5, 2}}},
{{{0, 1}, {2, 5}, {1, 0}, {6, 3}}},

{{{1, 2}, {5, 6}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {2, 1}, {6, 5}}},

{{{0, 6}, {1, 2}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {6, 0}, {2, 1}}},

{{{0, 2}, {0, 1}, {3, 0}, {1, 0}}},
{{{0, 3}, {0, 1}, {2, 0}, {1, 0}}},
};

const int newTestSetCount = (int) SK_ARRAY_COUNT(newTestSet);
static void oneOff(skiatest::Reporter* reporter, const CubicPts& cubic1, const CubicPts& cubic2,
        bool coin) {
    SkDCubic c1, c2;
    c1.debugSet(cubic1.fPts);
    c2.debugSet(cubic2.fPts);
    SkASSERT(ValidCubic(c1));
    SkASSERT(ValidCubic(c2));
#if ONE_OFF_DEBUG
    SkDebugf("computed quadratics given\n");
    SkDebugf("  {{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}},\n",
        cubic1[0].fX, cubic1[0].fY, cubic1[1].fX, cubic1[1].fY,
        cubic1[2].fX, cubic1[2].fY, cubic1[3].fX, cubic1[3].fY);
    SkDebugf("  {{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}},\n",
        cubic2[0].fX, cubic2[0].fY, cubic2[1].fX, cubic2[1].fY,
        cubic2[2].fX, cubic2[2].fY, cubic2[3].fX, cubic2[3].fY);
#endif
    SkIntersections intersections;
    intersections.intersect(c1, c2);
#if DEBUG_T_SECT_DUMP == 3
    SkDebugf("</div>\n\n");
    SkDebugf("<script type=\"text/javascript\">\n\n");
    SkDebugf("var testDivs = [\n");
    for (int index = 1; index <= gDumpTSectNum; ++index) {
        SkDebugf("sect%d,\n", index);
    }
#endif
    REPORTER_ASSERT(reporter, !coin || intersections.used() >= 2);
    double tt1, tt2;
    SkDPoint xy1, xy2;
    for (int pt3 = 0; pt3 < intersections.used(); ++pt3) {
        tt1 = intersections[0][pt3];
        xy1 = c1.ptAtT(tt1);
        tt2 = intersections[1][pt3];
        xy2 = c2.ptAtT(tt2);
        const SkDPoint& iPt = intersections.pt(pt3);
#if ONE_OFF_DEBUG
        SkDebugf("%s t1=%1.9g (%1.9g, %1.9g) (%1.9g, %1.9g) (%1.9g, %1.9g) t2=%1.9g\n",
                __FUNCTION__, tt1, xy1.fX, xy1.fY, iPt.fX,
                iPt.fY, xy2.fX, xy2.fY, tt2);
#endif
       REPORTER_ASSERT(reporter, xy1.approximatelyEqual(iPt));
       REPORTER_ASSERT(reporter, xy2.approximatelyEqual(iPt));
       REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
    }
    reporter->bumpTestCount();
}

static void oneOff(skiatest::Reporter* reporter, int outer, int inner) {
    const CubicPts& cubic1 = testSet[outer];
    const CubicPts& cubic2 = testSet[inner];
    oneOff(reporter, cubic1, cubic2, false);
}

static void newOneOff(skiatest::Reporter* reporter, int outer, int inner) {
    const CubicPts& cubic1 = newTestSet[outer];
    const CubicPts& cubic2 = newTestSet[inner];
    oneOff(reporter, cubic1, cubic2, false);
}

static void testsOneOff(skiatest::Reporter* reporter, int index) {
    const CubicPts& cubic1 = tests[index][0];
    const CubicPts& cubic2 = tests[index][1];
    oneOff(reporter, cubic1, cubic2, false);
}

static void oneOffTests(skiatest::Reporter* reporter) {
    for (int outer = 0; outer < testSetCount - 1; ++outer) {
        for (int inner = outer + 1; inner < testSetCount; ++inner) {
            oneOff(reporter, outer, inner);
        }
    }
    for (int outer = 0; outer < newTestSetCount - 1; ++outer) {
        for (int inner = outer + 1; inner < newTestSetCount; ++inner) {
            newOneOff(reporter, outer, inner);
        }
    }
}

#define DEBUG_CRASH 0

static void CubicIntersection_RandTest(skiatest::Reporter* reporter) {
    srand(0);
    const int kNumTests = 10000000;
#if !defined(SK_BUILD_FOR_WIN) && !defined(SK_BUILD_FOR_ANDROID)
    unsigned seed = 0;
#endif
    for (int test = 0; test < kNumTests; ++test) {
        CubicPts cubic1, cubic2;
        for (int i = 0; i < 4; ++i) {
            cubic1.fPts[i].fX = static_cast<double>(SK_RAND(seed)) / RAND_MAX * 100;
            cubic1.fPts[i].fY = static_cast<double>(SK_RAND(seed)) / RAND_MAX * 100;
            cubic2.fPts[i].fX = static_cast<double>(SK_RAND(seed)) / RAND_MAX * 100;
            cubic2.fPts[i].fY = static_cast<double>(SK_RAND(seed)) / RAND_MAX * 100;
        }
    #if DEBUG_CRASH
        char str[1024];
        snprintf(str, sizeof(str),
            "{{{%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}}},\n"
            "{{{%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}}},\n",
                cubic1[0].fX, cubic1[0].fY,  cubic1[1].fX, cubic1[1].fY, cubic1[2].fX, cubic1[2].fY,
                cubic1[3].fX, cubic1[3].fY,
                cubic2[0].fX, cubic2[0].fY,  cubic2[1].fX, cubic2[1].fY, cubic2[2].fX, cubic2[2].fY,
                cubic2[3].fX, cubic2[3].fY);
    #endif
        SkDRect rect1, rect2;
        SkDCubic c1, c2;
        c1.debugSet(cubic1.fPts);
        c2.debugSet(cubic2.fPts);
        rect1.setBounds(c1);
        rect2.setBounds(c2);
        bool boundsIntersect = rect1.fLeft <= rect2.fRight && rect2.fLeft <= rect2.fRight
                && rect1.fTop <= rect2.fBottom && rect2.fTop <= rect1.fBottom;
        if (test == -1) {
            SkDebugf("ready...\n");
        }
        SkIntersections intersections2;
        int newIntersects = intersections2.intersect(c1, c2);
        if (!boundsIntersect && newIntersects) {
    #if DEBUG_CRASH
            SkDebugf("%s %d unexpected intersection boundsIntersect=%d "
                    " newIntersects=%d\n%s %s\n", __FUNCTION__, test, boundsIntersect,
                    newIntersects, __FUNCTION__, str);
    #endif
            REPORTER_ASSERT(reporter, 0);
        }
        for (int pt = 0; pt < intersections2.used(); ++pt) {
            double tt1 = intersections2[0][pt];
            SkDPoint xy1 = c1.ptAtT(tt1);
            double tt2 = intersections2[1][pt];
            SkDPoint xy2 = c2.ptAtT(tt2);
            REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
        }
        reporter->bumpTestCount();
    }
}

static void intersectionFinder(int index0, int index1, double t1Seed, double t2Seed,
        double t1Step, double t2Step) {
    const CubicPts& cubic1 = newTestSet[index0];
    const CubicPts& cubic2 = newTestSet[index1];
    SkDPoint t1[3], t2[3];
    bool toggle = true;
    SkDCubic c1, c2;
    c1.debugSet(cubic1.fPts);
    c2.debugSet(cubic2.fPts);
    do {
        t1[0] = c1.ptAtT(t1Seed - t1Step);
        t1[1] = c1.ptAtT(t1Seed);
        t1[2] = c1.ptAtT(t1Seed + t1Step);
        t2[0] = c2.ptAtT(t2Seed - t2Step);
        t2[1] = c2.ptAtT(t2Seed);
        t2[2] = c2.ptAtT(t2Seed + t2Step);
        double dist[3][3];
        dist[1][1] = t1[1].distance(t2[1]);
        int best_i = 1, best_j = 1;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (i == 1 && j == 1) {
                    continue;
                }
                dist[i][j] = t1[i].distance(t2[j]);
                if (dist[best_i][best_j] > dist[i][j]) {
                    best_i = i;
                    best_j = j;
                }
            }
        }
        if (best_i == 0) {
            t1Seed -= t1Step;
        } else if (best_i == 2) {
            t1Seed += t1Step;
        }
        if (best_j == 0) {
            t2Seed -= t2Step;
        } else if (best_j == 2) {
            t2Seed += t2Step;
        }
        if (best_i == 1 && best_j == 1) {
            if ((toggle ^= true)) {
                t1Step /= 2;
            } else {
                t2Step /= 2;
            }
        }
    } while (!t1[1].approximatelyEqual(t2[1]));
    t1Step = t2Step = 0.1;
    double t10 = t1Seed - t1Step * 2;
    double t12 = t1Seed + t1Step * 2;
    double t20 = t2Seed - t2Step * 2;
    double t22 = t2Seed + t2Step * 2;
    SkDPoint test;
    while (!approximately_zero(t1Step)) {
        test = c1.ptAtT(t10);
        t10 += t1[1].approximatelyEqual(test) ? -t1Step : t1Step;
        t1Step /= 2;
    }
    t1Step = 0.1;
    while (!approximately_zero(t1Step)) {
        test = c1.ptAtT(t12);
        t12 -= t1[1].approximatelyEqual(test) ? -t1Step : t1Step;
        t1Step /= 2;
    }
    while (!approximately_zero(t2Step)) {
        test = c2.ptAtT(t20);
        t20 += t2[1].approximatelyEqual(test) ? -t2Step : t2Step;
        t2Step /= 2;
    }
    t2Step = 0.1;
    while (!approximately_zero(t2Step)) {
        test = c2.ptAtT(t22);
        t22 -= t2[1].approximatelyEqual(test) ? -t2Step : t2Step;
        t2Step /= 2;
    }
#if ONE_OFF_DEBUG
    SkDebugf("%s t1=(%1.9g<%1.9g<%1.9g) t2=(%1.9g<%1.9g<%1.9g)\n", __FUNCTION__,
        t10, t1Seed, t12, t20, t2Seed, t22);
    SkDPoint p10 = c1.ptAtT(t10);
    SkDPoint p1Seed = c1.ptAtT(t1Seed);
    SkDPoint p12 = c1.ptAtT(t12);
    SkDebugf("%s p1=(%1.9g,%1.9g)<(%1.9g,%1.9g)<(%1.9g,%1.9g)\n", __FUNCTION__,
        p10.fX, p10.fY, p1Seed.fX, p1Seed.fY, p12.fX, p12.fY);
    SkDPoint p20 = c2.ptAtT(t20);
    SkDPoint p2Seed = c2.ptAtT(t2Seed);
    SkDPoint p22 = c2.ptAtT(t22);
    SkDebugf("%s p2=(%1.9g,%1.9g)<(%1.9g,%1.9g)<(%1.9g,%1.9g)\n", __FUNCTION__,
        p20.fX, p20.fY, p2Seed.fX, p2Seed.fY, p22.fX, p22.fY);
#endif
}

static void CubicIntersection_IntersectionFinder() {
//   double t1Seed = 0.87;
//   double t2Seed = 0.87;
    double t1Step = 0.000001;
    double t2Step = 0.000001;
    intersectionFinder(0, 1, 0.855895664, 0.864850875, t1Step, t2Step);
    intersectionFinder(0, 1, 0.865207906, 0.865207887, t1Step, t2Step);
    intersectionFinder(0, 1, 0.865213351, 0.865208087, t1Step, t2Step);
}

static const CubicPts selfSet[] = {
    {{{2, 3}, {0, 4}, {3, 2}, {5, 3}}},
    {{{3, 6}, {2, 3}, {4, 0}, {3, 2}}},
    {{{0, 2}, {2, 3}, {5, 1}, {3, 2}}},
    {{{0, 2}, {3, 5}, {5, 0}, {4, 2}}},
    {{{3.34, 8.98}, {1.95, 10.27}, {3.76, 7.65}, {4.96, 10.64}}},
    {{{3.13, 2.74}, {1.08, 4.62}, {3.71, 0.94}, {2.01, 3.81}}},
    {{{6.71, 3.14}, {7.99, 2.75}, {8.27, 1.96}, {6.35, 3.57}}},
    {{{12.81, 7.27}, {7.22, 6.98}, {12.49, 8.97}, {11.42, 6.18}}},
};

int selfSetCount = (int) SK_ARRAY_COUNT(selfSet);

static void selfOneOff(skiatest::Reporter* reporter, int setIdx) {
    const CubicPts& cubic = selfSet[setIdx];
    SkPoint c[4];
    for (int i = 0; i < 4; ++i) {
        c[i] = cubic.fPts[i].asSkPoint();
    }
    SkScalar loopT[3];
    SkCubicType cubicType = SkClassifyCubic(c);
    int breaks = SkDCubic::ComplexBreak(c, loopT);
    SkASSERT(breaks < 2);
    if (breaks && cubicType == SkCubicType::kLoop) {
        SkIntersections i;
        SkPoint twoCubics[7];
        SkChopCubicAt(c, twoCubics, loopT[0]);
        SkDCubic chopped[2];
        chopped[0].set(&twoCubics[0]);
        chopped[1].set(&twoCubics[3]);
        int result = i.intersect(chopped[0], chopped[1]);
        REPORTER_ASSERT(reporter, result == 2);
        REPORTER_ASSERT(reporter, i.used() == 2);
        for (int index = 0; index < result; ++index) {
            SkDPoint pt1 = chopped[0].ptAtT(i[0][index]);
            SkDPoint pt2 = chopped[1].ptAtT(i[1][index]);
            REPORTER_ASSERT(reporter, pt1.approximatelyEqual(pt2));
            reporter->bumpTestCount();
        }
    }
}

static void cubicIntersectionSelfTest(skiatest::Reporter* reporter) {
    int firstFail = 0;
    for (int index = firstFail; index < selfSetCount; ++index) {
        selfOneOff(reporter, index);
    }
}

static const CubicPts coinSet[] = {
    {{{72.350448608398438, 27.966041564941406}, {72.58441162109375, 27.861515045166016},
        {72.818222045898437, 27.756658554077148}, {73.394996643066406, 27.49799919128418}}},
    {{{73.394996643066406, 27.49799919128418}, {72.818222045898437, 27.756658554077148},
        {72.58441162109375, 27.861515045166016}, {72.350448608398438, 27.966041564941406}}},

    {{{297.04998779296875, 43.928997039794922}, {297.04998779296875, 43.928997039794922},
        {300.69699096679688, 45.391998291015625}, {306.92498779296875, 43.08599853515625}}},
    {{{297.04998779296875, 43.928997039794922}, {297.04998779296875, 43.928997039794922},
        {300.69699096679688, 45.391998291015625}, {306.92498779296875, 43.08599853515625}}},

    {{{2, 3}, {0, 4}, {3, 2}, {5, 3}}},
    {{{2, 3}, {0, 4}, {3, 2}, {5, 3}}},

    {{{317, 711}, {322.52285766601562, 711}, {327, 715.4771728515625}, {327, 721}}},
    {{{324.07107543945312, 713.928955078125}, {324.4051513671875, 714.26300048828125},
            {324.71566772460937, 714.62060546875}, {325, 714.9990234375}}},
};

static int coinSetCount = (int) SK_ARRAY_COUNT(coinSet);

static void coinOneOff(skiatest::Reporter* reporter, int index) {
    const CubicPts& cubic1 = coinSet[index];
    const CubicPts& cubic2 = coinSet[index + 1];
    oneOff(reporter, cubic1, cubic2, true);
}

static void cubicIntersectionCoinTest(skiatest::Reporter* reporter) {
    int firstFail = 0;
    for (int index = firstFail; index < coinSetCount; index += 2) {
        coinOneOff(reporter, index);
    }
}

DEF_TEST(PathOpsCubicCoinOneOff, reporter) {
    coinOneOff(reporter, 0);
}

DEF_TEST(PathOpsCubicIntersectionOneOff, reporter) {
    newOneOff(reporter, 0, 1);
}

DEF_TEST(PathOpsCubicIntersectionTestsOneOff, reporter) {
    testsOneOff(reporter, 10);
}

DEF_TEST(PathOpsCubicSelfOneOff, reporter) {
    selfOneOff(reporter, 0);
}

DEF_TEST(PathOpsCubicIntersection, reporter) {
    oneOffTests(reporter);
    cubicIntersectionSelfTest(reporter);
    cubicIntersectionCoinTest(reporter);
    standardTestCases(reporter);
    if (false) CubicIntersection_IntersectionFinder();
    if (false) CubicIntersection_RandTest(reporter);
}
